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

#include "myaccount.h"
#include "objects/adminaccount.h"
#include "objects/skaffarierror.h"
#include "utils/language.h"

#include <Cutelyst/Plugins/Utils/Validator> // includes the main validator
#include <Cutelyst/Plugins/Utils/Validators> // includes all validator rules
#include <Cutelyst/Plugins/Utils/ValidatorResult> // includes the validator result
#include <Cutelyst/Plugins/StatusMessage>
#include <Cutelyst/Plugins/Authentication/authentication.h>
#include <Cutelyst/Engine>
#include <QTimeZone>


MyAccount::MyAccount(QObject *parent) : Controller(parent)
{

}


MyAccount::~MyAccount()
{

}



void MyAccount::index(Context *c)
{
    AuthenticationUser user = Authentication::user(c);
    SkaffariError e(c);
    AdminAccount aac = AdminAccount::get(c, &e, user.id().toULong());
    if (aac.isValid()) {

        const QVariantMap adminsConf = c->engine()->config(QStringLiteral("Admins"));
        const quint8 pwminlength = adminsConf.value(QStringLiteral("pwminlength"), 8).value<quint8>();

        auto req = c->req();
        if (req->isPost()) {
            static Validator v({
                            new ValidatorConfirmed(QStringLiteral("password")),
                            new ValidatorMin(QStringLiteral("password"), QMetaType::QString, pwminlength),
                            new ValidatorBetween(QStringLiteral("maxdisplay"), QMetaType::UInt, 0, 255),
                            new ValidatorBetween(QStringLiteral("warnlevel"), QMetaType::UInt, 0, 100),
                            new ValidatorIn(QStringLiteral("lang"), Language::supportedLangsList())
                        });

            ValidatorResult vr = v.validate(c, Validator::FillStashOnError);
            if (vr) {
                SkaffariError e(c);
                if (AdminAccount::update(c,
                                         &e,
                                         &aac,
                                         &user,
                                         req->parameters(),
                                         static_cast<QCryptographicHash::Algorithm>(adminsConf.value(QStringLiteral("pwmethod")).toInt()),
                                         adminsConf.value(QStringLiteral("pwrounds")).value<quint32>()
                                         )) {
                    c->setStash(QStringLiteral("status_msg"), c->translate("MyAccount", "Your account has been updated."));
                } else {
                    c->setStash(QStringLiteral("error_msg"), e.errorText());
                }
            }
        }

        c->stash({
                     {QStringLiteral("template"), QStringLiteral("myaccount/index.html")},
                     {QStringLiteral("site_title"), c->translate("MyAccount", "My account")},
                     {QStringLiteral("adminaccount"), QVariant::fromValue<AdminAccount>(aac)},
                     {QStringLiteral("langs"), QVariant::fromValue<QVector<Language>>(Language::supportedLangs())},
                     {QStringLiteral("timezones"), QVariant::fromValue<QList<QByteArray>>(QTimeZone::availableTimeZoneIds())}
                 });

    } else {
        c->setStash(QStringLiteral("not_found_text"), c->translate("MyAccount", "There is no admin account with database ID %1.").arg(user.id()));
        c->setStash(QStringLiteral("template"), QStringLiteral("404.html"));
        c->res()->setStatus(404);
    }
}
