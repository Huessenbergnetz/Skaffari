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
#include "validators/skvalidatoraccountexists.h"
#include "validators/skvalidatordomainexists.h"
#include "../common/global.h"

#include <Cutelyst/Plugins/Utils/Validator> // includes the main validator
#include <Cutelyst/Plugins/Utils/Validators> // includes all validator rules
#include <Cutelyst/Plugins/Utils/ValidatorResult> // includes the validator result
#include <Cutelyst/Plugins/Utils/validatorrequiredifstash.h>
#include <Cutelyst/Plugins/Utils/validatorpwquality.h>
#include <Cutelyst/Plugins/Utils/validatordomain.h>
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
#include <limits>

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
    } else {
        c->detach(c->getAction(QStringLiteral("error")));
    }
}

void DomainEditor::edit(Context *c)
{

    Domain dom = Domain::fromStash(c);

    AuthenticationUser user = Authentication::user(c);

    auto req = c->req();
    if (req->isPost()) {

        c->setStash(QStringLiteral("_def_boolean"), false);

        ValidatorResult vr;
        if (user.value(QStringLiteral("type")).value<qint16>() == 0) {

            static Validator v({
                            new ValidatorIn(QStringLiteral("transport"), QStringList({QStringLiteral("cyrus"), QStringLiteral("lmtp"), QStringLiteral("smtp"), QStringLiteral("uucp")})),
                            new ValidatorMin(QStringLiteral("maxAccounts"), QMetaType::UInt, 0),
                            new ValidatorBoolean(QStringLiteral("freeNames"), ValidatorMessages(), QStringLiteral("_def_boolean")),
                            new ValidatorBoolean(QStringLiteral("freeAddress"), ValidatorMessages(), QStringLiteral("_def_boolean")),
                            new ValidatorFileSize(QStringLiteral("quota"), ValidatorFileSize::ForceBinary, 0, std::numeric_limits<quota_size_t>::max()),
                            new ValidatorFileSize(QStringLiteral("domainQuota"), ValidatorFileSize::ForceBinary, 0, std::numeric_limits<quota_size_t>::max()),
                            new SkValidatorDomainExists(QStringLiteral("parent"))
                        });

            vr = v.validate(c, Validator::FillStashOnError|Validator::BodyParamsOnly);

        } else {

            static Validator v({
                            new ValidatorFileSize(QStringLiteral("quota"), ValidatorFileSize::ForceBinary, QVariant(), std::numeric_limits<quota_size_t>::max())
                        });

            vr = v.validate(c, Validator::FillStashOnError|Validator::BodyParamsOnly);
        }

        if (vr) {
            vr.addValue(QStringLiteral("folders"), req->bodyParam(QStringLiteral("folders")));
            SkaffariError e(c);
            if (dom.update(c, vr.values(), &e, user)) {
                c->stash({
                             {QStringLiteral("domain"), QVariant::fromValue<Domain>(dom)},
                             {QStringLiteral("status_msg"), c->translate("DomainEditor", "Successfully updated domain %1.").arg(dom.name())}
                         });
            } else {
                c->setStash(QStringLiteral("error_msg"), e.errorText());
                c->res()->setStatus(Response::InternalServerError);
            }
        } else {
            c->res()->setStatus(Response::BadRequest);
        }
    }

    HelpHash help;
    help.reserve(12);
    help.insert(QStringLiteral("prefix"), HelpEntry(c->translate("DomainEditor", "Prefix"), c->translate("DomainEditor", "The prefix might be used for automatically generated user names, especially if free names are not allowed for this domain.")));
    help.insert(QStringLiteral("created"), HelpEntry(c->translate("DomainEditor", "Created"), c->translate("DomainEditor", "Date and time this domain was created.")));
    help.insert(QStringLiteral("updated"), HelpEntry(c->translate("DomainEditor", "Updated"), c->translate("DomainEditor", "Date and time this domain was last updated.")));

    const QString domainQuotaTitle = c->translate("DomainEditor", "Domain quota");
    QString domainQuotaText;
    if (c->stash(QStringLiteral("userType")).value<qint16>() == 0) {
        // current user is an administrator
        help.insert(QStringLiteral("maxAccounts"), HelpEntry(c->translate("DomainEditor", "Maximum accounts"), c->translate("DomainEditor", "Limits the maximum number of user accounts that can be created in this domain. Set the value to 0 to disable the limit.")));
        domainQuotaText = c->translate("DomainEditor", "Total storage quota for all user accounts belonging to this domain. If the domain quota is set (not 0), each user account in the domain must have set its own quota. Set it to 0 to disable the domain quota. You can use the multipliers K, KiB, M, MiB, G, GiB, etc.");
    } else {
        // current user is a domain administrator
        help.insert(QStringLiteral("maxAccounts"), HelpEntry(c->translate("DomainEditor", "Accounts"), c->translate("DomainEditor", "The current number of user accounts in this domain and the maximum allowed number of user accounts in this domain.")));
        domainQuotaText = c->translate("DomainEditor", "Total storage quota for all user accounts belonging to this domain. If the domain quota is set (not 0), each user account in the domain must have set its own quota.");
    }
    help.insert(QStringLiteral("domainQuota"), HelpEntry(domainQuotaTitle, domainQuotaText));

    help.insert(QStringLiteral("quota"), HelpEntry(c->translate("DomainEditor", "Default quota"), c->translate("DomainEditor", "Default storage quota for new user accounts in this domain. You can use the multipliers K, Kib, M, MiB, G, GiB, etc.")));


    help.insert(QStringLiteral("folders"), HelpEntry(c->translate("DomainEditor", "Standard folders"), c->translate("DomainEditor", "Comma-separated list of folder names that are automatically created for new user accounts in this domain.")));
    help.insert(QStringLiteral("parent"), HelpEntry(c->translate("DomainEditor", "Parent domain"), c->translate("DomainEditor", "If you set a parent domain for this domain, new accounts in the parent domain automatically create email addresses for the child domain.")));
    help.insert(QStringLiteral("children"), HelpEntry(c->translate("DomainEditor", "Child domains"), c->translate("DomainEditor", "List of child domains of this domain. New accounts in this domain will automatically get email addresses for the child domains.")));
    help.insert(QStringLiteral("transport"), HelpEntry(c->translate("DomainEditor", "Transport"), c->translate("DomainEditor", "The transport mechanism for received emails for this domain. Defaults to Cyrus.")));
    help.insert(QStringLiteral("freeNames"), HelpEntry(c->translate("DomainEditor", "Allow free names"), c->translate("DomainEditor", "If enabled, account user names for this domain can be freely selected (if not in use already).")));
    help.insert(QStringLiteral("freeAddress"), HelpEntry(c->translate("DomainEditor", "Allow free addresses"), c->translate("DomainEditor", "If enabled, user accounts in this domain can have email addresses for all domains managed by Skaffari. If disabled, only email addresses for this domain can be added to user accounts in this domain.")));

    SkaffariError e(c);
    const std::vector<SimpleDomain> doms = SimpleDomain::list(c, &e, user.value(QStringLiteral("type")).value<qint16>(), user.id().value<dbid_t>(), true);
    if (e.type() != SkaffariError::NoError) {
        c->setStash(QStringLiteral("error_msg"), e.errorText());
    }

    c->stash({
                 {QStringLiteral("template"), QStringLiteral("domain/edit.html")},
                 {QStringLiteral("site_subtitle"), c->translate("DomainEditor", "Edit")},
                 {QStringLiteral("help"), QVariant::fromValue<HelpHash>(help)},
                 {QStringLiteral("domains"), QVariant::fromValue<std::vector<SimpleDomain>>(doms)}
             });
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
    auto dom = Domain::fromStash(c);

    const ParamsMultiMap p = c->req()->queryParameters();

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

    Pagination pag(dom.accounts(),
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

void DomainEditor::create(Context* c)
{
    if (AdminAccount::getUserType(c) >= AdminAccount::Administrator) {

        c->setStash(QStringLiteral("domainAsPrefix"), SkaffariConfig::imapDomainasprefix());

        auto r = c->req();
        if (r->isPost()) {

            c->setStash(QStringLiteral("_def_boolean"), false);

            static Validator v({
                                   new ValidatorRequiredIfStash(QStringLiteral("prefix"), QStringLiteral("domainAsPrefix"), QVariantList({QVariant::fromValue<bool>(false)})),
                                   new ValidatorAlphaDash(QStringLiteral("prefix"), true),
                                   new SkValidatorUniqueDb(QStringLiteral("prefix"), QStringLiteral("domain"), QStringLiteral("prefix")),
                                   new ValidatorRequired(QStringLiteral("domainName")),
                                   new ValidatorDomain(QStringLiteral("domainName"), false),
                                   new SkValidatorUniqueDb(QStringLiteral("domainName"), QStringLiteral("domain"), QStringLiteral("domain_name"), SkValidatorUniqueDb::DomainName),
                                   new ValidatorIn(QStringLiteral("transport"), QStringList({QStringLiteral("cyrus"), QStringLiteral("lmtp"), QStringLiteral("smtp"), QStringLiteral("uucp")})),
                                   new ValidatorMin(QStringLiteral("maxAccounts"), QMetaType::UInt, 0),
                                   new ValidatorBoolean(QStringLiteral("freeNames"), ValidatorMessages(), QStringLiteral("_def_boolean")),
                                   new ValidatorBoolean(QStringLiteral("freeAddress"), ValidatorMessages(), QStringLiteral("_def_boolean")),
                                   new ValidatorFileSize(QStringLiteral("quota"), ValidatorFileSize::ForceBinary, 0, std::numeric_limits<quota_size_t>::max()),
                                   new ValidatorFileSize(QStringLiteral("domainQuota"), ValidatorFileSize::ForceBinary, 0, std::numeric_limits<quota_size_t>::max()),
                                   new SkValidatorDomainExists(QStringLiteral("parent")),
                                   new SkValidatorAccountExists(QStringLiteral("abuseAccount")),
                                   new SkValidatorAccountExists(QStringLiteral("nocAccount")),
                                   new SkValidatorAccountExists(QStringLiteral("postmasterAccount")),
                                   new SkValidatorAccountExists(QStringLiteral("hostmasterAccount")),
                                   new SkValidatorAccountExists(QStringLiteral("webmasterAccount"))
                               });

            ValidatorResult vr = v.validate(c, Validator::FillStashOnError|Validator::BodyParamsOnly);
            vr.addValue(QStringLiteral("folders"), r->bodyParam(QStringLiteral("folders")));
            const auto params = vr.values();
            if (vr) {
                SkaffariError e(c);
                auto dom = Domain::create(c, params, &e);
                if (dom.isValid()) {
                    c->res()->redirect(c->uriForAction(QStringLiteral("/domain/edit"), QStringList(QString::number(dom.id())), QStringList(), StatusMessage::statusQuery(c, c->translate("DomainEditor", "Successfully created new domain %1").arg(dom.name()))));
                    return;
                } else {
                    c->setStash(QStringLiteral("error_msg"), e.errorText());
                }
            } else {
                c->res()->setStatus(Response::BadRequest);
            }

            SkaffariError getAccountsErrors(c);
            for (const QString &a : {QStringLiteral("abuseAccount"), QStringLiteral("nocAccount"), QStringLiteral("securityAccount"), QStringLiteral("postmasterAccount"), QStringLiteral("hostmasterAccount"), QStringLiteral("webmasterAccount")}) {
                const dbid_t aId = params.value(a, 0).value<dbid_t>();
                if (aId > 0) {
                    c->setStash(a, QVariant::fromValue<SimpleAccount>(SimpleAccount::get(c, &getAccountsErrors, aId)));
                }
            }
        }

        HelpHash help;
        help.reserve(16);
        help.insert(QStringLiteral("domainName"), HelpEntry(c->translate("DomainEditor", "Domain name"), c->translate("DomainEditor", "The name of the domain you want to manage emails for, like example.com. You can safely insert international domain names in UTF-8 encoding, it will be converted internally into ASCII compatible encoding.")));
        help.insert(QStringLiteral("prefix"), HelpEntry(c->translate("DomainEditor", "Prefix"), c->translate("DomainEditor", "The prefix might be used for automatically generated user names, especially if free names are not allowed for this domain.")));
        help.insert(QStringLiteral("maxAccounts"), HelpEntry(c->translate("DomainEditor", "Maximum accounts"), c->translate("DomainEditor", "Limits the maximum number of user accounts that can be created in this domain. Set the value to 0 to disable the limit.")));

        help.insert(QStringLiteral("domainQuota"), HelpEntry(c->translate("DomainEditor", "Domain quota"), c->translate("DomainEditor", "Total storage quota for all user accounts belonging to this domain. If the domain quota is set (not 0), each user account in the domain must have set its own quota. Set it to 0 to disable the domain quota. You can use the multipliers K, KiB, M, MiB, G, GiB, etc.")));
        help.insert(QStringLiteral("quota"), HelpEntry(c->translate("DomainEditor", "Default quota"), c->translate("DomainEditor", "Default storage quota for new user accounts in this domain. You can use the multipliers K, Kib, M, MiB, G, GiB, etc.")));

        help.insert(QStringLiteral("folders"), HelpEntry(c->translate("DomainEditor", "Standard folders"), c->translate("DomainEditor", "Comma-separated list of folder names that are automatically created for new user accounts in this domain.")));
        help.insert(QStringLiteral("parent"), HelpEntry(c->translate("DomainEditor", "Parent domain"), c->translate("DomainEditor", "If you set a parent domain for this domain, new accounts in the parent domain automatically create email addresses for the child domain.")));
        help.insert(QStringLiteral("transport"), HelpEntry(c->translate("DomainEditor", "Transport"), c->translate("DomainEditor", "The transport mechanism for received emails for this domain. Defaults to Cyrus.")));
        help.insert(QStringLiteral("freeNames"), HelpEntry(c->translate("DomainEditor", "Allow free names"), c->translate("DomainEditor", "If enabled, account user names for this domain can be freely selected (if not in use already).")));
        help.insert(QStringLiteral("freeAddress"), HelpEntry(c->translate("DomainEditor", "Allow free addresses"), c->translate("DomainEditor", "If enabled, user accounts in this domain can have email addresses for all domains managed by Skaffari. If disabled, only email addresses for this domain can be added to user accounts in this domain.")));
        help.insert(QStringLiteral("abuseAccount"), HelpEntry(c->translate("DomainEditor", "Abuse account"), c->translate("DomainEditor", "Used to report inappropriate public behavior. Will create abuse@domain.name address for the selected account.")));
        help.insert(QStringLiteral("nocAccount"), HelpEntry(c->translate("DomainEditor", "NOC account"), c->translate("DomainEditor", "Contact address for the network operations center, responsible for operating the network infrastructure. Will create noc@domain.name for the selected account.")));
        help.insert(QStringLiteral("securityAccount"), HelpEntry(c->translate("DomainEditor", "Security account"), c->translate("DomainEditor", "Contact address for the network security, responsible to handle security bulletins or queries. Will create security@domain.name for the selected account")));
        help.insert(QStringLiteral("postmasterAccount"), HelpEntry(c->translate("DomainEditor", "Postmaster account"), c->translate("DomainEditor", "Used to report issues with sending and receiving emails from and to this domain. Will create postmaster@domain.name address for the selected account.")));
        help.insert(QStringLiteral("hostmasterAccount"), HelpEntry(c->translate("DomainEditor", "Hostmaster account"), c->translate("DomainEditor", "Used to report issues with the domain name system (DNS) of this domain. Will create hostmaster@domain.name address for the selected account.")));
        help.insert(QStringLiteral("webmasterAccount"), HelpEntry(c->translate("DomainEditor", "Webmaster account"), c->translate("DomainEditor", "Used to contact the operator of the website(s) running on this domain. Will create webmaster@domain.name address for the selected account.")));

        SkaffariError e(c);
        AuthenticationUser user = Authentication::user(c);
        const std::vector<SimpleDomain> doms = SimpleDomain::list(c, &e, user.value(QStringLiteral("type")).value<qint16>(), user.id().value<dbid_t>(), true);
        if (e.type() != SkaffariError::NoError) {
            c->setStash(QStringLiteral("error_msg"), e.errorText());
        }

        c->stash(SkaffariConfig::getSettingsFromDB());
        ValidatorFileSize::inputPattern(c, QStringLiteral("quotaPattern"));
        c->stash({
                     {QStringLiteral("template"), QStringLiteral("domain/create.html")},
                     {QStringLiteral("site_title"), c->translate("DomainEditor", "Create domain")},
                     {QStringLiteral("help"), QVariant::fromValue<HelpHash>(help)},
                     {QStringLiteral("domains"), QVariant::fromValue<std::vector<SimpleDomain>>(doms)}
                });
    } else {
        c->res()->setStatus(403);
        c->detach(c->getAction(QStringLiteral("error")));
    }
}

void DomainEditor::remove(Context* c)
{
    const bool isAjax = Utils::isAjax(c);
    QJsonObject json;

    if (c->stash(QStringLiteral("userType")).value<qint16>() == 0) {

        auto req = c->req();

        if (req->isPost()) {
            auto dom = Domain::fromStash(c);

            if (dom.name() == req->bodyParam(QStringLiteral("domainName"))) {

                SkaffariError e(c);
                if (dom.remove(c, &e, req->bodyParam(QStringLiteral("newParentId"), QStringLiteral("0")).toULong(), false)) {

                    const QString statusMsg = c->translate("DomainEditor", "The domain %1 has been successfully deleted.").arg(dom.name());

                    if (isAjax) {
                        json.insert(QStringLiteral("status_msg"), statusMsg);
                        json.insert(QStringLiteral("deleted_id"), static_cast<qint64>(dom.id()));
                        json.insert(QStringLiteral("deleted_name"), dom.name());
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

                const QString errorMsg = c->translate("DomainEditor", "The specified name does not match the domain name.");

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
        // userType is not 0, so we are not an administrator and are not allowed to delete a domain

        c->res()->setStatus(Response::Forbidden);
        if (isAjax) {

            json.insert(QStringLiteral("error_msg"), c->translate("Domain", "Access denied. Only administrator users are allowed to delete domains."));

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

void DomainEditor::add_account(Context* c)
{
    Domain dom = Domain::fromStash(c);

    const quota_size_t freeQuota = (dom.domainQuota() > 0) ? ((dom.domainQuota() - dom.domainQuotaUsed()) * Q_UINT64_C(1024)) : std::numeric_limits<quota_size_t>::max();
    c->setStash(QStringLiteral("_freeQuota"), freeQuota);
    c->setStash(QStringLiteral("_minQuota"), (dom.domainQuota() > 0) ? 1024 : 0);

    auto req = c->req();

    if (req->isPost()) {

        const ParamsMultiMap p = req->bodyParameters();

        c->stash({
                     {QStringLiteral("imap"), Utils::checkCheckbox(p, QStringLiteral("imap"))},
                     {QStringLiteral("pop"), Utils::checkCheckbox(p, QStringLiteral("pop"))},
                     {QStringLiteral("sieve"), Utils::checkCheckbox(p, QStringLiteral("sieve"))},
                     {QStringLiteral("smtpauth"), Utils::checkCheckbox(p, QStringLiteral("smtpauth"))},
                     {QStringLiteral("catchall"), Utils::checkCheckbox(p, QStringLiteral("catchall"))},
                     {QStringLiteral("username"), p.value(QStringLiteral("username"))},
                     {QStringLiteral("localpart"), p.value(QStringLiteral("localpart"))},
                     {QStringLiteral("quota"), p.value(QStringLiteral("quota"))}
                 });

        static Validator v({
                               new ValidatorRequired(QStringLiteral("username")),
                               new ValidatorAlphaDash(QStringLiteral("username")),
                               new SkValidatorUniqueDb(QStringLiteral("username"), QStringLiteral("accountuser"), QStringLiteral("username"), SkValidatorUniqueDb::UserName),
                               new ValidatorRequired(QStringLiteral("localpart")),
                               new ValidatorRegularExpression(QStringLiteral("localpart"), QRegularExpression(QStringLiteral("[^@]"))),
                               new ValidatorRequired(QStringLiteral("password")),
                   #ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
                               new ValidatorPwQuality(QStringLiteral("password"), SkaffariConfig::accPwThreshold(), SkaffariConfig::accPwSettingsFile(), QStringLiteral("username")),
                   #else
                               new ValidatorMin(QStringLiteral("password"), QMetaType::QString, SkaffariConfig::accPwMinlength()),
                   #endif
                               new ValidatorConfirmed(QStringLiteral("password")),
                               new ValidatorRequired(QStringLiteral("validUntil")),
                               new ValidatorDateTime(QStringLiteral("validUntil"), QStringLiteral("userTz"), "yyyy-MM-ddTHH:mm"),
                               new ValidatorRequired(QStringLiteral("passwordExpires")),
                               new ValidatorDateTime(QStringLiteral("passwordExpires"), QStringLiteral("userTz"), "yyyy-MM-ddTHH:mm"),
                               new ValidatorBoolean(QStringLiteral("imap")),
                               new ValidatorBoolean(QStringLiteral("pop")),
                               new ValidatorBoolean(QStringLiteral("sieve")),
                               new ValidatorBoolean(QStringLiteral("smtpauth")),
                               new ValidatorBoolean(QStringLiteral("catchall")),
                               new ValidatorFileSize(QStringLiteral("quota"), ValidatorFileSize::ForceBinary, QStringLiteral("_minQuota"), QStringLiteral("_freeQuota"))
                           });

        const ValidatorResult vr = v.validate(c, Validator::FillStashOnError|Validator::BodyParamsOnly);
        if (vr) {

            // if this domain has a global quota limit, we have to calculate the
            // quota that is left
            bool enoughQuotaLeft = true;

            if (dom.domainQuota() > 0) {

                const quota_size_t accQuota = vr.value(QStringLiteral("quota")).value<quota_size_t>();

                if (freeQuota < accQuota) {

                    enoughQuotaLeft = false;

                    c->setStash(QStringLiteral("error_msg"),
                                //: %1 will be the free domain quota as string like 1.5 GiB
                                c->translate("DomainEditor", "There is not enough free quota on this domain. Please lower the quota for the new account to a maximum of %1.").arg(Utils::humanBinarySize(c, freeQuota)));
                }

                if ((dom.domainQuota() > 0) && (accQuota <= 0)) {

                    enoughQuotaLeft = false;

                    c->setStash(QStringLiteral("error_msg"),
                                //: %1 will be the overall domain quota limit as string like 1.5 GiB
                                c->translate("DomainEditor", "As this domain has an overall domain quota limit of %1, you have to specify a quota limit for every account that is part of this domain.").arg(Utils::humanBinarySize(c, dom.domainQuota())));
                }
            }

            if (enoughQuotaLeft) {

                SkaffariError e(c);
                Account account = Account::create(c, &e, vr.values(), dom, p.values(QStringLiteral("children")));
                if (account.isValid()) {

                    Session::deleteValue(c, QStringLiteral("domainQuotaUsed_") + QString::number(dom.id()));
                    //: %1 will be the user name of the new account, %2 will be the added email address
                    c->res()->redirect(c->uriForAction(QStringLiteral("/domain/accounts"), QStringList(QString::number(dom.id())), QStringList(), StatusMessage::statusQuery(c, c->translate("DomainEditor", "User account %1 successfully created with email address %2.").arg(account.username(), account.addresses().at(0)))));
                    return;

                } else {
                    c->setStash(QStringLiteral("error_msg"), e.errorText());
                }
            } else {
                c->res()->setStatus(Response::BadRequest);
            }

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

         QString username = dom.prefix();

        // if domainasprefix option is not set, user names can be either set free
        // (if they are not used by someone else and the domain has free names enabled),
        // or they are automatically generated from the domain prefix and an autoincrementing
        // number (this will also be generated as proposal for free usable names)
        if (!SkaffariConfig::imapDomainasprefix()) {

            if (dom.isFreeNamesEnabled()) {
                // if the domain allows free names, we only generate a proposal starting with 0000

                const QString cntStr = QString::number(dom.accounts());
                const quint8 digits = cntStr.size();
                if (Q_LIKELY(digits < 4)) {
                    username.append(QString(4 - digits, QLatin1Char('0')));
                }
                username.append(cntStr);

            } else {
                // if the domain does not allow free names, we generate a username
                // based on the domain prefix and an automatically incremented number

                if (Q_LIKELY(dom.accounts() > 0)) {

                    // lets get the last added account
                    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT username FROM accountuser WHERE domain_id = :domain_id ORDER BY id DESC LIMIT 1"));
                    q.bindValue(QStringLiteral(":domain_id"), dom.id());
                    if (!q.exec()) {
                        SkaffariError e(c, q.lastError(), c->translate("DomainEditor", "Failed to query the last added user account from the database."));
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

    HelpHash help;
    help.reserve(15);
    help.insert(QStringLiteral("accounts"), HelpEntry(c->translate("DomainEditor", "Accounts"), c->translate("DomainEditor", "Number of user accounts currently associated with this domain. If there is a limit on the maximum number, it is also displayed.")));

    const QString quotaTitle = c->translate("DomainEditor", "Quota");
    QString quotaText;
    const QString domainQuotaTitle = c->translate("DomainEditor", "Domain quota");
    QString domainQuotaText;
    if (dom.domainQuota() > 0) {
        domainQuotaText = c->translate("DomainEditor", "Used and total storage quota for this domain.");
        //: %1 will be the maximum quota as string like 1.5 GiB
        quotaText = c->translate("DomainEditor", "You must set a storage quota for this user account that does not exceed %1. You can use the multipliers K, KiB, M, MiB, G, GiB, etc.").arg(Utils::humanBinarySize(c, freeQuota));
    } else {
        domainQuotaText = c->translate("DomainEditor", "This domain has no overall quota limit, so the value shows the sum of the quota limit of all accounts in this domain.");
        quotaText = c->translate("DomainEditor", "You can optionally set a storage quota of up to %1 for this user account. If you set it to 0, the quota is disabled. You can use the multipliers K, KiB, M, MiB, G, GiB, etc.").arg(Utils::humanBinarySize(c, freeQuota));
    }
    help.insert(QStringLiteral("quota"), HelpEntry(quotaTitle, quotaText));
    help.insert(QStringLiteral("domainQuota"), HelpEntry(domainQuotaTitle, domainQuotaText));

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
    const int pwMinLength = static_cast<int>(SkaffariConfig::accPwMinlength());
    help.insert(QStringLiteral("password"), HelpEntry(c->translate("DomainEditor", "Password"), c->translate("DomainEditor", "Specify a password with a minimum length of %n character(s).", nullptr, pwMinLength)));
    help.insert(QStringLiteral("password_confirmation"), HelpEntry(c->translate("DomainEditor", "Password confirmation"), c->translate("DomainEditor", "Confirm the password by entering it again.")));
    help.insert(QStringLiteral("validUntil"), HelpEntry(c->translate("DomainEditor", "Valid until"), c->translate("DomainEditor", "You can set a date and time until this account is valid. To make it valid open-end, simply set a date far in the future.")));
    help.insert(QStringLiteral("passwordExpires"), HelpEntry(c->translate("DomainEditor", "Password expires"), c->translate("DomainEditor", "You can set a date and time until the password for this account is valid. To let the password never expire, simply set a date far in the future.")));
    help.insert(QStringLiteral("imap"), HelpEntry(c->translate("DomainEditor", "IMAP Access"), c->translate("DomainEditor", "If enabled, the user of this account can access the mailbox through the IMAP protocol.")));
    help.insert(QStringLiteral("pop"), HelpEntry(c->translate("DomainEditor", "POP3 Access"), c->translate("DomainEditor", "If enabled, the user of this account can access the mailbox through the POP3 protocol.")));
    help.insert(QStringLiteral("sieve"), HelpEntry(c->translate("DomainEditor", "Sieve Access"), c->translate("DomainEditor", "If enabled, the user of this account can manage own Sieve scripts on the server.")));
    help.insert(QStringLiteral("smtpauth"), HelpEntry(c->translate("DomainEditor", "SMTP Access"), c->translate("DomainEditor", "If enabled, the user of this account can send emails via this server through the SMTP protocol.")));
    help.insert(QStringLiteral("children"), HelpEntry(c->translate("DomainEditor", "Child domains"), c->translate("DomainEditor", "For all selected child domains there will be email addresses created with the same local part if not already existing.")));

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
                 {QStringLiteral("help"), QVariant::fromValue<HelpHash>(help)}
             });
}

void DomainEditor::check(Context *c)
{
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
        c->setStash(QStringLiteral("error_msg"), c->translate("DomainEditor", "Failed to query all account IDs for domain %1 from database: %2").arg(d.name(), q.lastError().text()));
        qCCritical(SK_DOMAIN, "Failed to query all account IDs for domain %s from database: %s", d.name().toUtf8().constData(), q.lastError().text().toUtf8().constData());
    }

    c->setStash(QStringLiteral("template"), QStringLiteral("domain/check.html"));
}

QStringList DomainEditor::trimFolderStrings(const QStringList& folders)
{
    QStringList trimmed;

    if (!folders.empty()) {
        for (const QString &folder : folders) {
            trimmed << folder.simplified();
        }
    }

    return trimmed;
}

#include "moc_domaineditor.cpp"
