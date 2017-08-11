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

#include "simpledomain_p.h"
#include "skaffarierror.h"
#include <Cutelyst/Context>
#include <Cutelyst/Plugins/Utils/Sql>
#include <QSqlQuery>
#include <QSqlError>
#include <QUrl>
#include <QJsonObject>
#include <QJsonValue>

SimpleDomain::SimpleDomain() : d(new SimpleDomainData)
{

}


SimpleDomain::SimpleDomain(dbid_t id, const QString &name) :
    d(new SimpleDomainData(id, name))
{

}


SimpleDomain::SimpleDomain(const SimpleDomain &other) :
    d(other.d)
{

}


SimpleDomain& SimpleDomain::operator=(const SimpleDomain &other)
{
    d = other.d;
    return *this;
}

SimpleDomain::~SimpleDomain()
{

}


dbid_t SimpleDomain::id() const
{
    return d->id;
}


QString SimpleDomain::name() const
{
    return d->name;
}


void SimpleDomain::setData(dbid_t id, const QString &name)
{
    d->id = id;
    d->name = name;
}


bool SimpleDomain::isValid() const
{
    return ((d->id > 0) && !d->name.isEmpty());
}


std::vector<SimpleDomain> SimpleDomain::list(Cutelyst::Context *c, SkaffariError *e, quint16 userType, dbid_t adminId)
{
    std::vector<SimpleDomain> lst;

    Q_ASSERT_X(c, "list simple domains", "invalid context object");
    Q_ASSERT_X(e, "list simple domains", "invalid error object");

    QSqlQuery q;

    if (userType == 0) {
        q = CPreparedSqlQueryThread(QStringLiteral("SELECT id, domain_name FROM domain ORDER BY domain_name ASC"));
    } else {
        q = CPreparedSqlQueryThread(QStringLiteral("SELECT dom.id, dom.domain_name FROM domain dom LEFT JOIN domainadmin da ON dom.id = da.domain_id WHERE da.admin_id = :admin_id ORDER BY dom.domain_name ASC"));
        q.bindValue(QStringLiteral(":admin_id"), adminId);
    }

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("SimpleDomain", "Failed to query the list of domains from the database."));
        return lst;
    }

    while (q.next()) {
        lst.push_back(SimpleDomain(q.value(0).value<dbid_t>(), QUrl::fromAce(q.value(1).toByteArray())));
    }

    if (lst.size() > 1) {
        SimpleDomainNameCollator sdnc(c->locale());
        std::sort(lst.begin(), lst.end(), sdnc);
    }

    return lst;
}


QJsonArray SimpleDomain::listJson(Cutelyst::Context *c, SkaffariError *e, quint16 userType, dbid_t adminId)
{
    QJsonArray lst;

    Q_ASSERT_X(c, "list simple domains as JSON", "invalid context object");
    Q_ASSERT_X(e, "list simple domains as JSON", "invalid error object");

    const std::vector<SimpleDomain> _lst = SimpleDomain::list(c, e, userType, adminId);

    if (!_lst.empty()) {
        for (const SimpleDomain &sd : _lst) {
            lst.append(QJsonValue(QJsonObject({
                                                  {QStringLiteral("id"), static_cast<qint64>(sd.id())},
                                                  {QStringLiteral("name"), sd.name()}
                                              })));
        }
    }

    return lst;
}


SimpleDomain SimpleDomain::get(Cutelyst::Context *c, SkaffariError *e, dbid_t id)
{
    SimpleDomain dom;

    Q_ASSERT_X(c, "get simple domain data", "invalid context object");
    Q_ASSERT_X(e, "get simple domain data", "invalid error object");

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT domain_name FROM domain WHERE id = :id"));
    q.bindValue(QStringLiteral(":id"), id);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("SimpleDomain", "Failed to query simple domain data for domain ID %1.").arg(id));
        return dom;
    }

    if (Q_UNLIKELY(!q.next())) {
        e->setErrorType(SkaffariError::InputError);
        e->setErrorText(c->translate("SimpleDomain", "Can not find domain with database ID %1.").arg(id));
        return dom;
    }

    dom.setData(id, q.value(0).toString());

    return dom;
}
