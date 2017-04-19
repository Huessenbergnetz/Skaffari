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
#include <cstdio>
#include <iostream>
#include <string>
#include <QVersionNumber>
#include <Cutelyst/Plugins/Authentication/credentialpassword.h>
#include <QCryptographicHash>

#include "database.h"
#include "imap.h"
#include "../common/password.h"

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
        printDone();
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

        printDone();
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
        if (configExists) {
            printMessage(tr("Failed to establish database connection. Please reenter your connection data."));
        } else {
            printMessage(tr("Please enter the data to connect to your database system."));
        }

        dbtype = readString(tr("DB Type"), dbtype);
        dbhost = readString(tr("DB Host"), dbhost);
        if (dbhost[0] != QChar('/')) {
            dbport = readPort(tr("DB Port"), dbport);
        }
        dbname = readString(tr("DB Name"), dbname);
        dbuser = readString(tr("DB User"), dbuser);
        dbpass = readString(tr("DB Password"), dbpass);

        printStatus(tr("Establishing database connection"));

        if (!db.open()) {
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

    if (adminCount == 0) {
        printMessage(tr("Please configure your admin password settings and create a new admin user."));

        os.beginGroup(QStringLiteral("Admins"));
        QCryptographicHash::Algorithm adminPasswordMethod = static_cast<QCryptographicHash::Algorithm>(os.value(QStringLiteral("pwmethod"), 4).toInt());
        quint32 adminPasswordRounds = os.value(QStringLiteral("pwrounds"), 32000).value<quint32>();
        quint8 adminPasswordMinLength = os.value(QStringLiteral("pwminlength"), 8).value<quint8>();
        os.endGroup();

        adminPasswordMethod = static_cast<QCryptographicHash::Algorithm>(readInt(tr("Password method"), static_cast<quint32>(adminPasswordMethod)));
        adminPasswordRounds = readInt(tr("Password rounds"), adminPasswordRounds);
        adminPasswordMinLength = readChar(tr("Minimum length"), adminPasswordMinLength);

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

    } else if ((adminCount == 0) || readBool(tr("Do you want to reset your admin password settings?"), false)) {

        os.beginGroup(QStringLiteral("Admins"));
        QCryptographicHash::Algorithm adminPasswordMethod = static_cast<QCryptographicHash::Algorithm>(os.value(QStringLiteral("pwmethod"), 4).toInt());
        quint32 adminPasswordRounds = os.value(QStringLiteral("pwrounds"), 32000).value<quint32>();
        quint8 adminPasswordMinLength = os.value(QStringLiteral("pwminlength"), 8).value<quint8>();
        os.endGroup();

        adminPasswordMethod = static_cast<QCryptographicHash::Algorithm>(readInt(tr("Password method"), static_cast<quint32>(adminPasswordMethod)));
        adminPasswordRounds = readInt(tr("Password rounds"), adminPasswordRounds);
        adminPasswordMinLength = readChar(tr("Minimum length"), adminPasswordMinLength);

        os.beginGroup(QStringLiteral("Admins"));
        os.setValue(QStringLiteral("pwmethod"), static_cast<int>(adminPasswordMethod));
        os.setValue(QStringLiteral("pwrounds"), adminPasswordRounds);
        os.setValue(QStringLiteral("pwminlength"), adminPasswordMinLength);
        os.endGroup();
        os.sync();
    }


    if (!configExists || readBool(tr("Do you want to reset the user password settings?"), false)) {

        os.beginGroup(QStringLiteral("Accounts"));
        Password::Type accountsPwType = static_cast<Password::Type>(os.value(QStringLiteral("pwtype"), 1).value<quint8>());
        Password::Method accountsPwMethod = static_cast<Password::Method>(os.value(QStringLiteral("pwmethod"), 0).value<quint8>());
        quint32 accountsPwRounds = os.value(QStringLiteral("pwrounds"), 5000).value<quint32>();
        quint8 accountsPwMinLength = os.value(QStringLiteral("pwminlength"), 8).value<quint8>();
        os.endGroup();

        accountsPwType = static_cast<Password::Type>(readInt(tr("Encryption type"), accountsPwType));
        if (accountsPwType == Password::Crypt) {
            accountsPwMethod = static_cast<Password::Method>(readChar(tr("Encryption method"), accountsPwMethod));
        } else if (accountsPwType == Password::MySQL) {
            accountsPwMethod = static_cast<Password::Method>(readChar(tr("Encryption method"), accountsPwMethod));
        }

        if (accountsPwType == Password::Crypt) {
            if ((accountsPwMethod == Password::CryptSHA256) || (accountsPwMethod == Password::CryptSHA512) || (accountsPwMethod == Password::CryptBcrypt)) {
                if ((accountsPwMethod == Password::CryptBcrypt) && (accountsPwRounds > 31)) {
                    accountsPwRounds = 12;
                }

                accountsPwRounds = readInt(tr("Encryption rounds"), accountsPwRounds);
            }
        }

        accountsPwMinLength = readChar(tr("Password min. length"), accountsPwMinLength);

        os.beginGroup(QStringLiteral("Accounts"));
        os.setValue(QStringLiteral("pwtype"), static_cast<quint8>(accountsPwType));
        os.setValue(QStringLiteral("pwmethod"), static_cast<quint8>(accountsPwMethod));
        os.setValue(QStringLiteral("pwrounds"), accountsPwRounds);
        os.setValue(QStringLiteral("pwminlength"), accountsPwMinLength);
        os.endGroup();
        os.sync();
    }

    printStatus(tr("Checking for Cyrus IMAP admin account"));
    QString cyrusAdmin = db.checkCyrusAdmin();
    if (!cyrusAdmin.isEmpty()) {
        printDone(cyrusAdmin);
    } else {
        printFailed(tr("None"));

        printMessage(tr("Create the Cyrus IMAP server admin account."));
        cyrusAdmin = readString(tr("Cyrus admin user"), QStringLiteral("cyrus"));
        Password cyrusAdminPw(readString(tr("Cyrus admin password"), QString()));

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
        if (imapaccess) {
            printDone();
            imap.logout();
        } else {
            printFailed();
        }
    }

    if (imappass.isEmpty() || !imapaccess || readBool(tr("Do you want to reset your IMAP connection settings?"), false)) {

        if (!imapaccess) {
            printMessage(tr("Connection to your IMAP server failed. Please reenter your connection data."));
        } else {
            printMessage(tr("Please enter the data to connect to your IMAP server."));
        }

        imaphost = readString(tr("IMAP Host"), imaphost);
        imapport = readPort(tr("IMAP Port"), imapport);
        imapuser = readString(tr("IMAP User"), imapuser);
        imappass = readString(tr("IMAP Password"), imappass);
        imapprotocol = readChar(tr("IMAP Protocol"), imapprotocol);
        imapencryption = readChar(tr("IMAP Encryption"), imapencryption);

        printStatus(tr("Establishing IMAP connection"));
        imap.setHost(imaphost);
        imap.setPort(imapport);
        imap.setUser(imapuser);
        imap.setPassword(imappass);
        imap.setProtocol(static_cast<QAbstractSocket::NetworkLayerProtocol>(imapprotocol));
        imap.setEncryptionType(static_cast<Imap::EncryptionType>(imapencryption));
        imapaccess = imap.login();
        if (imapaccess) {
            printDone();
            imap.logout();
        } else {
            printFailed();
            return imapError(tr("Failed to establish connection to IMAP server."));
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

    if (!configExists || readBool(tr("Set other IMAP settings?"), false)) {

        os.beginGroup(QStringLiteral("IMAP"));
        bool domainAsPrefix = os.value(QStringLiteral("domainasprefix"), false).toBool();
        bool fqun = os.value(QStringLiteral("fqun"), false).toBool();
        quint8 createmailbox = os.value(QStringLiteral("createmailbox"), 2).value<quint8>();
        os.endGroup();

        domainAsPrefix = readBool(tr("Domain as prefix"), domainAsPrefix);
        fqun = readBool(tr("FQUN"), fqun);
        createmailbox = readChar(tr("Create mailboxes"), createmailbox);

        os.beginGroup(QStringLiteral("IMAP"));
        os.setValue(QStringLiteral("domainasprefix"), domainAsPrefix);
        os.setValue(QStringLiteral("fqun"), fqun);
        os.setValue(QStringLiteral("createmailbox"), createmailbox);
        os.endGroup();
        os.sync();
    }


    if (!configExists || readBool(tr("Set default values?"), false)) {

        os.beginGroup(QStringLiteral("Defaults"));
        QString defaultLang = os.value(QStringLiteral("language"), QStringLiteral("en")).toString();
        quint32 defaultQuota = os.value(QStringLiteral("quota"), 10000).value<quint32>();
        quint32 defaultDomainQuota = os.value(QStringLiteral("domainquota"), 100000).value<quint32>();
        quint32 defaultMaxAccounts = os.value(QStringLiteral("maxaccounts"), 1000).value<quint32>();
        os.endGroup();

        defaultLang = readString(tr("Default language"), defaultLang);
        defaultQuota = readInt(tr("Default quota"), defaultQuota);
        defaultDomainQuota = readInt(tr("Default domain quota"), defaultDomainQuota);
        defaultMaxAccounts = readInt(tr("Default max accounts"), defaultMaxAccounts);

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


QString Setup::readString(const QString &name, const QString &defaultVal, const QStringList &desc) const
{
    QString inputVal;

    printDesc(desc);

    while (inputVal.isEmpty()) {
        if (!defaultVal.isEmpty()) {
            printf("%s [%s]: ", qUtf8Printable(name), qUtf8Printable(defaultVal));
        } else {
            printf("%s: ", qUtf8Printable(name));
        }
        std::string in;
        std::getline(std::cin, in);
        inputVal = QString::fromStdString(in);
        if (inputVal.isEmpty() && !defaultVal.isEmpty()) {
            inputVal = defaultVal;
        }
        if (inputVal.isEmpty()) {
            printf("%s\n", qUtf8Printable(tr("Can not be empty.")));
        }
    }

    return inputVal;
}


quint16 Setup::readPort(const QString &name, quint16 defaultVal, const QStringList &desc) const
{
    quint16 inputVal = 0;

    printDesc(desc);

    printf("%s [%i]: ", qUtf8Printable(name), defaultVal);
    std::string in;
    std::getline(std::cin, in);
    QString _inputVal = QString::fromStdString(in);
    if (_inputVal.isEmpty()) {
        inputVal = defaultVal;
    } else {
        inputVal = _inputVal.toUInt();
    }

    return inputVal;
}


quint8 Setup::readChar(const QString &name, quint8 defaultVal, const QStringList &desc) const
{
    quint8 inputVal = 0;

    printDesc(desc);

    printf("%s [%i]: ", qUtf8Printable(name), defaultVal);
    std::string in;
    std::getline(std::cin, in);
    QString _inputVal = QString::fromStdString(in);
    if (_inputVal.isEmpty()) {
        inputVal = defaultVal;
    } else {
        inputVal = _inputVal.toUShort();
    }

    return inputVal;
}


quint32 Setup::readInt(const QString &name, quint32 defaultVal, const QStringList &desc) const
{
    quint32 inputVal = 0;

    printDesc(desc);

    printf("%s [%i]: ", qUtf8Printable(name), defaultVal);
    std::string in;
    std::getline(std::cin, in);
    QString _inputVal = QString::fromStdString(in);
    if (_inputVal.isEmpty()) {
        inputVal = defaultVal;
    } else {
        inputVal = _inputVal.toULong();
    }

    return inputVal;
}


bool Setup::readBool(const QString &name, bool defaultVal, const QStringList &desc) const
{
    bool retVal = false;

    printDesc(desc);

    static const QStringList posVals({
                                         //: short for yes
                                         tr("Y"),
                                         //: short for no
                                         tr("N"),
                                         tr("Yes"),
                                         tr("No")
                                     });
    QString inStr;
    const QString defAnswer = defaultVal ? tr("Yes") : tr("No");
    while (!posVals.contains(inStr, Qt::CaseInsensitive)) {
        printf("%s [%s]: ", qUtf8Printable(name), qUtf8Printable(defAnswer));
        std::string in;
        std::getline(std::cin, in);
        inStr = QString::fromStdString(in);
        if (inStr.isEmpty()) {
            inStr = defAnswer;
        }
    }

    retVal = ((inStr.compare(tr("Y"), Qt::CaseInsensitive) == 0) || (inStr.compare(tr("Yes"), Qt::CaseInsensitive) == 0));

    return retVal;
}
