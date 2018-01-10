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
#include "objects/helpentry.h"
#include "utils/skaffariconfig.h"
#include "utils/utils.h"
#include "validators/skvalidatoruniquedb.h"
#include "validators/skvalidatorfilesize.h"
#include "validators/skvalidatoraccountexists.h"
#include "../common/global.h"

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
#include <QHash>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QNetworkCookie>

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

    if (e.type() != SkaffariError::NoError) {
        c->setStash(QStringLiteral("error_msg"), e.errorText());
    }
}


void DomainEditor::base(Context* c, const QString &id)
{
    const dbid_t domainId = SKAFFARI_STRING_TO_DBID(id);
    if (Domain::checkAccess(c, domainId)) {
        Domain::toStash(c, domainId);
    }
}


void DomainEditor::edit(Context *c)
{

    if (Domain::accessGranted(c)) {

        Domain dom = Domain::fromStash(c);

        AuthenticationUser user = Authentication::user(c);

        auto req = c->req();
        if (req->isPost()) {

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
                                new ValidatorBoolean(QStringLiteral("freeAddress")),
                                new SkValidatorFilesize(QStringLiteral("humanQuota")),
                                new SkValidatorFilesize(QStringLiteral("humanDomainQuota")),
                                new ValidatorInteger(QStringLiteral("parent")),
                                new ValidatorMin(QStringLiteral("parent"), QMetaType::UInt, 0)
                            });

                vr = v.validate(c, Validator::FillStashOnError);

            } else {

                static Validator v({
                                new ValidatorInteger(QStringLiteral("quota")),
                                new ValidatorMin(QStringLiteral("quota"), QMetaType::UInt, 0),
                                new SkValidatorFilesize(QStringLiteral("humanQuota"))
                            });

                vr = v.validate(c, Validator::FillStashOnError);
            }

            if (vr) {
                SkaffariError e(c);
                if (Domain::update(c, req->bodyParameters(), &e, &dom, user)) {
                    c->stash({
                                 {QStringLiteral("domain"), QVariant::fromValue<Domain>(dom)},
                                 {QStringLiteral("status_msg"), c->translate("DomainEditor", "Successfully updated domain %1.").arg(dom.getName())}
                             });
                } else {
                    c->setStash(QStringLiteral("error_msg"), e.errorText());
                }
            }
        }

        QHash<QString,HelpEntry> help;
        help.insert(QStringLiteral("prefix"), HelpEntry(c->translate("DomainEditor", "Prefix"), c->translate("DomainEditor", "The prefix might be used for automatically generated user names, especially if free names are not allowed for this domain.")));
        help.insert(QStringLiteral("created"), HelpEntry(c->translate("DomainEditor", "Created"), c->translate("DomainEditor", "Date and time this domain has been created in Skaffari.")));
        help.insert(QStringLiteral("updated"), HelpEntry(c->translate("DomainEditor", "Updated"), c->translate("DomainEditor", "Date and time this domain has been updated in Skafari.")));

        const QString domainQuotaTitle = c->translate("DomainEditor", "Domain quota");
        if (c->stash(QStringLiteral("userType")).value<qint16>() == 0) {
            // current user is a super administrator
            help.insert(QStringLiteral("maxAccounts"), HelpEntry(c->translate("DomainEditor", "Maximum accounts"), c->translate("DomainEditor", "The maximum accounts value limits the amount of accounts that can be created for this domain. Set it to 0 to disable the limit.")));
            help.insert(QStringLiteral("domainQuota"), HelpEntry(domainQuotaTitle, c->translate("DomainEditor", "Overall quota limit for all accounts that belong to this domain. If the domain quota is set, every account must have a quota defined. Set it to 0 to disable the domain quota.")));
            help.insert(QStringLiteral("humanDomainQuota"), HelpEntry(domainQuotaTitle, c->translate("DomainEditor", "Overall quota limit for all accounts that belong to this domain. If the domain quota is set, every account must have a quota defined. Set it to 0 to disable the domain quota. You can use the multipliers M, MiB, G, GiB, T and TiB. Without a multiplier, KiB is the default.")));
        } else {
            // current user is a domain administrator
            const QString domainQuotaText = c->translate("DomainEditor", "Overall quota limit for all accounts that belong to this domain. If the domain quota is set (not unlimited), every account must have a quota defined.");
            help.insert(QStringLiteral("maxAccounts"), HelpEntry(c->translate("DomainEditor", "Accounts"), c->translate("DomainEditor", "Shows the current amount of accounts created in this domain and the maximum number of accounts that can be created for this domain.")));
            help.insert(QStringLiteral("domainQuota"), HelpEntry(domainQuotaTitle, domainQuotaText));
            help.insert(QStringLiteral("humanDomainQuota"), HelpEntry(domainQuotaTitle, domainQuotaText));
        }

        const QString quotaTitle = c->translate("DomainEditor", "Default quota");
        help.insert(QStringLiteral("quota"), HelpEntry(quotaTitle, c->translate("DomainEditor", "Default quota for new accounts for this domain. This value can be changed individually for every account.")));
        help.insert(QStringLiteral("humanQuota"), HelpEntry(quotaTitle, c->translate("DomainEditor", "Default quota for new accounts for this domain. This value can be changed individually for every account. You can use the multipliers M, MiB, G, GiB, T and TiB. Without a multiplier, KiB is the default.")));


        help.insert(QStringLiteral("folders"), HelpEntry(c->translate("DomainEditor", "Standard folders"), c->translate("DomainEditor", "Comma separated list of folders that will be automatically created when creating a new account for this domain. You can safely insert localized folder names in UTF-8 encoding. They will be internally converted into UTF-7-IMAP encoding.")));
        help.insert(QStringLiteral("parent"), HelpEntry(c->translate("DomainEditor", "Parent domain"), c->translate("DomainEditor", "Setting a parent domain for this domain will automatically create email addresses for this domain when creating new accounts in the parent domain.")));
        help.insert(QStringLiteral("children"), HelpEntry(c->translate("DomainEditor", "Child domains"), c->translate("DomainEditor", "List of domains that are children of this domain. Creating new accounts in this domain will automatically add email addresses for the child domains to the new accounts.")));
        help.insert(QStringLiteral("transport"), HelpEntry(c->translate("DomainEditor", "Transport"), c->translate("DomainEditor", "The transport mechanism for received emails for this domain. Defaults to Cyrus.")));
        help.insert(QStringLiteral("freeNames"), HelpEntry(c->translate("DomainEditor", "Allow free names"), c->translate("DomainEditor", "If enabled, account user names for this domain can be freely selected (if not in use already).")));
        help.insert(QStringLiteral("freeAddress"), HelpEntry(c->translate("DomainEditor", "Allow free addresses"), c->translate("DomainEditor", "If enabled, user accounts in this domain can have email addresses for all domains managed by Skaffari. If disabled, only email addresses for this domain can be added to user accounts in this domain.")));

        SkaffariError e(c);
        const std::vector<SimpleDomain> doms = SimpleDomain::list(c, &e, user.value(QStringLiteral("type")).value<qint16>(), user.id().toULong(), true);
        if (e.type() != SkaffariError::NoError) {
            c->setStash(QStringLiteral("error_msg"), e.errorText());
        }

        c->stash({
                     {QStringLiteral("template"), QStringLiteral("domain/edit.html")},
                     {QStringLiteral("site_subtitle"), c->translate("DomainEditor", "Edit")},
                     {QStringLiteral("help"), QVariant::fromValue<QHash<QString,HelpEntry>>(help)},
                     {QStringLiteral("domains"), QVariant::fromValue<std::vector<SimpleDomain>>(doms)}
                 });
    }
}


#define SK_DOM_FILTER_COOKIE_ACCOUNTS_PER_PAGE 0
#define SK_DOM_FILTER_COOKIE_CURRENT_PAGE 1
#define SK_DOM_FILTER_COOKIE_SORT_BY 2
#define SK_DOM_FILTER_COOKIE_SORT_ORDER 3
#define SK_DOM_FILTER_COOKIE_SEARCH_ROLE 4
#define SK_DOM_FILTER_COOKIE_SEARCH_STRING 5
#define SK_DOM_FILTER_COOKIE_NAME "domain_filters"

void DomainEditor::accounts(Context* c)
{
    if (Domain::accessGranted(c)) {

        auto dom = Domain::fromStash(c);

        const ParamsMultiMap p = c->req()->parameters();

        const QString cookieData = c->req()->cookie(QStringLiteral(SK_DOM_FILTER_COOKIE_NAME));
        QStringList cookieDataList;

        if (!cookieData.isEmpty()) {
            cookieDataList = QString::fromLatin1(QByteArray::fromBase64(cookieData.toLatin1())).split(QLatin1Char(';'));
        }

        QString accountsPerPage, currentPage, sortBy, sortOrder, searchRole, searchString;

        if (cookieDataList.empty()) {
            accountsPerPage = p.value(QStringLiteral("accountsPerPage"), Session::value(c, QStringLiteral("maxdisplay"), 25).toString());
            currentPage = p.value(QStringLiteral("currentPage"), QStringLiteral("1"));
            sortBy = p.value(QStringLiteral("sortBy"), QStringLiteral("username"));
            sortOrder = p.value(QStringLiteral("sortOrder"), QStringLiteral("ASC"));
            searchRole = p.value(QStringLiteral("searchRole"), QStringLiteral("username"));
            searchString = p.value(QStringLiteral("searchString"));
        } else {
            accountsPerPage = p.value(QStringLiteral("accountsPerPage"), cookieDataList.at(SK_DOM_FILTER_COOKIE_ACCOUNTS_PER_PAGE));
            currentPage = p.value(QStringLiteral("currentPage"), cookieDataList.at(SK_DOM_FILTER_COOKIE_CURRENT_PAGE));
            sortBy = p.value(QStringLiteral("sortBy"), cookieDataList.at(SK_DOM_FILTER_COOKIE_SORT_BY));
            sortOrder = p.value(QStringLiteral("sortOrder"), cookieDataList.at(SK_DOM_FILTER_COOKIE_SORT_ORDER));
            searchRole = p.value(QStringLiteral("searchRole"), cookieDataList.at(SK_DOM_FILTER_COOKIE_SEARCH_ROLE));
            if (p.contains(QStringLiteral("searchRole"))) {
                searchString = p.value(QStringLiteral("searchString"));
            } else {
                searchString = p.value(QStringLiteral("searchString"), cookieDataList.at(SK_DOM_FILTER_COOKIE_SEARCH_STRING));
            }
        }

        if (accountsPerPage.isEmpty()) {
            accountsPerPage = Session::value(c, QStringLiteral("maxdisplay"), 25).toString();
        }

        if (currentPage.isEmpty()) {
            currentPage = QStringLiteral("1");
        }

        Pagination pag(dom.getAccounts(),
                       accountsPerPage.toInt(),
                       currentPage.toInt());

        static const QStringList sortByCols({QStringLiteral("username"), QStringLiteral("created_at"), QStringLiteral("updated_at"), QStringLiteral("valid_until"), QStringLiteral("quota")});
        if (!sortByCols.contains(sortBy)) {
            sortBy = QStringLiteral("username");
        }

        if ((sortOrder != QLatin1String("asc")) && (sortOrder != QLatin1String("desc"))) {
            sortOrder = QStringLiteral("asc");
        }

        static const QStringList searchCols({QStringLiteral("username"), QStringLiteral("email"), QStringLiteral("forward")});
        if (!searchCols.contains(searchRole)) {
            searchRole = QStringLiteral("username");
        }

        if (!searchString.isEmpty()) {
            searchString.remove(QRegularExpression(QStringLiteral("[^\\w-_\\.]"), QRegularExpression::UseUnicodePropertiesOption));
        }

        const bool isAjax = c->req()->header(QStringLiteral("Accept")).contains(QLatin1String("application/json"), Qt::CaseInsensitive);
        const bool loadAccounts = (!SkaffariConfig::tmplAsyncAccountList() || isAjax);

        SkaffariError e(c);
        if (loadAccounts) {
            pag = Account::list(c, &e, dom, pag, sortBy, sortOrder, searchRole, searchString);
        }

        const QString newCookieData = accountsPerPage + QLatin1Char(';') + currentPage + QLatin1Char(';') + sortBy + QLatin1Char(';') + sortOrder + QLatin1Char(';') + searchRole + QLatin1Char(';') + searchString;
        QNetworkCookie newCookie(QByteArrayLiteral(SK_DOM_FILTER_COOKIE_NAME), newCookieData.toLatin1().toBase64());
        const QString path = QLatin1String("/domain/") + QString::number(dom.id()) + QLatin1String("/accounts");
        newCookie.setPath(path);
        c->res()->setCookie(newCookie);

        if (isAjax) {
            QJsonObject json;

            if (e.type() != SkaffariError::NoError) {

                c->res()->setStatus(Response::InternalServerError);
                json.insert(QStringLiteral("error_msg"), e.errorText());

                c->res()->setJsonBody(QJsonDocument(json));

            } else {

                const std::vector<Account> lst = pag.value(QStringLiteral("accounts")).value<std::vector<Account>>();

                QJsonArray accounts;
                if (!lst.empty()) {
                    for (const Account &a : lst) {
                        accounts.push_back(a.toJson());
                    }
                }
                json.insert(QStringLiteral("accounts"), accounts);
                json.insert(QStringLiteral("searchString"), searchString);
                json.insert(QStringLiteral("searchRole"), searchRole);
                json.insert(QStringLiteral("sortOrder"), sortOrder);
                json.insert(QStringLiteral("sortBy"), sortBy);
                json.insert(QStringLiteral("accountsPerPage"), pag.limit());
                json.insert(QStringLiteral("currentPage"), pag.currentPage());
                json.insert(QStringLiteral("lastPage"), pag.lastPage());

                QVariantList pagesList;
                const QVector<int> pages = pag.pages();
                if (!pagesList.isEmpty()) {
                    for (int pageNo : pages) {
                        pagesList << pageNo;
                    }
                }
                json.insert(QStringLiteral("pages"), QJsonArray::fromVariantList(pagesList));

                c->res()->setJsonBody(QJsonDocument(json));
            }

        } else {

            if (e.type() != SkaffariError::NoError) {
                c->res()->setStatus(Response::InternalServerError);
                c->setStash(QStringLiteral("error_msg"), e.errorText());
            }

            if (loadAccounts) {
                c->setStash(QStringLiteral("pagination"), pag);
            }

            c->stash({
                         {QStringLiteral("template"), QStringLiteral("domain/accounts.html")},
                         {QStringLiteral("site_subtitle"), c->translate("DomainEditor", "Accounts")},
                         {QStringLiteral("searchString"), searchString},
                         {QStringLiteral("searchRole"), searchRole},
                         {QStringLiteral("sortOrder"), sortOrder},
                         {QStringLiteral("sortBy"), sortBy}
                    });
        }
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
                                   new SkValidatorUniqueDb(QStringLiteral("prefix"), QStringLiteral("domain"), QStringLiteral("prefix")),
                                   new ValidatorRequired(QStringLiteral("domainName")),
                                   new SkValidatorUniqueDb(QStringLiteral("domainName"), QStringLiteral("domain"), QStringLiteral("domain_name"), SkValidatorUniqueDb::DomainName),
                                   new ValidatorIn(QStringLiteral("transport"), QStringList({QStringLiteral("cyrus"), QStringLiteral("lmtp"), QStringLiteral("smtp"), QStringLiteral("uucp")})),
                                   new ValidatorInteger(QStringLiteral("maxAccounts")),
                                   new ValidatorMin(QStringLiteral("maxAccounts"), QMetaType::UInt, 0),
                                   new ValidatorInteger(QStringLiteral("quota")),
                                   new ValidatorMin(QStringLiteral("quota"), QMetaType::UInt, 0),
                                   new ValidatorInteger(QStringLiteral("domainQuota")),
                                   new ValidatorMin(QStringLiteral("domainQuota"), QMetaType::UInt, 0),
                                   new ValidatorBoolean(QStringLiteral("freeNames")),
                                   new ValidatorBoolean(QStringLiteral("freeAddress")),
                                   new SkValidatorFilesize(QStringLiteral("humanQuota")),
                                   new SkValidatorFilesize(QStringLiteral("humanDomainQuota")),
                                   new ValidatorInteger(QStringLiteral("parent")),
                                   new ValidatorMin(QStringLiteral("parent"), QMetaType::UInt, 0),
                                   new ValidatorInteger(QStringLiteral("abuseAccount")),
                                   new ValidatorMin(QStringLiteral("abuseAccount"), QMetaType::UInt, 0),
                                   new SkValidatorAccountExists(QStringLiteral("abuseAccount")),
                                   new ValidatorInteger(QStringLiteral("nocAccount")),
                                   new ValidatorMin(QStringLiteral("nocAccount"), QMetaType::UInt, 0),
                                   new SkValidatorAccountExists(QStringLiteral("nocAccount")),
                                   new ValidatorInteger(QStringLiteral("postmasterAccount")),
                                   new ValidatorMin(QStringLiteral("postmasterAccount"), QMetaType::UInt, 0),
                                   new SkValidatorAccountExists(QStringLiteral("postmasterAccount")),
                                   new ValidatorInteger(QStringLiteral("hostmasterAccount")),
                                   new ValidatorMin(QStringLiteral("hostmasterAccount"), QMetaType::UInt, 0),
                                   new SkValidatorAccountExists(QStringLiteral("hostmasterAccount")),
                                   new ValidatorInteger(QStringLiteral("webmasterAccount")),
                                   new ValidatorMin(QStringLiteral("webmasterAccount"), QMetaType::UInt, 0),
                                   new SkValidatorAccountExists(QStringLiteral("webmasterAccount"))
                               });

            const ValidatorResult vr = v.validate(c, Validator::FillStashOnError);
            const auto params = r->bodyParams();
            if (vr) {
                SkaffariError e(c);
                auto dom = Domain::create(c, params, &e);
                if (dom.isValid()) {
                    c->res()->redirect(c->uriForAction(QStringLiteral("/domain/edit"), QStringList(QString::number(dom.id())), QStringList(), StatusMessage::statusQuery(c, c->translate("DomainEditor", "Successfully created new domain %1").arg(dom.getName()))));
                    return;
                } else {
                    c->setStash(QStringLiteral("error_msg"), e.errorText());
                }
            }
            SkaffariError getAccountsErrors(c);
            for (const QString &a : {QStringLiteral("abuseAccount"), QStringLiteral("nocAccount"), QStringLiteral("securityAccount"), QStringLiteral("postmasterAccount"), QStringLiteral("hostmasterAccount"), QStringLiteral("webmasterAccount")}) {
                const dbid_t aId = SKAFFARI_STRING_TO_DBID(params.value(a, QStringLiteral("0")));
                if (aId > 0) {
                    c->setStash(a, QVariant::fromValue<SimpleAccount>(SimpleAccount::get(c, &getAccountsErrors, aId)));
                }
            }
        }

        QHash<QString,HelpEntry> help;
        help.insert(QStringLiteral("domainName"), HelpEntry(c->translate("DomainEditor", "Domain name"), c->translate("DomainEditor", "The name of the domain you want to manage emails for, like example.com. You can safely insert international domain names in UTF-8 encoding, it will be converted internally into ASCII compatible encoding.")));
        help.insert(QStringLiteral("prefix"), HelpEntry(c->translate("DomainEditor", "Prefix"), c->translate("DomainEditor", "The prefix might be used for automatically generated user names, especially if free names are not allowed for this domain.")));
        help.insert(QStringLiteral("maxAccounts"), HelpEntry(c->translate("DomainEditor", "Maximum accounts"), c->translate("DomainEditor", "The maximum accounts value limits the amount of accounts that can be created for this domain. Set it to 0 to disable the limit.")));

        const QString domainQuotaTitle = c->translate("DomainEditor", "Domain quota");
        const QString quotaTitle = c->translate("DomainEditor", "Default quota");
        help.insert(QStringLiteral("domainQuota"), HelpEntry(domainQuotaTitle, c->translate("DomainEditor", "Overall quota limit for all accounts that belong to this domain. If the domain quota is set, every account must have a quota defined. Set it to 0 to disable the domain quota. ")));
        help.insert(QStringLiteral("humanDomainQuota"), HelpEntry(domainQuotaTitle, c->translate("DomainEditor", "Overall quota limit for all accounts that belong to this domain. If the domain quota is set, every account must have a quota defined. Set it to 0 to disable the domain quota. You can use the multipliers M, MiB, G, GiB, T and TiB. Without a multiplier, KiB is the default.")));
        help.insert(QStringLiteral("quota"), HelpEntry(quotaTitle, c->translate("DomainEditor", "Default quota for new accounts for this domain. This value can be changed individually for every account.")));
        help.insert(QStringLiteral("humanQuota"), HelpEntry(quotaTitle, c->translate("DomainEditor", "Default quota for new accounts for this domain. This value can be changed individually for every account. You can use the multipliers M, MiB, G, GiB, T and TiB. Without a multiplier, KiB is the default.")));

        help.insert(QStringLiteral("folders"), HelpEntry(c->translate("DomainEditor", "Standard folders"), c->translate("DomainEditor", "Comma separated list of folders that will be automatically created when creating a new account for this domain. You can safely insert localized folder names in UTF-8 encoding. They will be internally converted into UTF-7-IMAP encoding.")));
        help.insert(QStringLiteral("parent"), HelpEntry(c->translate("DomainEditor", "Parent domain"), c->translate("DomainEditor", "Setting a parent domain for this domain will automatically create email addresses for this domain when creating new accounts in the parent domain.")));
        help.insert(QStringLiteral("transport"), HelpEntry(c->translate("DomainEditor", "Transport"), c->translate("DomainEditor", "The transport mechanism for received emails for this domain. Defaults to Cyrus.")));
        help.insert(QStringLiteral("freeNames"), HelpEntry(c->translate("DomainEditor", "Allow free names"), c->translate("DomainEditor", "If enabled, account user names for this domain can be freely selected (if not in use already).")));
        help.insert(QStringLiteral("freeAddress"), HelpEntry(c->translate("DomainEditor", "Allow free addresses"), c->translate("DomainEditor", "If enabled, user accounts in this domain can have email addresses for all domains managed by Skaffari. If disabled, only email addresses for this domain can be added to user accounts in this domain.")));
        help.insert(QStringLiteral("abuseAccount"), HelpEntry(c->translate("DomainEditor", "Abuse account"), c->translate("DomainEditor", "Used to report inappropriate public behavior. Will create abuse@domain.name address for the selected account.")));
        help.insert(QStringLiteral("nocAccount"), HelpEntry(c->translate("DomainEditor", "NOC account"), c->translate("DomainEditor", "Contact address for the network operations center, responsible for operating the network infrastructure. Will create noc@domain.name for the selected account.")));
        help.insert(QStringLiteral("securityAccount"), HelpEntry(c->translate("DomainEditor", "Security account"), c->translate("DomainEditor", "Contact address for the network security, responsible to handle security bulletins or queries. Will create security@domain.name for the selected account")));
        help.insert(QStringLiteral("postmasterAccount"), HelpEntry(c->translate("DomainEditor", "Postmaster account"), c->translate("DomainEditor", "Used to report issues with sending and receiving emails from and to ths domain. Will create postmaster@domain.name address for the selected account.")));
        help.insert(QStringLiteral("hostmasterAccount"), HelpEntry(c->translate("DomainEditor", "Hostmaster account"), c->translate("DomainEditor", "Used to report issues with the domain name system (DNS) of this domain. Will create hostmaster@domain.name address for the selected account.")));
        help.insert(QStringLiteral("webmasterAccount"), HelpEntry(c->translate("DomainEditor", "Webmaster account"), c->translate("DomainEditor", "Used to contact the operator of the website(s) running on this domain. Will create webmaster@domain.name address for the selected account.")));

        SkaffariError e(c);
        AuthenticationUser user = Authentication::user(c);
        const std::vector<SimpleDomain> doms = SimpleDomain::list(c, &e, user.value(QStringLiteral("type")).value<qint16>(), user.id().toULong(), true);
        if (e.type() != SkaffariError::NoError) {
            c->setStash(QStringLiteral("error_msg"), e.errorText());
        }

        c->stash(SkaffariConfig::getSettingsFromDB());
        c->stash({
                     {QStringLiteral("template"), QStringLiteral("domain/create.html")},
                     {QStringLiteral("domainAsPrefix"), SkaffariConfig::imapDomainasprefix()},
                     {QStringLiteral("site_title"), c->translate("DomainEditor", "Create domain")},
                     {QStringLiteral("help"), QVariant::fromValue<QHash<QString,HelpEntry>>(help)},
                     {QStringLiteral("domains"), QVariant::fromValue<std::vector<SimpleDomain>>(doms)}
                });
    }
}



void DomainEditor::remove(Context* c)
{
    if (Domain::accessGranted(c)) {

        const bool isAjax = c->req()->header(QStringLiteral("Accept")).contains(QLatin1String("application/json"), Qt::CaseInsensitive);
        QJsonObject json;

        if (c->stash(QStringLiteral("userType")).value<quint16>() == 0) {

            auto req = c->req();

            if (req->isPost()) {
                auto dom = Domain::fromStash(c);

                if (dom.getName() == req->bodyParam(QStringLiteral("domainName"))) {

                    SkaffariError e(c);
                    if (Domain::remove(c, &dom, &e, req->bodyParam(QStringLiteral("newParentId"), QStringLiteral("0")).toULong(), false)) {

                        const QString statusMsg = c->translate("DomainEditor", "Successfully deleted domain %1.").arg(dom.getName());

                        if (isAjax) {
                            json.insert(QStringLiteral("status_msg"), statusMsg);
                            json.insert(QStringLiteral("deleted_id"), static_cast<qint64>(dom.id()));
                            json.insert(QStringLiteral("deleted_name"), dom.getName());
                        } else {
                            c->res()->redirect(c->uriFor(QStringLiteral("/domain"), StatusMessage::statusQuery(c, statusMsg)));
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

                    const QString errorMsg = c->translate("DomainEditor", "The entered name does not match the domain name.");

                    if (isAjax) {
                        json.insert(QStringLiteral("error_msg"), errorMsg);
                    } else {
                        c->setStash(QStringLiteral("error_msg"), errorMsg);
                    }
                    c->res()->setStatus(Response::BadRequest);
                }

            } else {

                // this is not an post request, for ajax, we will only allow post
                if (isAjax) {
                    json.insert(QStringLiteral("error_msg"), QJsonValue(c->translate("DomainEditor", "For AJAX requests, this route is only available via POST requests.")));
                    c->response()->setStatus(Response::MethodNotAllowed);
                    c->response()->setHeader(QStringLiteral("Allow"), QStringLiteral("POST"));
                }
            }

        } else {
            // userType is not 0, so we are not a super user and are not allowed to delete a domain

            c->res()->setStatus(Response::Forbidden);
            if (isAjax) {

                json.insert(QStringLiteral("error_msg"), c->translate("Domain", "Access denied. Only super users are allowed to delete domains."));

            } else {
                c->stash({
                             {QStringLiteral("template"), QStringLiteral("403.html")},
                             {QStringLiteral("site_title"), c->translate("Domain", "Access denied")}
                         });
                return;
            }
        }

        if (isAjax) {
            c->res()->setJsonBody(QJsonDocument(json));
        } else {
            c->stash({
                         {QStringLiteral("template"), QStringLiteral("domain/remove.html")},
                         {QStringLiteral("site_subtitle"), c->translate("DomainEditor", "Remove")}
                     });
        }
    }
}



void DomainEditor::add_account(Context* c)
{
    AuthenticationUser user = Authentication::user(c);
    Domain dom = c->stash(QStringLiteral("domain")).value<Domain>();

    if (Q_LIKELY((user.value(QStringLiteral("type")).value<qint16>() == 0) || user.value(QStringLiteral("domains")).value<QVariantList>().contains(dom.id()))) {

        const quota_size_t freeQuota = (dom.getDomainQuota() - dom.getDomainQuotaUsed());

        auto req = c->req();

        if (req->isPost()) {

            const ParamsMultiMap p = req->bodyParameters();

            qCDebug(SK_ACCOUNT) << "Start creating a new account";

            c->stash({
                         {QStringLiteral("imap"), p.contains(QStringLiteral("imap"))},
                         {QStringLiteral("pop"), p.contains(QStringLiteral("pop"))},
                         {QStringLiteral("sieve"), p.contains(QStringLiteral("sieve"))},
                         {QStringLiteral("smtpauth"), p.contains(QStringLiteral("smtpauth"))},
                         {QStringLiteral("catchall"), p.contains(QStringLiteral("catchall"))},
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
                                   new ValidatorRequired(QStringLiteral("validUntil")),
                                   new ValidatorDateTime(QStringLiteral("validUntil"), QStringLiteral("yyyy-MM-ddTHH:mm")),
                                   new ValidatorRequired(QStringLiteral("passwordExpires")),
                                   new ValidatorDateTime(QStringLiteral("passwordExpires"), QStringLiteral("yyyy-MM-ddTHH:mm")),
                                   new ValidatorBoolean(QStringLiteral("imap")),
                                   new ValidatorBoolean(QStringLiteral("pop")),
                                   new ValidatorBoolean(QStringLiteral("sieve")),
                                   new ValidatorBoolean(QStringLiteral("smtpauth")),
                                   new ValidatorBoolean(QStringLiteral("catchall")),
                                   new SkValidatorFilesize(QStringLiteral("humanQuota")),
                                   new ValidatorInteger(QStringLiteral("quota")),
                                   new ValidatorMin(QStringLiteral("quota"), QMetaType::Int, 0)
                               });

            const ValidatorResult vr = v.validate(c, Validator::FillStashOnError);
            if (vr) {

                // if this domain has a global quota limit, we have to calculate the
                // quota that is left
                bool enoughQuotaLeft = true;

                if (dom.getDomainQuota() > 0) {

                    quota_size_t accQuota = 0;
                    bool quotaOk = true;

                    if (p.contains(QStringLiteral("humanQuota"))) {
                        accQuota = Utils::humanToIntSize(c, p.value(QStringLiteral("humanQuota")), &quotaOk);
                        if (!quotaOk) {
                            c->setStash(QStringLiteral("error_msg"),
                                        c->translate("DomainEditor", "Failed to convert human readable quota size string into valid integer value."));
                            enoughQuotaLeft = false;
                        }
                    } else {
                        accQuota = p.value(QStringLiteral("quota")).toULong();
                    }

                    if (quotaOk) {
                        if (freeQuota < accQuota) {

                            enoughQuotaLeft = false;

                            c->setStash(QStringLiteral("error_msg"),
                                        c->translate("DomainEditor", "There is not enough free quota on this domain. Please lower the quota for the new account to a maximum of %1 KiB.").arg(freeQuota));
                        }

                        if ((dom.getDomainQuota() > 0) && (accQuota <= 0)) {

                            enoughQuotaLeft = false;

                            c->setStash(QStringLiteral("error_msg"),
                                        c->translate("DomainEditor", "As this domain has an overall domain quota limit of %1, you have to specify a quota limit for every account that is part of this domain.").arg(Utils::humanBinarySize(c, dom.getDomainQuota())));
                        }
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
                         {QStringLiteral("smtpauth"), true},
                         {QStringLiteral("catchall"), false}
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

        QHash<QString,HelpEntry> help;
        help.insert(QStringLiteral("accounts"), HelpEntry(c->translate("DomainEditor", "Accounts"), c->translate("DomainEditor", "Number of accounts that are currently part of this domain. If there is a maximum account limit defined, it will be shown, too.")));

        const QString quotaTitle = c->translate("DomainEditor", "Quota");
        const QString domainQuotaTitle = c->translate("DomainEditor", "Domain quota");
        if (dom.getDomainQuota() > 0) {
            help.insert(QStringLiteral("domainQuota"), HelpEntry(domainQuotaTitle, c->translate("DomainEditor", "Used and total amount of storage quota for this domain.")));
            help.insert(QStringLiteral("quota"), HelpEntry(quotaTitle, c->translate("DomainEditor", "You have to set a storage quota for this account that does not exceed %1 KiB.").arg(freeQuota * Q_UINT64_C(1024))));
            help.insert(QStringLiteral("humanQuota"), HelpEntry(quotaTitle, c->translate("DomainEditor", "You have to set a storage quota for this account that does not exceed %1. You can use the multipliers M, MiB, G, GiB, T and TiB. Without a multiplier, KiB is the default.").arg(Utils::humanBinarySize(c, freeQuota * Q_UINT64_C(1024)))));
        } else {
            help.insert(QStringLiteral("domainQuota"), HelpEntry(domainQuotaTitle, c->translate("DomainEditor", "This domain has no overall quota limit, so the value shows the sum of the quota limit of all accounts in this domain.")));
            help.insert(QStringLiteral("quota"), HelpEntry(quotaTitle, c->translate("DomainEditor", "You can freely set a storage quota for this account or set the quota to 0 to disable it.")));
            help.insert(QStringLiteral("humanQuota"), HelpEntry(quotaTitle, c->translate("DomainEditor", "You can freely set a storage quota for this account or set the quota to 0 to disable it. You can use the multipliers M, MiB, G, GiB, T and TiB. Without a multiplier, KiB is the default.")));
        }

        const QString usernameTitle = c->translate("DomainEditor", "User name");
        if (!SkaffariConfig::imapDomainasprefix()) {
            if (dom.isFreeNamesEnabled()) {
                help.insert(QStringLiteral("username"), HelpEntry(usernameTitle, c->translate("DomainEditor", "You can freely select a username, as long as it is not in use by another account. The user name has not to be the same as the local part of the email address.")));
            } else {
                help.insert(QStringLiteral("username"), HelpEntry(usernameTitle, c->translate("DomainEditor", "You can not define your own username but have to use the system generated user name.")));
            }
        } else {
            if (SkaffariConfig::imapFqun()) {
                help.insert(QStringLiteral("username"), HelpEntry(usernameTitle, c->translate("DomainEditor", "The email address defined here will be the user name for this account.")));
            } else {
                help.insert(QStringLiteral("username"), HelpEntry(usernameTitle, c->translate("DomainEditor", "The email address defined here will be the user name for this account, but with the @ sign substituted by a dot.")));
            }
        }

        help.insert(QStringLiteral("localpart"), HelpEntry(c->translate("DomainEditor", "Email address"), c->translate("DomainEditor", "Enter the local part of the main email address for this account. You can add more alias addresses to this account later on.")));
        help.insert(QStringLiteral("password"), HelpEntry(c->translate("DomainEditor", "Password"), c->translate("DomainEditor", "Specify a password with a minimum length of %n character(s).", "", SkaffariConfig::accPwMinlength())));
        help.insert(QStringLiteral("password_confirmation"), HelpEntry(c->translate("DomainEditor", "Password confirmation"), c->translate("DomainEditor", "Confirm your entered password.")));
        help.insert(QStringLiteral("validUntil"), HelpEntry(c->translate("DomainEditor", "Valid until"), c->translate("DomainEditor", "You can set a date and time until this account is valid. To make it valid open-end, simply set a date far in the future.")));
        help.insert(QStringLiteral("passwordExpires"), HelpEntry(c->translate("DomainEditor", "Password expires"), c->translate("DomainEditor", "You can set a date and time until the password for this account is valid. To let the password never expire, simply set a date far in the future.")));
        help.insert(QStringLiteral("imap"), HelpEntry(c->translate("DomainEditor", "IMAP Access"), c->translate("DomainEditor", "If enabled, the user of this account can access the mailbox through the IMAP protocol.")));
        help.insert(QStringLiteral("pop"), HelpEntry(c->translate("DomainEditor", "POP3 Access"), c->translate("DomainEditor", "If enabled, the user of this account can access the mailbox through the POP3 protocol.")));
        help.insert(QStringLiteral("sieve"), HelpEntry(c->translate("DomainEditor", "Sieve Access"), c->translate("DomainEditor", "If enabled, the user of this account can manage own Sieve scripts on the server.")));
        help.insert(QStringLiteral("smtpauth"), HelpEntry(c->translate("DomainEditor", "SMTP Access"), c->translate("DomainEditor", "If enabled, the user of this account can send emails via this server through the SMTP protocol.")));
        help.insert(QStringLiteral("children"), HelpEntry(c->translate("DomainEditor", "Child domains"), c->translate("DomainEditor", "For all selected child domains there will be email adresses created with the same local part if not already existing.")));

        SkaffariError getCatchAllUserError(c);
        const QString catchAllUser = dom.getCatchAllAccount(c, &getCatchAllUserError);
        const QString catchAllLabel = c->translate("DomainEditor", "Catch All");
        const QString catchAllText = catchAllUser.isEmpty()
                ? c->translate("DomainEditor", "If enabled, this user will receive all emails sent to addresses not defined for this domain.")
                : c->translate("DomainEditor", "If enabled, this user will receive all emails sent to addresses not defined for this domain. The currently defined user %1 will not receive any messages to undefined addresses anymore.").arg(catchAllUser);
        help.insert(QStringLiteral("catchall"), HelpEntry(catchAllLabel, catchAllText));

        c->stash({
                     {QStringLiteral("template"), QStringLiteral("domain/add_account.html")},
                     {QStringLiteral("site_subtitle"), c->translate("DomainEditor", "Add account")},
                     {QStringLiteral("domainasprefix"), SkaffariConfig::imapDomainasprefix()},
                     {QStringLiteral("fqun"), SkaffariConfig::imapFqun()},
                     {QStringLiteral("minpasswordlength"), SkaffariConfig::accPwMinlength()},
                     {QStringLiteral("freequota"), freeQuota},
                     {QStringLiteral("help"), QVariant::fromValue<QHash<QString,HelpEntry>>(help)}
                 });
    }
}


void DomainEditor::check(Context *c)
{
    if (Domain::accessGranted(c)) {

        auto d = Domain::fromStash(c);

        QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT id FROM accountuser WHERE domain_id = :domain_id"));
        q.bindValue(QStringLiteral(":domain_id"), d.id());

        if (Q_LIKELY(q.exec())) {
            QStringList accountIds;
            while (q.next()) {
                accountIds << QString::number(q.value(0).value<dbid_t>());
            }
            c->setStash(QStringLiteral("accountids"), accountIds.join(QLatin1Char(',')));
        } else {
            c->setStash(QStringLiteral("error_msg"), c->translate("DomainEditor", "Failed to query all account IDs for domain %1 from database: %2").arg(d.getName(), q.lastError().text()));
            qCCritical(SK_DOMAIN, "Failed to query all account IDs for domain %s from database: %s", d.getName().toUtf8().constData(), q.lastError().text().toUtf8().constData());
        }

        c->stash({
                     {QStringLiteral("template"), QStringLiteral("domain/check.html")},
                     {QStringLiteral("baseurl"), c->req()->base()}
                 });
    }
}


QStringList DomainEditor::trimFolderStrings(const QStringList& folders)
{
    QStringList trimmed;

    if (folders.empty()) {
        return trimmed;
    }

    for (const QString &folder : folders) {
        trimmed << folder.simplified();
    }

    return trimmed;
}

#include "moc_domaineditor.cpp"
