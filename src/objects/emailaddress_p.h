/*
 * Skaffari - a mail account administration web interface based on Cutelyst
 * Copyright (C) 2018 Matthias Fehring <kontakt@buschmann23.de>
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

#ifndef SKAFFARIEMAILADDRESS_P_H
#define SKAFFARIEMAILADDRESS_P_H

#include "emailaddress.h"
#include <QSharedData>
#include <QCollator>

class EmailAddressNameCollator : public QCollator
{
public:
    explicit EmailAddressNameCollator(const QLocale &locale) :
        QCollator(locale)
    {}

    bool operator() (const EmailAddress &left, const EmailAddress &right) { return (compare(left.name(), right.name())); }
};

class EmailAddressData : public QSharedData
{
public:
    EmailAddressData() {}

    EmailAddressData(dbid_t _id, dbid_t _aceId, const QString &_name) :
        name(_name),
        id(_id),
        aceId(_aceId)
    {}

    EmailAddressData(const EmailAddressData &other) :
        QSharedData(other),
        name(other.name),
        id(other.id),
        aceId(other.aceId)
    {}

    ~EmailAddressData() {}

    QString name;
    dbid_t id = 0;
    dbid_t aceId = 0;
};

#endif // SKAFFARIEMAILADDRESS_P_H
