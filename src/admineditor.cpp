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

#include "admineditor.h"
#include "objects/adminaccount.h"
#include "objects/skaffarierror.h"
#include "objects/simpledomain.h"
#include "objects/helpentry.h"
#include "utils/skaffariconfig.h"
#include "utils/utils.h"
#include <Cutelyst/Plugins/StatusMessage>
#include <Cutelyst/Plugins/Utils/Validator> // includes the main validator
#include <Cutelyst/Plugins/Utils/ValidatorResult> // includes the validator result
#include <Cutelyst/Plugins/Utils/validatorrequired.h>
#include <Cutelyst/Plugins/Utils/validatoralphadash.h>
#include <Cutelyst/Plugins/Utils/validatorconfirmed.h>
#ifdef PWQUALITY_ENABLED
#include <Cutelyst/Plugins/Utils/validatorpwquality.h>
#else
#include <Cutelyst/Plugins/Utils/validatormin.h>
#endif
#include <Cutelyst/Plugins/Authentication/authentication.h>
#include <QVector>
#include <QVariant>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

AdminEditor::AdminEditor(QObject *parent) : Controller(parent)
{

}


AdminEditor::~AdminEditor()
{

}



void AdminEditor::index(Context *c)
{
    if (checkAccess(c)) {
        SkaffariError e(c);
        QVector<AdminAccount> accounts = AdminAccount::list(c, &e);

        if (Q_UNLIKELY(e.type() != SkaffariError::NoError)) {
            c->setStash(QStringLiteral("error_msg"), e.errorText());
        }

        c->stash({
                     {QStringLiteral("adminaccounts"), QVariant::fromValue<QVector<AdminAccount>>(accounts)},
                     {QStringLiteral("template"), QStringLiteral("admin/index.html")},
                     {QStringLiteral("site_title"), c->translate("AdminEditor", "Administrators")}
                 });
    }
}




void AdminEditor::base(Context *c, const QString &id)
{
    if (checkAccess(c)) {
        AdminAccount::toStash(c, id.toULong());
    }
}



void AdminEditor::create(Context *c)
{
    if (checkAccess(c)) {

        auto req = c->req();
        if (req->isPost()) {

            static Validator v({
                                   new ValidatorRequired(QStringLiteral("username")),
                                   new ValidatorAlphaDash(QStringLiteral("username")),
                                   new ValidatorRequired(QStringLiteral("password")),
                                   new ValidatorConfirmed(QStringLiteral("password")),
                       #ifdef PWQUALITY_ENABLED
                                   new ValidatorPwQuality(QStringLiteral("password"), SkaffariConfig::admPwThreshold(), SkaffariConfig::admPwSettingsFile(), QStringLiteral("username"))
                       #else
                                   new ValidatorMin(QStringLiteral("password"), QMetaType::QString, SkaffariConfig::admPwMinlength())
                       #endif
                               });

            const ValidatorResult vr = v.validate(c, Validator::FillStashOnError|Validator::BodyParamsOnly);
            if (vr) {
                SkaffariError e(c);
                AdminAccount::create(c, req->bodyParameters(), &e);

                if (e.type() == SkaffariError::NoError) {
                    c->res()->redirect(c->uriForAction(QStringLiteral("/admin/index"),
                                                       StatusMessage::statusQuery(c, c->translate("AdminEditor", "Successfully created new administrator %1.").arg(req->param(QStringLiteral("username"))))
                                                       )
                                       );
                } else {
                    c->setStash(QStringLiteral("error_msg"), e.errorText());
                }
            }

            c->setStash(QStringLiteral("assocdomains"), QVariant::fromValue<QStringList>(req->bodyParameters().values(QStringLiteral("assocdomains"))));
        }

        SkaffariError e(c);
        // if access has been granted, user type is 0
        std::vector<SimpleDomain> domains = SimpleDomain::list(c, &e, 0, 0);

        if (e.type() != SkaffariError::NoError) {
            c->setStash(QStringLiteral("error_msg"), e.errorText());
        } else {
            c->setStash(QStringLiteral("domains"), QVariant::fromValue<std::vector<SimpleDomain>>(domains));
        }

        QHash<QString,HelpEntry> help;
        help.insert(QStringLiteral("username"), HelpEntry(c->translate("AdminEditor", "User name"), c->translate("AdminEditor", "The user name for the new administrator. Can only contain alpha-numeric characters as well as dashes and underscores.")));
        const int pwMinLength = static_cast<int>(SkaffariConfig::admPwMinlength());
        help.insert(QStringLiteral("password"), HelpEntry(c->translate("AdminEditor", "Password"), c->translate("AdminEditor", "Specify a password with a minimum length of %n character(s).", nullptr, pwMinLength)));
        help.insert(QStringLiteral("password_confirmation"), HelpEntry(c->translate("AdminEditor", "Password confirmation"), c->translate("AdminEditor", "Confirm the password by entering it again.")));
        help.insert(QStringLiteral("type"), HelpEntry(c->translate("AdminEditor", "Type"), c->translate("AdminEditor", "An administrator has access to the whole system, while a domain manager only has access to the associated domains.")));
        help.insert(QStringLiteral("assocdomains"), HelpEntry(c->translate("AdminEditor", "Domains"), c->translate("AdminEditor", "For domain managers, select the associated domains the user is responsible for.")));

        c->stash({
                     {QStringLiteral("help"), QVariant::fromValue<QHash<QString,HelpEntry>>(help)},
                     {QStringLiteral("template"), QStringLiteral("admin/create.html")},
                     {QStringLiteral("site_title"), c->translate("AdminUser", "Create administrator")}
                 });

    }
}



void AdminEditor::edit(Context *c)
{
    if (accessGranted(c)) {

        auto req = c->req();
        if (req->isPost()) {

            auto aac = AdminAccount::fromStash(c);
            c->setStash(QStringLiteral("_pwq_username"), aac.username());

            static Validator v({
                                   new ValidatorConfirmed(QStringLiteral("password")),
                       #ifdef PWQUALITY_ENABLED
                                   new ValidatorPwQuality(QStringLiteral("password"),
                                   SkaffariConfig::admPwThreshold(),
                                   SkaffariConfig::admPwSettingsFile(),
                                   QStringLiteral("_pwq_username"))
                       #else
                                   new ValidatorMin(QStringLiteral("password"), QMetaType::QString, SkaffariConfig::admPwMinlength())
                       #endif
                               });

            ValidatorResult vr = v.validate(c, Validator::FillStashOnError|Validator::BodyParamsOnly);

            if (vr) {

                SkaffariError e(c);
                if (aac.update(c, &e, req->bodyParameters())) {
                    c->stash({
                                 {QStringLiteral("adminaccount"), QVariant::fromValue<AdminAccount>(aac)},
                                 {QStringLiteral("status_msg"), c->translate("AdminEditor", "Successfully updated administrator %1.").arg(aac.username())}
                             });
                } else {
                    c->setStash(QStringLiteral("error_msg"), e.errorText());
                    StatusMessage::errorQuery(c, e.errorText());
                }

            }
        }

        SkaffariError e(c);
        // if access has been granted, user type is 0
        std::vector<SimpleDomain> domains = SimpleDomain::list(c, &e, 0, 0);

        if (e.type() != SkaffariError::NoError) {
            c->setStash(QStringLiteral("error_msg"), e.errorText());
        } else {
            c->setStash(QStringLiteral("domains"), QVariant::fromValue<std::vector<SimpleDomain>>(domains));
        }

        QHash<QString,HelpEntry> help;
        help.insert(QStringLiteral("created"), HelpEntry(c->translate("AdminEditor", "Created"), c->translate("AdminEditor", "Date and time this account was created.")));
        help.insert(QStringLiteral("updated"), HelpEntry(c->translate("AdminEditor", "Updated"), c->translate("AdminEditor", "Date and time this account was last updated.")));
        help.insert(QStringLiteral("username"), HelpEntry(c->translate("AdminEditor", "User name"), c->translate("AdminEditor", "The user name of the administrator.")));
        const int pwMinLength = static_cast<int>(SkaffariConfig::admPwMinlength());
        help.insert(QStringLiteral("password"), HelpEntry(c->translate("AdminEditor", "New password"), c->translate("AdminEditor", "Enter a new password with a minimum length of %n character(s) or leave the field blank to avoid changing the password.", nullptr, pwMinLength)));
        help.insert(QStringLiteral("password_confirmation"), HelpEntry(c->translate("AdminEditor", "Confirm new password"), c->translate("AdminEditor", "Confirm the new password by entering it again.")));
        help.insert(QStringLiteral("type"), HelpEntry(c->translate("AdminEditor", "Type"), c->translate("AdminEditor", "An administrator has access to the whole system, while a domain manager only has access to the associated domains.")));
        help.insert(QStringLiteral("assocdomains"), HelpEntry(c->translate("AdminEditor", "Domains"), c->translate("AdminEditor", "For domain managers, select the associated domains the user is responsible for.")));

        c->stash({
                     {QStringLiteral("help"), QVariant::fromValue<QHash<QString,HelpEntry>>(help)},
                     {QStringLiteral("template"), QStringLiteral("admin/edit.html")},
                     {QStringLiteral("site_subtitle"), c->translate("AdminEditor", "Edit")}
                 });
    }
}


void AdminEditor::remove(Context *c)
{
    if (accessGranted(c)) {
        auto req = c->req();

        const bool isAjax = Utils::isAjax(c);
        QJsonObject json;

        if (req->isPost()) {

            auto aac = AdminAccount::fromStash(c);

            if (aac.username() == req->bodyParam(QStringLiteral("adminName"))) {

                SkaffariError e(c);
                if (aac.remove(c, &e)) {

                    const QString statusMsg = c->translate("AdminEditor", "Successfully removed administrator %1.").arg(aac.username());

                    if (isAjax) {
                        json.insert(QStringLiteral("status_msg"), statusMsg);
                        json.insert(QStringLiteral("admin_id"), static_cast<qint64>(aac.id()));
                        json.insert(QStringLiteral("admin_name"), aac.username());
                    } else {
                        c->res()->redirect(c->uriFor(QStringLiteral("/admin"), StatusMessage::statusQuery(c, statusMsg)));
                    }

                } else {

                    c->res()->setStatus(Response::InternalServerError);

                    if (isAjax) {
                        json.insert(QStringLiteral("error_msg"), e.errorText());
                    } else {
                        c->setStash(QStringLiteral("error_msg"), e.errorText());
                    }

                }

            } else {
                c->res()->setStatus(Response::BadRequest);

                const QString errorMsg = c->translate("AdminEditor", "The entered user name does not match the user name of the administrator you want to delete.");

                if (isAjax) {
                    json.insert(QStringLiteral("error_msg"), errorMsg);
                } else {
                    c->setStash(QStringLiteral("error_msg"), errorMsg);
                }

            }

        } else {
            // this is not a post request, for ajax, we will only allow post
            if (isAjax) {
                json.insert(QStringLiteral("error_msg"), QJsonValue(c->translate("AdminEditor", "For AJAX requests, this route is only available via POST requests.")));
                c->response()->setStatus(Response::MethodNotAllowed);
                c->response()->setHeader(QStringLiteral("Allow"), QStringLiteral("POST"));
            }
        }

        if (isAjax) {
            c->res()->setJsonBody(QJsonDocument(json));
        } else {
            c->stash({
                         {QStringLiteral("site_subtitle"), c->translate("AdminEditor", "Remove")},
                         {QStringLiteral("template"), QStringLiteral("admin/remove.html")}
                     });
        }
    }
}


bool AdminEditor::checkAccess(Context *c)
{
    if (Q_LIKELY(c->stash(QStringLiteral("userType")).value<quint16>() == 0)) {
        return true;
    }

    if (Utils::isAjax(c)) {
        QJsonObject json({
                             {QStringLiteral("error_msg"), c->translate("AdminEditor", "You are not authorized to access this resource or to perform this action.")}
                         });
        c->res()->setJsonBody(QJsonDocument(json));
    } else {
        c->stash({
                     {QStringLiteral("site_title"), c->translate("AdminEditor", "Access denied")},
                     {QStringLiteral("template"), QStringLiteral("403.html")}
                 });
    }
    c->res()->setStatus(403);

    return false;
}


bool AdminEditor::accessGranted(Context *c)
{
    const quint16 status = c->res()->status();
    return ((status != 404) && (status != 403));
}

#include "moc_admineditor.cpp"
