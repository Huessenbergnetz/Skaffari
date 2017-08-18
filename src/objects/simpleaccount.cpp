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
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonObject>
#include <QJsonValue>

SimpleAccount::SimpleAccount() : d(new SimpleAccountData)
{

}


SimpleAccount::SimpleAccount(dbid_t id, const QString &username) :
    d(new SimpleAccountData(id, username))
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


void SimpleAccount::setData(dbid_t id, const QString &username)
{
    d->id = id;
    d->username = username;
}


bool SimpleAccount::isValid() const
{
    return ((d->id > 0) && !d->username.isEmpty());
}
