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
#include "objects/simpleaccount.h"
#include "objects/helpentry.h"
#include "utils/skaffariconfig.h"
#include "utils/utils.h"
#include "validators/skvalidatordomainexists.h"
#include "../common/global.h"

#include <Cutelyst/Plugins/Session/Session>
#include <Cutelyst/Plugins/Utils/Validator> // includes the main validator
#include <Cutelyst/Plugins/Utils/ValidatorResult> // includes the validator result
#include <Cutelyst/Plugins/Utils/validatorconfirmed.h>
#include <Cutelyst/Plugins/Utils/validatorrequired.h>
#include <Cutelyst/Plugins/Utils/validatordatetime.h>
#include <Cutelyst/Plugins/Utils/validatorboolean.h>
#include <Cutelyst/Plugins/Utils/validatorfilesize.h>
#include <Cutelyst/Plugins/Utils/validatormin.h>
#include <Cutelyst/Plugins/Utils/validatorpwquality.h>
#include <Cutelyst/Plugins/Utils/validatoremail.h>
#include <Cutelyst/Plugins/StatusMessage>
#include <Cutelyst/Plugins/Authentication/authentication.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <limits>
#include <utility>

using namespace Cutelyst;

AccountEditor::AccountEditor(QObject *parent) : Controller(parent)
{
}

AccountEditor::~AccountEditor()
{
}

void AccountEditor::base(Context* c, const QString &domainId, const QString& accountId)
{
    const dbid_t domId = SKAFFARI_STRING_TO_DBID(domainId);
    if (Domain::checkAccess(c, domId)) {
        Domain::toStash(c, domId);
        Account::toStash(c, SKAFFARI_STRING_TO_DBID(accountId));
    }
}



void AccountEditor::edit(Context* c)
{
    if (Domain::accessGranted(c)) {
        Account a = Account::fromStash(c);
        Domain dom = Domain::fromStash(c);

        const quota_size_t freeQuota = (dom.domainQuota() - dom.domainQuotaUsed() + a.quota());
        if (dom.domainQuota() > 0) {
            c->setStash(QStringLiteral("freeQuota"), freeQuota * Q_UINT64_C(1024));
            c->setStash(QStringLiteral("minQuota"), 1024);
        } else {
            c->setStash(QStringLiteral("freeQuota"), std::numeric_limits<quota_size_t>::max());
            c->setStash(QStringLiteral("minQuota"), 0);
        }

        auto req = c->req();

        if (req->isPost()) {

            c->setStash(QStringLiteral("_def_boolean"), false);
            c->setStash(QStringLiteral("_pwq_username"), a.username());

            static Validator v({
                                   new ValidatorConfirmed(QStringLiteral("password")),
                       #ifdef PWQUALITY_ENABLED
                                   new ValidatorPwQuality(QStringLiteral("password"), SkaffariConfig::accPwThreshold(), SkaffariConfig::accPwSettingsFile(), QStringLiteral("_pwq_username")),
                       #else
                                   new ValidatorMin(QStringLiteral("password"), QMetaType::QString, SkaffariConfig::accPwMinlength()),
                       #endif
                                   new ValidatorRequired(QStringLiteral("validUntil")),
                                   new ValidatorDateTime(QStringLiteral("validUntil"), QStringLiteral("userTz"), "yyyy-MM-ddTHH:mm"),
                                   new ValidatorRequired(QStringLiteral("passwordExpires")),
                                   new ValidatorDateTime(QStringLiteral("passwordExpires"), QStringLiteral("userTz"), "yyyy-MM-ddTHH:mm"),
                                   new ValidatorBoolean(QStringLiteral("imap"), ValidatorMessages(), QStringLiteral("_def_boolean")),
                                   new ValidatorBoolean(QStringLiteral("pop"), ValidatorMessages(), QStringLiteral("_def_boolean")),
                                   new ValidatorBoolean(QStringLiteral("sieve"), ValidatorMessages(), QStringLiteral("_def_boolean")),
                                   new ValidatorBoolean(QStringLiteral("smtpauth"), ValidatorMessages(), QStringLiteral("_def_boolean")),
                                   new ValidatorBoolean(QStringLiteral("catchall"), ValidatorMessages(), QStringLiteral("_def_boolean")),
                                   new ValidatorRequired(QStringLiteral("quota")),
                                   new ValidatorFileSize(QStringLiteral("quota"), ValidatorFileSize::ForceBinary, QStringLiteral("minQuota"), QStringLiteral("freeQuota"))
                               });

            const ValidatorResult vr = v.validate(c, Validator::FillStashOnError|Validator::BodyParamsOnly);
            if (vr) {
                SkaffariError e(c);
                if (a.update(c, &e, &dom, vr.values())) {
                    c->stash({
                                 {QStringLiteral("status_msg"), c->translate("AccountEditor", "User account %1 successfully updated.").arg(a.username())},
                                 {QStringLiteral("account"), QVariant::fromValue<Account>(a)},
                                 {QStringLiteral("domain"), QVariant::fromValue<Domain>(dom)}
                             });
                } else {
                    c->setStash(QStringLiteral("error_msg"), e.errorText());
                    c->res()->setStatus(500);
                }
            } else {
                c->res()->setStatus(400);
            }
        }

        QHash<QString,HelpEntry> help;
        help.insert(QStringLiteral("created"), HelpEntry(c->translate("AccountEditor", "Created"), c->translate("AccountEditor", "Date and time this user account was created.")));
        help.insert(QStringLiteral("updated"), HelpEntry(c->translate("AccountEditor", "Updated"), c->translate("AccountEditor", "Date and time this user account was last updated.")));

        const QString quotaTitle = c->translate("DomainEditor", "Quota");
        if (dom.domainQuota() > 0) {
            // %1 will be something like 1.5 GB
            help.insert(QStringLiteral("quota"), HelpEntry(quotaTitle, c->translate("AccountEditor", "You must set a storage quota for this account that does not exceed %1. You can use the multipliers K, KiB, M, MiB, G, GiB, etc.").arg(Utils::humanBinarySize(c, freeQuota * Q_UINT64_C(1024)))));
        } else {
            // %1 will be something like 1.5 TiB
            help.insert(QStringLiteral("quota"), HelpEntry(quotaTitle, c->translate("AccountEditor", "You can optionally set a storage quota for this account that does not exceed %1. To disable the storage quota, set it to 0. You can use the multipliers K, KiB, M, MiB, G, GiB, etc.").arg(Utils::humanBinarySize(c, std::numeric_limits<quota_size_t>::max()))));
        }
        const int pwMinLength = static_cast<int>(SkaffariConfig::accPwMinlength());
        help.insert(QStringLiteral("password"), HelpEntry(c->translate("AccountEditor", "New password"), c->translate("AccountEditor", "Enter a new password with a minimum length of %n character(s) or leave the field blank to avoid changing the password.", nullptr, pwMinLength)));
        help.insert(QStringLiteral("password_confirmation"), HelpEntry(c->translate("AccountEditor", "New password confirmation"), c->translate("AccountEditor", "Confirm the new password by entering it again.")));
        help.insert(QStringLiteral("validUntil"), HelpEntry(c->translate("AccountEditor", "Valid until"), c->translate("AccountEditor", "You can set a date and time until this account is valid. To make it valid open-end, simply set a date far in the future.")));
        help.insert(QStringLiteral("passwordExpires"), HelpEntry(c->translate("AccountEditor", "Password expires"), c->translate("AccountEditor", "You can set a date and time until the password for this account is valid. To let the password never expire, simply set a date far in the future.")));
        help.insert(QStringLiteral("imap"), HelpEntry(c->translate("AccountEditor", "IMAP Access"), c->translate("AccountEditor", "If enabled, the user of this account can access the mailbox through the IMAP protocol.")));
        help.insert(QStringLiteral("pop"), HelpEntry(c->translate("AccountEditor", "POP3 Access"), c->translate("AccountEditor", "If enabled, the user of this account can access the mailbox through the POP3 protocol.")));
        help.insert(QStringLiteral("sieve"), HelpEntry(c->translate("AccountEditor", "Sieve Access"), c->translate("AccountEditor", "If enabled, the user of this account can manage own Sieve scripts on the server.")));
        help.insert(QStringLiteral("smtpauth"), HelpEntry(c->translate("AccountEditor", "SMTP Access"), c->translate("AccountEditor", "If enabled, the user of this account can send emails via this server through the SMTP protocol.")));

        SkaffariError getCatchAllError(c);
        const QString catchAllUser = dom.getCatchAllAccount(c, &getCatchAllError);
        const QString catchAllTitle = c->translate("AccountEditor", "Catch All");
        QString catchAllHelp;
        if (catchAllUser == a.username()) {
            catchAllHelp = c->translate("AccountEditor", "If disabled, this user will not receive emails anymore that have been sent to addresses not defined for this domain. There will also be no other user that will receive emails to undefined addresses.");
        } else {
            if (catchAllUser.isEmpty()) {
                catchAllHelp = c->translate("AccountEditor", "If enabled, this user will receive all emails sent to addresses not defined for this domain.");
            } else {
                catchAllHelp = c->translate("AccountEditor", "If enabled, this user will receive all emails sent to addresses not defined for this domain. The currently defined user %1 will not receive any messages to undefined addresses anymore.").arg(catchAllUser);
            }
        }
        help.insert(QStringLiteral("catchall"), HelpEntry(catchAllTitle, catchAllHelp));

        ValidatorFileSize::inputPattern(c, QStringLiteral("quotaPattern"));

        c->stash({
                     {QStringLiteral("template"), QStringLiteral("account/edit.html")},
                     {QStringLiteral("help"), QVariant::fromValue<QHash<QString,HelpEntry>>(help)}
                 });
    }
}


void AccountEditor::remove(Context* c)
{
    if (Domain::accessGranted(c)) {
        auto a = Account::fromStash(c);
        auto dom = Domain::fromStash(c);

        const bool isAjax = Utils::isAjax(c);
        QJsonObject json;

        if (c->req()->isPost()) {

            if (c->req()->bodyParam(QStringLiteral("accountName")) == a.username()) {

                SkaffariError e(c);
                if (a.remove(c, &e)) {

                    const QString statusMsg = c->translate("AccountEditor", "User account %1 successfully removed.").arg(a.username());

                    if (isAjax) {
                        json = QJsonObject({
                                               {QStringLiteral("status_msg"), statusMsg},
                                               {QStringLiteral("account_id"), static_cast<qint64>(a.id())},
                                               {QStringLiteral("account_name"), a.username()},
                                               {QStringLiteral("domain_id"), static_cast<qint64>(dom.id())},
                                               {QStringLiteral("domain_name"), dom.name()}
                                           });

                    } else {
                        c->res()->redirect(c->uriForAction(QStringLiteral("/domain/accounts"), QStringList(QString::number(dom.id())), QStringList(), StatusMessage::statusQuery(c, statusMsg)));
                    }

                } else {

                    c->res()->setStatus(Response::InternalServerError);

                    const QString errorMsg = c->translate("AccountEditor", "Failed to remove account %1").arg(e.errorText());

                    if (isAjax) {
                        json.insert(QStringLiteral("error_msg"), errorMsg);
                    } else {
                        c->setStash(QStringLiteral("error_msg"), errorMsg);
                    }
                }

            } else {

                c->res()->setStatus(Response::BadRequest);

                const QString errorMsg = c->translate("AccountEditor", "The entered user name does not match the user name of the account you want to delete.");

                if (isAjax) {
                    json.insert(QStringLiteral("error_msg"), errorMsg);
                } else {
                    c->setStash(QStringLiteral("error_msg"), errorMsg);
                }
            }
        } else {
            // this is not an post request, for ajax, we will only allow post
            if (isAjax) {
                json.insert(QStringLiteral("error_msg"), QJsonValue(c->translate("AccountEditor", "For AJAX requests, this route is only available via POST requests.")));
                c->response()->setStatus(Response::MethodNotAllowed);
                c->response()->setHeader(QStringLiteral("Allow"), QStringLiteral("POST"));
            }
        }

        if (isAjax) {
            c->res()->setJsonBody(QJsonDocument(json));
        } else {
            c->setStash(QStringLiteral("template"), QStringLiteral("account/remove.html"));
        }
    }
}




void AccountEditor::addresses(Context *c)
{
    if (Domain::accessGranted(c)) {

        auto d = Domain::fromStash(c);

        if (d.isFreeAddressEnabled()) {
            AuthenticationUser user = Authentication::user(c);
            SkaffariError sde(c);
            const qint16 userType = user.value(QStringLiteral("type")).value<qint16>();
            const std::vector<SimpleDomain> maildomains = SimpleDomain::list(c, &sde, userType, user.id().value<dbid_t>());
            c->setStash(QStringLiteral("maildomains"), QVariant::fromValue<std::vector<SimpleDomain>>(maildomains));
        } else if (!d.children().empty()) {
            QVector<SimpleDomain> maildomains = d.children();
            maildomains.prepend(d.toSimple());
            c->setStash(QStringLiteral("maildomains"), QVariant::fromValue<QVector<SimpleDomain>>(maildomains));
        }

        c->setStash(QStringLiteral("template"), QStringLiteral("account/addresses.html"));
    }
}



void AccountEditor::edit_address(Context *c, const QString &address)
{
    if (Domain::accessGranted(c)) {

        auto a = Account::fromStash(c);

        const QString oldAddress = QUrl::fromPercentEncoding(address.toLatin1());
        const bool isAjax = Utils::isAjax(c);
        QJsonObject json;

        if (!a.addresses().contains(oldAddress)) {

            const QString errorMsg = c->translate("AccountEditor", "The email address %1 is not part of the user account %2.").arg(oldAddress, a.username());

            if (isAjax) {
                json.insert(QStringLiteral("error_msg"), QJsonValue(errorMsg));
                c->res()->setJsonBody(QJsonDocument(json));
            } else {
                c->stash({
                             {QStringLiteral("template"), QStringLiteral("404.html")},
                             {QStringLiteral("not_found_text"), errorMsg}
                         });
            }

            c->res()->setStatus(Response::NotFound);
            return;
        }

        auto d = Domain::fromStash(c);

        if (!isAjax) {

            std::pair<QString,QString> parts = Account::addressParts(oldAddress);

            if (parts.first.isEmpty() || parts.second.isEmpty()) {
                c->setStash(QStringLiteral("error_msg"), c->translate("AccountEditor", "Invalid email address."));
            } else {
                c->stash({
                             {QStringLiteral("localpart"), parts.first},
                             {QStringLiteral("maildomain"), parts.second}
                         });
            }

            if (d.isFreeAddressEnabled()) {
                AuthenticationUser user = Authentication::user(c);
                SkaffariError sde(c);
                const std::vector<SimpleDomain> maildomains = SimpleDomain::list(c, &sde, user.value(QStringLiteral("type")).value<quint8>(), user.id().value<dbid_t>());
                c->setStash(QStringLiteral("maildomains"), QVariant::fromValue<std::vector<SimpleDomain>>(maildomains));
            } else if (!d.children().empty()) {
                QVector<SimpleDomain> maildomains = d.children();
                maildomains.prepend(d.toSimple());
                c->setStash(QStringLiteral("maildomains"), QVariant::fromValue<QVector<SimpleDomain>>(maildomains));
            }
        }

        if (c->req()->isPost()) {

            static Validator v({
                                   new ValidatorRequired(QStringLiteral("newlocalpart")),
                                   new ValidatorRequired(QStringLiteral("newmaildomain")),
                                   new ValidatorMin(QStringLiteral("newmaildomain"), QMetaType::UInt, 1),
                                   new SkValidatorDomainExists(QStringLiteral("newmaildomain"))
                               });

            const ValidatorResult vr = (isAjax) ? v.validate(c) : v.validate(c, Validator::FillStashOnError|Validator::BodyParamsOnly);

            if (vr) {

                SkaffariError e(c);
                const QString newAddress = a.updateEmail(c, &e, vr.values(), oldAddress);
                if (e.type() == SkaffariError::NoError) {

                    const QString statusMsg = c->translate("AccountEditor", "Successfully changed email address from %1 to %2.").arg(oldAddress, newAddress);

                    if (isAjax) {

                        json.insert(QStringLiteral("domain_id"), static_cast<qint64>(d.id()));
                        json.insert(QStringLiteral("account_id"), static_cast<qint64>(a.id()));
                        json.insert(QStringLiteral("status_msg"), statusMsg);
                        json.insert(QStringLiteral("old_address"), oldAddress);
                        json.insert(QStringLiteral("new_address"), newAddress);

                    } else {
                        const ParamsMultiMap successQueryParams = StatusMessage::statusQuery(c, statusMsg);
                        c->res()->redirect(c->uriForAction(QStringLiteral("/account/addresses"), QStringList({QString::number(d.id()), QString::number(a.id())}), QStringList(), successQueryParams));
                        return;
                    }

                } else {

                    if (isAjax) {
                        json.insert(QStringLiteral("error_msg"), e.errorText());
                    } else {
                        c->setStash(QStringLiteral("error_msg"), e.errorText());
                    }
                    c->res()->setStatus(Response::InternalServerError);
                }

            } else {

                c->res()->setStatus(Response::BadRequest);

                if (isAjax) {
                    json.insert(QStringLiteral("field_errors"), QJsonValue(vr.errorsJsonObject()));
                }
            }

        } else {
            // this is not an post request, for ajax, we will only allow post
            if (isAjax) {
                json.insert(QStringLiteral("error_msg"), QJsonValue(c->translate("AccountEditor", "For AJAX requests, this route is only available via POST requests.")));
                c->response()->setStatus(Response::MethodNotAllowed);
                c->response()->setHeader(QStringLiteral("Allow"), QStringLiteral("POST"));
            }
        }

        if (isAjax) {
            c->res()->setJsonObjectBody(json);
        } else {
            c->stash({
                         {QStringLiteral("template"), QStringLiteral("account/edit_address.html")},
                         {QStringLiteral("site_subtitle"), oldAddress}
                     });
        }
    }
}



void AccountEditor::remove_address(Context *c, const QString &address)
{
    if (Domain::accessGranted(c)) {

        const bool isAjax = Utils::isAjax(c);

        const QString _address = QUrl::fromPercentEncoding(address.toLatin1());
        auto a = Account::fromStash(c);
        QJsonObject json;

        if (a.addresses().size() <= 1) {

            const QString errorMsg = c->translate("AccountEditor", "You can not remove the last email address for this account. Remove the entire account instead.");

            if (isAjax) {

                json.insert(QStringLiteral("error_msg"), QJsonValue(errorMsg));
                c->res()->setJsonBody(QJsonDocument(json));

            } else {

                c->res()->redirect(c->uriForAction(QStringLiteral("/account/addresses"),
                                                   QStringList({QString::number(a.domainId()), QString::number(a.id())}),
                                                   QStringList(),
                                                   StatusMessage::errorQuery(c, errorMsg)));
            }

            c->res()->setStatus(Response::BadRequest);

            return;
        }

        if (!a.addresses().contains(_address)) {

            const QString notFoundText = c->translate("AccountEditor", "The email address to remove does not belong to the account %1.").arg(a.username());

            if (isAjax) {

                json.insert(QStringLiteral("error_msg"), QJsonValue(notFoundText));
                c->res()->setJsonBody(QJsonDocument(json));

            } else {
                c->stash({
                             {QStringLiteral("template"), QStringLiteral("404.html")},
                             {QStringLiteral("not_found_text"), notFoundText}
                         });
            }

            c->res()->setStatus(Response::NotFound);
            return;
        }

        const Domain dom = Domain::fromStash(c);

        if (Account::addressParts(_address).second == dom.name()) {
            int domainAddressCount = 0;
            for (const QString &addr : a.addresses()) {
                if (Account::addressParts(addr).second == dom.name()) {
                    domainAddressCount++;
                }
            }

            if (domainAddressCount < 2) {

                const QString errorMsg = c->translate("AccountEditor", "You can not remove the last email address that matches the domain this account belongs to.");

                if (isAjax) {

                    json.insert(QStringLiteral("error_msg"), QJsonValue(errorMsg));
                    c->res()->setJsonBody(QJsonDocument(json));

                } else {

                    c->res()->redirect(c->uriForAction(QStringLiteral("/account/addresses"),
                                                       QStringList({QString::number(a.domainId()), QString::number(a.id())}),
                                                       QStringList(),
                                                       StatusMessage::errorQuery(c, errorMsg)));
                }

                c->res()->setStatus(Response::BadRequest);

                return;
            }
        }

        if (c->req()->isPost()) {

            if (c->req()->bodyParam(QStringLiteral("email")) != _address) {

                const QString errorMsg = c->translate("AccountEditor", "The entered email address does not match the address you want to remove.");

                if (isAjax) {
                    json.insert(QStringLiteral("error_msg"), errorMsg);
                } else {
                    c->setStash(QStringLiteral("error_msg"), errorMsg);
                }

                c->res()->setStatus(Response::BadRequest);

            } else {
                SkaffariError e(c);
                if (a.removeEmail(c, &e, _address)) {

                    const QString statusMsg = c->translate("AccountEditor", "Successfully removed email address %1 from account %2.").arg(_address, a.username());

                    if (isAjax) {

                        json.insert(QStringLiteral("status_msg"), QJsonValue(statusMsg));
                        json.insert(QStringLiteral("address_count"), a.addresses().size());
                        json.insert(QStringLiteral("address"), _address);

                    } else {
                        c->res()->redirect(c->uriForAction(QStringLiteral("/account/addresses"),
                                                           QStringList({QString::number(a.domainId()), QString::number(a.id())}),
                                                           QStringList(),
                                                           StatusMessage::statusQuery(c, statusMsg)));
                        return;
                    }

                } else {
                    c->res()->setStatus(Response::InternalServerError);
                    if (isAjax) {
                        json.insert(QStringLiteral("error_msg"), e.errorText());
                    } else {
                        c->setStash(QStringLiteral("error_msg"), e.errorText());
                    }
                }
            }
        }

        if (isAjax) {
            if (!c->req()->isPost()) {
                json.insert(QStringLiteral("error_msg"), QJsonValue(c->translate("AccountEditor", "For AJAX requests, this route is only available for POST requests.")));
                c->response()->setStatus(Response::MethodNotAllowed);
                c->response()->setHeader(QStringLiteral("Allow"), QStringLiteral("POST"));
            }

            const QJsonDocument jsonDoc(json);
            c->response()->setJsonBody(jsonDoc);

            return;
        }

        c->stash({
                     {QStringLiteral("template"), QStringLiteral("account/remove_address.html")},
                     {QStringLiteral("address"), address}
                 });
    }
}


void AccountEditor::add_address(Context *c)
{
    if (Domain::accessGranted(c)) {
        auto d = Domain::fromStash(c);
        auto a = Account::fromStash(c);

        const bool isAjax = Utils::isAjax(c);
        QJsonObject json;

        if (c->req()->isPost()) {

            static Validator v({
                                   new ValidatorRequired(QStringLiteral("newlocalpart")),
                                   new ValidatorRequired(QStringLiteral("newmaildomain")),
                                   new ValidatorMin(QStringLiteral("newmaildomain"), QMetaType::UInt, 1),
                                   new SkValidatorDomainExists(QStringLiteral("newmaildomain"))
                               });

            const ValidatorResult vr = isAjax ? v.validate(c) : v.validate(c, Validator::FillStashOnError|Validator::BodyParamsOnly);

            if (vr) {

                SkaffariError e(c);
                const QString newAddress = a.addEmail(c, &e, vr.values());
                if (e.type() == SkaffariError::NoError) {

                    const QString statusMsg = c->translate("AccountEditor", "Successfully added email address %1 to account %2.").arg(newAddress, a.username());

                    if (isAjax) {

                        c->response()->setStatus(Response::Created);

                        json.insert(QStringLiteral("status_msg"), QJsonValue(statusMsg));
                        json.insert(QStringLiteral("address"), QJsonValue(newAddress));
                        json.insert(QStringLiteral("account_id"), QJsonValue(static_cast<qint64>(a.id())));
                        json.insert(QStringLiteral("domain_id"), QJsonValue(static_cast<qint64>(d.id())));
                        json.insert(QStringLiteral("address_count"), QJsonValue(a.addresses().size()));

                    } else {
                        c->res()->redirect(c->uriForAction(QStringLiteral("/account/addresses"),
                                                           QStringList({QString::number(d.id()), QString::number(a.id())}),
                                                           QStringList(),
                                                           StatusMessage::statusQuery(c, statusMsg)));
                        return;
                    }

                } else {

                    if (isAjax) {
                        json.insert(QStringLiteral("error_msg"), e.errorText());
                    } else {
                        c->setStash(QStringLiteral("error_msg"), e.errorText());
                    }

                    c->res()->setStatus(Response::InternalServerError);
                }

            } else {

                c->res()->setStatus(Response::BadRequest);

                if (isAjax) {
                    json.insert(QStringLiteral("field_errors"), QJsonValue(vr.errorsJsonObject()));
                }
            }
        }

        if (isAjax) {
            if (!c->req()->isPost()) {
                json.insert(QStringLiteral("error_msg"), QJsonValue(c->translate("AccountEditor", "For AJAX requests, this route is only available for POST requests.")));
                c->response()->setStatus(Response::MethodNotAllowed);
                c->response()->setHeader(QStringLiteral("Allow"), QStringLiteral("POST"));
            }

            c->res()->setJsonObjectBody(json);

            return;
        }

        if (d.isFreeAddressEnabled()) {
            const AuthenticationUser user = Authentication::user(c);
            SkaffariError sde(c);
            std::vector<SimpleDomain> maildomains = SimpleDomain::list(c, &sde, user);
            c->setStash(QStringLiteral("maildomains"), QVariant::fromValue<std::vector<SimpleDomain>>(maildomains));
        } else if (!d.children().empty()) {
            QVector<SimpleDomain> maildomains = d.children();
            maildomains.prepend(d.toSimple());
            c->setStash(QStringLiteral("maildomains"), QVariant::fromValue<QVector<SimpleDomain>>(maildomains));
        }

        c->setStash(QStringLiteral("template"), QStringLiteral("account/add_address.html"));
    }
}



void AccountEditor::forwards(Context *c)
{
    if (Domain::accessGranted(c)) {

        c->setStash(QStringLiteral("template"), QStringLiteral("account/forwards.html"));
    }
}


void AccountEditor::remove_forward(Context *c, const QString &forward)
{
    if (Domain::accessGranted(c)) {
        auto a = Account::fromStash(c);

        const bool isAjax = Utils::isAjax(c);

        QJsonObject json;

        if (!a.forwards().contains(forward)) {
            const QString notFoundText = c->translate("AccountEditor", "The forward email address to remove does not belong to the account %1.").arg(a.username());

            if (isAjax) {
                json.insert(QStringLiteral("error_msg"), QJsonValue(notFoundText));
                c->res()->setJsonObjectBody(json);
            } else {
                c->stash({
                             {QStringLiteral("template"), QStringLiteral("404.html")},
                             {QStringLiteral("not_found_text"), notFoundText}
                         });
            }

            c->res()->setStatus(Response::NotFound);
            return;
        }

        if (c->req()->isPost()) {

            if (c->req()->bodyParam(QStringLiteral("email")) != forward) {
                const QString errorMsg = c->translate("AccountEditor", "The entered email address does not match the forward address you want to remove.");

                if (isAjax) {
                    json.insert(QStringLiteral("error_msg"), QJsonValue(errorMsg));
                } else {
                    c->setStash(QStringLiteral("error_msg"), errorMsg);
                }

                c->res()->setStatus(Response::BadRequest);

            } else {
                SkaffariError e(c);
                if (a.removeForward(c, &e, forward)) {

                    const QString statusMsg = c->translate("AccountEditor", "Successfully removed forward email address %1 from account %2.").arg(forward, a.username());

                    if (isAjax) {
                        json.insert(QStringLiteral("status_msg"), QJsonValue(statusMsg));
                        json.insert(QStringLiteral("forward"), QJsonValue(forward));
                        json.insert(QStringLiteral("account_id"), QJsonValue(static_cast<qint64>(a.id())));
                        json.insert(QStringLiteral("domain_id"), QJsonValue(static_cast<qint64>(a.domainId())));
                        json.insert(QStringLiteral("forward_count"), QJsonValue(a.forwards().size()));
                        json.insert(QStringLiteral("keep_local"), QJsonValue(a.keepLocal()));
                    } else {
                        c->res()->redirect(c->uriForAction(QStringLiteral("/account/forwards"),
                                                           QStringList({QString::number(a.domainId()), QString::number(a.id())}),
                                                           QStringList(),
                                                           StatusMessage::statusQuery(c, statusMsg)));
                        return;
                    }
                } else {
                    c->res()->setStatus(Response::InternalServerError);
                    if (isAjax) {
                        json.insert(QStringLiteral("error_msg"), QJsonValue(e.errorText()));
                    } else {
                        c->setStash(QStringLiteral("error_msg"), e.errorText());
                    }
                }
            }
        }

        if (isAjax) {
            if (!c->req()->isPost()) {
                json.insert(QStringLiteral("error_msg"), QJsonValue(c->translate("AccountEditor", "For AJAX requests, this route is only available for POST requests.")));
                c->res()->setStatus(Response::MethodNotAllowed);
                c->res()->setHeader(QStringLiteral("Allow"), QStringLiteral("POST"));
            }

            c->res()->setJsonObjectBody(json);
            return;
        }

        c->stash({
                     {QStringLiteral("template"), QStringLiteral("account/remove_forward.html")},
                     {QStringLiteral("forward"), forward}
                 });
    }
}


void AccountEditor::add_forward(Context *c)
{
    if (Domain::accessGranted(c)) {
        auto a = Account::fromStash(c);
        const bool isAjax = Utils::isAjax(c);
        QJsonObject json;

        if (c->req()->isPost()) {

            static Validator v({
                                   new ValidatorRequired(QStringLiteral("newforward")),
                                   new ValidatorEmail(QStringLiteral("newforward"))
                               });

            const ValidatorResult vr = isAjax ? v.validate(c) : v.validate(c, Validator::FillStashOnError|Validator::BodyParamsOnly);

            if (vr) {

                SkaffariError e(c);
                const QString newForward = vr.value(QStringLiteral("newforward")).toString();
                if (a.addForward(c, &e, newForward)) {

                    const QString statusMsg = c->translate("AccountEditor", "Successfully added forward email address %1 to account %2.").arg(newForward, a.username());

                    if (isAjax) {
                        c->res()->setStatus(Response::Created);

                        json.insert(QStringLiteral("status_msg"), QJsonValue(statusMsg));
                        json.insert(QStringLiteral("forward"), QJsonValue(newForward));
                        json.insert(QStringLiteral("account_id"), QJsonValue(static_cast<qint64>(a.id())));
                        json.insert(QStringLiteral("domain_id"), QJsonValue(static_cast<qint64>(a.domainId())));
                        json.insert(QStringLiteral("forward_count"), QJsonValue(a.forwards().size()));
                        json.insert(QStringLiteral("keep_local"), QJsonValue(a.keepLocal()));
                    } else {
                        c->res()->redirect(c->uriForAction(QStringLiteral("/account/forwards"),
                                                           QStringList({QString::number(a.domainId()), QString::number(a.id())}),
                                                           QStringList(),
                                                           StatusMessage::statusQuery(c, statusMsg)));
                        return;
                    }

                } else {
                    if (isAjax) {
                        json.insert(QStringLiteral("error_msg"), e.errorText());
                    } else {
                        c->setStash(QStringLiteral("error_msg"), e.errorText());
                    }

                    c->res()->setStatus(Response::InternalServerError);
                }

            } else {
                c->res()->setStatus(Response::BadRequest);

                if (isAjax) {
                    const auto errors = vr.errors();
                    if (!errors.empty()) {
                        QJsonObject fieldErrors;
                        QHash<QString,QStringList>::const_iterator i = errors.constBegin();
                        while (i != errors.constEnd()) {
                            fieldErrors.insert(i.key(), QJsonValue(QJsonArray::fromStringList(i.value())));
                            ++i;
                        }
                        json.insert(QStringLiteral("field_errors"), QJsonValue(fieldErrors));
                    }
                }
            }
        }

        if (isAjax) {
            if (!c->req()->isPost()) {
                json.insert(QStringLiteral("error_msg"), QJsonValue(c->translate("AccountEditor", "For AJAX requests, this route is only available for POST requests.")));
                c->response()->setStatus(Response::MethodNotAllowed);
                c->response()->setHeader(QStringLiteral("Allow"), QStringLiteral("POST"));
            }

            c->res()->setJsonObjectBody(json);
            return;
        }

        QHash<QString,HelpEntry> help;
        help.insert(QStringLiteral("newforward"), HelpEntry(c->translate("AccountEditor", "New forward"), c->translate("AcountEditor", "Enter a valid email address to which you want to forward emails received for account %1.").arg(a.username())));

        c->stash({
                     {QStringLiteral("template"), QStringLiteral("account/add_forward.html")},
                     {QStringLiteral("help"), QVariant::fromValue<QHash<QString,HelpEntry>>(help)}
                 });
    }
}


void AccountEditor::edit_forward(Context *c, const QString &oldForward)
{
    if (Domain::accessGranted(c)) {
        auto a = Account::fromStash(c);

        const bool isAjax = Utils::isAjax(c);
        QJsonObject json;

        if (c->req()->isPost()) {

            static Validator v({
                                   new ValidatorRequired(QStringLiteral("newforward")),
                                   new ValidatorEmail(QStringLiteral("newforward"))
                               });

            const ValidatorResult vr = isAjax ? v.validate(c) : v.validate(c, Validator::FillStashOnError|Validator::BodyParamsOnly);

            if (vr) {

                SkaffariError e(c);
                const QString newForward = c->req()->bodyParam(QStringLiteral("newforward"));
                if (a.editForward(c, &e, oldForward, newForward)) {

                    const QString statusMsg = c->translate("AccountEditor", "Successfully changed forward %1 into %2 for account %3.").arg(oldForward, newForward, a.username());
                    auto d = Domain::fromStash(c);

                    if (isAjax) {
                        json.insert(QStringLiteral("status_msg"), QJsonValue(statusMsg));
                        json.insert(QStringLiteral("old_forward"), QJsonValue(oldForward));
                        json.insert(QStringLiteral("new_forward"), QJsonValue(newForward));
                        json.insert(QStringLiteral("account_id"), QJsonValue(static_cast<qint64>(a.id())));
                        json.insert(QStringLiteral("domain_id"), QJsonValue(static_cast<qint64>(d.id())));
                    } else {
                        c->res()->redirect(c->uriForAction(QStringLiteral("/account/forwards"),
                                                           QStringList({QString::number(d.id()), QString::number(a.id())}),
                                                           QStringList(),
                                                           StatusMessage::statusQuery(c, statusMsg)));
                        return;
                    }

                } else {

                    if (isAjax) {
                        json.insert(QStringLiteral("error_msg"), e.errorText());
                    } else {
                        c->setStash(QStringLiteral("error_msg"), e.errorText());
                    }

                    if (e.type() == SkaffariError::InputError) {
                        c->res()->setStatus(Response::BadRequest);
                    } else {
                        c->res()->setStatus(Response::InternalServerError);
                    }
                }
            } else {
                c->res()->setStatus(Response::BadRequest);

                if (isAjax) {
                    const auto errors = vr.errors();
                    if (!errors.empty()) {
                        QJsonObject fieldErrors;
                        QHash<QString,QStringList>::const_iterator i = errors.constBegin();
                        while (i != errors.constEnd()) {
                            fieldErrors.insert(i.key(), QJsonValue(QJsonArray::fromStringList(i.value())));
                            ++i;
                        }
                        json.insert(QStringLiteral("field_errors"), QJsonValue(fieldErrors));
                    }
                }
            }
        }

        if (isAjax) {
            if (!c->req()->isPost()) {
                json.insert(QStringLiteral("error_msg"), QJsonValue(c->translate("AccountEditor", "For AJAX requests, this route is only available for POST requests.")));
                c->res()->setStatus(Response::MethodNotAllowed);
                c->res()->setHeader(QStringLiteral("Allow"), QStringLiteral("POST"));
            }

            c->res()->setJsonObjectBody(json);
            return;
        }

        QHash<QString,HelpEntry> help;
        help.insert(QStringLiteral("newforward"), HelpEntry(c->translate("AccountEditor", "Edit forward"), c->translate("AccountEditor", "Change the forward email address to a different valid email address to which you want to forward emails received for account %1.").arg(a.username())));

        c->stash({
                     {QStringLiteral("template"), QStringLiteral("account/edit_forward.html")},
                     {QStringLiteral("help"), QVariant::fromValue<QHash<QString,HelpEntry>>(help)},
                     {QStringLiteral("oldforward"), oldForward}
                 });

    }
}


void AccountEditor::keep_local(Context *c)
{
    if (Domain::accessGranted(c)) {

        auto a = Account::fromStash(c);

        const bool isAjax = Utils::isAjax(c);
        QJsonObject json;

        if (c->req()->isPost()) {
            const bool _keepLocal = (c->req()->bodyParameter(QStringLiteral("keeplocal"), QStringLiteral("false")) == QLatin1String("true"));

            SkaffariError e(c);
            if (a.changeKeepLocal(c, &e, _keepLocal)) {

                const QString statusMsg = _keepLocal
                        ? c->translate("AccountEditor", "Successfully enabled the keeping of forwarded emails in the local mailbox of account %1.").arg(a.username())
                        : c->translate("AccountEditor", "Successfully disabled the keeping of forwarded emails in the local mailbox of account %1.").arg(a.username());

                if (isAjax) {
                    json.insert(QStringLiteral("status_msg"), QJsonValue(statusMsg));
                    json.insert(QStringLiteral("account_id"), QJsonValue(static_cast<qint64>(a.id())));
                    json.insert(QStringLiteral("domain_id"), QJsonValue(static_cast<qint64>(a.domainId())));
                    json.insert(QStringLiteral("keep_local"), QJsonValue(a.keepLocal()));
                } else {
                    c->res()->redirect(c->uriForAction(QStringLiteral("/account/forwards"),
                                                       QStringList({QString::number(a.domainId()), QString::number(a.id())}),
                                                       QStringList(),
                                                       StatusMessage::statusQuery(c, statusMsg)));
                    return;
                }

            } else {
                if (isAjax) {
                    json.insert(QStringLiteral("error_msg"), e.errorText());
                } else {
                    c->setStash(QStringLiteral("error_msg"), e.errorText());
                }
                c->res()->setStatus(Response::BadRequest);
            }
        }

        if (isAjax) {
            if (!c->req()->isPost()) {
                json.insert(QStringLiteral("error_msg"), QJsonValue(c->translate("AccountEditor", "For AJAX requests, this route is only available for POST requests.")));
                c->res()->setStatus(Response::MethodNotAllowed);
                c->res()->setHeader(QStringLiteral("Allow"), QStringLiteral("POST"));
            }

            c->res()->setJsonObjectBody(json);
            return;
        }

        c->stash({
                     {QStringLiteral("template"), QStringLiteral("account/keep_local.html")}
                 });
    }
}


void AccountEditor::check(Context *c)
{
    if (Domain::accessGranted(c)) {

        const bool isAjax = Utils::isAjax(c);

        static Validator v({
                               new ValidatorBoolean(QStringLiteral("checkChildAddresses"))
                           });

        const ValidatorResult vr = isAjax ? v.validate(c) : v.validate(c, Validator::FillStashOnError|Validator::BodyParamsOnly);

        auto a = Account::fromStash(c);

        if (vr) {
            auto d = Domain::fromStash(c);

            SkaffariError e(c);
            const QStringList actions = a.check(c, &e, d, c->req()->bodyParameters());

            Account::toStash(c, a);

            if (isAjax) {

                QJsonObject result;
                result.insert(QStringLiteral("username"), QJsonValue(a.username()));

                if (e.type() != SkaffariError::NoError) {
                    result.insert(QStringLiteral("error_msg"), QJsonValue(e.errorText()));
                    c->response()->setStatus(Response::InternalServerError);
                } else {
                    if (!actions.empty()) {
                        result.insert(QStringLiteral("actions"), QJsonValue(QJsonArray::fromStringList(actions)));
                        result.insert(QStringLiteral("account"), a.toJson());
                    } else {
                        result.insert(QStringLiteral("status_msg"), QJsonValue(c->translate("AccountEditor", "Nothing to do. Everything seems to be ok with this account.")));
                    }
                }

                QJsonDocument json(result);

                c->response()->setJsonBody(json);

            } else {
                if (e.type() != SkaffariError::NoError) {
                    c->setStash(QStringLiteral("error_msg"), e.errorText());
                    c->response()->setStatus(Response::InternalServerError);
                }

                c->setStash(QStringLiteral("actions"), actions);
                c->setStash(QStringLiteral("template"), QStringLiteral("account/check.html"));
            }
        } else {
            if (isAjax) {
                QJsonObject result;
                result.insert(QStringLiteral("username"), a.username());
                result.insert(QStringLiteral("error_msg"), vr.errorStrings().at(0));

                QJsonDocument json(result);

                c->response()->setJsonBody(json);
            } else {
                c->setStash(QStringLiteral("template"), QStringLiteral("account/check.html"));
            }
            c->response()->setStatus(Response::BadRequest);
        }
    }
}


void AccountEditor::list(Context *c)
{
    AuthenticationUser user = Authentication::user(c);

    const dbid_t domainId = SKAFFARI_STRING_TO_DBID(c->req()->queryParam(QStringLiteral("domainId"), QStringLiteral("0")));
    const QString searchString = c->req()->queryParam(QStringLiteral("searchString"));
    const dbid_t adminId = user.id().value<dbid_t>();
    const qint16 userType = user.value(QStringLiteral("type")).value<qint16>();

    SkaffariError e(c);
    const QJsonArray accounts = SimpleAccount::listJson(c, &e, userType, adminId, domainId, searchString);
    QJsonObject o;
    o.insert(QStringLiteral("accounts"), accounts);
    if (e.type() != SkaffariError::NoError) {
        o.insert(QStringLiteral("error_msg"), e.errorText());
        c->res()->setStatus(Response::InternalServerError);
    }
    QJsonDocument json(o);
    c->res()->setJsonBody(json);
}

#include "moc_accounteditor.cpp"
