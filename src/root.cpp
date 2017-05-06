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
#include "utils/language.h"
#include "../common/config.h"

#include <Cutelyst/Plugins/Authentication/authentication.h>
#include <Cutelyst/Plugins/StatusMessage>
#include <Cutelyst/Plugins/Session/Session>
#include <Cutelyst/Engine>
#include <QLocale>

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


bool Root::Auto(Context* c)
{
    AuthenticationUser user = Authentication::user(c);

    QString lang = Session::value(c, QStringLiteral("lang")).toString();
    if (Q_UNLIKELY(lang.isEmpty())) {
        const QStringList acceptedLangs = c->req()->header(QStringLiteral("Accept-Language")).split(QLatin1Char(','), QString::SkipEmptyParts);
        if (Q_LIKELY(!acceptedLangs.empty())) {
            for (const QString &al : acceptedLangs) {
                const QString langPart = al.section(QLatin1Char(';'), 0, 0);
                if (Language::supportedLangsList().contains(langPart)) {
                    lang = langPart;
                    break;
                }
            }
        }
        if (lang.isEmpty()) {
            lang = c->engine()->config(QStringLiteral("Defaults")).value(QStringLiteral("language"), QStringLiteral(SK_DEF_DEF_LANGUAGE)).toString();
        }
    }

    c->setLocale(QLocale(lang));
    c->setStash(QStringLiteral("lang"), lang);

    if (c->controller() == c->controller(QStringLiteral("Login"))) {
        return true;
    }

    if (Q_UNLIKELY(user.isNull())) {
        c->res()->redirect(c->uriFor(QStringLiteral("/login")));
        return false;
    }

    StatusMessage::load(c);

    c->stash({
                 {QStringLiteral("userId"), QVariant::fromValue<quint32>(user.id().toULong())},
                 {QStringLiteral("userType"), user.value(QStringLiteral("type"))},
                 {QStringLiteral("userName"), user.value(QStringLiteral("username"))},
                 {QStringLiteral("userMaxDisplay"), Session::value(c, QStringLiteral("maxdisplay"), SK_DEF_DEF_MAXDISPLAY).value<quint8>()},
                 {QStringLiteral("userWarnLevel"), Session::value(c, QStringLiteral("warnlevel"), SK_DEF_DEF_WARNLEVEL).value<quint8>()}
             });

    return true;
}


