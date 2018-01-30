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
#include "utils/utils.h"
#include "utils/language.h"
#include "utils/skaffariconfig.h"
#include "objects/skaffarierror.h"
#include "../common/config.h"

#include <Cutelyst/Plugins/Authentication/authentication.h>
#include <Cutelyst/Plugins/StatusMessage>
#include <Cutelyst/Plugins/Session/Session>
#include <Cutelyst/Plugins/Utils/Sql>
#include <Cutelyst/Application>

#include <QLocale>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QFile>
#include <QJsonParseError>
#include <QSqlQuery>
#include <QSqlError>

#include "../common/global.h"

using namespace Cutelyst;

Root::Root(QObject *parent) : Controller(parent)
{
}

Root::~Root()
{
}

void Root::index(Context *c)
{
    const bool isSuperUser = c->stash(QStringLiteral("userType")).value<qint16>() == 0;
    const dbid_t adminId = c->stash(QStringLiteral("userId")).value<dbid_t>();

    QSqlQuery q;

    if (isSuperUser) {
        q = CPreparedSqlQueryThread(QStringLiteral("SELECT (SELECT COUNT(*) FROM accountuser) - 1 AS accounts, (SELECT COUNT(*) FROM adminuser) AS admins, (SELECT COUNT(*) FROM domain) AS domains, (SELECT SUM(quota) FROM accountuser) AS accountquota, (SELECT SUM(domainquota) FROM domain) AS domainquota, (SELECT COUNT(*) FROM virtual WHERE alias LIKE '%@%') AS addresses"));
    } else {
        q = CPreparedSqlQueryThread(QStringLiteral("SELECT (SELECT COUNT(*) FROM accountuser au JOIN domainadmin da ON au.domain_id = da.domain_id WHERE da.admin_id = :admin_id) AS accounts, (SELECT COUNT(*) FROM adminuser) AS admins, (SELECT COUNT(*) FROM domain dom JOIN domainadmin da ON dom.id = da.domain_id WHERE da.admin_id = :admin_id) AS domains, (SELECT SUM(au.quota) FROM accountuser au JOIN domainadmin da ON au.domain_id = da.domain_id WHERE da.admin_id = :admin_id) AS accountquota, (SELECT SUM(dom.domainquota) FROM domain dom JOIN domainadmin da ON dom.id = da.domain_id WHERE da.admin_id = :admin_id) AS domainquota, (SELECT COUNT(*) FROM virtual vi JOIN accountuser au ON vi.username = au.username JOIN domainadmin da ON au.domain_id = da.domain_id WHERE da.admin_id = :admin_id AND vi.alias LIKE '%@%') AS addresses"));
        q.bindValue(QStringLiteral(":admin_id"), adminId);
    }

    dbid_t accounts = 0;
    dbid_t admins = 0;
    dbid_t domains = 0;
    quota_size_t accountquota = 0;
    quota_size_t domainquota = 0;
    dbid_t addresses = 0;

    if (Q_LIKELY(q.exec())) {
        if (Q_LIKELY(q.next())) {
            accounts = q.value(0).value<dbid_t>();
            admins = q.value(1).value<dbid_t>();
            domains = q.value(2).value<dbid_t>();
            accountquota = q.value(3).value<quota_size_t>();
            domainquota = q.value(4).value<quota_size_t>();
            addresses = q.value(5).value<dbid_t>();
        }
    }

    if (isSuperUser) {
        q = CPreparedSqlQueryThread(QStringLiteral("SELECT dom.id AS id, dom.domain_name AS name, dom.created_at AS created FROM domain dom ORDER BY dom.created_at DESC LIMIT 5"));
    } else {
        q = CPreparedSqlQueryThread(QStringLiteral("SELECT dom.id AS id, dom.domain_name AS name, dom.created_at AS created FROM domain dom JOIN domainadmin da ON dom.id = da.domain_id WHERE da.admin_id = :admin_id ORDER BY dom.created_at DESC LIMIT 5"));
        q.bindValue(QStringLiteral(":admin_id"), adminId);
    }

    if (Q_LIKELY(q.exec())) {
        const QVariantList domList = Sql::queryToMapList(q);
        if (Q_LIKELY(!domList.empty())) {
            c->setStash(QStringLiteral("domains_last_added"), domList);
        }
    } else {
        c->setStash(QStringLiteral("domains_last_added_error"), c->translate("Root", "Failed to query last added domains from database: %1").arg(q.lastError().text()));
    }

    if (isSuperUser) {
        q = CPreparedSqlQueryThread(QStringLiteral("SELECT au.id AS id, au.domain_id AS domainId, au.created_at AS created, au.username AS username, dom.domain_name AS domainName FROM accountuser au JOIN domain dom ON dom.id = au.domain_id ORDER BY au.created_at DESC LIMIT 5"));
    } else {
        q = CPreparedSqlQueryThread(QStringLiteral("SELECT au.id AS id, au.domain_id AS domainId, au.created_at AS created, au.username AS username, dom.domain_name AS domainName FROM accountuser au JOIN domain dom ON dom.id = au.domain_id JOIN domainadmin da ON au.domain_id = da.domain_id WHERE da.admin_id = :admin_id ORDER BY au.created_at DESC LIMIT 5"));
        q.bindValue(QStringLiteral(":admin_id"), adminId);
    }

    if (Q_LIKELY(q.exec())) {
        const QVariantList accList = Sql::queryToMapList(q);
        if (Q_LIKELY(!accList.empty())) {
            c->setStash(QStringLiteral("accounts_last_added"), accList);
        }
    } else {
        c->setStash(QStringLiteral("accounts_last_added_error"), c->translate("Root", "Failed to query last added accounts from the database: %1").arg(q.lastError().text()));
    }

	c->stash({
                 {QStringLiteral("template"), QStringLiteral("dashboard.html")},
                 {QStringLiteral("site_title"), c->translate("Root", "Dashboard")},
                 {QStringLiteral("account_count"), QVariant::fromValue<dbid_t>(accounts)},
                 {QStringLiteral("admin_count"), QVariant::fromValue<dbid_t>(admins)},
                 {QStringLiteral("domain_count"), QVariant::fromValue<dbid_t>(domains)},
                 {QStringLiteral("accountquota_assigned"), QVariant::fromValue<quota_size_t>(accountquota)},
                 {QStringLiteral("domainquota_assigned"), QVariant::fromValue<quota_size_t>(domainquota)},
                 {QStringLiteral("address_count"), QVariant::fromValue<dbid_t>(addresses)}
             });
}

void Root::about(Context *c)
{
    QStringList description;
    description.push_back(c->translate("Root", "Skaffari is a web application for managing email accounts, based on Cutelyst and written in Qt/C++. It serves as a link and access to a combination of SQL database, IMAP and SMTP server. Skaffari bundles administrative tasks such as the creation of new email domains and email accounts, as well as the creation of new email addresses and email forwards."));
    description.push_back(c->translate("Root", "Administrators can be either global or only responsible for specific domains. Individual domains and accounts can be subject to certain restrictions such as storage space, number of accounts or user names."));
    description.push_back(c->translate("Root", "Skaffari has been tested to work with Cyrus IMAP, Postfix and pam_mysql and was inspired by a PHP-based tool called web-cyradm."));
    description.push_back(c->translate("Root", "By the way, Skaffari is the Old High German word for steward."));

    QVariantList coreComponents;
    coreComponents.push_back(QVariantMap({
                                             {QStringLiteral("name"), QStringLiteral("Skaffari")},
                                             {QStringLiteral("version"), QStringLiteral(SKAFFARI_VERSION)},
                                             {QStringLiteral("url"), QStringLiteral("https://github.com/Huessenbergnetz/Skaffari")},
                                             {QStringLiteral("author"), QStringLiteral("Matthias Fehring")},
                                             {QStringLiteral("authorUrl"), QStringLiteral("https://www.buschmann23.de")},
                                             {QStringLiteral("license"), QStringLiteral("GNU Affero General Public License 3.0")},
                                             {QStringLiteral("licenseUrl"), QStringLiteral("https://github.com/Huessenbergnetz/Skaffari/blob/master/LICENSE")}
                                         }));

    coreComponents.push_back(QVariantMap({
                                             {QStringLiteral("name"), QStringLiteral("Cutelyst")},
                                             {QStringLiteral("version"), QString::fromLatin1(Application::cutelystVersion())},
                                             {QStringLiteral("url"), QStringLiteral("https://www.cutelyst.org")},
                                             {QStringLiteral("author"), QStringLiteral("Daniel Nicoletti")},
                                             {QStringLiteral("authorUrl"), QStringLiteral("https://dantti.wordpress.com/")},
                                             {QStringLiteral("license"), QStringLiteral("GNU Lesser General Public License 2.1")},
                                             {QStringLiteral("licenseUrl"), QStringLiteral("https://github.com/cutelyst/cutelyst/blob/master/COPYING")}
                                         }));
    coreComponents.push_back(QVariantMap({
                                             {QStringLiteral("name"), QStringLiteral("Qt")},
                                             {QStringLiteral("version"), QString::fromLatin1(qVersion())},
                                             {QStringLiteral("url"), QStringLiteral("https://www.qt.io/")},
                                             {QStringLiteral("author"), QStringLiteral("The Qt Company")},
                                             {QStringLiteral("authorUrl"), QStringLiteral("https://www.qt.io")},
                                             {QStringLiteral("license"), QStringLiteral("GNU Lesser General Public License 2.1")},
                                             {QStringLiteral("licenseUrl"), QStringLiteral("https://doc.qt.io/qt-5.6/lgpl.html")}
                                         }));
    coreComponents.push_back(QVariantMap({
                                             {QStringLiteral("name"), QStringLiteral("Grantlee")},
                                             {QStringLiteral("version"), QStringLiteral(GRANTLEE_VERSION)},
                                             {QStringLiteral("url"), QStringLiteral("http://www.grantlee.org")},
                                             {QStringLiteral("author"), QStringLiteral("Stephen Kelly")},
                                             {QStringLiteral("authorUrl"), QStringLiteral("https://steveire.wordpress.com/")},
                                             {QStringLiteral("license"), QStringLiteral("GNU Lesser General Public License 2.1")},
                                             {QStringLiteral("licenseUrl"), QStringLiteral("https://github.com/steveire/grantlee/blob/master/COPYING.LIB")}
                                         }));

    if (SkaffariConfig::useMemcached()) {
        coreComponents.push_back(QVariantMap({
                                                 {QStringLiteral("name"), QStringLiteral("libMemcached")},
                                                 {QStringLiteral("version"), QStringLiteral("1.0.18")},
                                                 {QStringLiteral("url"), QStringLiteral("http://libmemcached.org")},
                                                 {QStringLiteral("author"), QStringLiteral("Data Differential")},
                                                 {QStringLiteral("authorUrl"), QStringLiteral("http://www.datadifferential.com/")},
                                                 {QStringLiteral("license"), QStringLiteral("BSD License")},
                                                 {QStringLiteral("licenseUrl"), QStringLiteral("http://libmemcached.org/License.html")}
                                             }));
    }

    QFile tmplMetadataFile(SkaffariConfig::tmplBasePath() + QLatin1String("/metadata.json"));
    if (Q_LIKELY(tmplMetadataFile.exists())) {
        if (Q_LIKELY(tmplMetadataFile.open(QIODevice::ReadOnly|QIODevice::Text))) {
            QJsonParseError jpe;
            QJsonDocument tmplMetadataJson(QJsonDocument::fromJson(tmplMetadataFile.readAll(), &jpe));
            if (jpe.error == QJsonParseError::NoError) {
                c->setStash(QStringLiteral("templatemetadata"), tmplMetadataJson.object().toVariantMap());
            } else {
                c->setStash(QStringLiteral("error_msg"), c->translate("Root", "Failed to parse JSON from template metadata file at %1: %2").arg(tmplMetadataFile.fileName(), jpe.errorString()));
            }
        } else {
            c->setStash(QStringLiteral("error_msg"), c->translate("Root", "Failed to open template metadata file at %1.").arg(tmplMetadataFile.fileName()));
        }
    } else {
        c->setStash(QStringLiteral("error_msg"), c->translate("Root", "Can not find template metadata file at %1.").arg(tmplMetadataFile.fileName()));
    }

    c->stash({
                 {QStringLiteral("template"), QStringLiteral("about.html")},
                 {QStringLiteral("site_title"), c->translate("Root", "About")},
                 {QStringLiteral("core_components"), coreComponents},
                 {QStringLiteral("description"), description}
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

void Root::csrfdenied(Context *c)
{
    c->res()->setStatus(403);
    Language::setLang(c);
    if (Utils::isAjax(c)) {
        c->res()->setJsonObjectBody(QJsonObject({
                                              {QStringLiteral("error_msg"), QJsonValue(c->stash(QStringLiteral("error_msg")).toString())}
                                          }));
    } else {
        c->stash({
                     {QStringLiteral("template"), QStringLiteral("csrfdenied.html")},
                     {QStringLiteral("no_wrapper"), QStringLiteral("1")}
                 });
    }
}

bool Root::Auto(Context* c)
{
    AuthenticationUser user = Authentication::user(c);

    Language::setLang(c);

    if (c->controller() == c->controller(QStringLiteral("Login"))) {
        return true;
    }

    if (Q_UNLIKELY(user.isNull())) {
        if (Utils::isAjax(c)) {
            c->res()->setStatus(Response::Unauthorized);
            c->res()->setJsonBody(QJsonDocument(QJsonObject({
                                                                {QStringLiteral("error_msg"), QJsonValue(c->translate("Root", "You have to login at first."))}
                                                            })));
        } else {
            c->res()->redirect(c->uriFor(QStringLiteral("/login")));
        }
        return false;
    }

    StatusMessage::load(c);

    c->stash({
                 {QStringLiteral("userId"), QVariant::fromValue<dbid_t>(user.id().toULong())},
                 {QStringLiteral("userType"), user.value(QStringLiteral("type"))},
                 {QStringLiteral("userName"), user.value(QStringLiteral("username"))},
                 {QStringLiteral("userMaxDisplay"), Session::value(c, QStringLiteral("maxdisplay"), SkaffariConfig::defMaxdisplay()).value<quint8>()},
                 {QStringLiteral("userWarnLevel"), Session::value(c, QStringLiteral("warnlevel"), SkaffariConfig::defWarnlevel()).value<quint8>()},
                 {QStringLiteral("userTz"), Session::value(c, QStringLiteral("tz"), SkaffariConfig::defTimezone()).toByteArray()}
             });

    return true;
}

#include "moc_root.cpp"
