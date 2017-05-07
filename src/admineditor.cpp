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
#include "utils/skaffariconfig.h"
#include <Cutelyst/Plugins/StatusMessage>
#include <Cutelyst/Plugins/Utils/Validator> // includes the main validator
#include <Cutelyst/Plugins/Utils/Validators> // includes all validator rules
#include <Cutelyst/Plugins/Utils/ValidatorResult> // includes the validator result
#include <Cutelyst/Plugins/Authentication/authentication.h>
#include <QVector>
#include <QVariant>

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
                                   new ValidatorMin(QStringLiteral("password"), QMetaType::QString, SkaffariConfig::admPwMinlength()),
                                   new ValidatorConfirmed(QStringLiteral("password"))
                               });

            ValidatorResult vr = v.validate(c, Validator::FillStashOnError);
            if (vr) {
                SkaffariError e(c);
                AdminAccount::create(c, req->parameters(), &e);

                if (e.type() == SkaffariError::NoError) {
                    c->res()->redirect(c->uriForAction(QStringLiteral("/admin/index"),
                                                       StatusMessage::statusQuery(c, c->translate("AdminEditor", "Successfully created new administrator %1.").arg(req->param(QStringLiteral("username"))))
                                                       )
                                       );
                }
            }

            c->setStash(QStringLiteral("assocdomains"), QVariant::fromValue<QStringList>(req->parameters().values(QStringLiteral("assocdomains"))));
        }

        SkaffariError e(c);
        // if access has been granted, user type is 0
        std::vector<SimpleDomain> domains = SimpleDomain::list(c, &e, 0, 0);

        if (e.type() != SkaffariError::NoError) {
            c->setStash(QStringLiteral("error_msg"), e.errorText());
        } else {
            c->setStash(QStringLiteral("domains"), QVariant::fromValue<std::vector<SimpleDomain>>(domains));
        }

        c->stash({
                     {QStringLiteral("template"), QStringLiteral("admin/create.html")},
                     {QStringLiteral("site_title"), c->translate("AdminUser", "Create admin")}
                 });

    }
}



void AdminEditor::edit(Context *c)
{
    if (accessGranted(c)) {

        auto req = c->req();
        if (req->isPost()) {

            static Validator v({
                                   new ValidatorConfirmed(QStringLiteral("password")),
                                   new ValidatorMin(QStringLiteral("password"), QMetaType::QString, SkaffariConfig::admPwMinlength())
                               });

            ValidatorResult vr = v.validate(c, Validator::FillStashOnError);

            if (vr) {

                auto aac = AdminAccount::fromStash(c);

                SkaffariError e(c);
                AdminAccount::update(c,
                                     &e,
                                     &aac,
                                     req->parameters());

                if (e.type() == SkaffariError::NoError) {
                    c->setStash(QStringLiteral("status_msg"), c->translate("AdminEditor", "Successfully updated admin %1.").arg(aac.getUsername()));
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

        c->stash({
                     {QStringLiteral("template"), QStringLiteral("admin/edit.html")},
                     {QStringLiteral("site_subtitle"), c->translate("AdminEditor", "Edit")}
                 });
    }
}


void AdminEditor::remove(Context *c)
{
    if (accessGranted(c)) {
        auto req = c->req();

        if (req->isPost()) {

            auto aac = AdminAccount::fromStash(c);

            if (aac.getUsername() == req->param(QStringLiteral("adminName"))) {
                SkaffariError e(c);
                if (AdminAccount::remove(c, &e, aac)) {
                    c->res()->redirect(c->uriFor(QStringLiteral("/admin"), StatusMessage::statusQuery(c, c->translate("AdminEditor", "Successfully remove admin %1.").arg(aac.getUsername()))));
                } else {
                    c->setStash(QStringLiteral("error_msg"), e.errorText());
                }
            } else {
                c->setStash(QStringLiteral("error_msg"), c->translate("AdminEditor", "The entered user name does not match the user name of the admin you want to delete."));
            }

        }

        c->stash({
                     {QStringLiteral("site_subtitle"), c->translate("AdminEditor", "Remove")},
                     {QStringLiteral("template"), QStringLiteral("admin/remove.html")}
                 });

    }

}


bool AdminEditor::checkAccess(Context *c)
{
    if (Q_LIKELY(c->stash(QStringLiteral("userType")).value<quint16>() == 0)) {
        return true;
    }

    c->stash({
                 {QStringLiteral("site_title"), c->translate("AdminEditor", "Access denied")},
                 {QStringLiteral("template"), QStringLiteral("403.html")}
             });
    c->res()->setStatus(403);

    return false;
}


bool AdminEditor::accessGranted(Context *c)
{
    const quint16 status = c->res()->status();
    return ((status != 404) && (status != 403));
}
