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

#include "logout.h"

#include <Cutelyst/Plugins/Authentication/authentication.h>
#include <Cutelyst/Plugins/Authentication/authenticationuser.h>
#include <Cutelyst/Plugins/Session/Session>

using namespace Cutelyst;

Logout::Logout(QObject *parent) : Controller(parent)
{
}

Logout::~Logout()
{
}

void Logout::index(Context *c)
{
    Authentication *auth = c->plugin<Authentication*>();

    const QString userName = auth->user(c).value(QStringLiteral("username")).toString();
    auth->logout(c);

    qCInfo(SK_LOGIN, "User %s logged out.", qUtf8Printable(userName));

    Session::deleteSession(c, QStringLiteral("User logged out."));

    c->response()->redirect(c->uriFor(QLatin1String("/login")));
}

#include "moc_logout.cpp"
