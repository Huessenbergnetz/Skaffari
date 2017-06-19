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

#include "accounteditor.h"

#include "objects/account.h"
#include "objects/domain.h"
#include "objects/skaffarierror.h"
#include "objects/simpledomain.h"
#include "objects/helpentry.h"
#include "utils/skaffariconfig.h"
#include "utils/utils.h"

#include <Cutelyst/Plugins/Session/Session>
#include <Cutelyst/Plugins/Utils/Validator> // includes the main validator
#include <Cutelyst/Plugins/Utils/Validators> // includes all validator rules
#include <Cutelyst/Plugins/Utils/ValidatorResult> // includes the validator result
#include <Cutelyst/Plugins/StatusMessage>
#include <Cutelyst/Plugins/Authentication/authentication.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

using namespace Cutelyst;

AccountEditor::AccountEditor(QObject *parent) : Controller(parent)
{
}

AccountEditor::~AccountEditor()
{
}

void AccountEditor::base(Context* c, const QString &domainId, const QString& accountId)
{
    const quint32 domId = domainId.toULong();
    if (Domain::checkAccess(c, domId)) {
        Domain::toStash(c, domId);
        Account::toStash(c, Domain::fromStash(c), accountId.toULong());
    }
}



void AccountEditor::edit(Context* c)
{
    if (Domain::accessGranted(c)) {
        Account a = Account::fromStash(c);
        Domain dom = Domain::fromStash(c);

        const quint32 freeQuota = (dom.getDomainQuota() - dom.getDomainQuotaUsed() + a.getQuota());

        auto req = c->req();

        if (req->isPost()) {

            static Validator v({
                                   new ValidatorMin(QStringLiteral("quota"), QMetaType::UInt, 0),
                                   new ValidatorConfirmed(QStringLiteral("password")),
                                   new ValidatorMin(QStringLiteral("password"), QMetaType::QString, SkaffariConfig::accPwMinlength()),
                                   new ValidatorRequired(QStringLiteral("validUntil")),
                                   new ValidatorDateTime(QStringLiteral("validUntil"), QStringLiteral("yyyy-MM-dd HH:mm:ss"))
                               });

            ValidatorResult vr = v.validate(c, Validator::FillStashOnError);
            if (vr) {

                ParamsMultiMap p = c->req()->bodyParams();

                // if this domain has a global quota limit, we have to calculate the
                // quota that is left
                bool enoughQuotaLeft = true;
                if (dom.getDomainQuota() > 0) {

                    if ((freeQuota - a.getQuota()) < p.value(QStringLiteral("quota")).toULong()) {

                        enoughQuotaLeft = false;

                        c->setStash(QStringLiteral("error_msg"),
                                    c->translate("AccountEditor", "There is not enough free quota on this domain. Please lower the quota for the account to a maximum of %1 KiB.").arg(freeQuota));
                    }

                }

                if (enoughQuotaLeft) {
                    SkaffariError e(c);
                    if (Account::update(c,
                                        &e,
                                        &a,
                                        p)) {

                        Session::deleteValue(c, QStringLiteral("domainQuotaUsed_") + QString::number(dom.id()));
                        c->setStash(QStringLiteral("status_msg"), c->translate("AccountEditor", "Successfully updated account %1.").arg(a.getUsername()));
                        c->setStash(QStringLiteral("account"), QVariant::fromValue<Account>(a));
                    } else {
                        c->setStash(QStringLiteral("error_msg"), e.errorText());
                    }
                }
            }
        }

        QHash<QString,HelpEntry> help;
        help.insert(QStringLiteral("created"), HelpEntry(c->translate("AccountEditor", "Created"), c->translate("AcountEditor", "Date and time this account has been created.")));
        help.insert(QStringLiteral("updated"), HelpEntry(c->translate("AccountEditor", "Updated"), c->translate("AccountEditor", "Date and time this account has been updated the last time.")));
        help.insert(QStringLiteral("quota"), HelpEntry(c->translate("AccountEditor", "Quota"), c->translate("AccountEditor", "")));
        if (dom.getDomainQuota() > 0) {
            help.insert(QStringLiteral("quota"), HelpEntry(c->translate("AccountEditor", "Quota"), c->translate("AccountEditor", "You have to set a storage quota for this account that does not exceed %1.").arg(Utils::humanBinarySize(c, static_cast<quint64>(freeQuota) * Q_UINT64_C(1024)))));
        } else {
            help.insert(QStringLiteral("quota"), HelpEntry(c->translate("AccountEditor", "Quota"), c->translate("AccountEditor", "You can freely set a storage quota for this account or set the quota to 0 to disable it.")));
        }
        help.insert(QStringLiteral("password"), HelpEntry(c->translate("AccountEditor", "Password"), c->translate("AccountEditor", "Specify a password with a minimum length of %n character(s).", "", SkaffariConfig::accPwMinlength())));
        help.insert(QStringLiteral("password_confirmation"), HelpEntry(c->translate("AccountEditor", "Password confirmation"), c->translate("AccountEditor", "Confirm your entered password.")));
        help.insert(QStringLiteral("validUntil"), HelpEntry(c->translate("AccountEditor", "Valid until"), c->translate("AccountEditor", "You can set a date and time until this account is valid. To make it valid open-end, simply set a date far in the future.")));
        help.insert(QStringLiteral("imap"), HelpEntry(c->translate("AccountEditor", "IMAP Access"), c->translate("AccountEditor", "If enabled, the user of this account can access the mailbox through the IMAP protocol.")));
        help.insert(QStringLiteral("pop"), HelpEntry(c->translate("AccountEditor", "POP3 Access"), c->translate("AccountEditor", "If enabled, the user of this account can access the mailbox through the POP3 protocol.")));
        help.insert(QStringLiteral("sieve"), HelpEntry(c->translate("AccountEditor", "Sieve Access"), c->translate("AccountEditor", "If enabled, the user of this account can manage own Sieve scripts on the server.")));
        help.insert(QStringLiteral("smtpauth"), HelpEntry(c->translate("AccountEditor", "SMTP Access"), c->translate("AccountEditor", "If enabled, the user of this account can send emails via this server through the SMTP protocol.")));


        c->stash({
                     {QStringLiteral("template"), QStringLiteral("account/edit.html")},
                     {QStringLiteral("edit"), true},
                     {QStringLiteral("help"), QVariant::fromValue<QHash<QString,HelpEntry>>(help)}
                 });
    }
}


void AccountEditor::remove(Context* c)
{
    if (Domain::accessGranted(c)) {
        auto a = Account::fromStash(c);
        auto dom = Domain::fromStash(c);

        if (c->req()->isPost()) {

            if (c->req()->bodyParam(QStringLiteral("accountName")) == a.getUsername()) {

                SkaffariError e(c);
                if (Account::remove(c, &e, a.getUsername(), &dom)) {
                    c->res()->redirect(c->uriForAction(QStringLiteral("/domain/accounts"), QStringList(QString::number(dom.id())), QStringList(), StatusMessage::statusQuery(c, c->translate("AccountEditor", "Successfully removed account of user %1.").arg(a.getUsername()))));
                } else {
                    c->setStash(QStringLiteral("error_msg"), c->translate("AccountEditor", "Failed to remove account. %1").arg(e.errorText()));
                }
            } else {
                c->setStash(QStringLiteral("error_msg"), c->translate("AccountEditor", "The entered user name does not match the user name of the account you want to delete."));
            }
        }

        c->setStash(QStringLiteral("template"), QStringLiteral("account/remove.html"));
    }
}




void AccountEditor::addresses(Context *c)
{
    if (Domain::accessGranted(c)) {


        c->stash({
                     {QStringLiteral("template"), QStringLiteral("account/addresses.html")}
                 });
    }
}



void AccountEditor::email(Context *c, const QString &address)
{
    if (Domain::accessGranted(c)) {
        auto a = Account::fromStash(c);

        if (!a.getAddresses().contains(address)) {
            c->stash({
                         {QStringLiteral("template"), QStringLiteral("404.html")},
                         {QStringLiteral("not_found_text"), c->translate("AccountEditor", "The requested email address does not belong to the account %1.").arg(a.getUsername())}
                     });
            c->res()->setStatus(404);
            return;
        }

        const QStringList parts = address.split(QLatin1Char('@'));

        if (parts.size() == 2) {
            c->stash({
                         {QStringLiteral("localpart"), parts.at(0)},
                         {QStringLiteral("maildomain"), parts.at(1)}
                     });
        } else {
            c->setStash(QStringLiteral("error_msg"), c->translate("AccountEditor", "Invalid email address"));
        }

        auto d = Domain::fromStash(c);
        if (d.isFreeAddressEnabled()) {
            AuthenticationUser user = Authentication::user(c);
            SkaffariError sde(c);
            std::vector<SimpleDomain> maildomains = SimpleDomain::list(c, &sde, user.value(QStringLiteral("type")).value<quint8>(), user.id().toULong());
            c->setStash(QStringLiteral("maildomains"), QVariant::fromValue<std::vector<SimpleDomain>>(maildomains));
        }

        if (c->req()->isPost()) {

            static Validator v({
                                   new ValidatorRequired(QStringLiteral("newlocalpart")),
                                   new ValidatorRequired(QStringLiteral("newmaildomain"))
                               });

            ValidatorResult vr = v.validate(c, Validator::FillStashOnError);
            if (vr) {
                const ParamsMultiMap p = c->req()->bodyParams();
                if (!d.isFreeNamesEnabled() && (p.value(QStringLiteral("newmaildomain")) != d.getName())) {
                    c->setStash(QStringLiteral("error_msg"), c->translate("AccountEditor", "You can not create email addresses for other domains as long as free addresses are not allowed for this domain."));
                } else {
                    SkaffariError e(c);
                    if (Account::updateEmail(c, &e, &a, d, p, address)) {
                        const ParamsMultiMap successQueryParams = StatusMessage::statusQuery(c, c->translate("AccountEditor", "Successfully updated email address %1 to %2.").arg(address, p.value(QStringLiteral("newlocalpart")) + QLatin1Char('@') + p.value(QStringLiteral("newmaildomain"))));
                        c->res()->redirect(c->uriForAction(QStringLiteral("/account/addresses"), QStringList({QString::number(d.id()), QString::number(a.getId())}), QStringList(), successQueryParams));
                        return;
                    } else {
                        c->setStash(QStringLiteral("error_msg"), e.errorText());
                    }
                }
            }
        }

        c->stash({
                     {QStringLiteral("template"), QStringLiteral("account/email.html")},
                     {QStringLiteral("site_subtitle"), address}
                 });
    }
}



void AccountEditor::remove_email(Context *c, const QString &address)
{
    if (Domain::accessGranted(c)) {
        auto a = Account::fromStash(c);
        if (a.getAddresses().size() <= 1) {
            c->res()->redirect(c->uriForAction(QStringLiteral("/account/addresses"),
                                               QStringList({QString::number(a.getDomainId()), QString::number(a.getId())}),
                                               QStringList(),
                                               StatusMessage::errorQuery(c, c->translate("AccountEditor", "You can not remove the last email address for this account. Remove the entire account instead."))));
            return;
        }

        if (!a.getAddresses().contains(address)) {
            c->stash({
                         {QStringLiteral("template"), QStringLiteral("404.html")},
                         {QStringLiteral("not_found_text"), c->translate("AccountEditor", "The requested email address does not belong to the account %1.").arg(a.getUsername())}
                     });
            c->res()->setStatus(404);
            return;
        }

        if (c->req()->isPost()) {

            if (c->req()->bodyParam(QStringLiteral("email")) != address) {
                c->setStash(QStringLiteral("error_msg"), c->translate("AccountEditor", "The entered email address does not match the address you want to remove."));
            } else {
                SkaffariError e(c);
                if (Account::removeEmail(c, &e, &a, address)) {
                     c->res()->redirect(c->uriForAction(QStringLiteral("/account/addresses"),
                                                        QStringList({QString::number(a.getDomainId()), QString::number(a.getId())}),
                                                        QStringList(),
                                                        StatusMessage::statusQuery(c, c->translate("AccountEditor", "Successfully removed email address %1 from account %2.").arg(address, a.getUsername()))));
                } else {
                    c->setStash(QStringLiteral("error_msg"), e.errorText());
                }
            }

        }

        c->stash({
                     {QStringLiteral("template"), QStringLiteral("account/remove_email.html")},
                     {QStringLiteral("address"), address}
                 });
    }
}


void AccountEditor::new_email(Context *c)
{
    if (Domain::accessGranted(c)) {
        auto d = Domain::fromStash(c);
        auto a = Account::fromStash(c);

        if (c->req()->isPost()) {

            static Validator v({
                                   new ValidatorRequired(QStringLiteral("newlocalpart")),
                                   new ValidatorRequired(QStringLiteral("newmaildomain"))
                               });

            ValidatorResult vr = v.validate(c, Validator::FillStashOnError);
            if (vr) {
                const ParamsMultiMap p = c->req()->bodyParams();
                if (!d.isFreeNamesEnabled() && (p.value(QStringLiteral("newmaildomain")) != d.getName())) {
                    c->setStash(QStringLiteral("error_msg"), c->translate("AccountEditor", "You can not create email addresses for other domains as long as free addresses are not allowed for this domain."));
                } else {
                    SkaffariError e(c);
                    if (Account::addEmail(c, &e, &a, d, p)) {
                        const QString newEmailAddress = p.value(QStringLiteral("newlocalpart")) + QLatin1Char('@') + p.value(QStringLiteral("newmaildomain"));
                        c->res()->redirect(c->uriForAction(QStringLiteral("/account/addresses"),
                                                           QStringList({QString::number(d.id()), QString::number(a.getId())}),
                                                           QStringList(),
                                                           StatusMessage::statusQuery(c, c->translate("AccountEditor", "Successfully added email address %1 to account %2.").arg(newEmailAddress, a.getUsername()))));
                        return;
                    } else {
                        c->setStash(QStringLiteral("error_msg"), e.errorText());
                    }
                }
            }

        }

        if (d.isFreeAddressEnabled()) {
            AuthenticationUser user = Authentication::user(c);
            SkaffariError sde(c);
            std::vector<SimpleDomain> maildomains = SimpleDomain::list(c, &sde, user.value(QStringLiteral("type")).value<quint8>(), user.id().toULong());
            c->setStash(QStringLiteral("maildomains"), QVariant::fromValue<std::vector<SimpleDomain>>(maildomains));
        }

        c->setStash(QStringLiteral("template"), QStringLiteral("account/new_email.html"));
    }
}



void AccountEditor::forwards(Context *c)
{
    if (Domain::accessGranted(c)) {
        auto a = Account::fromStash(c);

        if (c->req()->isPost()) {

            SkaffariError e(c);
            if (Account::updateForwards(c, &e, &a, c->req()->bodyParams())) {
                c->stash({
                             {QStringLiteral("status_msg"), c->translate("AccountEditor", "Successfully updated forward mail addresses for account %1.").arg(a.getUsername())},
                             {QStringLiteral("account"), QVariant::fromValue<Account>(a)}
                         });
            } else {
                c->stash({
                             {QStringLiteral("error_msg"), e.errorText()},
                             {QStringLiteral("forwards"), c->req()->bodyParams(QStringLiteral("forward"))}
                         });
            }
        }

        c->setStash(QStringLiteral("template"), QStringLiteral("account/forwards.html"));
    }
}


void AccountEditor::check(Context *c)
{
    if (Domain::accessGranted(c)) {
        auto a = Account::fromStash(c);
        auto d = Domain::fromStash(c);

        SkaffariError e(c);
        QStringList actions;
        Account::check(c, &e, &a, d, &actions);

        if (c->req()->header(QStringLiteral("Accept")).contains(QLatin1String("application/json"), Qt::CaseInsensitive)) {

            QJsonObject result;
            result.insert(QStringLiteral("account"), QJsonValue(a.getUsername()));

            if (e.type() != SkaffariError::NoError) {
                result.insert(QStringLiteral("error_msg"), QJsonValue(e.errorText()));
                c->response()->setStatus(Response::InternalServerError);
            } else {
                result.insert(QStringLiteral("actions"), QJsonValue(QJsonArray::fromStringList(actions)));
            }

            QJsonDocument json(result);

            const QByteArray jsonBody = json.toJson(QJsonDocument::Compact);

            c->response()->setJsonBody(json);
            c->response()->setContentEncoding(QStringLiteral("UTF-8"));
            c->response()->setContentLength(jsonBody.length());

        } else {
            if (e.type() != SkaffariError::NoError) {
                c->setStash(QStringLiteral("error_msg"), e.errorText());
                c->response()->setStatus(Response::InternalServerError);
            }

            c->setStash(QStringLiteral("actions"), actions);
            c->setStash(QStringLiteral("template"), QStringLiteral("account/check.html"));
        }
    }
}
