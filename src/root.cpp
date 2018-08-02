/*
 * Skaffari - a mail account administration web interface based on Cutelyst
 * Copyright (C) 2017-2018 Matthias Fehring <mf@huessenbergnetz.de>
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
#include "objects/adminaccount.h"
#include "../common/config.h"
#include "../common/global.h"

#include <Cutelyst/Plugins/Authentication/authentication.h>
#include <Cutelyst/Plugins/StatusMessage>
#include <Cutelyst/Plugins/Session/Session>
#include <Cutelyst/Plugins/Utils/Sql>
#include <Cutelyst/Plugins/Memcached/Memcached>
#include <Cutelyst/Application>

#include <QLocale>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QFile>
#include <QJsonParseError>
#include <QSqlQuery>
#include <QSqlError>

#include <unicode/uversion.h>

using namespace Cutelyst;

Root::Root(QObject *parent) : Controller(parent)
{
}

Root::~Root()
{
}

void Root::index(Context *c)
{
    const bool      isAdmin = AdminAccount::getUserType(c) >= AdminAccount::Administrator;
    const dbid_t    adminId = AdminAccount::getUserId(c);

    QSqlQuery q;

    if (isAdmin) {
        q = CPreparedSqlQueryThread(QStringLiteral("SELECT "
                                                   "(SELECT COUNT(*) FROM accountuser) - 1 AS accounts, "
                                                   "(SELECT COUNT(*) FROM adminuser) AS admins, "
                                                   "(SELECT COUNT(*) FROM domain WHERE idn_id = 0) AS domains, "
                                                   "(SELECT SUM(quota) FROM accountuser) AS accountquota, "
                                                   "(SELECT SUM(domainquota) FROM domain) AS domainquota, "
                                                   "(SELECT COUNT(*) FROM virtual WHERE alias LIKE '%@%' AND idn_id = 0) AS addresses"));
    } else {
        q = CPreparedSqlQueryThread(QStringLiteral("SELECT "
                                                   "(SELECT COUNT(*) FROM accountuser au JOIN domainadmin da ON au.domain_id = da.domain_id WHERE da.admin_id = :admin_id) AS accounts, "
                                                   "(SELECT COUNT(*) FROM adminuser) AS admins, "
                                                   "(SELECT COUNT(*) FROM domain dom JOIN domainadmin da ON dom.id = da.domain_id WHERE dom.idn_id = 0 AND da.admin_id = :admin_id) AS domains, "
                                                   "(SELECT SUM(au.quota) FROM accountuser au JOIN domainadmin da ON au.domain_id = da.domain_id WHERE da.admin_id = :admin_id) AS accountquota, "
                                                   "(SELECT SUM(dom.domainquota) FROM domain dom JOIN domainadmin da ON dom.id = da.domain_id WHERE da.admin_id = :admin_id) AS domainquota, "
                                                   "(SELECT COUNT(*) FROM virtual vi JOIN accountuser au ON vi.username = au.username JOIN domainadmin da ON au.domain_id = da.domain_id WHERE da.admin_id = :admin_id AND vi.alias LIKE '%@%' AND vi.idn_id = 0) AS addresses"));
        q.bindValue(QStringLiteral(":admin_id"), adminId);
    }

    dbid_t          accounts        = 0;
    dbid_t          admins          = 0;
    dbid_t          domains         = 0;
    quota_size_t    accountquota    = 0;
    quota_size_t    domainquota     = 0;
    dbid_t          addresses       = 0;

    if (Q_LIKELY(q.exec())) {
        if (Q_LIKELY(q.next())) {
            accounts        = q.value(0).value<dbid_t>();
            admins          = q.value(1).value<dbid_t>();
            domains         = q.value(2).value<dbid_t>();
            accountquota    = q.value(3).value<quota_size_t>();
            domainquota     = q.value(4).value<quota_size_t>();
            addresses       = q.value(5).value<dbid_t>();
        }
    } else {
        c->setStash(QStringLiteral("error_msg"), c->translate("Root", "Failed to query statistics from the database: %1").arg(q.lastError().text()));
    }

    if (isAdmin) {
        q = CPreparedSqlQueryThread(QStringLiteral("SELECT dom.id AS id, dom.domain_name AS name, dom.created_at AS created FROM domain dom "
                                                   "WHERE dom.idn_id = 0 ORDER BY dom.created_at DESC LIMIT 5"));
    } else {
        q = CPreparedSqlQueryThread(QStringLiteral("SELECT dom.id AS id, dom.domain_name AS name, dom.created_at AS created FROM domain dom "
                                                   "JOIN domainadmin da ON dom.id = da.domain_id "
                                                   "WHERE dom.idn_id = 0 AND da.admin_id = :admin_id ORDER BY dom.created_at DESC LIMIT 5"));
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

    if (isAdmin) {
        q = CPreparedSqlQueryThread(QStringLiteral("SELECT au.id AS id, au.domain_id AS domainId, au.created_at AS created, au.username AS username, dom.domain_name AS domainName "
                                                   "FROM accountuser au JOIN domain dom ON dom.id = au.domain_id "
                                                   "ORDER BY au.created_at DESC LIMIT 5"));
    } else {
        q = CPreparedSqlQueryThread(QStringLiteral("SELECT au.id AS id, au.domain_id AS domainId, au.created_at AS created, au.username AS username, dom.domain_name AS domainName "
                                                   "FROM accountuser au JOIN domain dom ON dom.id = au.domain_id JOIN domainadmin da ON au.domain_id = da.domain_id WHERE da.admin_id = :admin_id "
                                                   "ORDER BY au.created_at DESC LIMIT 5"));
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
                 {QStringLiteral("template"),               QStringLiteral("dashboard.html")},
                 {QStringLiteral("site_title"),             c->translate("Root", "Dashboard")},
                 {QStringLiteral("account_count"),          QVariant::fromValue<dbid_t>(accounts)},
                 {QStringLiteral("admin_count"),            QVariant::fromValue<dbid_t>(admins)},
                 {QStringLiteral("domain_count"),           QVariant::fromValue<dbid_t>(domains)},
                 {QStringLiteral("accountquota_assigned"),  QVariant::fromValue<quota_size_t>(accountquota)},
                 {QStringLiteral("domainquota_assigned"),   QVariant::fromValue<quota_size_t>(domainquota)},
                 {QStringLiteral("address_count"),          QVariant::fromValue<dbid_t>(addresses)}
             });
}

void Root::about(Context *c)
{
    std::vector<QString> description;
    description.reserve(4);
    description.push_back(c->translate("Root", "Skaffari is a web application for managing email accounts, based on Cutelyst and written in Qt/C++. It serves as a link and access to a combination of SQL database, IMAP and SMTP server. Skaffari bundles administrative tasks such as the creation of new email domains and email accounts, as well as the creation of new email addresses and email forwards."));
    description.push_back(c->translate("Root", "Administrators can be either global or only responsible for specific domains. Individual domains and accounts can be subject to certain restrictions such as storage space, number of accounts or user names."));
    description.push_back(c->translate("Root", "Skaffari has been tested to work with Cyrus IMAP, Postfix and pam_mysql and was inspired by a PHP-based tool called web-cyradm."));
    description.push_back(c->translate("Root", "By the way, Skaffari is the Old High German word for steward."));

    std::vector<std::map<QString,QString>> coreComponents;
#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
    coreComponents.reserve(SkaffariConfig::useMemcached() ? 7 : 6);
#else
    coreComponents.reserve(SkaffariConfig::useMemcached() ? 6 : 5);
#endif

    coreComponents.push_back(createCoreComponentInfo(QStringLiteral("Skaffari"),
                                                     QStringLiteral(SKAFFARI_VERSION),
                                                     QStringLiteral("https://github.com/Huessenbergnetz/Skaffari"),
                                                     QStringLiteral("Matthias Fehring"),
                                                     QStringLiteral("https://www.buschmann23.de"),
                                                     QStringLiteral("GNU Affero General Public License 3.0"),
                                                     QStringLiteral("https://github.com/Huessenbergnetz/Skaffari/blob/master/LICENSE")));

    coreComponents.push_back(createCoreComponentInfo(QStringLiteral("Cutelyst"),
                                                     QString::fromLatin1(Application::cutelystVersion()),
                                                     QStringLiteral("https://www.cutelyst.org"),
                                                     QStringLiteral("Daniel Nicoletti"),
                                                     QStringLiteral("https://dantti.wordpress.com"),
                                                     QStringLiteral("GNU Lesser General Public License 2.1"),
                                                     QStringLiteral("https://github.com/cutelyst/cutelyst/blob/master/COPYING")));

    coreComponents.push_back(createCoreComponentInfo(QStringLiteral("Grantlee"),
                                                     QStringLiteral(GRANTLEE_VERSION),
                                                     QStringLiteral("http://www.grantlee.org"),
                                                     QStringLiteral("Stephen Kelly"),
                                                     QStringLiteral("https://steveire.wordpress.com/"),
                                                     QStringLiteral("GNU Lesser General Public License 2.1"),
                                                     QStringLiteral("https://github.com/steveire/grantlee/blob/master/COPYING.LIB")));

    coreComponents.push_back(createCoreComponentInfo(QStringLiteral("Qt"),
                                                     QString::fromLatin1(qVersion()),
                                                     QStringLiteral("https://www.qt.io/"),
                                                     QStringLiteral("The Qt Company"),
                                                     QStringLiteral("https://www.qt.io"),
                                                     QStringLiteral("GNU Lesser General Public License 2.1"),
                                                     QStringLiteral("https://doc.qt.io/qt-5.6/lgpl.html")));

    coreComponents.push_back(createCoreComponentInfo(QStringLiteral("ICU"),
                                                     getICUversion(),
                                                     QStringLiteral("http://site.icu-project.org/"),
                                                     QStringLiteral("ICU Project"),
                                                     QStringLiteral("http://site.icu-project.org/"),
                                                     QStringLiteral("Unicode License"),
                                                     QStringLiteral("http://source.icu-project.org/repos/icu/icu/tags/latest/LICENSE")));

    if (SkaffariConfig::useMemcached()) {
        coreComponents.push_back(createCoreComponentInfo(QStringLiteral("libMemcached"),
                                                         Memcached::libMemcachedVersion().toString(),
                                                         QStringLiteral("http://libmemcached.org"),
                                                         QStringLiteral("Data Differential"),
                                                         QStringLiteral("http://www.datadifferential.com/"),
                                                         QStringLiteral("BSD License"),
                                                         QStringLiteral("http://libmemcached.org/License.html")));
    }

#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
    coreComponents.push_back(createCoreComponentInfo(QStringLiteral("libpwquality"),
                                                     QStringLiteral(LIBPWQUALITY_VERSION),
                                                     QStringLiteral("https://github.com/libpwquality/libpwquality"),
                                                     QStringLiteral("Tomáš Mráz"),
                                                     QStringLiteral("https://github.com/t8m"),
                                                     QStringLiteral("GNU General Public License 2.0"),
                                                     QStringLiteral("https://github.com/libpwquality/libpwquality/blob/master/COPYING")));
#endif

    QFile tmplMetadataFile(SkaffariConfig::tmplBasePath() + QLatin1String("/metadata.json"));
    if (Q_LIKELY(tmplMetadataFile.exists())) {
        if (Q_LIKELY(tmplMetadataFile.open(QIODevice::ReadOnly|QIODevice::Text))) {
            QJsonParseError jpe;
            QJsonDocument tmplMetadataJson(QJsonDocument::fromJson(tmplMetadataFile.readAll(), &jpe));
            if (Q_LIKELY(jpe.error == QJsonParseError::NoError)) {
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
                 {QStringLiteral("template"),           QStringLiteral("about.html")},
                 {QStringLiteral("site_title"),         c->translate("Root", "About")},
                 {QStringLiteral("core_components"),    QVariant::fromValue<std::vector<std::map<QString,QString>>>(coreComponents)},
                 {QStringLiteral("description"),        QVariant::fromValue<std::vector<QString>>(description)}
             });
}

void Root::defaultPage(Context *c)
{
    c->res()->setStatus(404);
    c->setStash(QStringLiteral("template"),     QStringLiteral("404.html"));
    c->setStash(QStringLiteral("site_title"),   c->translate("Root", "Not found"));
}

void Root::csrfdenied(Context *c)
{
    c->res()->setStatus(403);
    if (Utils::isAjax(c)) {
        c->res()->setJsonObjectBody({{QStringLiteral("error_msg"), QJsonValue(c->stash(QStringLiteral("error_msg")).toString())}});
    } else {
        c->setStash(QStringLiteral("template"),     QStringLiteral("csrfdenied.html"));
        c->setStash(QStringLiteral("no_wrapper"),   QStringLiteral("1"));
    }
}

void Root::error(Context *c)
{
    const SkaffariError e = SkaffariError::fromStash(c);
    QString error_text;
    QString error_title;
    if (e.type() == SkaffariError::NoError) {
        switch(c->res()->status()) {
        case 404:
            error_title = c->translate("Root", "Not found");
            error_text  = c->translate("Root", "The requested resource could not be found or the requested page is not available.");
            break;
        case 403:
            error_title = c->translate("Root", "Access denied");
            error_text  = c->translate("Root", "You are not authorized to access this resource or to perform this action.");
            break;
        default:
            c->res()->setStatus(500);
            error_title = c->translate("Root", "Unknown error");
            error_text  = c->translate("Root", "Sorry but an unknown error occured while processing your request.");
            break;
        }
    } else {
        c->res()->setStatus(e.status());
        error_title = e.typeTitle(c);
        error_text  = e.errorText();
    }
    if (Utils::isAjax(c)) {
        c->res()->setJsonObjectBody({{QStringLiteral("error_msg"), QJsonValue(error_text)}});
    } else {
        const QString siteTitle = QString::number(c->res()->status()) + QLatin1String(" - ") + error_title;
        c->stash({
                     {QStringLiteral("template"),       QStringLiteral("error.html")},
                     {QStringLiteral("site_title"),     siteTitle},
                     {QStringLiteral("error_title"),    error_title},
                     {QStringLiteral("error_text"),     error_text},
                     {QStringLiteral("error_code"),     c->res()->status()}
                 });
    }
}

bool Root::Auto(Context* c)
{
    const AuthenticationUser user = Authentication::user(c);

    if (c->controller() == c->controller(QStringLiteral("Login"))) {
        return true;
    }

    if (Q_UNLIKELY(user.isNull())) {
        if (Utils::isAjax(c)) {
            c->res()->setStatus(Response::Unauthorized);
            c->res()->setJsonObjectBody({{QStringLiteral("error_msg"), QJsonValue(c->translate("Root", "You have to login at first."))}});
        } else {
            c->res()->redirect(c->uriFor(QStringLiteral("/login")));
        }
        return false;
    }

    StatusMessage::load(c);

    c->stash({
                 {QStringLiteral("user"),           QVariant::fromValue<AdminAccount>(AdminAccount(user))},
                 {QStringLiteral("userId"),         user.id()},
                 {QStringLiteral("userType"),       user.value(QStringLiteral("type"))},
                 {QStringLiteral("userName"),       user.value(QStringLiteral("username"))},
                 {QStringLiteral("userMaxDisplay"), Session::value(c, QStringLiteral("maxdisplay"), SkaffariConfig::defMaxdisplay()).value<quint8>()},
                 {QStringLiteral("userWarnLevel"),  Session::value(c, QStringLiteral("warnlevel"), SkaffariConfig::defWarnlevel()).value<quint8>()},
                 {QStringLiteral("userTz"),         Session::value(c, QStringLiteral("tz"), SkaffariConfig::defTimezone()).toByteArray()}
             });

    return true;
}

QString Root::getICUversion() const
{
    UVersionInfo uver;
    u_getVersion(uver);
    return QStringLiteral("%1.%2.%3").arg(uver[0]).arg(uver[1]).arg(uver[2]);
}

std::map<QString, QString> Root::createCoreComponentInfo(const QString &name, const QString &version, const QString &url, const QString &author, const QString &authorUrl, const QString &license, const QString &licenseUrl) const
{
    return std::map<QString,QString>({
                                     {QStringLiteral("name"),       name},
                                     {QStringLiteral("version"),    version},
                                     {QStringLiteral("url"),        url},
                                     {QStringLiteral("author"),     author},
                                     {QStringLiteral("authorUrl"),  authorUrl},
                                     {QStringLiteral("license"),    license},
                                     {QStringLiteral("licenseUrl"), licenseUrl}
                });
}

#include "moc_root.cpp"
