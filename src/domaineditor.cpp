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

#include "domaineditor.h"
#include "objects/domain.h"
#include "objects/account.h"
#include "objects/skaffarierror.h"
#include "utils/skaffariconfig.h"

#include <Cutelyst/Plugins/Utils/Validator> // includes the main validator
#include <Cutelyst/Plugins/Utils/Validators> // includes all validator rules
#include <Cutelyst/Plugins/Utils/ValidatorResult> // includes the validator result
#include <Cutelyst/Plugins/StatusMessage>
#include <Cutelyst/Plugins/Authentication/authentication.h>
#include <Cutelyst/Plugins/Utils/Sql>
#include <Cutelyst/Plugins/Session/Session>
#include <Cutelyst/Plugins/Utils/Pagination>
#include <QSqlQuery>
#include <QSqlError>
#include <QRegularExpression>

using namespace Cutelyst;

DomainEditor::DomainEditor(QObject *parent) : Controller(parent)
{
}

DomainEditor::~DomainEditor()
{
}


void DomainEditor::index(Context *c)
{
    SkaffariError e(c);
    auto doms = Domain::list(c, &e, Authentication::user(c));

    c->stash({
                 {QStringLiteral("domains"), QVariant::fromValue<std::vector<Domain>>(doms)},
                 {QStringLiteral("template"), QStringLiteral("domain/index.html")},
                 {QStringLiteral("site_title"), c->translate("DomainEditor", "Domains")}
             });
}


void DomainEditor::base(Context* c, const QString &id)
{
    const quint32 domainId = id.toULong();
    if (Domain::checkAccess(c, domainId)) {
        Domain::toStash(c, domainId);
    }
}


void DomainEditor::edit(Context *c)
{

    if (Domain::accessGranted(c)) {

        Domain dom = Domain::fromStash(c);

        auto req = c->req();
        if (req->isPost()) {

            AuthenticationUser user = Authentication::user(c);

            ValidatorResult vr;
            if (user.value(QStringLiteral("type")).value<qint16>() == 0) {

                static Validator v({
                                new ValidatorIn(QStringLiteral("transport"), QStringList({QStringLiteral("cyrus"), QStringLiteral("lmtp"), QStringLiteral("smtp"), QStringLiteral("uucp")})),
                                new ValidatorInteger(QStringLiteral("maxAccounts")),
                                new ValidatorMin(QStringLiteral("maxAccounts"), QMetaType::UInt, 0),
                                new ValidatorInteger(QStringLiteral("quota")),
                                new ValidatorMin(QStringLiteral("quota"), QMetaType::UInt, 0),
                                new ValidatorInteger(QStringLiteral("domainQuota")),
                                new ValidatorMin(QStringLiteral("domainQuota"), QMetaType::UInt, 0),
                                new ValidatorBoolean(QStringLiteral("freeNames")),
                                new ValidatorBoolean(QStringLiteral("freeAddress"))
                            });

                vr = v.validate(c, Validator::FillStashOnError);

            } else {

                static Validator v({
                                new ValidatorMin(QStringLiteral("quota"), QMetaType::UInt, 0)
                            });

                vr = v.validate(c, Validator::FillStashOnError);
            }

            if (vr) {
                SkaffariError e(c);
                if (Domain::update(c, req->parameters(), &e, &dom, user)) {
                    c->stash({
                                 {QStringLiteral("domain"), QVariant::fromValue<Domain>(dom)},
                                 {QStringLiteral("status_msg"), c->translate("DomainEditor", "Successfully updated domain %1.").arg(dom.getName())}
                             });
                } else {
                    c->setStash(QStringLiteral("error_msg"), e.errorText());
                }
            }
        }



        c->stash({
                     {QStringLiteral("template"), QStringLiteral("domain/edit.html")},
                     {QStringLiteral("edit"), true},
                     {QStringLiteral("site_subtitle"), c->translate("DomainEditor", "Edit")}
                 });
    }
}




void DomainEditor::accounts(Context* c)
{
    if (Domain::accessGranted(c)) {

        auto dom = Domain::fromStash(c);

        ParamsMultiMap p = c->req()->parameters();

        Pagination pag(dom.getAccounts(),
                       p.value(QStringLiteral("accountsPerPage"), Session::value(c, QStringLiteral("maxdisplay"), 25).toString()).toInt(),
                       p.value(QStringLiteral("currentPage"), QStringLiteral("1")).toInt());

        QString sortBy = p.value(QStringLiteral("sortBy"), QStringLiteral("username"));
        static QStringList sortByCols({QStringLiteral("username"), QStringLiteral("created_at"), QStringLiteral("updated_at"), QStringLiteral("valid_until")});
        if (!sortByCols.contains(sortBy)) {
            sortBy = QStringLiteral("username");
        }

        QString sortOrder = p.value(QStringLiteral("sortOrder"), QStringLiteral("ASC"));
        if ((sortOrder != QLatin1String("ASC")) && (sortOrder != QLatin1String("DESC"))) {
            sortOrder = QStringLiteral("ASC");
        }

        SkaffariError e(c);
        pag = Account::list(c, &e, dom, pag, sortBy, sortOrder);

        c->stash({
                     {QStringLiteral("pagination"), pag},
                     {QStringLiteral("template"), QStringLiteral("domain/accounts.html")},
                     {QStringLiteral("site_subtitle"), c->translate("DomainEditor", "Accounts")}
                });
    }
}



void DomainEditor::create(Context* c)
{
    if (Domain::checkAccess(c)) {

        auto r = c->req();
        if (r->isPost()) {

            static Validator v({
                                   new ValidatorRequiredIf(QStringLiteral("prefix"), QStringLiteral("domainAsPrefix"), QStringList(QStringLiteral("false"))),
                                   new ValidatorAlphaDash(QStringLiteral("prefix")),
                                   new ValidatorRequired(QStringLiteral("domainName")),
                                   new ValidatorIn(QStringLiteral("transport"), QStringList({QStringLiteral("cyrus"), QStringLiteral("lmtp"), QStringLiteral("smtp"), QStringLiteral("uucp")})),
                                   new ValidatorInteger(QStringLiteral("maxAccounts")),
                                   new ValidatorMin(QStringLiteral("maxAccounts"), QMetaType::UInt, 0),
                                   new ValidatorInteger(QStringLiteral("quota")),
                                   new ValidatorMin(QStringLiteral("quota"), QMetaType::UInt, 0),
                                   new ValidatorInteger(QStringLiteral("domainQuota")),
                                   new ValidatorMin(QStringLiteral("domainQuota"), QMetaType::UInt, 0),
                                   new ValidatorBoolean(QStringLiteral("freeNames")),
                                   new ValidatorBoolean(QStringLiteral("freeAddress"))
                               });

            const ValidatorResult vr = v.validate(c, Validator::FillStashOnError);
            if (vr) {
                SkaffariError e(c);
                auto dom = Domain::create(c, r->params(), &e);
                if (dom.isValid()) {
                    c->res()->redirect(c->uriForAction(QStringLiteral("/domain/edit"), QStringList(QString::number(dom.id())), QStringList(), StatusMessage::statusQuery(c, c->translate("DomainEditor", "Successfully created new domain %1").arg(dom.getName()))));
                } else {
                    c->setStash(QStringLiteral("error_msg"), e.errorText());
                }
            }
        }

        c->stash({
                     {QStringLiteral("defQuota"), SkaffariConfig::defQuota()},
                     {QStringLiteral("defDomainQuota"), SkaffariConfig::defDomainquota()},
                     {QStringLiteral("defMaxAccounts"), SkaffariConfig::defMaxaccounts()},
                     {QStringLiteral("template"), QStringLiteral("domain/create.html")},
                     {QStringLiteral("domainAsPrefix"), SkaffariConfig::imapDomainasprefix()},
                     {QStringLiteral("edit"), false},
                     {QStringLiteral("site_title"), c->translate("DomainEditor", "Create domain")}
                });
    }
}



void DomainEditor::remove(Context* c)
{
    if (Domain::accessGranted(c)) {

        if (c->stash(QStringLiteral("userType")).value<quint16>() == 0) {

        auto req = c->req();

        if (req->isPost()) {
            Domain dom = c->stash(QStringLiteral("domain")).value<Domain>();

            if (dom.getName() == req->param(QStringLiteral("domainName"))) {

                SkaffariError e(c);
                if (Domain::remove(c, &dom, &e)) {
                    c->res()->redirect(c->uriFor(QStringLiteral("/domain"), StatusMessage::statusQuery(c, c->translate("DomainEditor", "Successfully deleted domain %1.").arg(dom.getName()))));
                } else {
                    c->setStash(QStringLiteral("error_msg"), e.errorText());
                }

            } else {
                c->setStash(QStringLiteral("error_msg"), c->translate("DomainEditor", "The entered name does not match the domain name."));
            }
        }

        c->stash({
                     {QStringLiteral("template"), QStringLiteral("domain/remove.html")},
                     {QStringLiteral("site_subtitle"), c->translate("DomainEditor", "Remove")}
                 });
        } else {
            c->stash({
                         {QStringLiteral("template"), QStringLiteral("403.html")},
                         {QStringLiteral("site_title"), c->translate("Domain", "Access denied")}
                     });
            c->res()->setStatus(403);
        }
    }
}



void DomainEditor::add_account(Context* c)
{
    AuthenticationUser user = Authentication::user(c);
    Domain dom = c->stash(QStringLiteral("domain")).value<Domain>();

    if (Q_LIKELY((user.value(QStringLiteral("type")).value<qint16>() == 0) || user.value(QStringLiteral("domains")).value<QVariantList>().contains(dom.id()))) {

        const quint32 freeQuota = (dom.getDomainQuota() - dom.getDomainQuotaUsed());

        auto req = c->req();

        if (req->isPost()) {

            const ParamsMultiMap p = req->bodyParameters();

            qCDebug(SK_ACCOUNT) << "Start creating a new account";

            c->stash({
                         {QStringLiteral("imap"), p.contains(QStringLiteral("imap"))},
                         {QStringLiteral("pop"), p.contains(QStringLiteral("pop"))},
                         {QStringLiteral("sieve"), p.contains(QStringLiteral("sieve"))},
                         {QStringLiteral("smtpauth"), p.contains(QStringLiteral("smtpauth"))},
                         {QStringLiteral("username"), p.value(QStringLiteral("username"))},
                         {QStringLiteral("localpart"), p.value(QStringLiteral("localpart"))},
                         {QStringLiteral("quota"), p.value(QStringLiteral("quota"))}
                     });

            static Validator v({
                                   new ValidatorRequired(QStringLiteral("username")),
                                   new ValidatorAlphaDash(QStringLiteral("username")),
                                   new ValidatorRequired(QStringLiteral("localpart")),
                                   new ValidatorRegularExpression(QStringLiteral("localpart"), QRegularExpression(QStringLiteral("[^@]"))),
                                   new ValidatorRequired(QStringLiteral("password")),
                                   new ValidatorMin(QStringLiteral("password"), QMetaType::QString, SkaffariConfig::accPwMinlength()),
                                   new ValidatorConfirmed(QStringLiteral("password")),
                                   new ValidatorDateTime(QStringLiteral("validUntil"), QStringLiteral("yyyy-MM-dd HH:mm:ss"))
                               });

            const ValidatorResult vr = v.validate(c, Validator::FillStashOnError);
            if (vr) {

                // if this domain has a global quota limit, we have to calculate the
                // quota that is left
                bool enoughQuotaLeft = true;
                if (dom.getDomainQuota() > 0) {

                    if (freeQuota < p.value(QStringLiteral("quota")).toULong()) {

                        enoughQuotaLeft = false;

                        c->setStash(QStringLiteral("error_msg"),
                                    c->translate("DomainEditor", "There is not enough free quota on this domain. Please lower the quota for the new account to a maximum of %1 KiB.").arg(freeQuota));
                    }

                }

                if (enoughQuotaLeft) {

                    SkaffariError e(c);
                    Account account = Account::create(c,
                                                      &e,
                                                      req->parameters(),
                                                      dom);
                    if (account.isValid()) {

                        Session::deleteValue(c, QStringLiteral("domainQuotaUsed_") + QString::number(dom.id()));
                        c->res()->redirect(c->uriForAction(QStringLiteral("/domain/accounts"), QStringList(QString::number(dom.id())), QStringList(), StatusMessage::statusQuery(c, c->translate("DomainEditor", "Successfully created account %1 with email address %2.").arg(account.getUsername(), account.getAddresses().at(0)))));
                        return;

                    } else {
                        qCDebug(SK_ACCOUNT) << e.errorText();
                        c->setStash(QStringLiteral("error_msg"), e.errorText());
                    }
                }

            } else {
                qCDebug(SK_ACCOUNT) << "Failed to create account. Invalid input data:" << vr.errorStrings();
            }

        } else {

            // set default values for checkboxes when loading the page
            c->stash({
                         {QStringLiteral("imap"), true},
                         {QStringLiteral("pop"), true},
                         {QStringLiteral("sieve"), true},
                         {QStringLiteral("smtpauth"), true}
                     });

             QString username = dom.getPrefix();

            // if domainasprefix option is not set, user names can be either set free
            // (if they are not used by someone else and the domain has free names enabled),
            // or they are automatically generated from the domain prefix and an autoincrementing
            // number (this will also be generated as proposal for free usable names)
            if (!SkaffariConfig::imapDomainasprefix()) {

                if (dom.isFreeNamesEnabled()) {
                    // if the domain allows free names, we only generate a proposal starting with 0000

                    const QString cntStr = QString::number(dom.getAccounts());
                    const quint8 digits = cntStr.size();
                    if (Q_LIKELY(digits < 4)) {
                        username.append(QString(4 - digits, QLatin1Char('0')));
                    }
                    username.append(cntStr);

                } else {
                    // if the domain does not allow free names, we generate a username
                    // based on the domain prefix and an automatically incremented number

                    if (Q_LIKELY(dom.getAccounts() > 0)) {

                        // lets get the last added account
                        QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT username FROM accountuser WHERE domain_id = :domain_id ORDER BY id DESC LIMIT 1"));
                        q.bindValue(QStringLiteral(":domain_id"), dom.id());
                        if (!q.exec()) {
                            SkaffariError e(c, q.lastError(), c->translate("DomainEditor", "Failed to query last added account from database."));
                            c->setStash(QStringLiteral("error_msg"), e.errorText());
                            c->res()->setStatus(500);
                            return;
                        }

                        if (Q_LIKELY(q.next())) {

                            QRegularExpression re(QStringLiteral("\\d+$"));
                            QRegularExpressionMatch match = re.match(q.value(0).toString());
                            if (Q_LIKELY(match.hasMatch())) {

                                const QString cntStr = match.captured(0);
                                const quint32 currentCnt = cntStr.toULong();
                                const quint32 nextCnt = currentCnt + 1;
                                const quint8 digits = QString::number(nextCnt).size();
                                if (Q_LIKELY(digits < 4)) {
                                    username.append(QString(4 - digits, QLatin1Char('0')));
                                }
                                username.append(QString::number(nextCnt));

                            } else {
                                // fallback if there is no match
                                username.append(QLatin1String("0000"));
                            }

                        } else {
                            // fallback if there is no record
                            username.append(QLatin1String("0000"));
                        }

                    } else {
                        // this is the first account
                        username.append(QLatin1String("0000"));
                    }
                }
            }
            c->setStash(QStringLiteral("username"), username);
        }

        c->stash({
                     {QStringLiteral("template"), QStringLiteral("domain/add_account.html")},
                     {QStringLiteral("site_subtitle"), c->translate("DomainEditor", "Add account")},
                     {QStringLiteral("domainasprefix"), SkaffariConfig::imapDomainasprefix()},
                     {QStringLiteral("fqun"), SkaffariConfig::imapFqun()},
                     {QStringLiteral("minpasswordlength"), SkaffariConfig::accPwMinlength()},
                     {QStringLiteral("freequota"), freeQuota}
                 });
    }
}


QStringList DomainEditor::trimFolderStrings(const QStringList& folders)
{
    QStringList trimmed;

    if (folders.isEmpty()) {
        return trimmed;
    }

    for (const QString &folder : folders) {
        trimmed << folder.simplified();
    }

    return trimmed;
}


