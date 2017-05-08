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

#include "webcyradmimporter.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QHash>
#include <QSettings>
#include <QVersionNumber>
#include <QCryptographicHash>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <Cutelyst/Plugins/Authentication/credentialpassword.h>
#include <QTimeZone>

#include "database.h"
#include "imap.h"
#include "../common/password.h"
#include "../common/config.h"

#include <QDebug>

WebCyradmImporter::WebCyradmImporter(const QString &confFileName, const QString &iniFileName) :
    m_webCyradmConfFile(confFileName), m_iniFile(iniFileName)
{

}



int WebCyradmImporter::exec() const
{
    printMessage(tr("Start to import web-cyradm configuration and database."));

    printStatus(tr("Checking Skaffari configuration file"));

    bool configExists = m_iniFile.exists();
    if (configExists) {
        if (!m_iniFile.isReadable()) {
            printFailed();
            return fileError(tr("Configuration file exists at %1 but is not readable.").arg(m_iniFile.absoluteFilePath()));
        }

        if (!m_iniFile.isWritable()) {
            printFailed();
            return fileError(tr("Configuration file exists at %1 but is not writable.").arg(m_iniFile.absoluteFilePath()));
        }
        printDone(tr("Found"));
        printMessage(tr("Using existing configuration file at %1.").arg(m_iniFile.absoluteFilePath()));

    } else {

        QDir confDir = m_iniFile.absoluteDir();
        QFileInfo confDirInfo(confDir.absolutePath());
        if (!confDir.exists() && !confDir.mkpath(confDir.absolutePath())) {
            printFailed();
            return fileError(tr("Failed to create configuation directory at %1.").arg(confDir.absolutePath()));
        } else if (confDir.exists() && !confDirInfo.isWritable()) {
            printFailed();
            return fileError(tr("Can not write to configuration directory at %1.").arg(confDir.absolutePath()));
        }

        printDone(tr("Created"));
        printMessage(tr("Creating configuration file at %1.").arg(m_iniFile.absoluteFilePath()));
    }



    printStatus(tr("Checking web-cyradm configuration file"));
    if (m_webCyradmConfFile.exists()) {

        if (!m_webCyradmConfFile.isReadable()) {
            printFailed();
            return fileError(tr("web-cyradm configuration file exists but is not readable."));
        }

        printDone(tr("Found"));

    } else {
        printFailed();
        return fileError(tr("web-cyradm configuration file not found at %1.").arg(m_webCyradmConfFile.absoluteFilePath()));
    }

    printStatus(tr("Reading web-cyradm configuration"));

    QFile wcdm(m_webCyradmConfFile.absoluteFilePath());
    if (!wcdm.open(QFile::ReadOnly|QFile::Text)) {
        printFailed();
        return fileError(tr("Unable to read web-cyradm configuration file."));
    }

    QString php;
    QTextStream in(&wcdm);
    QHash<QString,QString> vars;
    bool insideStatement = false;
    const QRegularExpression regex(QStringLiteral("^\\$(\\w+)\\s*=\\s*(.*);"));
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (!insideStatement) {
            insideStatement = line.startsWith(QLatin1Char('$'));
        }
        if (insideStatement) {
            php.append(line);
            if (line.endsWith(QLatin1Char(';'))) {
                QRegularExpressionMatch match = regex.match(php);
                if (match.hasMatch()) {
                    const QString key = match.captured(1);
                    QString value = match.captured(2);
                    const bool array1 = value.startsWith(QLatin1String("array"));
                    const bool array2 = value.startsWith(QLatin1Char('['));
                    if (array1 || array2) {
                        if (array1) {
                            value.remove(QRegularExpression(QStringLiteral("array\\s*\\(")));
                            value.chop(1);
                        } else {
                            value.remove(0,1);
                            value.chop(1);;
                        }
                        vars.insert(key, value);
                    } else if (value.startsWith(QLatin1Char('"'))) {
                        value.remove(0,1);
                        value.chop(1);;
                        vars.insert(key, value);
                    } else {
                        vars.insert(key, value);
                    }
                }
                php.clear();
                insideStatement = false;
            }
        }
    }


    const QString wcdmDbArray = vars.value(QStringLiteral("DB"));
    QString wdbtype = getArrayEntry(wcdmDbArray, QStringLiteral("TYPE"));
    if (wdbtype == QLatin1String("mysql")) {
        wdbtype = QStringLiteral("QMYSQL");
    } else {
        printFailed();
        return dbError(tr("Skaffari currently only supports MySQL/MariaDB for import, not %1.").arg(wdbtype));
    }
    QString wdbhost = getArrayEntry(wcdmDbArray, QStringLiteral("HOST"));
    QString wdbname = getArrayEntry(wcdmDbArray, QStringLiteral("NAME"));
    QString wdbuser = getArrayEntry(wcdmDbArray, QStringLiteral("USER"));
    QString wdbpass = getArrayEntry(wcdmDbArray, QStringLiteral("PASS"));
    quint16 wdbport = 3306;

    const QString imapArray = vars.value(QStringLiteral("CYRUS"));
    QString imaphost = getArrayEntry(imapArray, QStringLiteral("HOST"));
    quint16 imapport = getArrayEntry(imapArray, QStringLiteral("PORT")).toShort();
    QString imapuser = getArrayEntry(imapArray, QStringLiteral("ADMIN"));
    QString imappass = getArrayEntry(imapArray, QStringLiteral("PASS"));
    quint8 imapprotocol = SK_DEF_IMAP_PROTOCOL;
    quint8 imapencryption = SK_DEF_IMAP_ENCRYPTION;
    QString imappeername;

    QString defaultLang = vars.value(QStringLiteral("DEFAULTLANG"));
    quint32 defaultQuota = vars.value(QStringLiteral("DEFAULT_QUOTA")).toLong();
    quint32 defaultDomainQuota = vars.value(QStringLiteral("DEFAULT_DOMAIN_QUOTA")).toLong();
    QString defaultTimezone;
    quint32 defaultMaxAccounts = 1000;

    QStringList supportedLangs(SKAFFARI_SUPPORTED_LANGS);

    const QString pwtype = vars.value(QStringLiteral("CRYPT"));
    Password::Method accountsPwMethod = static_cast<Password::Method>(SK_DEF_ACC_PWMETHOD);
    Password::Algorithm accountsPwAlgo = static_cast<Password::Algorithm>(SK_DEF_ADM_PWALGORITHM);
    quint32 accountsPwRounds = SK_DEF_ACC_PWROUNDS;
    quint8 accountsPwMinLength = SK_DEF_ACC_PWMINLENGTH;
    QString accountsPwMethodString;
    if ((pwtype == QLatin1String("plain")) || (pwtype == QLatin1String("0"))) {
        accountsPwMethod = Password::PlainText;
        accountsPwMethodString = tr("Plain text");
    } else if ((pwtype == QLatin1String("crypt")) || (pwtype == QLatin1String("1"))) {
        accountsPwMethod = Password::Crypt;
        accountsPwMethodString = QStringLiteral("crypt(3)");
    } else if ((pwtype == QLatin1String("mysql")) || (pwtype == QLatin1String("2"))) {
        accountsPwMethod = Password::MySQL;
        accountsPwMethodString = QStringLiteral("MySQL");
    } else if ((pwtype == QLatin1String("md5")) || (pwtype == QLatin1String("3"))) {
        accountsPwMethod = Password::MD5;
        accountsPwMethodString = QStringLiteral("MD5");
    } else {
        return configError(tr("Can not determine password encryption method."));
    }

    bool domainAsPrefix = vars.value(QStringLiteral("DOMAIN_AS_PREFIX")).toInt() > 0;
    bool fqun = vars.value(QStringLiteral("FQUN")).toInt() > 0;
    quint8 createmailbox = 3;

    printDone();

    printTable({
                   {tr("DB Type"), wdbtype},
                   {tr("DB Host"), wdbhost},
                   {tr("DB Port"), QString::number(wdbport)},
                   {tr("DB Name"), wdbname},
                   {tr("DB User"), wdbuser},
                   {tr("DB Password"), QStringLiteral("********")},
                   {tr("IMAP Host"), imaphost},
                   {tr("IMAP Port"), QString::number(imapport)},
                   {tr("IMAP Protocol"), Imap::networkProtocolToString(imapprotocol)},
                   {tr("IMAP Encryption"), Imap::encryptionTypeToString(imapencryption)},
                   {tr("IMAP User"), imapuser},
                   {tr("IMAP Password"), QStringLiteral("********")},
                   {QStringLiteral("Domain as prefix"), domainAsPrefix ? tr("yes") : tr("no")},
                   {QStringLiteral("FQUN"), fqun ? tr("yes") : tr("no")},
                   {tr("Default language"), defaultLang},
                   {tr("Default domain quota"), QString::number(defaultDomainQuota) + QLatin1String("KiB")},
                   {tr("Default account quota"), QString::number(defaultQuota) + QLatin1String("KiB")},
                   {tr("Accounts password encryption"), accountsPwMethodString}
               }, tr("Imported web-cyradm settings"));

    bool wdbaccess = false;
    Database wdb;
    printStatus(tr("Establishing connection to web-cyradm database"));
    wdbaccess = wdb.open(wdbtype, wdbhost, wdbport, wdbname, wdbuser, wdbpass);

    while (!wdbaccess) {
        printFailed();
        printError(wdb.lastDbError().text());
        printDesc(tr("Failed to establish connection to your web-cyradm database. Please check your connection data."));
        printMessage(QString());
        QVariantHash wdbconf = askDatabaseConfig({
                                                     {QStringLiteral("type"), wdbtype},
                                                     {QStringLiteral("host"), wdbhost},
                                                     {QStringLiteral("port"), wdbport},
                                                     {QStringLiteral("name"), wdbname},
                                                     {QStringLiteral("user"), wdbuser}
                                                });

        printStatus(tr("Establishing database connection with new data."));
        wdbtype = wdbconf.value(QStringLiteral("type")).toString();
        wdbhost = wdbconf.value(QStringLiteral("host")).toString();
        wdbport = wdbconf.value(QStringLiteral("port")).value<quint16>();
        wdbname = wdbconf.value(QStringLiteral("name")).toString();
        wdbuser = wdbconf.value(QStringLiteral("user")).toString();
        wdbpass = wdbconf.value(QStringLiteral("password")).toString();
        wdbaccess = wdb.open(wdbtype, wdbhost, wdbport, wdbname, wdbuser, wdbpass, QStringLiteral("webcyradmdb"));
    }

    printDone();



    bool imapaccess = false;
    Imap imap;
    printStatus(tr("Establishing connection to IMAP server"));
    imap.setHost(imaphost);
    imap.setPort(imapport);
    imap.setUser(imapuser);
    imap.setPassword(imappass);
    imap.setProtocol(static_cast<QAbstractSocket::NetworkLayerProtocol>(imapprotocol));
    imap.setEncryptionType(static_cast<Imap::EncryptionType>(imapencryption));
    if (domainAsPrefix || fqun) {
        imap.setHierarchySeparator(QLatin1Char('/'));
    } else {
        imap.setHierarchySeparator(QLatin1Char('.'));
    }
    imapaccess = imap.login();

    while (!imapaccess) {
        printFailed();
        printError(tr("Failed to connect to the IMAP server for the following reason. Please check your connection data."));
        printError(imap.lastError());
        const QVariantHash imapConf = askImapConfig({
                                                        {QStringLiteral("host"), imaphost},
                                                        {QStringLiteral("port"), imapport},
                                                        {QStringLiteral("user"), imapuser},
                                                        {QStringLiteral("protocol"), imapprotocol},
                                                        {QStringLiteral("encryption"), imapencryption},
                                                        {QStringLiteral("peername"), QStringLiteral("none")}
                                                    });
        imaphost = imapConf.value(QStringLiteral("host")).toString();
        imapport = imapConf.value(QStringLiteral("port")).value<quint16>();
        imapuser = imapConf.value(QStringLiteral("user")).toString();
        imappass = imapConf.value(QStringLiteral("password")).toString();
        imapencryption = imapConf.value(QStringLiteral("encryption")).value<quint8>();
        imapprotocol = imapConf.value(QStringLiteral("protocol")).value<quint8>();
        imappeername = imapConf.value(QStringLiteral("peername")).toString();

        printStatus(tr("Establishing IMAP connection"));
        imap.setHost(imaphost);
        imap.setPort(imapport);
        imap.setUser(imapuser);
        imap.setPassword(imappass);
        imap.setProtocol(static_cast<QAbstractSocket::NetworkLayerProtocol>(imapprotocol));
        imap.setEncryptionType(static_cast<Imap::EncryptionType>(imapencryption));
        imap.setPeerVerifyName(imappeername);
        imapaccess = imap.login();
    }

    imap.logout();
    printDone();

    printDesc(QStringList({
                              tr("Skaffari uses a slightly different database layout, but still compatible to pam_mysql."),
                              tr("Because of that and because of backup reasons, Skaffari creates a new database and imports the data from your web-cyradm database."),
                              tr("In the next step, please enter the data for the connection to your Skaffari database."),
                              QString()
                          }));

    bool sdbaccess = false;
    QString sdbtype, sdbhost, sdbname, sdbuser, sdbpass;
    quint16 sdbport;
    Database sdb;
    while (!sdbaccess) {
        QVariantHash dbconf = askDatabaseConfig({
                                                    {QStringLiteral("type"), QStringLiteral("QMYSQL")},
                                                    {QStringLiteral("host"), QStringLiteral("localhost")},
                                                    {QStringLiteral("port"), 3306},
                                                    {QStringLiteral("name"), QStringLiteral("skaffaridb")},
                                                    {QStringLiteral("user"), QStringLiteral("skaffari")}
                                                });
        printStatus(tr("Establishing connection to Skaffari database"));
        sdbtype = dbconf.value(QStringLiteral("type")).toString();
        sdbhost = dbconf.value(QStringLiteral("host")).toString();
        sdbport = dbconf.value(QStringLiteral("port")).value<quint16>();
        sdbname = dbconf.value(QStringLiteral("name")).toString();
        sdbuser = dbconf.value(QStringLiteral("user")).toString();
        sdbpass = dbconf.value(QStringLiteral("pass")).toString();
        sdbaccess = sdb.open(sdbtype, sdbhost, sdbport, sdbname, sdbuser, sdbpass, QStringLiteral("skaffaridb"));
        if (sdbaccess) {
            printDone();;
        } else {
            printFailed();
        }
    }

    printDesc(QStringList({
                              QString(),
                              tr("Before importing the web-cyradm data, we need to set some options that are new in Skaffari."),
                              QString()
                          }));

    const QVariantHash adminPwConf = askPbkdf2Config();
    const QCryptographicHash::Algorithm adminPwAlgorithm = static_cast<QCryptographicHash::Algorithm>(adminPwConf.value(QStringLiteral("method")).value<quint8>());
    const quint32 adminPwRounds = adminPwConf.value(QStringLiteral("rounds")).value<quint32>();
    const quint8 adminPwMinLength = adminPwConf.value(QStringLiteral("minlength")).value<quint8>();


    if (accountsPwMethod == Password::Crypt) {
        printMessage(QString());
        printDesc(QStringList({
                                  tr("You are using the crypt(3) function to encrypt user account passwords for that web-cyradm does not offer further options, but Skaffari does."),
                                  tr("crypt(3) uses a storage format string that allows changing the algorithm used to derive the key."),
                                  QString()
                              }));
        accountsPwAlgo = static_cast<Password::Algorithm>(readChar(tr("Encryption method"),
                                                                SK_DEF_ACC_PWALGORITHM,
                                                                QStringList({
                                                                                tr("The crypt(3) method supports different algorithms to derive a key from a password. To see which algorithms are supported on your system, use man crypt. Especially the bcrypt algorithm that uses Blowfish is not available on every system because it is not part of the default crypt(3) distribution. The not recommended hashing methods are provided for backwards compatibility and if you have to store passwords for use across different operating systems."),
                                                                                QString(),
                                                                                tr("Supported algorithms:"),
                                                                                tr("0: Default - points to SHA-256"),
                                                                                tr("1: Traditional DES-based - not recommended"),
                                                                                tr("2: FreeBSD-style MD5-based - not recommended"),
                                                                                tr("3: SHA-256 based"),
                                                                                tr("4: SHA-512 based"),
                                                                                tr("5: OpenBSD-style Blowfish-based (bcrypt) - not supported everywhere")
                                                                            }),
                                                                QList<quint8>({0,1,2,3,4,5})));

        const QStringList accountsPwRoundsDesc = (accountsPwAlgo == Password::CryptBcrypt) ? QStringList(tr("Crypt(3) with bcrypt supports an iteration count to increase the time cost for creating the derived key. The iteration count passed to the crypt function is the base-2 logarithm of the actual iteration count. Supported values are between 4 and 31.")) : QStringList(tr("Crypt(3) with SHA-256 and SHA-512 supports an iteration count from 1000 to 999999999. The iterations are used to increase the time cost for creating the derived key."));
        if ((accountsPwAlgo == Password::CryptBcrypt) && (accountsPwRounds > 31)) {
            accountsPwRounds = 12;
        }

        accountsPwRounds = readInt(tr("Encryption rounds"), accountsPwRounds, accountsPwRoundsDesc);


    } else if (accountsPwMethod == Password::MySQL) {
        printMessage(QString());
        printDesc(QStringList({
                                  tr("You are using the MySQL password function to encrypt user account passwords for that web-cyradm does not offer furter options, but Skaffari does."),
                                  QString()
                              }));
        accountsPwAlgo = static_cast<Password::Algorithm>(readChar(tr("Encryption method"),
                                                                  accountsPwAlgo,
                                                                  QStringList({
                                                                                  tr("MySQL supports two different password hashing methods, a new one and an old one. If possible, you should use the new method. The old method is provided for backwards compatibility."),
                                                                                  QString(),
                                                                                  tr("Supported methods:"),
                                                                                  tr("0: default - points to MySQL new"),
                                                                                  tr("6: MySQL new"),
                                                                                  tr("7: MySQL old")
                                                                              }),
                                                                              QList<quint8>({0,6,7})));
    }

    accountsPwMinLength = readChar(tr("Account password minimum length"), accountsPwMinLength, QStringList(tr("The required minimum length for user account passwords created or changed via Skaffari.")));

    createmailbox = readChar(tr("Create mailboxes"),
                             createmailbox,
                             QStringList({
                                             tr("Skaffari can create the mailboxes and all default folders on the IMAP server after creating a new user account. Alternatively the IMAP server can create default folders and account quotas on the first user login or first incoming email for the new account (has to be configured in your imapd.conf file). Skaffari is more flexible on creating different default folders for different domains."),
                                             tr("Available behavior:"),
                                             tr("0: Skaffari does nothing - all will be created by the IMAP server on first login or first incoming email"),
                                             tr("1: Login after creation - Skaffari relies on the IMAP server to create folders and quotas, but will perform a login after account creation to initiate the creation by the IMAP server"),
                                             tr("2: Only set quota - Skaffari will login to the new account after creation to let the IMAP server create the mailbox and will then set the quota."),
                                             tr("3: Create by Skaffari - Skaffari will create the new mailbox and the default folders and will set the account quota after adding a new account.")
                                         }));

    defaultMaxAccounts = readInt(tr("Default maximum accounts"), defaultMaxAccounts, QStringList(tr("Default maximum account number for new domains.")));

    if (!supportedLangs.contains(defaultLang)) {
        if (!supportedLangs.contains(defaultLang.left(2))) {
            defaultLang = readString(tr("Default language"), QStringLiteral(SK_DEF_DEF_LANGUAGE), QStringList(tr("The default language used for newly created administrators and as fallback option.")), supportedLangs);
        } else {
            defaultLang = defaultLang.left(2);
        }
    }

    while (!QTimeZone::isTimeZoneIdAvailable(defaultTimezone.toUtf8())) {
        defaultTimezone = readString(tr("Default timezone"), QStringLiteral(SK_DEF_DEF_TIMEZONE), QStringList(tr("Default timezone for newly created administrators and as fallback option. The timezone will be used to show localized date and time values. Please enter a valid IANA timezone ID.")));
    }

    printMessage(QString());
    printDesc(QStringList({
                              tr("Start importing web-cyradm database."),
                              QString(),
                              tr("This will delete all content in the Skaffari database.")
                          }));
    if (!readBool(tr("Are you sure you want to import web-cyradm data?"), false)) {
        return 99;
    }

    sdb.deleteAll();

    printStatus(tr("Performing database installation"));
    if (!sdb.installDatabase()) {
        printFailed();
        return dbError(sdb.lastDbError());
    } else {
        printDone();
    }

    QSqlQuery wq(wdb.getDb()); // web-cyradm query
    QSqlQuery sq(sdb.getDb()); // Skaffari query
    QDateTime currentTime = QDateTime::currentDateTimeUtc();

    printStatus(tr("Importing domains"));
    if (!wq.exec(QStringLiteral("SELECT domain_name, prefix, maxaccounts, quota, domainquota, transport, freenames, freeaddress, folders FROM domain"))) {
        printFailed();
        return dbError(wq.lastError());
    }

    QHash<QString,quint32> domainPrefixId;
    QHash<QString,quint32> domainNameId;
    QHash<quint32,QStringList> domainFolders;

    if (sq.prepare(QStringLiteral("INSERT INTO domain (domain_name, prefix, maxaccounts, quota, domainquota, transport, freenames, freeaddress, created_at, updated_at) VALUES (?,?,?,?,?,?,?,?,?,?)"))) {
        while (wq.next()) {
            const QString name = wq.value(0).toString();
            const QString prefix = wq.value(1).toString();
            sq.addBindValue(name);
            sq.addBindValue(prefix);
            sq.addBindValue(wq.value(2));
            sq.addBindValue(wq.value(3));
            sq.addBindValue(wq.value(4));
            sq.addBindValue(wq.value(5));
            sq.addBindValue(wq.value(6).toString() == QLatin1String("YES"));
            sq.addBindValue(wq.value(7).toString() == QLatin1String("YES"));
            sq.addBindValue(currentTime);
            sq.addBindValue(currentTime);

            const QStringList folders = wq.value(8).toString().split(QLatin1Char(','));

            if (sq.exec()) {
                const quint32 id = sq.lastInsertId().value<quint32>();
                domainPrefixId.insert(prefix, id);
                domainNameId.insert(name, id);
                if (!folders.empty()) {
                    domainFolders.insert(id, folders);
                }
            } else {
                printFailed();
                return dbError(sq.lastError());
            }
        }
    } else {
        printFailed();
        return dbError(sq.lastError());
    }

    printDone(tr("%n domain(s)", "", domainPrefixId.size()));

    printStatus(tr("Importing domain default folders"));

    if (!domainFolders.empty()) {
        if (sq.prepare(QStringLiteral("INSERT INTO folder (domain_id, name) VALUES (?,?)"))) {

            auto i = domainFolders.constBegin();
            while (i != domainFolders.constEnd()) {
                const quint32 id = i.key();
                const QStringList folders = i.value();
                for (const QString &folder : folders) {
                    sq.addBindValue(id);
                    sq.addBindValue(folder);
                    if (!sq.exec()) {
                        printFailed();
                        return dbError(sq.lastError());
                    }
                }
                ++i;
            }

        } else {
            printFailed();
            return dbError(sq.lastError());
        }
        printDone(tr("%n domain(s)", "", domainFolders.size()));
    } else {
        printDesc(tr("None"));
    }


    printStatus(tr("Importing admin accounts"));

    if (!wq.exec(QStringLiteral("SELECT username, password, type FROM adminuser"))) {
        printFailed();
        return dbError(wq.lastError());
    }

    QHash<QString,quint32> adminNameIds;

    if (sq.prepare(QStringLiteral("INSERT INTO adminuser (username, password, type, created_at, updated_at) VALUES (?,?,?,?,?)"))) {
        while (wq.next()) {
            const QString name = wq.value(0).toString();
            sq.addBindValue(name);
            sq.addBindValue(wq.value(1).toString());
            sq.addBindValue(wq.value(2).value<quint8>());
            sq.addBindValue(currentTime);
            sq.addBindValue(currentTime);

            if (sq.exec()) {
                adminNameIds.insert(name, sq.lastInsertId().value<quint32>());
            } else {
                printFailed();
                return dbError(sq.lastError());
            }
        }
        printDone(tr("%n admin(s)", "", adminNameIds.size()));
    } else {
        printFailed();
        return dbError(sq.lastError());
    }

    printStatus(tr("Importing admin to domain connections"));

    if (!wq.exec(QStringLiteral("SELECT domain_name, adminuser FROM domainadmin WHERE domain_name != '*'"))) {
        printFailed();
        return dbError(wq.lastError());
    }

    if (sq.prepare(QStringLiteral("INSERT INTO domainadmin (domain_id, admin_id) VALUES (?,?)"))) {
        int adminDomainConnectionCount = 0;
        while (wq.next()) {
            sq.addBindValue(domainNameId.value(wq.value(0).toString()));
            sq.addBindValue(adminNameIds.value(wq.value(1).toString()));

            if (!sq.exec()) {
                printFailed();
                return dbError(sq.lastError());
            }
            adminDomainConnectionCount++;
        }
        printDone(tr("%n connection(s)", "", adminDomainConnectionCount));
    } else {
        printFailed();
        return dbError(sq.lastError());
    }


    printStatus(tr("Importing admin settings"));

    if (!wq.exec(QStringLiteral("SELECT username, maxdisplay, warnlevel FROM settings"))) {
        printFailed();
        return dbError(wq.lastError());
    }

    if (sq.prepare(QStringLiteral("INSERT INTO settings (admin_id, maxdisplay, warnlevel, tz, lang) VALUES (?,?,?,?,?)"))) {
        while (wq.next()) {
            sq.addBindValue(adminNameIds.value(wq.value(0).toString()));
            sq.addBindValue(wq.value(1));
            sq.addBindValue(wq.value(2));
            sq.addBindValue(defaultTimezone);
            sq.addBindValue(defaultLang);

            if (!sq.exec()) {
                printFailed();
                return dbError(sq.lastError());
            }
        }
        printDone(tr("%n admin(s)", "", adminNameIds.size()));
    } else {
        printFailed();
        return dbError(sq.lastError());
    }


    printStatus(tr("Importing user accounts and quota"));

    if (!wq.exec(QStringLiteral("SELECT username, password, prefix, domain_name, imap, pop, sieve, smtpauth FROM accountuser"))) {
        printFailed();
        return dbError(wq.lastError());
    }

    if (sq.prepare(QStringLiteral("INSERT INTO accountuser (domain_id, username, password, prefix, domain_name, imap, pop, sieve, smtpauth, quota, created_at, updated_at) "
                                  "VALUES (?,?,?,?,?,?,?,?,?,?,?,?)"))) {

        int usercount = 0;
        imap.login();
        while (wq.next()) {
            const QString username = wq.value(0).toString();
            const QString domainName = wq.value(3).toString();
            std::pair<quint32,quint32> quota = imap.getQuota(username);
            sq.addBindValue(domainNameId.value(domainName));
            sq.addBindValue(username);
            sq.addBindValue(wq.value(1));
            sq.addBindValue(wq.value(2));
            sq.addBindValue(domainName);
            sq.addBindValue(wq.value(4));
            sq.addBindValue(wq.value(5));
            sq.addBindValue(wq.value(6));
            sq.addBindValue(wq.value(7));
            sq.addBindValue(quota.second);
            sq.addBindValue(currentTime);
            sq.addBindValue(currentTime);

            if (!sq.exec()) {
                printFailed();
                return dbError(sq.lastError());
            }
            usercount++;
        }
        imap.logout();
        printDone(tr("%n user(s)", "", usercount));

    } else {
        printFailed();
        return dbError(sq.lastError());
    }


    printStatus(tr("Importing virtual entries"));

    if (!wq.exec(QStringLiteral("SELECT alias, dest, username, status FROM virtual"))) {
        printFailed();
        return dbError(wq.lastError());
    }

    if (sq.prepare(QStringLiteral("INSERT INTO virtual (alias, dest, username, status) VALUES (?,?,?,?)"))) {
        int virtualCount = 0;
        while (wq.next()) {
            sq.addBindValue(wq.value(0));
            sq.addBindValue(wq.value(1));
            sq.addBindValue(wq.value(2));
            sq.addBindValue(wq.value(3));

            if (!sq.exec()) {
                printFailed();
                return dbError(sq.lastError());
            }
            virtualCount++;
        }
        printDone(tr("%n entrie(s)", "", virtualCount));
    } else {
        printFailed();
        return dbError(sq.lastError());
    }


    printStatus(tr("Importing aliases"));

    if (!wq.exec(QStringLiteral("SELECT alias, dest, username, status FROM alias"))) {
        printFailed();
        return dbError(wq.lastError());
    }

    if (sq.prepare(QStringLiteral("INSERT INTO alias (alias, dest, username, status) VALUES (?,?,?,?)"))) {
        int aliasCount = 0;
        while (wq.next()) {
            sq.addBindValue(wq.value(0));
            sq.addBindValue(wq.value(1));
            sq.addBindValue(wq.value(2));
            sq.addBindValue(wq.value(3));

            if (!sq.exec()) {
                printFailed();
                return dbError(sq.lastError());
            }
            aliasCount++;
        }
        printDone(tr("%n alias(es)", "", aliasCount));
    } else {
        printFailed();
        return dbError(sq.lastError());
    }


    printStatus(tr("Importing log entries"));

    if (!wq.exec(QStringLiteral("SELECT id, msg, user, host, time, pid FROM log"))) {
        printFailed();
        return dbError(wq.lastError());
    }

    if (sq.prepare(QStringLiteral("INSERT INTO log (id, msg, user, host, time, pid) VALUES (?,?,?,?,?,?)"))) {
        int logEntries = 0;
        while (wq.next()) {
            sq.addBindValue(wq.value(0));
            sq.addBindValue(wq.value(1));
            sq.addBindValue(wq.value(2));
            sq.addBindValue(wq.value(3));
            sq.addBindValue(wq.value(4));
            sq.addBindValue(wq.value(5));

            if (!sq.exec()) {
                printFailed();
                return dbError(sq.lastError());
            }
            logEntries++;
        }
        printDone(tr("%n entrie(s)", "", logEntries));
    } else {
        printFailed();
        return dbError(sq.lastError());
    }



    printStatus(tr("Updating domain data"));

    auto domainIt = domainPrefixId.constBegin();
    while (domainIt != domainPrefixId.constEnd()) {
        quint32 users;
        quint32 quotaused;
        const quint32 domainId = domainIt.value();
        if (sq.prepare(QStringLiteral("SELECT COUNT(*) FROM accountuser WHERE domain_id = ?"))) {
            sq.addBindValue(domainId);
            if (sq.exec() && sq.next()) {
                users = sq.value(0).value<quint32>();
            } else {
                printFailed();
                return dbError(sq.lastError());
            }
        } else {
            printFailed();
            return dbError(sq.lastError());
        }
        if (sq.prepare(QStringLiteral("SELECT SUM(quota) FROM accountuser WHERE domain_id = ?"))) {
            sq.addBindValue(domainId);
            if (sq.exec() && sq.next()) {
                quotaused = sq.value(0).value<quint32>();
            } else {
                printFailed();
                return dbError(sq.lastError());
            }
        } else {
            printFailed();
            return dbError(sq.lastError());
        }
        if (sq.prepare(QStringLiteral("UPDATE domain SET domainquotaused = ?, accountcount = ? WHERE id = ?"))) {
            sq.addBindValue(quotaused);
            sq.addBindValue(users);
            sq.addBindValue(domainId);
            if (!sq.exec()) {
                printFailed();
                return dbError(sq.lastError());
            }
        } else {
            printFailed();
            return dbError(sq.lastError());
        }
        domainIt++;
    }

    printDone();


    printDesc(QStringList({
                              QString(),
                              tr("Skaffari uses PBKDF2 to secure administrator passwords. Because this is different to the way web-cyradm stores administrator passwords, the password for every administrator has to be reset. In the following you will be asked for a new password for every imported administrator."),
                              QString()
                          }));

    auto adminIt = adminNameIds.constBegin();
    while (adminIt != adminNameIds.constEnd()) {
        const QString pw = readString(tr("Password for %1").arg(adminIt.key()), QString());
        const QByteArray pwEnc = Cutelyst::CredentialPassword::createPassword(pw.toUtf8(), adminPwAlgorithm, adminPwRounds, 24, 27);
        printStatus(tr("Setting password for administrator %1").arg(adminIt.key()));
        if (sq.prepare(QStringLiteral("UPDATE adminuser SET password = ? WHERE id = ?"))) {
            sq.addBindValue(pwEnc);
            sq.addBindValue(adminIt.value());
            if (!sq.exec()) {
                printFailed();
                return dbError(sq.lastError());
            }
            printDone();
        } else {
            printFailed();
            return dbError(sq.lastError());
        }
        adminIt++;
    }

    printStatus(tr("Writing configuration to file"));
    QSettings settings(m_iniFile.absoluteFilePath(), QSettings::IniFormat);

    settings.beginGroup(QStringLiteral("Accounts"));
    settings.setValue(QStringLiteral("pwmethod"), static_cast<quint8>(accountsPwMethod));
    settings.setValue(QStringLiteral("pwalgorithm"), static_cast<quint8>(accountsPwAlgo));
    settings.setValue(QStringLiteral("pwrounds"), accountsPwRounds);
    settings.setValue(QStringLiteral("pwminlength"), accountsPwMinLength);
    settings.endGroup();

    settings.beginGroup(QStringLiteral("Admins"));
    settings.setValue(QStringLiteral("pwalgorithm"), static_cast<quint8>(adminPwAlgorithm));
    settings.setValue(QStringLiteral("pwrounds"), adminPwRounds);
    settings.setValue(QStringLiteral("pwminlength"), adminPwMinLength);
    settings.endGroup();

    settings.beginGroup(QStringLiteral("Database"));
    settings.setValue(QStringLiteral("type"), sdbtype);
    settings.setValue(QStringLiteral("host"), sdbhost);
    settings.setValue(QStringLiteral("port"), sdbport);
    settings.setValue(QStringLiteral("name"), sdbname);
    settings.setValue(QStringLiteral("user"), sdbuser);
    settings.setValue(QStringLiteral("password"), sdbpass);
    settings.endGroup();

    settings.beginGroup(QStringLiteral("IMAP"));
    settings.setValue(QStringLiteral("host"), imaphost);
    settings.setValue(QStringLiteral("port"), imapport);
    settings.setValue(QStringLiteral("user"), imapuser);
    settings.setValue(QStringLiteral("password"), imappass);
    settings.setValue(QStringLiteral("protocol"), imapprotocol);
    settings.setValue(QStringLiteral("encryption"), imapencryption);
    settings.setValue(QStringLiteral("fqun"), fqun);
    settings.setValue(QStringLiteral("domainasprefix"), domainAsPrefix);
    settings.setValue(QStringLiteral("createmailbox"), createmailbox);
    settings.setValue(QStringLiteral("peername"), imappeername);
    settings.endGroup();

    settings.beginGroup(QStringLiteral("Defaults"));
    settings.setValue(QStringLiteral("domainquota"), defaultDomainQuota);
    settings.setValue(QStringLiteral("quota"), defaultQuota);
    settings.setValue(QStringLiteral("maxaccounts"), defaultMaxAccounts);
    settings.setValue(QStringLiteral("language"), defaultLang);
    settings.setValue(QStringLiteral("timezone"), defaultTimezone);
    settings.endGroup();

    settings.sync();

    switch (settings.status()) {
    case QSettings::NoError:
        printDone();
        break;
    case QSettings::AccessError:
        printFailed();
        return fileError(tr("Failed to write configuration to file."));
    case QSettings::FormatError:
        printFailed();
        return configError(tr("Failed to write configuration to file."));
    default:
        printFailed();
        return error(tr("Unknown error occured while writing configuration to file."));
    }

    printSuccess(tr("Successfully configured Skaffari and imported web-cyradm data."));

    return 0;
}



QString WebCyradmImporter::getArrayEntry(const QString &array, const QString &key) const
{
    QString result;
    if (!array.isEmpty()) {
        QRegularExpression regex(QStringLiteral("\\W?%1\\W?\\s*=>\\s*\\W?([\\w.-_]+)\\W?").arg(key));
        QRegularExpressionMatch match = regex.match(array);
        if (match.hasMatch()) {
            result = match.captured(1);
        }
    }
    return result;
}
