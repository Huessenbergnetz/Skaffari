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
        id(_id),
        username(_username),
        type(_type),
        domains(_domains)
    {}

    AdminAccountData(const AdminAccountData &other) :
        QSharedData(other),
        id(other.id),
        username(other.username),
        type(other.type),
        domains(other.domains),
        lang(other.lang),
        tz(other.tz),
        maxDisplay(other.maxDisplay),
        warnLevel(other.warnLevel),
        tmpl(other.tmpl),
        created(other.created),
        updated(other.updated)
    {}

    ~AdminAccountData() {}

    dbid_t id;
	QString username;
    qint16 type;
    QList<dbid_t> domains;
    QString lang;
    QByteArray tz;
    quint8 maxDisplay;
    quint8 warnLevel;
    QString tmpl;
    QDateTime created;
    QDateTime updated;
};

#endif // SKAFFAIR_ADMINACCOUNT_P_H
