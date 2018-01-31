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
#include <QSharedData>

class AdminAccountData : public QSharedData
{
public:
    AdminAccountData() {}

    AdminAccountData(dbid_t _id, const QString &_username, qint16 _type, const QList<dbid_t> &_domains) :
        domains(_domains),
        username(_username),
        id(_id),
        type(_type)
    {}

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

    QList<dbid_t> domains;
    QString username;
    QString lang;
    QString tmpl;
    QByteArray tz;
    QDateTime created;
    QDateTime updated;
    dbid_t id;
    qint16 type;
    quint8 maxDisplay;
    quint8 warnLevel;
};

#endif // SKAFFAIR_ADMINACCOUNT_P_H
