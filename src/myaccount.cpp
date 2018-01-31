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
#include "utils/skaffariconfig.h"
#include "objects/helpentry.h"

#include <Cutelyst/Plugins/Utils/Validator> // includes the main validator
#include <Cutelyst/Plugins/Utils/Validators> // includes all validator rules
#include <Cutelyst/Plugins/Utils/ValidatorResult> // includes the validator result
#include <Cutelyst/Plugins/StatusMessage>
#include <Cutelyst/Plugins/Authentication/authentication.h>
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

        auto req = c->req();
        if (req->isPost()) {
            static Validator v({
                                   new ValidatorConfirmed(QStringLiteral("password")),
                                   new ValidatorMin(QStringLiteral("password"), QMetaType::QString, SkaffariConfig::admPwMinlength()),
                                   new ValidatorBetween(QStringLiteral("maxdisplay"), QMetaType::UShort, 0, 255),
                                   new ValidatorBetween(QStringLiteral("warnlevel"), QMetaType::UShort, 0, 100),
                                   new ValidatorIn(QStringLiteral("lang"), Language::supportedLangsList())
                               });

            ValidatorResult vr = v.validate(c, Validator::FillStashOnError);
            if (vr) {
                SkaffariError e(c);
                if (AdminAccount::update(c, &e, &aac, &user, req->bodyParameters())) {
                    c->setStash(QStringLiteral("status_msg"), c->translate("MyAccount", "Your account has been updated."));
                } else {
                    c->setStash(QStringLiteral("error_msg"), e.errorText());
                }
            }
        }

        QHash<QString,HelpEntry> help;
        help.insert(QStringLiteral("created"), HelpEntry(c->translate("MyAccount", "Created"), c->translate("MyAccount", "Date and time your account was created.")));
        help.insert(QStringLiteral("updated"), HelpEntry(c->translate("MyAccount", "Updated"), c->translate("MyAccount", "Date and time your account was last updated.")));
        help.insert(QStringLiteral("password"), HelpEntry(c->translate("MyAccount", "New password"), c->translate("MyAccount", "Enter a new password with a minimum length of %n character(s) or leave the field blank to avoid changing the password.", "", SkaffariConfig::accPwMinlength())));
        help.insert(QStringLiteral("password_confirmation"), HelpEntry(c->translate("MyAccount", "Confirm new password"), c->translate("MyAccount", "Confirm your new password by entering it again.")));
        help.insert(QStringLiteral("maxdisplay"), HelpEntry(c->translate("MyAccount", "Max display"), c->translate("MyAccount", "Set the number of results you want to load in paginated lists like the account list (minimum 15, maximum 255)")));
        help.insert(QStringLiteral("warnlevel"), HelpEntry(c->translate("MyAccount", "Warn level"), c->translate("MyAccount", "Set the percentage limit that will show warnings on number of accounts and quota usage.")));
        help.insert(QStringLiteral("lang"), HelpEntry(c->translate("MyAccount", "Language"), c->translate("MyAccount", "Select one of the supported languages.")));
        help.insert(QStringLiteral("tz"), HelpEntry(c->translate("MyAccount", "Time zone"), c->translate("MyAccount", "Select your time zone to enter and display date and time values appropriately.")));

        c->stash({
                     {QStringLiteral("template"), QStringLiteral("myaccount/index.html")},
                     {QStringLiteral("site_title"), c->translate("MyAccount", "My account")},
                     {QStringLiteral("adminaccount"), QVariant::fromValue<AdminAccount>(aac)},
                     {QStringLiteral("langs"), QVariant::fromValue<QVector<Language>>(Language::supportedLangs())},
                     {QStringLiteral("timezones"), QVariant::fromValue<QList<QByteArray>>(QTimeZone::availableTimeZoneIds())},
                     {QStringLiteral("help"), QVariant::fromValue<QHash<QString,HelpEntry>>(help)}
                 });

    } else {
        c->setStash(QStringLiteral("not_found_text"), c->translate("MyAccount", "There is no administrator account with database ID %1.").arg(user.id()));
        c->setStash(QStringLiteral("template"), QStringLiteral("404.html"));
        c->res()->setStatus(404);
    }
}

#include "moc_myaccount.cpp"
