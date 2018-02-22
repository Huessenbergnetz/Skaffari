/*
 * Skaffari - a mail account administration web interface based on Cutelyst
 * Copyright (C) 2017 Matthias Fehring <kontakt@buschmann23.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "simpleaccount_p.h"
#include "skaffarierror.h"
#include <Cutelyst/Context>
#include <Cutelyst/Plugins/Utils/Sql>
#include <Cutelyst/Plugins/Authentication/authentication.h>
#include <Cutelyst/Plugins/Authentication/authenticationuser.h>
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonValue>

Q_LOGGING_CATEGORY(SK_SIMPLEACCOUNT, "skaffari.simpleaccount")

SimpleAccount::SimpleAccount() : d(new SimpleAccountData)
{

}

SimpleAccount::SimpleAccount(dbid_t id, const QString &username, const QString &domainname) :
    d(new SimpleAccountData(id, username, domainname))
{

}

SimpleAccount::SimpleAccount(const SimpleAccount &other) :
    d(other.d)
{

}

SimpleAccount& SimpleAccount::operator=(const SimpleAccount &other)
{
    d = other.d;
    return *this;
}

SimpleAccount::~SimpleAccount()
{

}

dbid_t SimpleAccount::id() const
{
    return d->id;
}

QString SimpleAccount::username() const
{
    return d->username;
}

QString SimpleAccount::domainname() const
{
    return d->domainname;
}

void SimpleAccount::setData(dbid_t id, const QString &username, const QString &domainname)
{
    d->id = id;
    d->username = username;
    d->domainname = domainname;
}

bool SimpleAccount::isValid() const
{
    return ((d->id > 0) && !d->username.isEmpty() && !d->domainname.isEmpty());
}

QJsonObject SimpleAccount::toJson() const
{
    QJsonObject o;

    o.insert(QStringLiteral("id"), static_cast<qint64>(d->id));
    o.insert(QStringLiteral("username"), d->username);
    o.insert(QStringLiteral("domainname"), d->domainname);

    return o;
}

std::vector<SimpleAccount> SimpleAccount::list(Cutelyst::Context *c, SkaffariError *e, qint16 userType, dbid_t adminId, dbid_t domainId, const QString searchString)
{
    std::vector<SimpleAccount> lst;

    Q_ASSERT_X(c, "list simple accounts", "invalid context object");
    Q_ASSERT_X(e, "list simple accounts", "invalid error object");

    QSqlQuery q;

    QString _search;
    if (!searchString.isEmpty()) {
        _search = QLatin1Char('%') + searchString + QLatin1Char('%');
    }

    if (userType == 0) {
        if (domainId == 0) {
            if (searchString.isEmpty()) {
                q = CPreparedSqlQueryThread(QStringLiteral("SELECT a.id, a.username, d.domain_name FROM accountuser a LEFT JOIN domain d ON a.domain_id = d.id ORDER BY username ASC"));
            } else {
                q = CPreparedSqlQueryThread(QStringLiteral("SELECT a.id, a.username, d.domain_name FROM accountuser a LEFT JOIN domain d ON a.domain_id = d.id WHERE a.username LIKE :search ORDER BY a.username ASC"));
                q.bindValue(QStringLiteral(":search"), _search);
            }
        } else {
            if (searchString.isEmpty()) {
                q = CPreparedSqlQueryThread(QStringLiteral("SELECT a.id, a.username, d.domain_name FROM accountuser a LEFT JOIN domain d ON a.domain_id = d.id WHERE a.domain_id = :domain_id ORDER BY a.username ASC"));
            } else {
                q = CPreparedSqlQueryThread(QStringLiteral("SELECT a.id, a.username, d.domain_name FROM accountuser a LEFT JOIN domain d ON a.domain_id = d.id WHERE a.username LIKE :search AND a.domain_id = :domain_id ORDER BY a.username ASC"));
                q.bindValue(QStringLiteral(":search"), _search);
            }
            q.bindValue(QStringLiteral(":domain_id"), domainId);
        }
    } else {
        if (domainId == 0) {
            if (searchString.isEmpty()) {
                q = CPreparedSqlQueryThread(QStringLiteral("SELECT ac.id, ac.username, do.domain_name FROM accountuser ac LEFT JOIN domainadmin da ON ac.domain_id = da.domain_id LEFT JOIN domain do ON ac.domain_id = do.id WHERE da.admin_id = :admin_id ORDER BY ac.username ASC"));
            } else {
                q = CPreparedSqlQueryThread(QStringLiteral("SELECT ac.id, ac.username, do.domain_name FROM accountuser ac LEFT JOIN domainadmin da ON ac.domain_id = da.domain_id LEFT JOIN domain do ON ac.domain_id = do.id WHERE da.admin_id = :admin_id AND ac.username LIKE :search ORDER BY ac.username ASC"));
                q.bindValue(QStringLiteral(":search"), _search);
            }
            q.bindValue(QStringLiteral(":admin_id"), adminId);
        } else {
            if (searchString.isEmpty()) {
                q = CPreparedSqlQueryThread(QStringLiteral("SELECT ac.id, ac.username, do.domain_name FROM accountuser ac LEFT JOIN domainadmin da on ac.domain_id = da.domain_id LEFT JOIN domain do ON ac.domain_id = do.id WHERE da.admin_id = :admin_id AND da.domain_id = :domain_id ORDER BY ac.username ASC"));
            } else {
                q = CPreparedSqlQueryThread(QStringLiteral("SELECT ac.id, ac.username, do.domain_name FROM accountuser ac LEFT JOIN domainadmin da on ac.domain_id = da.domain_id LEFT JOIN domain do ON ac.domain_id = do.id WHERE da.admin_id = :admin_id AND da.domain_id = :domain_id AND ac.username LIKE :search ORDER BY ac.username ASC"));
                q.bindValue(QStringLiteral(":search"), _search);
            }
            q.bindValue(QStringLiteral(":admin_id"), adminId);
            q.bindValue(QStringLiteral(":domain_id"), domainId);
        }
    }

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("SimpleAccount", "Failed to query list of accounts from database."));
        return lst;
    }

    while (q.next()) {
        lst.push_back(SimpleAccount(q.value(0).value<dbid_t>(),
                                    q.value(1).toString(),
                                    QUrl::fromAce(q.value(2).toByteArray())));
    }

    if (lst.size() > 1) {
        SimpleAccountCollator sac(c->locale());
        std::sort(lst.begin(), lst.end(), sac);
    }

    return lst;
}

QJsonArray SimpleAccount::listJson(Cutelyst::Context *c, SkaffariError *e, qint16 userType, dbid_t adminId, dbid_t domainId, const QString searchString)
{
    QJsonArray lst;

    Q_ASSERT_X(c, "list simple accounts json", "invalid context object");
    Q_ASSERT_X(e, "list simple accounts json", "invalid error object");

    const std::vector<SimpleAccount> _lst = SimpleAccount::list(c, e, userType, adminId, domainId, searchString);

    if (!_lst.empty()) {
        for (const SimpleAccount &ac : _lst) {
            lst.push_back(QJsonValue(ac.toJson()));
        }
    }

    return lst;
}

SimpleAccount SimpleAccount::get(Cutelyst::Context *c, SkaffariError *e, dbid_t id)
{
    SimpleAccount a;

    Q_ASSERT_X(c, "get simple account", "invalid context object");
    Q_ASSERT_X(e, "get simple account", "invalid error object");

    if (id > 0) {

        QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT a.id, a.username, d.domain_name FROM accountuser a LEFT JOIN domain d ON a.domain_id = d.id WHERE a.id = :id"));
        q.bindValue(QStringLiteral(":id"), id);

        if (Q_LIKELY(q.exec())) {
            if (q.next()) {
                a.setData(q.value(0).value<dbid_t>(), q.value(1).toString(), q.value(2).toString());
            } else {
                qCWarning(SK_SIMPLEACCOUNT, "Can not find account with ID %u in database.", id);
                e->setErrorType(SkaffariError::NotFound);
                e->setErrorText(c->translate("SimpleAccount", "Can not find account with database ID %1.").arg(id));
            }
        } else {
            e->setSqlError(q.lastError(), c->translate("SimpleAccount", "Failed to query account from database."));
            qCWarning(SK_SIMPLEACCOUNT, "Failed to query account with ID %u from database: %s", id, qUtf8Printable(q.lastError().text()));
        }
    }

    return a;
}
