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

#ifndef SIMPLEACCOUNT_P_H
#define SIMPLEACCOUNT_P_H

#include "simpleaccount.h"
#include <QSharedData>
#include <QCollator>

class SimpleAccountCollator : public QCollator
{
public:
    explicit SimpleAccountCollator(const QLocale &locale) :
        QCollator(locale)
    {}

    bool operator() (const SimpleAccount &left, const SimpleAccount &right) { return (compare(left.username(), right.username()) > 0); }
};


class SimpleAccountData : public  QSharedData
{
public:
    SimpleAccountData() {}

    SimpleAccountData(dbid_t _id, const QString &_username, const QString &_domainname) :
        username(_username),
        domainname(_domainname),
        id(_id)
    {}

    SimpleAccountData(const SimpleAccountData &other) :
        QSharedData(other),
        username(other.username),
        domainname(other.domainname),
        id(other.id)
    {}

    ~SimpleAccountData() {}

    QString username;
    QString domainname;
    dbid_t id = 0;
};

#endif // SIMPLEACCOUNT_P_H
