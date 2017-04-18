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
    bool configExists = m_confFile.exists();
    if (configExists) {
        if (!m_confFile.isReadable()) {
            printf("%s\n", qUtf8Printable(tr("Configuration file exists at %1 but is not readable.").arg(m_confFile.absoluteFilePath())));
            return 1;
        }

        if (!m_confFile.isWritable()) {
            printf("%s\n", qUtf8Printable(tr("Configuration file exists at %1 but is not writable.").arg(m_confFile.absoluteFilePath())));
            return 1;
        }

        printf("%s\n", qUtf8Printable(tr("Using existing configuration file at %1.").arg(m_confFile.absoluteFilePath())));

    } else {

        QDir confDir = m_confFile.absoluteDir();
        QFileInfo confDirInfo(confDir.absolutePath());
        if (!confDir.exists() && !confDir.mkpath(confDir.absolutePath())) {
            printf("%s\n", qUtf8Printable(tr("Failed to create configuation directory at %1.").arg(confDir.absolutePath())));
            return 1;
        } else if (confDir.exists() && !confDirInfo.isWritable()) {
            printf("%s\n", qUtf8Printable(tr("Can not write to configuration directory at %1.").arg(confDir.absolutePath())));
            return 1;
        }

        printf("%s\n", qUtf8Printable(tr("Creating configuration file at %1.").arg(m_confFile.absoluteFilePath())));
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
    if (configExists) {
        dbaccess = db.open(dbtype, dbhost, dbport, dbname, dbuser, dbpass);
        if (dbaccess) {
            printf("%s\n", qUtf8Printable(tr("Successfully establisched database connection.")));
        }
    }

    if (!dbaccess) {
        if (configExists) {
            printf("%s\n", qUtf8Printable(tr("Failed to establish database connection. Please reenter your connection data.")));
        } else {
            printf("%s\n", qUtf8Printable(tr("Please enter the data to connect to your database system.")));
        }

        dbtype = readString(tr("DB Type"), dbtype);
        dbhost = readString(tr("DB Host"), dbhost);
        if (dbhost[0] != QChar('/')) {
            dbport = readPort(tr("DB Port"), dbport);
        }
        dbname = readString(tr("DB Name"), dbname);
        dbuser = readString(tr("DB User"), dbuser);
        dbpass = readString(tr("DB Password"), dbpass);

        printf("%s\n", qUtf8Printable(tr("Trying to establish database connection.")));

        if (!db.open()) {
            printf("%s\n", qUtf8Printable(tr("Failed to establish database connection. Aborting.")));
            return 1;
        }

        printf("%s\n", qUtf8Printable("Connection to database successfully established. Writing configuration to file."));

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

    const QVersionNumber installedVersion = db.installedVersion();
    if (!installedVersion.isNull()) {
        printf("%s\n", qUtf8Printable(tr("Found Skaffari database installation of version %1.").arg(installedVersion.toString())));
    } else {
        printf("%s\n", qUtf8Printable(tr("Performing database installation.")));
        if (!db.installDatabase()) {
            return 1;
        }
    }

    if (dbaccess && !db.checkAdmin()) {
        printf("%s\n", qUtf8Printable(tr("No admin users found in database. Please configure your admin password settings and create a new admin user.")));

        os.beginGroup(QStringLiteral("Admins"));
        QCryptographicHash::Algorithm adminPasswordMethod = static_cast<QCryptographicHash::Algorithm>(os.value(QStringLiteral("pwmethod"), 4).toInt());
        quint32 adminPasswordRounds = os.value(QStringLiteral("pwrounds"), 32000).value<quint32>();
        quint8 adminPasswordMinLength = os.value(QStringLiteral("pwminlength"), 8).value<quint8>();
        os.endGroup();

        adminPasswordMethod = static_cast<QCryptographicHash::Algorithm>(readInt(tr("Admin password method"), static_cast<quint32>(adminPasswordMethod)));
        adminPasswordRounds = readInt(tr("Admin password rounds"), adminPasswordRounds);
        adminPasswordMinLength = readChar(tr("Admin password min length"), adminPasswordMinLength);

        const QString adminUser = readString(tr("Admin user name"), QStringLiteral("admin"));
        const QString adminPass = readString(tr("Admin password"), QString());

        const QByteArray pw = Cutelyst::CredentialPassword::createPassword(adminPass.toUtf8(), adminPasswordMethod, adminPasswordRounds, 24, 27);

        if (!db.setAdmin(adminUser, pw)) {
            return 1;
        }

        os.beginGroup(QStringLiteral("Admins"));
        os.setValue(QStringLiteral("pwmethod"), static_cast<int>(adminPasswordMethod));
        os.setValue(QStringLiteral("pwrounds"), adminPasswordRounds);
        os.setValue(QStringLiteral("pwminlength"), adminPasswordMinLength);
        os.endGroup();
        os.sync();

        printf("%s\n", qUtf8Printable(tr("Successfully created administrator account for user admin.")));
    }


    if (!configExists || readBool(tr("Set the mail account password encryption?"), false)) {

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

    os.beginGroup(QStringLiteral("IMAP"));
    QString imapuser = os.value(QStringLiteral("user"), QStringLiteral("cyrus")).toString();
    QString imappass = os.value(QStringLiteral("password")).toString();
    QString imaphost = os.value(QStringLiteral("host"), QStringLiteral("localhost")).toString();
    quint16 imapport = os.value(QStringLiteral("port"), 143).value<quint16>();
    quint8 imapprotocol = os.value(QStringLiteral("protocol"), 2).value<quint8>();
    quint8 imapencryption = os.value(QStringLiteral("encryption"), 1).value<quint8>();
    os.endGroup();

    bool imapaccess = false;
    Imap ic(imapuser, imappass, imaphost, imapport, static_cast<QAbstractSocket::NetworkLayerProtocol>(imapprotocol), static_cast<Imap::EncryptionType>(imapencryption));
    if (configExists) {
        printf("%s\n", qUtf8Printable(tr("Try to login to the IMAP server.")));
        imapaccess = ic.login();
        if (imapaccess) {
            printf("%s\n", qUtf8Printable(tr("Login to IMAP server was successful.")));
            const QStringList caps = ic.getCapabilities();
            ic.logout();
            if (caps.empty()) {
                return 1;
            }
            printf("%s\n", qUtf8Printable(tr("Your IMAP server supports the following capabilities: %1").arg(caps.join(QStringLiteral(", ")))));
        }
    }

    if (!imapaccess) {
        if (configExists) {
            printf("%s\n", qUtf8Printable(tr("Failed to login into the IMAP server. Please reenter your connection data.")));
        } else {
            printf("%s\n", qUtf8Printable(tr("Please enter the data to connect to your IMAP server.")));
        }

        imaphost = readString(tr("IMAP Host"), imaphost);
        imapport = readPort(tr("IMAP Port"), imapport);
        imapuser = readString(tr("IMAP User"), imapuser);
        imappass = readString(tr("IMAP Password"), imappass);
        imapprotocol = readChar(tr("IMAP Protocol"), imapprotocol);
        imapencryption = readChar(tr("IMAP Encryption"), imapencryption);

        ic.setHost(imaphost);
        ic.setPort(imapport);
        ic.setUser(imapuser);
        ic.setPassword(imappass);
        ic.setProtocol(static_cast<QAbstractSocket::NetworkLayerProtocol>(imapprotocol));
        ic.setEncryptionType(static_cast<Imap::EncryptionType>(imapencryption));

        printf("%s\n", qUtf8Printable(tr("Try to login into the IMAP server.")));

        if (!ic.login()) {
            printf("%s\n", qUtf8Printable(tr("Failed to login into the IMAP server. Aborting.")));
            return 1;
        }

        printf("%s\n", qUtf8Printable(tr("Login to IMAP server was successful.")));

        const QStringList caps = ic.getCapabilities();
        ic.logout();
        if (caps.empty()) {
            return 1;
        }

        printf("%s\n", qUtf8Printable(tr("Your IMAP server supports the following capabilities: %1").arg(caps.join(QStringLiteral(", ")))));

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

    printf("%s\n", qUtf8Printable(tr("Skaffari setup was successful.")));

    return 0;
}


QString Setup::readString(const QString &name, const QString &defaultVal, const QString &desc) const
{
    QString inputVal;

    if (!desc.isEmpty()) {
        printf("%s\n", qUtf8Printable(desc));
    }

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


quint16 Setup::readPort(const QString &name, quint16 defaultVal, const QString &desc) const
{
    quint16 inputVal = 0;

    if (!desc.isEmpty()) {
        printf("%s\n", qUtf8Printable(desc));
    }


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


quint8 Setup::readChar(const QString &name, quint8 defaultVal, const QString &desc) const
{
    quint8 inputVal = 0;

    if (!desc.isEmpty()) {
        printf("%s\n", qUtf8Printable(desc));
    }

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


quint32 Setup::readInt(const QString &name, quint32 defaultVal, const QString &desc) const
{
    quint32 inputVal = 0;

    if (!desc.isEmpty()) {
        printf("%s\n", qUtf8Printable(desc));
    }

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


bool Setup::readBool(const QString &name, bool defaultVal, const QString &desc) const
{
    bool retVal = false;

    if (!desc.isEmpty()) {
        printf("%s\n", qUtf8Printable(desc));
    }

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
