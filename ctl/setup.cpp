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

#include "setup.h"
#include <QSettings>
#include <QDir>
#include <QVersionNumber>
#include <Cutelyst/Plugins/Authentication/credentialpassword.h>
#include <QCryptographicHash>
#include <QStringList>
#include <QTimeZone>

#include "database.h"
#include "imap.h"
#include "../common/password.h"
#include "../common/config.h"

Setup::Setup(const QString &confFile) :
    m_confFile(confFile)
{

}


int Setup::exec() const
{
    printMessage(tr("Start to configure Skaffari."));

    printStatus(tr("Checking configuration file"));

    bool configExists = m_confFile.exists();
    if (configExists) {
        if (!m_confFile.isReadable()) {
            printFailed();
            return fileError(tr("Configuration file exists at %1 but is not readable.").arg(m_confFile.absoluteFilePath()));
        }

        if (!m_confFile.isWritable()) {
            printFailed();
            return fileError(tr("Configuration file exists at %1 but is not writable.").arg(m_confFile.absoluteFilePath()));
        }
        printDone(tr("Found"));
        printMessage(tr("Using existing configuration file at %1.").arg(m_confFile.absoluteFilePath()));

    } else {

        QDir confDir = m_confFile.absoluteDir();
        QFileInfo confDirInfo(confDir.absolutePath());
        if (!confDir.exists() && !confDir.mkpath(confDir.absolutePath())) {
            printFailed();
            return fileError(tr("Failed to create configuation directory at %1.").arg(confDir.absolutePath()));
        } else if (confDir.exists() && !confDirInfo.isWritable()) {
            printFailed();
            return fileError(tr("Can not write to configuration directory at %1.").arg(confDir.absolutePath()));
        }

        printDone(tr("Created"));
        printMessage(tr("Creating configuration file at %1.").arg(m_confFile.absoluteFilePath()));
    }


    QSettings os(m_confFile.absoluteFilePath(), QSettings::IniFormat);
    os.beginGroup(QStringLiteral("Database"));
    QString dbhost = os.value(QStringLiteral("host"), QStringLiteral("localhost")).toString();
    QString dbname = os.value(QStringLiteral("name"), QStringLiteral("maildb")).toString();
    QString dbpass = os.value(QStringLiteral("password")).toString();
    QString dbtype = os.value(QStringLiteral("type"), QStringLiteral("QMYSQL")).toString();
    QString dbuser = os.value(QStringLiteral("user"), QStringLiteral("skaffari")).toString();
    quint16 dbport = os.value(QStringLiteral("port"), 3306).value<quint16>();
    os.endGroup();

    Database db;

    bool dbaccess = false;
    if (configExists && !dbpass.isEmpty()) {
        printTable({
                       {tr("Type"), dbtype},
                       {tr("Host"), dbhost},
                       {tr("Port"), QString::number(dbport)},
                       {tr("Name"), dbname},
                       {tr("User"), dbuser},
                       {tr("Password"), QStringLiteral("********")}
                   }, tr("Database settings"));
        printStatus(tr("Establishing database connection"));
        dbaccess = db.open(dbtype, dbhost, dbport, dbname, dbuser, dbpass);
        if (dbaccess) {
            printDone();
        } else {
            printFailed();
        }
    }

    if (!dbaccess) {
        printDesc(tr("Please enter the data to connect to your database system."));

        dbtype = readString(tr("DB Type"),
                            dbtype,
                            QStringList({
                                            tr("The type of database you are using, identified by the Qt driver name."),
                                            tr("See %1 for a list of drivers supported by Qt.").arg(QStringLiteral("http://doc.qt.io/qt-5/sql-driver.html")),
                                            tr("Currently supported by Skaffari: %1").arg(QStringLiteral("QMYSQL"))}),
                            QStringList({QStringLiteral("QMYSQL")}));
        printf("%s\n", dbtype.toUtf8().constData());
        dbhost = readString(tr("DB Host"),
                            dbhost,
                            QStringList({
                                            tr("The host your database server is running on. By default this is the local host."),
                                            tr("You can use localhost, a remote host identified by hostname or IP address or an absolute path to a local socket file.")
                                        }));
        if (dbhost[0] != QLatin1Char('/')) {
            dbport = readPort(tr("DB Port"), dbport, QStringList({tr("The port your database server is listening on.")}));
        }
        dbname = readString(tr("DB Name"), dbname, QStringList({tr("The name of the database used for Skaffari and the SMTP and POP/IMAP servers.")}));
        dbuser = readString(tr("DB User"), dbuser, QStringList({tr("The name of the database user that has read and write access to the database defined in the previous step.")}));
        dbpass = readString(tr("DB Password"), dbpass, QStringList({tr("The password of the database user defined in the previous step.")}));

        printStatus(tr("Establishing database connection"));

        if (!db.open(dbtype, dbhost, dbport, dbname, dbuser, dbpass)) {
            printFailed();
            return dbError(db.lastDbError());
        } else {
            printDone();
        }

        os.beginGroup(QStringLiteral("Database"));
        os.setValue(QStringLiteral("type"), dbtype);
        os.setValue(QStringLiteral("host"), dbhost);
        os.setValue(QStringLiteral("port"), dbport);
        os.setValue(QStringLiteral("name"), dbname);
        os.setValue(QStringLiteral("user"), dbuser);
        os.setValue(QStringLiteral("password"), dbpass);
        os.endGroup();
        os.sync();
    }

    printStatus(tr("Checking database layout"));

    const QVersionNumber installedVersion = db.installedVersion();
    if (!installedVersion.isNull()) {
        printDone(installedVersion.toString());
    } else {
        printFailed();
        printStatus(tr("Performing database installation"));
        if (!db.installDatabase()) {
            printFailed();
            return dbError(db.lastDbError());
        } else {
            printDone();
        }
    }

    printStatus(tr("Searching for available admin accounts"));
    const uint adminCount = db.checkAdmin();
    if (adminCount > 0) {
        printDone(tr("Found %1").arg(adminCount));
    } else {
        printFailed(tr("None"));
    }

    //: %1 will be substituted by a link to pbkdf2test GitHub repo, %2 will be substituded by an URL to a Wikipedia page about PBKDF2
    const QString pbkdf2Desc = tr("Skaffari uses PBKDF2 to secure the administrator passwords. PBKDF2 can use different hashing algorithms and iteration counts to produce a derived key and to increase the cost for the derivation. To better secure your administartor passwords you should use values that lead to a time consumption of around 0.5s on your system for creating the derived key. This might be a good compromise between security and user experience. To test different settings with the PBKDF2 implementation of Cutelyst/Skaffari you can use %1. See %2 to learn more about PBKDF2.").arg(QStringLiteral("https://github.com/Buschtrommel/pbkdf2test"), tr("https://en.wikipedia.org/wiki/PBKDF2"));

    const QStringList pbkdf2AlgoDesc({
                                         tr("The PBKDF2 implementation of Cutelyst/Skaffari supports the following hashing algorithms:"),
                                         QStringLiteral(" 3: SHA-224"),
                                         QStringLiteral(" 4: SHA-256"),
                                         QStringLiteral(" 5: SHA-384"),
                                         QStringLiteral(" 6: SHA-512"),
                                         QStringLiteral(" 7: SHA3-224"),
                                         QStringLiteral(" 8: SHA3-256"),
                                         QStringLiteral(" 9: SHA3-384"),
                                         QStringLiteral("10: SHA3-512"),
                                     });

    const QStringList pbkdf2IterDesc({tr("The iteration count is used to increase the cost for deriving the key from the password.")});

    const QStringList adminMinPwDesc({tr("Required minimum length for administrator passwords.")});

    os.beginGroup(QStringLiteral("Admins"));
    QCryptographicHash::Algorithm adminPasswordMethod = static_cast<QCryptographicHash::Algorithm>(os.value(QStringLiteral("pwmethod"), 4).toInt());
    quint32 adminPasswordRounds = os.value(QStringLiteral("pwrounds"), 32000).value<quint32>();
    quint8 adminPasswordMinLength = os.value(QStringLiteral("pwminlength"), 8).value<quint8>();
    os.endGroup();

    if (adminCount == 0) {
        printDesc(tr("Please configure your admin password settings and create a new admin user."));
        printDesc(QString());

        printDesc(pbkdf2Desc);
        printMessage(QString());

        adminPasswordMethod = static_cast<QCryptographicHash::Algorithm>(readChar(tr("PBKDF2 algorithm"), static_cast<quint8>(adminPasswordMethod), pbkdf2AlgoDesc, QList<quint8>({3,4,5,6,7,8,9,10})));
        adminPasswordRounds = readInt(tr("PBKDF2 iterations"), adminPasswordRounds, pbkdf2IterDesc);
        adminPasswordMinLength = readChar(tr("Password minimum length"), adminPasswordMinLength, adminMinPwDesc);

        printMessage(QString());

        printDesc(tr("Create a super user administrator account to login into your Skaffari installation. This account can create further super user accounts and domain administrators."));

        const QString adminUser = readString(tr("User name"), QStringLiteral("admin"));
        const QString adminPass = readString(tr("Password"), QString());

        const QByteArray pw = Cutelyst::CredentialPassword::createPassword(adminPass.toUtf8(), adminPasswordMethod, adminPasswordRounds, 24, 27);

        printStatus(tr("Creating new admin account in database"));
        if (!db.setAdmin(adminUser, pw)) {
            printFailed();
            return dbError(db.lastDbError());
        }

        printDone();

        os.beginGroup(QStringLiteral("Admins"));
        os.setValue(QStringLiteral("pwmethod"), static_cast<int>(adminPasswordMethod));
        os.setValue(QStringLiteral("pwrounds"), adminPasswordRounds);
        os.setValue(QStringLiteral("pwminlength"), adminPasswordMinLength);
        os.endGroup();
        os.sync();

    } else if ((adminCount == 0) || readBool(tr("Do you want to set the admin password settings?"), false)) {

        printDesc(pbkdf2Desc);
        printMessage(QString());

        adminPasswordMethod = static_cast<QCryptographicHash::Algorithm>(readChar(tr("PBKDF2 algorithm"), static_cast<quint8>(adminPasswordMethod), pbkdf2AlgoDesc, QList<quint8>({3,4,5,6,7,8,9,10})));
        adminPasswordRounds = readInt(tr("PBKDF2 iterations"), adminPasswordRounds, pbkdf2IterDesc);
        adminPasswordMinLength = readChar(tr("Password minimum length"), adminPasswordMinLength, adminMinPwDesc);

        os.beginGroup(QStringLiteral("Admins"));
        os.setValue(QStringLiteral("pwmethod"), static_cast<int>(adminPasswordMethod));
        os.setValue(QStringLiteral("pwrounds"), adminPasswordRounds);
        os.setValue(QStringLiteral("pwminlength"), adminPasswordMinLength);
        os.endGroup();
        os.sync();
    }


    if (!configExists || readBool(tr("Do you want to set the user password settings?"), false)) {

        printDesc(tr("Skaffari is designed to work together with pam_mysql. Because of that, user account passwords are stored in a different format than the admin accounts. The format of the user account passwords has to be compatible to the password hashing methods supported by pam_mysql. See the README of your pam_mysql installation for the supported encryption methods."));
        printDesc(QString());

        os.beginGroup(QStringLiteral("Accounts"));
        Password::Type accountsPwType = static_cast<Password::Type>(os.value(QStringLiteral("pwtype"), 1).value<quint8>());
        Password::Method accountsPwMethod = static_cast<Password::Method>(os.value(QStringLiteral("pwmethod"), 0).value<quint8>());
        quint32 accountsPwRounds = os.value(QStringLiteral("pwrounds"), 5000).value<quint32>();
        quint8 accountsPwMinLength = os.value(QStringLiteral("pwminlength"), 8).value<quint8>();
        os.endGroup();

        accountsPwType = static_cast<Password::Type>(readChar(tr("Encryption type"),
                                                              static_cast<quint8>(accountsPwType),
                                                              QStringList({
                                                                              tr("The basic method to encrypt the user's password. Some methods support further settings that will be defined in the next steps. If it is possible to use for you, the recommended type is to use the crypt(3) function, because it supports modern hashing algorithms together with salts and an extensible storage format. The other encryption methods are there for backwards compatibility."),
                                                                              tr("Supported methods:"),
                                                                              tr("0: no encryption - highly discouraged"),
                                                                              tr("1: crypt(3) function - recommended"),
                                                                              tr("2: MySQL password function"),
                                                                              tr("3: plain hex MD5 - not recommended"),
                                                                              tr("4: plain hex SHA1 - not recommended")
                                                                          }),
                                                              QList<quint8>({0,1,2,3,4})));
        if (accountsPwType == Password::Crypt) {
            accountsPwMethod = static_cast<Password::Method>(readChar(tr("Encryption method"),
                                                                      static_cast<quint8>(accountsPwMethod),
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
        } else if (accountsPwType == Password::MySQL) {
            accountsPwMethod = static_cast<Password::Method>(readChar(tr("Encryption method"),
                                                                      accountsPwMethod,
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

        if (accountsPwType == Password::Crypt) {
            if ((accountsPwMethod == Password::CryptSHA256) || (accountsPwMethod == Password::CryptSHA512) || (accountsPwMethod == Password::CryptBcrypt)) {
                QStringList accountsPwRoundsDesc = (accountsPwMethod == Password::CryptBcrypt) ? QStringList(tr("Crypt(3) with bcrypt supports an iteration count to increase the time cost for creating the derived key. The iteration count passed to the crypt function is the base-2 logarithm of the actual iteration count. Supported values are between 4 and 31.")) : QStringList(tr("Crypt(3) with SHA-256 and SHA-512 supports an iteration count from 1000 to 999999999. The iterations are used to increase the time cost for creating the derived key."));
                if ((accountsPwMethod == Password::CryptBcrypt) && (accountsPwRounds > 31)) {
                    accountsPwRounds = 12;
                }

                accountsPwRounds = readInt(tr("Encryption rounds"), accountsPwRounds, accountsPwRoundsDesc);
            }
        }

        accountsPwMinLength = readChar(tr("Password minimum length"), accountsPwMinLength, QStringList(tr("The required minimum length for user account passwords created or changed via Skaffari.")));

        os.beginGroup(QStringLiteral("Accounts"));
        os.setValue(QStringLiteral("pwtype"), static_cast<quint8>(accountsPwType));
        os.setValue(QStringLiteral("pwmethod"), static_cast<quint8>(accountsPwMethod));
        os.setValue(QStringLiteral("pwrounds"), accountsPwRounds);
        os.setValue(QStringLiteral("pwminlength"), accountsPwMinLength);
        os.endGroup();
        os.sync();
    }

    printStatus(tr("Checking for IMAP admin account"));
    QString cyrusAdmin = db.checkCyrusAdmin();
    if (!cyrusAdmin.isEmpty()) {
        printDone(cyrusAdmin);
    } else {
        //: status for not finding any IMAP admin users
        printFailed(tr("None"));

        printDesc(tr("The IMAP admin user is defined in the imapd.conf with the admins: key. The admin user name you enter here has also to be defined in the imapd.conf. It is used to perform administrative tasks on the IMAP server, like setting quotas and creating/deleting mailboxes and mailbox folders. The user will be created in the database defined for Skafarri."));

        cyrusAdmin = readString(tr("IMAP admin user"), QStringLiteral("cyrus"));
        Password cyrusAdminPw(readString(tr("IMAP admin password"), QString()));

        printStatus(tr("Creating Cyrus admin user in database"));
        os.beginGroup(QStringLiteral("Accounts"));
        Password::Type accountsPwType = static_cast<Password::Type>(os.value(QStringLiteral("pwtype"), 1).value<quint8>());
        Password::Method accountsPwMethod = static_cast<Password::Method>(os.value(QStringLiteral("pwmethod"), 0).value<quint8>());
        quint32 accountsPwRounds = os.value(QStringLiteral("pwrounds"), 5000).value<quint32>();
        os.endGroup();
        QByteArray cyrusAdminPwEnc = cyrusAdminPw.encrypt(accountsPwType, accountsPwMethod, accountsPwRounds);
        if (cyrusAdminPwEnc.isEmpty()) {
            printFailed();
            return configError(tr("Failed to encrypt Cyrus admin password."));
        }

        if (!db.setCryusAdmin(cyrusAdmin, cyrusAdminPwEnc)) {
            printFailed();
            return dbError(db.lastDbError());
        }

        printDone();
    }


    os.beginGroup(QStringLiteral("IMAP"));
    QString imapuser = os.value(QStringLiteral("user"), cyrusAdmin).toString();
    QString imappass = os.value(QStringLiteral("password")).toString();
    QString imaphost = os.value(QStringLiteral("host"), QStringLiteral("localhost")).toString();
    quint16 imapport = os.value(QStringLiteral("port"), 143).value<quint16>();
    quint8 imapprotocol = os.value(QStringLiteral("protocol"), 2).value<quint8>();
    quint8 imapencryption = os.value(QStringLiteral("encryption"), 1).value<quint8>();
    os.endGroup();

    bool imapaccess = false;
    Imap imap;

    if (configExists && !imappass.isEmpty()) {
        printTable({
                       {tr("Host"), imaphost},
                       {tr("Port"), QString::number(imapport)},
                       {tr("Protocol"), Imap::networkProtocolToString(imapprotocol)},
                       {tr("Encryption"), Imap::encryptionTypeToString(imapencryption)},
                       {tr("User"), imapuser},
                       {tr("Password"), QStringLiteral("********")}
                   }, tr("IMAP settings"));

        printStatus(tr("Establishing IMAP connection"));
        imap.setHost(imaphost);
        imap.setPort(imapport);
        imap.setUser(imapuser);
        imap.setPassword(imappass);
        imap.setProtocol(static_cast<QAbstractSocket::NetworkLayerProtocol>(imapprotocol));
        imap.setEncryptionType(static_cast<Imap::EncryptionType>(imapencryption));
        imapaccess = imap.login();
        if (Q_LIKELY(imapaccess)) {
            if (Q_UNLIKELY(!imap.logout())) {
                printFailed();
                return imapError(imap.lastError());
            }
            printDone();
        } else {
            printFailed();
        }
    }

    if (imappass.isEmpty() || !imapaccess || readBool(tr("Do you want to set the IMAP connection settings?"), false)) {

        if (!imapaccess) {
            printDesc(tr("Connection to your IMAP server failed. Please reenter your connection data."));
        } else {
            printDesc(tr("Please enter the data to connect to your IMAP server."));
        }
        printDesc(QString());
        printDesc(tr("Connection to your IMAP server as admin is used to perform tasks like setting quotas and creating/deleting mailboxes and folders. The user account has to be defined as admin in the imapd.conf file in the admins: key."));
        printDesc(QString());

        imaphost = readString(tr("IMAP Host"), imaphost, QStringList(tr("The host the IMAP server is running on.")));
        imapport = readPort(tr("IMAP Port"), imapport, QStringList(tr("The port your IMAP server is listening on.")));
        imapuser = readString(tr("IMAP User"), imapuser, QStringList(tr("The user name of the IMAP admin user.")));
        imappass = readString(tr("IMAP Password"), QString(), QStringList(tr("Password for the IMAP admin user.")));
        imapprotocol = readChar(tr("IMAP Protocol"),
                                imapprotocol,
                                QStringList({
                                                tr("The network layer protocol to connect to the IMAP server."),
                                                tr("Available protocols:"),
                                                QStringLiteral("0: IPv4"),
                                                QStringLiteral("1: IPv6"),
                                                tr("2: Either IPv4 or IPv6")
                                            }),
                                QList<quint8>({0,1,2}));
        imapencryption = readChar(tr("IMAP Encryption"),
                                  imapencryption,
                                  QStringList({
                                                  tr("The method to encrypt the connection to the IMAP server."),
                                                  tr("Available methods:"),
                                                  tr("0: unsecured"),
                                                  QStringLiteral("1: StartTLS"),
                                                  QStringLiteral("2: IMAPS")
                                              }),
                                  QList<quint8>({0,1,2}));

        printStatus(tr("Establishing IMAP connection"));
        imap.setHost(imaphost);
        imap.setPort(imapport);
        imap.setUser(imapuser);
        imap.setPassword(imappass);
        imap.setProtocol(static_cast<QAbstractSocket::NetworkLayerProtocol>(imapprotocol));
        imap.setEncryptionType(static_cast<Imap::EncryptionType>(imapencryption));
        imapaccess = imap.login();
        if (Q_LIKELY(imapaccess)) {
            if (Q_UNLIKELY(!imap.logout())) {
                printFailed();
                return imapError(imap.lastError());
            }
            printDone();
        } else {
            printFailed();
            return imapError(imap.lastError());
        }

        os.beginGroup(QStringLiteral("IMAP"));
        os.setValue(QStringLiteral("host"), imaphost);
        os.setValue(QStringLiteral("port"), imapport);
        os.setValue(QStringLiteral("user"), imapuser);
        os.setValue(QStringLiteral("password"), imappass);
        os.setValue(QStringLiteral("protocol"), imapprotocol);
        os.setValue(QStringLiteral("encryption"), imapencryption);
        os.endGroup();
        os.sync();

    }

    if (!configExists || readBool(tr("Do you want to set the IMAP behavior settings?"), false)) {

        printDesc(tr("Depending on the settings of your IMAP server, you can set different behavior of Skaffari when managing your email accounts."));
        printDesc(QString());

        os.beginGroup(QStringLiteral("IMAP"));
        bool domainAsPrefix = os.value(QStringLiteral("domainasprefix"), false).toBool();
        bool fqun = os.value(QStringLiteral("fqun"), false).toBool();
        quint8 createmailbox = os.value(QStringLiteral("createmailbox"), 3).value<quint8>();
        os.endGroup();

        domainAsPrefix = readBool(tr("Domain as prefix"),
                                  domainAsPrefix,
                                  QStringList({
                                                  tr("If you want to use email addresses with dots in them such as john.doe@example.com you can activate this option."),
                                                  tr("NOTE: you have to set the following line in your imapd.conf file:"),
                                                  QStringLiteral("unixhierarchysep: yes")
                                              }));
        fqun = readBool(tr("Fully qualified user name"),
                        fqun,
                        QStringList({
                                        tr("If you wish to use user names like email addresses you can activate this option."),
                                        tr("NOTE: you also have to add this lines to your imapd.conf file:"),
                                        QStringLiteral("unixhierarchysep: yes"),
                                        QStringLiteral("virtdomains: yes")
                                    }));
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

        os.beginGroup(QStringLiteral("IMAP"));
        os.setValue(QStringLiteral("domainasprefix"), domainAsPrefix);
        os.setValue(QStringLiteral("fqun"), fqun);
        os.setValue(QStringLiteral("createmailbox"), createmailbox);
        os.endGroup();
        os.sync();
    }


    if (!configExists || readBool(tr("Do you want to set the default values?"), false)) {

        os.beginGroup(QStringLiteral("Defaults"));
        QString defaultLang = os.value(QStringLiteral("language"), QStringLiteral("en")).toString();
        quint32 defaultQuota = os.value(QStringLiteral("quota"), 10000).value<quint32>();
        quint32 defaultDomainQuota = os.value(QStringLiteral("domainquota"), 100000).value<quint32>();
        quint32 defaultMaxAccounts = os.value(QStringLiteral("maxaccounts"), 1000).value<quint32>();
        QString defaultTimezone = os.value(QStringLiteral("timezone"), QStringLiteral("UTC")).toString();
        os.endGroup();

        defaultLang = readString(tr("Default language"), defaultLang, QStringList({tr("The default language will be used as fallback language if the user's language is not set or can not be determined."), tr("Currently supported languages: %1").arg(QStringList(SKAFFARI_SUPPORTED_LANGS).join(QChar(QChar::Space)))}), QStringList(SKAFFARI_SUPPORTED_LANGS));
        defaultTimezone = readString(tr("Default timezone"), QStringLiteral("UTC"), QStringList(tr("Default timezone for newly created administrators and as fallback option. The timezone will be used to show localized date and time values. Please enter a valid IANA timezone ID.")));
        while (!QTimeZone::isTimeZoneIdAvailable(defaultTimezone.toUtf8())) {
            defaultTimezone = readString(tr("Default timezone"), QStringLiteral("UTC"), QStringList(tr("Default timezone for newly created administrators and as fallback option. The timezone will be used to show localized date and time values. Please enter a valid IANA timezone ID.")));
        }
        defaultQuota = readInt(tr("Default quota"), defaultQuota, QStringList(tr("The default quota in KiB for new accounts when creating a new domain. This can be changed when creating a new domain or editing an exisiting one.")));
        defaultDomainQuota = readInt(tr("Default domain quota"), defaultDomainQuota, QStringList(tr("The default domain quota in KiB when creating new domains. This can be changed when creating a new domain or editing an exisiting one.")));
        defaultMaxAccounts = readInt(tr("Default max accounts"), defaultMaxAccounts, QStringList(tr("The default number of maximum accounts for creating new domains. This can be changed when creating a new domain or editing an exisiting one.")));

        os.beginGroup(QStringLiteral("Defaults"));
        os.setValue(QStringLiteral("language"), defaultLang);
        os.setValue(QStringLiteral("quota"), defaultQuota);
        os.setValue(QStringLiteral("domainquota"), defaultDomainQuota);
        os.setValue(QStringLiteral("maxaccounts"), defaultMaxAccounts);
        os.endGroup();
        os.sync();
    }

    printSuccess(tr("Skaffari setup was successful."));

    return 0;
}

