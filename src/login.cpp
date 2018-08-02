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

#include "login.h"

#include "utils/skaffariconfig.h"
#include "utils/qtimezonevariant_p.h"

#include <Cutelyst/Plugins/Authentication/authentication.h>
#include <Cutelyst/Plugins/Authentication/authenticationuser.h>
#include <Cutelyst/Plugins/Session/Session>

#include <QHostAddress>

Q_LOGGING_CATEGORY(SK_LOGIN, "skaffari.login")

using namespace Cutelyst;

Login::Login(QObject *parent) : Controller(parent)
{
}

Login::~Login()
{
}

void Login::index(Context *c)
{
    auto req = c->req();
    const QString username = req->bodyParam(QStringLiteral("username"));

    if (req->isPost()) {
        const QString password = req->bodyParam(QStringLiteral("password"));

        if(!username.isEmpty() && !password.isEmpty()) {
            if (Authentication::authenticate(c, req->bodyParams())) {
                auto user = Authentication::user(c);

                Session::setValue(c, QStringLiteral("maxdisplay"), user.value(QStringLiteral("maxdisplay"), SkaffariConfig::defMaxdisplay()));
                Session::setValue(c, QStringLiteral("warnlevel"), user.value(QStringLiteral("warnlevel"), SkaffariConfig::defWarnlevel()));
                Session::setValue(c, QStringLiteral("lang"), QLocale(user.value(QStringLiteral("lang"), SkaffariConfig::defLanguage()).toString()));
                QTimeZone tz(user.value(QStringLiteral("tz"), SkaffariConfig::defTimezone()).toByteArray());
                Session::setValue(c, QStringLiteral("timeZone"), QVariant::fromValue<QTimeZone>(tz));
                Session::setValue(c, QStringLiteral("tz"), user.value(QStringLiteral("tz"), SkaffariConfig::defTimezone()));

                const QVariantList domList = user.value(QStringLiteral("domains")).toList();
                if (domList.size() == 1) {
                    c->res()->redirect(c->uriForAction(QStringLiteral("/domain/accounts"), QStringList(domList.first().toString())));
                } else {
                    c->res()->redirect(c->uriFor(QStringLiteral("/")));
                }
                qCInfo(SK_LOGIN, "User %s successfully logged in from IP %s", qUtf8Printable(username), qUtf8Printable(req->addressString()));

                return;
            } else {
                qCWarning(SK_LOGIN, "Bad username or password for user %s from IP %s", qUtf8Printable(username), qUtf8Printable(req->addressString()));
                c->setStash(QStringLiteral("error_msg"), c->translate("Login", "Arrrgh, bad username or password!"));
                c->res()->setStatus(Response::Forbidden);
            }
        }
    }

    c->setStash(QStringLiteral("username"), username);
    c->setStash(QStringLiteral("no_wrapper"), QStringLiteral("1"));
    c->setStash(QStringLiteral("template"), QStringLiteral("login.html"));
}

#include "moc_login.cpp"
