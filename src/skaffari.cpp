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

#include "skaffari.h"

#include <Cutelyst/Application>
#include <Cutelyst/Plugins/StaticSimple/StaticSimple>
#include <Cutelyst/Plugins/View/Grantlee/grantleeview.h>
#include <Cutelyst/Plugins/Session/Session>
#include <Cutelyst/Plugins/Authentication/authentication.h>
#include <Cutelyst/Plugins/Authentication/credentialpassword.h>
#include <Cutelyst/Plugins/Authentication/authenticationrealm.h>
#include <Cutelyst/Plugins/Utils/Sql>
#include <Cutelyst/Plugins/StatusMessage>
#include <Cutelyst/Plugins/Memcached/Memcached>
#include <Cutelyst/Plugins/MemcachedSessionStore/MemcachedSessionStore>
#include <Cutelyst/Engine>
#include <Cutelyst/Plugins/CSRFProtection/CSRFProtection>
#include <Cutelyst/Plugins/Utils/LangSelect>

#include <grantlee5/grantlee/metatype.h>
#include <grantlee5/grantlee/engine.h>

#include <QSqlDatabase>
#include <QSqlError>
#include <QDir>
#include <QMetaType>
#include <QCoreApplication>
#include <QTranslator>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>

extern "C"
{
#ifdef WITH_SYSTEMD
#define SD_JOURNAL_SUPPRESS_LOCATION
#include <systemd/sd-journal.h>
#endif

#include <syslog.h>
}

#include "objects/domain.h"
#include "objects/simpleadmin.h"
#include "objects/simpledomain.h"
#include "objects/simpleaccount.h"
#include "objects/adminaccount.h"
#include "objects/account.h"
#include "objects/folder.h"
#include "objects/helpentry.h"
#include "objects/emailaddress.h"
#include "objects/skaffarierror.h"

#include "utils/language.h"
#include "utils/skaffariconfig.h"

#include "../common/config.h"
#include "../common/global.h"
#include "root.h"
#include "authstoresql.h"
#include "login.h"
#include "logout.h"
#include "domaineditor.h"
#include "accounteditor.h"
#include "admineditor.h"
#include "myaccount.h"
#include "settingseditor.h"

Q_LOGGING_CATEGORY(SK_CORE, "skaffari.core")

using namespace Cutelyst;

bool Skaffari::isInitialized = false;
bool Skaffari::messageHandlerInstalled = false;

#ifdef WITH_SYSTEMD
void journaldMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    int prio = LOG_INFO;
    switch (type) {
    case QtDebugMsg:
        prio = LOG_DEBUG;
        break;
    case QtInfoMsg:
        prio = LOG_INFO;
        break;
    case QtWarningMsg:
        prio = LOG_WARNING;
        break;
    case QtCriticalMsg:
        prio = LOG_CRIT;
        break;
    case QtFatalMsg:
        prio = LOG_ALERT;
        break;
    }

#ifdef QT_DEBUG
    sd_journal_send("PRIORITY=%i", prio, "SYSLOG_FACILITY=1", "SYSLOG_IDENTIFIER=%s", context.category, "SYSLOG_PID=%lli", QCoreApplication::applicationPid(), "MESSAGE=%s", qFormatLogMessage(type, context, msg).toUtf8().constData(), "CODE_FILE=%s", context.file, "CODE_LINE=%s", context.line, "CODE_FUNC=%s", context.function, NULL);
#else
    sd_journal_send("PRIORITY=%i", prio, "SYSLOG_FACILITY=1", "SYSLOG_IDENTIFIER=%s", context.category, "SYSLOG_PID=%lli", QCoreApplication::applicationPid(), "MESSAGE=%s", qFormatLogMessage(type, context, msg).toUtf8().constData(), NULL);
#endif

    if (prio == 0) {
        abort();
    }
}
#endif

void syslogMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    int prio = LOG_INFO;
    switch (type) {
    case QtDebugMsg:
        prio = LOG_DEBUG;
        break;
    case QtInfoMsg:
        prio = LOG_INFO;
        break;
    case QtWarningMsg:
        prio = LOG_WARNING;
        break;
    case QtCriticalMsg:
        prio = LOG_CRIT;
        break;
    case QtFatalMsg:
        prio = LOG_ALERT;
        break;
    }

    openlog(context.category, LOG_PID, LOG_USER);
    syslog(prio, "%s", qFormatLogMessage(type, context, msg).toUtf8().constData());
    closelog();

    if (prio == 0) {
        abort();
    }
}

Skaffari::Skaffari(QObject *parent) : Application(parent)
{
}

Skaffari::~Skaffari()
{
}

bool Skaffari::init()
{
    const QVariantMap generalConfig = engine()->config(QStringLiteral("Skaffari"));

    if (!messageHandlerInstalled) {
        const QString backend = generalConfig.value(QStringLiteral("logging_backend")).toString();
        if (backend.compare(QLatin1String("syslog"), Qt::CaseInsensitive) == 0) {
            qSetMessagePattern(QStringLiteral("%{message}"));
            qInstallMessageHandler(syslogMessageOutput);
            qCInfo(SK_CORE, "Using syslog logging backend.");
        }
#ifdef WITH_SYSTEMD
        else if (backend.compare(QLatin1String("journald"), Qt::CaseInsensitive) == 0) {
            qSetMessagePattern(QStringLiteral("%{message}"));
            qInstallMessageHandler(journaldMessageOutput);
            qCInfo(SK_CORE, "Using systemd's journald logging backend.");
        }
#endif
        else {
            qCInfo(SK_CORE, "Using stdout logging backend.");
        }
        messageHandlerInstalled = true;
    }

    QCoreApplication::setApplicationName(QStringLiteral("Skaffari"));
    QCoreApplication::setApplicationVersion(QStringLiteral(SKAFFARI_VERSION));

    const auto supportedLocales = loadTranslationsFromDir(QStringLiteral("skaffari"), QStringLiteral(SKAFFARI_L10NDIR), QStringLiteral("_"));
    qCInfo(SK_CORE) << "Loaded the following locales:" << supportedLocales;

    qRegisterMetaType<quota_size_t>("quota_size_t");
    qRegisterMetaType<dbid_t>("dbid_t");
    qRegisterMetaType<Folder>();
    qRegisterMetaType<Domain>();
    qRegisterMetaType<SimpleAdmin>();
    qRegisterMetaType<SimpleDomain>();
    qRegisterMetaType<SimpleAccount>();
    qRegisterMetaType<AdminAccount>();
    qRegisterMetaType<Language>();
    qRegisterMetaType<Account>();
    qRegisterMetaType<HelpEntry>();
    qRegisterMetaType<HelpHash>("HelpHash");
    qRegisterMetaType<EmailAddress>();
    qRegisterMetaType<SkaffariError>();

    Grantlee::registerMetaType<Folder>();
    Grantlee::registerMetaType<Domain>();
    Grantlee::registerMetaType<SimpleAdmin>();
    Grantlee::registerMetaType<SimpleDomain>();
    Grantlee::registerMetaType<SimpleAccount>();
    Grantlee::registerMetaType<AdminAccount>();
    Grantlee::registerMetaType<Language>();
    Grantlee::registerMetaType<Account>();
    Grantlee::registerMetaType<HelpEntry>();
    Grantlee::registerMetaType<EmailAddress>();

    const QString tmplName = generalConfig.value(QStringLiteral("template"), QStringLiteral("default")).toString();
    const QString tmplBasePath = QStringLiteral(SKAFFARI_TMPLDIR) + QLatin1Char('/') + tmplName;
    SkaffariConfig::setTmplBasePath(tmplBasePath);

    if (!isInitialized) {
        QVariantMap tmplConfig;
        QFile tmplConfigFile(tmplBasePath + QLatin1String("/metadata.json"));
        if (tmplConfigFile.exists()) {
            qCDebug(SK_CORE, "Found template metadata file.");
            if (tmplConfigFile.open(QIODevice::ReadOnly|QIODevice::Text)) {
                QJsonParseError jpe;
                QJsonDocument tmplJsonConfig(QJsonDocument::fromJson(tmplConfigFile.readAll(), &jpe));
                if (jpe.error != QJsonParseError::NoError) {
                    qCCritical(SK_CORE, "Failed to parse template metadata file: %s", qUtf8Printable(jpe.errorString()));
                    return false;
                }

                tmplConfig = tmplJsonConfig.object().value(QStringLiteral("config")).toObject().toVariantMap();
            } else {
                qCCritical(SK_CORE, "Failed to open template metadata file %s.", qUtf8Printable(tmplConfigFile.fileName()));
                return false;
            }
        }

        qCDebug(SK_CORE) << "Initializing configuration.";
        SkaffariConfig::load(generalConfig,
                             engine()->config(QStringLiteral("Accounts")),
                             engine()->config(QStringLiteral("Admins")),
                             engine()->config(QStringLiteral("IMAP")),
                             tmplConfig);

        if (SkaffariConfig::imapUser().isEmpty()) {
            qCCritical(SK_CORE) << "No valid IMAP user defined.";
            return false;
        }

        if (SkaffariConfig::imapPassword().isEmpty()) {
            qCCritical(SK_CORE) << "No valid IMAP password defined.";
            return false;
        }

        // initialize DB one time to prevent https://bugreports.qt.io/browse/QTBUG-54872
        if (!initDb()) {
            return false;
        }

        SkaffariConfig::loadSettingsFromDB();

        isInitialized = true;
    }

    QString sitePath = tmplBasePath + QLatin1String("/site");

    auto view = new GrantleeView(this);
    view->setTemplateExtension(QStringLiteral(".html"));
    view->setWrapper(QStringLiteral("wrapper.html"));
    view->setCache(false);
    view->setIncludePaths({sitePath});
    view->engine()->addDefaultLibrary(QStringLiteral("grantlee_i18ntags"));
    view->engine()->addDefaultLibrary(QStringLiteral("grantlee_skaffari"));

    const QString tmplTransFilePath = tmplBasePath + QLatin1String("/l10n");
    view->loadTranslationsFromDir(tmplName, tmplBasePath + QLatin1String("/l10n"), QStringLiteral("_"));

    new Root(this);
    new Login(this);
    new Logout(this);
    new DomainEditor(this);
    new AccountEditor(this);
    new AdminEditor(this);
    new MyAccount(this);
    new SettingsEditor(this);

    auto staticSimple = new StaticSimple(this);
    const QString staticPath = tmplBasePath + QLatin1String("/static");
    staticSimple->setIncludePaths({staticPath, QStringLiteral(SKAFFARI_STATICDIR)});

    if (SkaffariConfig::useMemcached()) {
        auto memc = new Memcached(this);
        memc->setDefaultConfig({
                                   {QStringLiteral("binary_protocol"), true}
                               });
    }

    auto sess = new Session(this);

    if (SkaffariConfig::useMemcachedSession()) {
        sess->setStorage(new MemcachedSessionStore(this, this));
    }

    auto csrf = new CSRFProtection(this);
    csrf->setDefaultDetachTo(QStringLiteral("/csrfdenied"));

    new StatusMessage(this);

    auto auth = new Authentication(this);
    auto cred = new CredentialPassword;
    cred->setPasswordType(CredentialPassword::Hashed);
    auto store = new AuthStoreSql;
    auth->addRealm(store, cred);

    auto lsp = new LangSelect(this, LangSelect::Session);
    lsp->setFallbackLocale(QLocale(QLocale::English));
    lsp->setSupportedLocales(supportedLocales);
    lsp->setSessionKey(QStringLiteral("lang"));

    defaultHeaders().setHeader(QStringLiteral("X-Frame-Options"), QStringLiteral("DENY"));
    defaultHeaders().setHeader(QStringLiteral("X-Content-Type-Options"), QStringLiteral("nosniff"));
    defaultHeaders().setHeader(QStringLiteral("X-XSS-Protection"), QStringLiteral("1; mode=block"));
    defaultHeaders().setHeader(QStringLiteral("Content-Security-Policy"), QStringLiteral("default-src 'none'; script-src 'self'; style-src 'self'; font-src 'self'; img-src 'self' data:; connect-src 'self'; base-uri 'self'; form-action 'self';"));
    defaultHeaders().setHeader(QStringLiteral("X-Robots-Tag"), QStringLiteral("none"));
    defaultHeaders().setHeader(QStringLiteral("Referrer-Policy"), QStringLiteral("no-referrer, strict-origin-when-cross-origin"));

    return true;
}


bool Skaffari::postFork()
{

    initDb();

    return true;
}

bool Skaffari::initDb() const
{
    const QVariantMap dbconfig = engine()->config(QStringLiteral("Database"));
    const QString dbtype = dbconfig.value(QStringLiteral("type")).toString();
    const QString dbname = dbconfig.value(QStringLiteral("name")).toString();
    const QString dbuser = dbconfig.value(QStringLiteral("user")).toString();
    const QString dbpass = dbconfig.value(QStringLiteral("password")).toString();
    const QString dbhost = dbconfig.value(QStringLiteral("host"), QStringLiteral("localhost")).toString();
    const int dbport = dbconfig.value(QStringLiteral("port"), QStringLiteral("3306")).toInt();

    qCDebug(SK_CORE) << "Establishing database connection";
    QSqlDatabase db;
    const QString dbConName = Sql::databaseNameThread();
    if (dbtype == QLatin1String("QMYSQL")) {
        if (dbname.isEmpty()) {
            qCCritical(SK_CORE) << "No database name set!";
            return false;
        }
        if (dbuser.isEmpty()) {
            qCCritical(SK_CORE) << "No database user set!";
            return false;
        }
        if (dbpass.isEmpty()) {
            qCCritical(SK_CORE) << "No database password set!";
            return false;
        }

        db = QSqlDatabase::addDatabase(dbtype, dbConName);
        if (Q_LIKELY(db.isValid())) {
            db.setDatabaseName(dbname);
            db.setUserName(dbuser);
            db.setPassword(dbpass);

            if (dbhost[0] == QLatin1Char('/')) {
                db.setConnectOptions(QStringLiteral("UNIX_SOCKET=%1;MYSQL_OPT_RECONNECT=1;CLIENT_INTERACTIVE=1").arg(dbhost));
            } else {
                db.setConnectOptions(QStringLiteral("MYSQL_OPT_RECONNECT=1;CLIENT_INTERACTIVE=1"));
                db.setHostName(dbhost);
                db.setPort(dbport);
            }
        } else {
            qCCritical(SK_CORE) << "Failed to obtain database object.";
            return false;
        }
    } else {
        qCCritical(SK_CORE) << dbtype << "is not a supported database type.";
        return false;
    }

    if (Q_UNLIKELY(!db.open())) {
        qCCritical(SK_CORE) << "Failed to establish database connection:" << db.lastError().text();
        return false;
    }

    return true;
}

#include "moc_skaffari.cpp"
