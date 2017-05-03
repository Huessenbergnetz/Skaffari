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
#include <Cutelyst/Engine>
#include <grantlee5/grantlee/metatype.h>
#include <grantlee5/grantlee/engine.h>
#include <QSqlDatabase>
#include <QSqlError>
#include <QDir>
#include <QMetaType>
#include <QCoreApplication>

#include "objects/domain.h"
#include "objects/simpleadmin.h"
#include "objects/simpledomain.h"
#include "objects/adminaccount.h"
#include "objects/account.h"
#include "objects/folder.h"

#include "utils/language.h"

#include "../common/config.h"
#include "root.h"
#include "authstoresql.h"
#include "login.h"
#include "logout.h"
#include "domaineditor.h"
#include "accounteditor.h"
#include "skaffariengine.h"
#include "admineditor.h"
#include "myaccount.h"

Q_LOGGING_CATEGORY(SK_CORE, "skaffari.core")

using namespace Cutelyst;

Skaffari::Skaffari(QObject *parent) : Application(parent)
{
}

Skaffari::~Skaffari()
{
}

bool Skaffari::init()
{
    QCoreApplication::setApplicationName(QStringLiteral("Skaffari"));

    qRegisterMetaType<Folder>();
    qRegisterMetaType<Domain>();
    qRegisterMetaType<SimpleAdmin>();
    qRegisterMetaType<SimpleDomain>();
    qRegisterMetaType<AdminAccount>();
    qRegisterMetaType<Language>();
    qRegisterMetaType<Account>();

    Grantlee::registerMetaType<Folder>();
    Grantlee::registerMetaType<Domain>();
    Grantlee::registerMetaType<SimpleAdmin>();
    Grantlee::registerMetaType<SimpleDomain>();
    Grantlee::registerMetaType<AdminAccount>();
    Grantlee::registerMetaType<Language>();
    Grantlee::registerMetaType<Account>();

    QString tmplBasePath = QStringLiteral(SKAFFARI_TMPLDIR) + QLatin1Char('/') + config(QStringLiteral("template"), QStringLiteral("default")).toString();
    QString sitePath = tmplBasePath + QLatin1String("/site");

    auto view = new GrantleeView(this);
    view->setTemplateExtension(QStringLiteral(".html"));
    view->setWrapper(QStringLiteral("wrapper.html"));
	view->setCache(false);
    view->setIncludePaths({sitePath});
    view->engine()->addDefaultLibrary(QStringLiteral("grantlee_i18ntags"));

	new Root(this);
	new Login(this);
	new Logout(this);
	new DomainEditor(this);
	new AccountEditor(this);
    new AdminEditor(this);
    new MyAccount(this);


    auto staticSimple = new StaticSimple(this);
    QString staticPath = tmplBasePath + QLatin1String("/static");
    staticSimple->setIncludePaths({staticPath});

	new Session(this);

    new StatusMessage(this);

    auto auth = new Authentication(this);
    auto credential = new CredentialPassword(auth);
    credential->setPasswordType(CredentialPassword::Hashed);
    auto store = new AuthStoreSql(this);
    auto realm = new AuthenticationRealm(store, credential, auth);
    store->setParent(realm);
    auth->addRealm(store, credential);

    defaultHeaders().setHeader(QStringLiteral("X-Frame-Options"), QStringLiteral("DENY"));
    defaultHeaders().setHeader(QStringLiteral("X-Content-Type-Options"), QStringLiteral("nosniff"));
    defaultHeaders().setHeader(QStringLiteral("X-XSS-Protection"), QStringLiteral("1; mode=block"));
    defaultHeaders().setHeader(QStringLiteral("Content-Security-Policy"), QStringLiteral("default-src 'none'; script-src 'self'; style-src 'self'; font-src 'self'; img-src 'self' data:; connect-src 'self';"));

    return true;
}


bool Skaffari::postFork()
{
    const QVariantMap dbconfig = this->engine()->config(QStringLiteral("Database"));
    const QString dbtype = dbconfig.value(QStringLiteral("type")).toString();
    const QString dbname = dbconfig.value(QStringLiteral("name")).toString();
    const QString dbuser = dbconfig.value(QStringLiteral("user")).toString();
    const QString dbpass = dbconfig.value(QStringLiteral("password")).toString();
    const QString dbhost = dbconfig.value(QStringLiteral("host")).toString();
    const int dbport = dbconfig.value(QStringLiteral("port")).toInt();

    QSqlDatabase db;
    if (dbtype == QLatin1String("QMYSQL")) {
        if (dbname.isEmpty() || dbuser.isEmpty() || dbpass.isEmpty()) {
            qCCritical(SK_CORE) << "Database name, database user or database password can not be empty.";
            return false;
        }

        db = QSqlDatabase::addDatabase(dbtype, Sql::databaseNameThread());
        db.setDatabaseName(dbname);
        db.setUserName(dbuser);
        db.setPassword(dbpass);

        if (dbhost[0] == QLatin1Char('/')) {
            db.setConnectOptions(QStringLiteral("UNIX_SOCKET=%1").arg(dbhost));
        } else {
            db.setHostName(dbhost);
            db.setPort(dbport);
        }
    } else {
        qCCritical(SK_CORE) << "No supported databse type in configuration.";
        return false;
    }

    if (!db.open()) {
        qCCritical(SK_CORE) << "Failed to establish database connection:" << db.lastError().text();
        return false;
    }

    auto engine = new SkaffariEngine(this);

    if (!engine->init(this->engine()->config(QStringLiteral("Admins")),
                      this->engine()->config(QStringLiteral("Defaults")),
                      this->engine()->config(QStringLiteral("Accounts")),
                      this->engine()->config(QStringLiteral("IMAP")))) {
		return false;
	}

    const QVector<Controller*> constControllers = controllers();
    for (Controller *c : constControllers) {
        auto sengine = dynamic_cast<SEngine*>(c);
        if (sengine) {
            sengine->engine = engine;
        }
    }

    return true;
}

#include "moc_skaffari.cpp"
