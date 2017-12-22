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

#include "root.h"
#include "objects/domain.h"
#include "utils/utils.h"
#include "utils/language.h"
#include "utils/skaffariconfig.h"

#include <Cutelyst/Plugins/Authentication/authentication.h>
#include <Cutelyst/Plugins/StatusMessage>
#include <Cutelyst/Plugins/Session/Session>

#include <QLocale>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "../common/global.h"

using namespace Cutelyst;

Root::Root(QObject *parent) : Controller(parent)
{
}

Root::~Root()
{
}

void Root::index(Context *c)
{
	c->stash({
                 {QStringLiteral("template"), QStringLiteral("dashboard.html")},
                 {QStringLiteral("site_title"), c->translate("Root", "Dashboard")}
             });
}

void Root::defaultPage(Context *c)
{
    c->stash({
                 {QStringLiteral("template"), QStringLiteral("404.html")},
                 {QStringLiteral("site_title"), c->translate("Root", "Not found")}
             });
    c->res()->setStatus(404);
}

void Root::csrfdenied(Context *c)
{
    c->res()->setStatus(403);
    if (Utils::isAjax(c)) {
        c->res()->setJsonBody(QJsonObject({
                                              {QStringLiteral("error_msg"), QJsonValue(c->stash(QStringLiteral("error_msg")).toString())}
                                          }));
    } else {
        Language::setLang(c);
        c->stash({
                     {QStringLiteral("template"), QStringLiteral("csrfdenied.html")},
                     {QStringLiteral("no_wrapper"), QStringLiteral("1")}
                 });
    }
}

bool Root::Auto(Context* c)
{
    AuthenticationUser user = Authentication::user(c);

    Language::setLang(c);

    if (c->controller() == c->controller(QStringLiteral("Login"))) {
        return true;
    }

    if (Q_UNLIKELY(user.isNull())) {
        if (Utils::isAjax(c)) {
            c->res()->setStatus(Response::Unauthorized);
            c->res()->setJsonBody(QJsonDocument(QJsonObject({
                                                                {QStringLiteral("error_msg"), QJsonValue(c->translate("Root", "You have to login at first."))}
                                                            })));
        } else {
            c->res()->redirect(c->uriFor(QStringLiteral("/login")));
        }
        return false;
    }

    StatusMessage::load(c);

    c->stash({
                 {QStringLiteral("userId"), QVariant::fromValue<dbid_t>(user.id().toULong())},
                 {QStringLiteral("userType"), user.value(QStringLiteral("type"))},
                 {QStringLiteral("userName"), user.value(QStringLiteral("username"))},
                 {QStringLiteral("userMaxDisplay"), Session::value(c, QStringLiteral("maxdisplay"), SkaffariConfig::defMaxdisplay()).value<quint8>()},
                 {QStringLiteral("userWarnLevel"), Session::value(c, QStringLiteral("warnlevel"), SkaffariConfig::defWarnlevel()).value<quint8>()}
             });

    return true;
}

#include "moc_root.cpp"
