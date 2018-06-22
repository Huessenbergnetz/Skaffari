/*
 * Skaffari - a mail account administration web interface based on Cutelyst
 * Copyright (C) 2018 Matthias Fehring <mf@huessenbergnetz.de>
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

#include "emailaddress.h"
#include "skaffarierror.h"
#include <Cutelyst/Context>
#include <Cutelyst/Plugins/Utils/Sql>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QSharedData>
#include <QCollator>
#include <algorithm>

class EmailAddressNameCollator : public QCollator
{
public:
    explicit EmailAddressNameCollator(const QLocale &locale) :
        QCollator(locale)
    {}

    bool operator() (const EmailAddress &left, const EmailAddress &right) { return (compare(left.name(), right.name())); }
};

class EmailAddress::Data : public QSharedData
{
public:
    Data() : QSharedData() {}

    Data(dbid_t _id, dbid_t _aceId, const QString &_name) :
        QSharedData(),
        id(_id),
        aceId(_aceId)
    {
        const int atIdx = _name.lastIndexOf(QLatin1Char('@'));
        if ((atIdx > 0) && (atIdx < (_name.size() - 1))) {
            local = _name.left(atIdx);
            domain = _name.mid(atIdx + 1);
        }
    }

    ~Data() {}

    QString local;
    QString domain;
    dbid_t id = 0;
    dbid_t aceId = 0;
};

EmailAddress::EmailAddress() : d(new Data)
{

}

EmailAddress::EmailAddress(dbid_t id, dbid_t aceId, const QString &name) :
    d(new Data(id, aceId, name))
{

}

EmailAddress::EmailAddress(const EmailAddress &other) :
    d(other.d)
{

}

EmailAddress::EmailAddress(EmailAddress &&other) noexcept :
    d(std::move(other.d))
{
    other.d = nullptr;
}

EmailAddress& EmailAddress::operator=(const EmailAddress &other)
{
    d = other.d;
    return *this;
}

EmailAddress& EmailAddress::operator=(EmailAddress &&other) noexcept
{
    swap(other);
    return *this;
}

EmailAddress::~EmailAddress()
{

}

void EmailAddress::swap(EmailAddress &other) noexcept
{
    std::swap(d, other.d);
}

dbid_t EmailAddress::id() const
{
    return d->id;
}

dbid_t EmailAddress::aceId() const
{
    return d->aceId;
}

bool EmailAddress::isIdn() const
{
    return (d->aceId > 0);
}

QString EmailAddress::name() const
{
    return d->local + QLatin1Char('@') + d->domain;
}

QString EmailAddress::localPart() const
{
    return d->local;
}

QString EmailAddress::domainPart() const
{
    return d->domain;
}

bool EmailAddress::isValid() const
{
    return ((d->id > 0) && !d->local.isEmpty() && !d->domain.isEmpty());
}

std::vector<EmailAddress> EmailAddress::list(Cutelyst::Context *c, SkaffariError &e, const QString &username)
{
    std::vector<EmailAddress> lst;

    Q_ASSERT_X(c, "list email addresses", "invalid context object pointer");

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT id, ace_id, alias FROM virtual WHERE dest = :username AND username = :username AND idn_id = 0 ORDER BY alias ASC"));
    q.bindValue(QStringLiteral(":username"), username);

    if (Q_LIKELY(q.exec())) {
        if (q.size() > -1) {
            lst.reserve(static_cast<std::vector<EmailAddress>::size_type>(q.size()));
        }

        while (q.next()) {
            const QString address = q.value(2).toString();
            if (Q_LIKELY(!address.startsWith(QLatin1Char('@')))) {
                lst.emplace_back(q.value(0).value<dbid_t>(), q.value(1).value<dbid_t>(), address);
            }
        }

        if (lst.size() > 1) {
            EmailAddressNameCollator eanc(c->locale());
            std::sort(lst.begin(), lst.end(), eanc);
        }
    } else {
        e.setSqlError(q.lastError(), c->translate("EmailAddress", "Failed to query the list of email addresses for account %1.").arg(username));
    }

    return lst;
}

EmailAddress EmailAddress::get(Cutelyst::Context *c, SkaffariError &e, dbid_t id)
{
    EmailAddress address;

    Q_ASSERT_X(c, "get email address by id", "invalid context object pointer");

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT ace_id, alias FROM virtual WHERE id = :id"));
    q.bindValue(QStringLiteral(":id"), id);

    if (Q_LIKELY(q.exec())) {
        if (q.next()) {
            address = EmailAddress(id, q.value(0).value<dbid_t>(), q.value(1).toString());
        } else {
            e.setErrorType(SkaffariError::NotFound);
            e.setErrorText(c->translate("EmailAddress", "Can not find email address with database ID %1.").arg(id));
        }
    } else {
        e.setSqlError(q.lastError(), c->translate("EmailAddress", "Failed to query email address with ID %1 from the database.").arg(id));
    }

    return address;
}

EmailAddress EmailAddress::get(Cutelyst::Context *c, SkaffariError &e, const QString &alias)
{
    EmailAddress address;

    Q_ASSERT_X(c, "get email address by alias", "invalid context object pointer");

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT id, ace_id FROM virtual WHERE alias = :alias"));
    q.bindValue(QStringLiteral(":alias"), alias);

    if (Q_LIKELY(q.exec())) {
        if (q.next()) {
            address = EmailAddress(q.value(0).value<dbid_t>(), q.value(1).value<dbid_t>(), alias);
        } else {
            e.setErrorType(SkaffariError::NotFound);
            e.setErrorText(c->translate("EmailAddress", "Can not find email address %1.").arg(alias));
        }
    } else {
        e.setSqlError(q.lastError(), c->translate("EmailAddress", "Failed to query email address %1 from the database.").arg(alias));
    }

    return address;
}

QDebug operator<<(QDebug dbg, const EmailAddress &address)
{
    QDebugStateSaver saver(dbg);
    Q_UNUSED(saver);
    dbg.nospace() << "EmailAddress(";
    dbg << "ID: " << address.id();
    dbg << ", ACE ID: " << address.aceId();
    dbg << ", Name: " << address.name();
    dbg << ')';
    return dbg.maybeSpace();
}
