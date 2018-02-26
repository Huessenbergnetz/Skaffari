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

#ifndef SKAFFARI_ADMINACCOUNT_P_H
#define SKAFFARI_ADMINACCOUNT_P_H

#include "adminaccount.h"
#include <Cutelyst/Plugins/Authentication/authenticationuser.h>
#include <QSharedData>

class AdminAccountData : public QSharedData
{
public:
    AdminAccountData() {}

    AdminAccountData(dbid_t _id, const QString &_username, quint8 _type, const QList<dbid_t> &_domains) :
        domains(_domains),
        username(_username),
        id(_id)
    {
        type = AdminAccountData::getUserType(_type);
    }

    AdminAccountData(dbid_t _id, const QString &_username, AdminAccount::AdminAccountType _type, const QList<dbid_t> &_domains) :
        domains(_domains),
        username(_username),
        id(_id),
        type(_type)
    {}

    AdminAccountData(const Cutelyst::AuthenticationUser &user) :
        username(user.value(QStringLiteral("username")).toString()),
        lang(user.value(QStringLiteral("lang")).toString()),
        tmpl(user.value(QStringLiteral("style")).toString()),
        tz(user.value(QStringLiteral("tz")).toString()),
        id(user.id().value<dbid_t>()),
        maxDisplay(user.value(QStringLiteral("maxdisplay")).value<quint8>()),
        warnLevel(user.value(QStringLiteral("warnLevel")).value<quint8>())
    {
        created = user.value(QStringLiteral("created_at")).toDateTime();
        created.setTimeSpec(Qt::UTC);
        updated = user.value(QStringLiteral("updated_at")).toDateTime();
        updated.setTimeSpec(Qt::UTC);
        type = AdminAccountData::getUserType(user.value(QStringLiteral("type")).value<quint8>());
    }

    AdminAccountData(const AdminAccountData &other) :
        QSharedData(other),
        domains(other.domains),
        username(other.username),
        lang(other.lang),
        tmpl(other.tmpl),
        tz(other.tz),
        created(other.created),
        updated(other.updated),
        id(other.id),
        type(other.type),
        maxDisplay(other.maxDisplay),
        warnLevel(other.warnLevel)
    {}

    ~AdminAccountData() {}

    static AdminAccount::AdminAccountType getUserType(quint8 _type)
    {
        switch (_type) {
        case 255:
            return AdminAccount::SuperUser;
        case 254:
            return AdminAccount::Administrator;
        case 127:
            return AdminAccount::DomainMaster;
        default:
            return AdminAccount::Disabled;
        }
    }

    QList<dbid_t> domains;
    QString username;
    QString lang;
    QString tmpl;
    QString tz;
    QDateTime created;
    QDateTime updated;
    dbid_t id;
    AdminAccount::AdminAccountType type;
    quint8 maxDisplay;
    quint8 warnLevel;
};

#endif // SKAFFAIR_ADMINACCOUNT_P_H
