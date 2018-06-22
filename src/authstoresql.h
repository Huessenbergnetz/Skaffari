/*
 * Skaffari - a mail account administration web interface based on Cutelyst
 * Copyright (C) 2017-2018 Matthias Fehring <mf@huessenbergnetz.de>
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

#ifndef AUTHSTORESQL_H
#define AUTHSTORESQL_H

#include <Cutelyst/Plugins/Authentication/authenticationstore.h>

using namespace Cutelyst;

/*!
 * \ingroup skaffaricore
 * \brief SQL based Cutelyst authentication store.
 */
class AuthStoreSql : public AuthenticationStore
{
    Q_OBJECT
public:
    explicit AuthStoreSql(QObject *parent = nullptr);
    
    AuthenticationUser findUser(Context *c, const ParamsMultiMap &userinfo) override;
};

#endif // AUTHSTORESQL_H
