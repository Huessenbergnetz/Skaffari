/*
 * Skaffari - a mail account administration web interface based on Cutelyst
 * Copyright (C) 2018 Matthias Fehring <mf@huessenbergnetz.de>
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

#include "configchecker.h"
#include "../common/config.h"
#include "../common/password.h"
#include "imap.h"
#include <QSettings>
#include <QFileInfo>
#include <QHostAddress>
#include <Cutelyst/Plugins/Utils/validatordomain.h>

ConfigChecker::ConfigChecker(const QString &confFile, bool quiet) :
    ConsoleOutput(quiet), m_confFile(confFile, false, false, quiet)
{

}

int ConfigChecker::exec() const
{
    printMessage(tr("Checking configuration file %1.").arg(m_confFile.absoluteFilePath()));

    if (!m_confFile.exists()) {
        return fileError(tr("Configuration file %1 does not exist.").arg(m_confFile.absoluteFilePath()));
    }

    QSettings s(m_confFile.absoluteFilePath(), QSettings::IniFormat);
    if (s.status() != QSettings::NoError) {
        return fileError(tr("Can not read configuration from %1.").arg(m_confFile.absoluteFilePath()));
    }

    s.beginGroup(QStringLiteral("Database"));
    const QString dbtype = s.value(QStringLiteral("type"), QStringLiteral("QMYSQL")).toString();
    if (dbtype.isEmpty()) {
        return configErrorWithKey(tr("Empty database type."), QStringLiteral("Database/type"));
    }
    if (!QStringList(SKAFFARI_SUPPORTED_SQL_DRIVERS).contains(dbtype)) {
        return configErrorWithKey(tr("Invalid value for database type."), QStringLiteral("Database/type"));
    }
    const QString dbhost = s.value(QStringLiteral("host"), QStringLiteral("localhost")).toString();
    if (dbhost.isEmpty()) {
        return configErrorWithKey(tr("Empty database host name."), QStringLiteral("Database/type"));
    }
    if (dbhost.startsWith(QLatin1Char('/'))) {
        QFileInfo dbSocket(dbhost);
        if (!dbSocket.exists()) {
            return configErrorWithKey(tr("Database socket file does not exist."), QStringLiteral("Database/host"));
        }
        if (!dbSocket.isReadable()) {
            return configErrorWithKey(tr("Database socket file is not readable."), QStringLiteral("Database/host"));
        }
        if (!dbSocket.isWritable()) {
            return configErrorWithKey(tr("Database socket file is not writable."), QStringLiteral("Database/host"));
        }
    } else {
        QHostAddress hostAddress(dbhost);
        if (!(!hostAddress.isNull() || Cutelyst::ValidatorDomain::validate(dbhost, false) || dbhost == QLatin1String("localhost"))) {
            return configErrorWithKey(tr("Database host is not a valid domain name or IP address."), QStringLiteral("Database/host"));
        }
        const uint dbport = s.value(QStringLiteral("port"), 3306).toUInt();
        if (dbport < 1 || dbport > static_cast<uint>(65535)) {
            return configErrorWithKey(tr("Database port is not valid."), QStringLiteral("Database/port"));
        }
    }
    if (s.value(QStringLiteral("name")).toString().isEmpty()) {
        return configErrorWithKey(tr("Empty database name."), QStringLiteral("Database/name"));
    }
    if (s.value(QStringLiteral("user")).toString().isEmpty()) {
        return configErrorWithKey(tr("Empty database user name."), QStringLiteral("Database/user"));
    }
    s.endGroup();



    s.beginGroup(QStringLiteral("Admins"));
    const int pwalgoint = s.value(QStringLiteral("pwalgorithm"), SK_DEF_ADM_PWALGORITHM).toInt();
    if (pwalgoint < SK_MIN_ADM_PWALGORITHM || pwalgoint > SK_MAX_ADM_PWALGORITHM) {
        return configErrorWithKey(tr("Invalid value for the administrator password hashing algorithm."), QStringLiteral("Admins/pwalgorithm"));
    }
    if (s.value(QStringLiteral("pwrounds"), SK_DEF_ADM_PWROUNDS).toInt() < 1000) {
        return configErrorWithKey(tr("For security reasons please use at least 1000 rounds for the administrator passwords."), QStringLiteral("Admins/pwrounds"));
    }
#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
    const int pwthreshold = s.value(QStringLiteral("pwthreshold"), SK_DEF_ADM_PWTHRESHOLD).toInt();
    if (pwthreshold < 0 || pwthreshold > 100) {
        return configErrorWithKey(tr("Invalid value for the administrator password quality threshold."), QStringLiteral("Admins/pwthreshold"));
    }
#else
    const uint pwminlength = s.value(QStringLiteral("pwminlength"), SK_DEF_ADM_PWMINLENGTH).toUInt();
    if (pwminlength > static_cast<uint>(255)) {
        return configErrorWithKey(tr("Invalid value for the administrator password minimum length."), QStringLiteral("Admins/pwminlength"));
    }
#endif
    s.endGroup();



    s.beginGroup(QStringLiteral("Accounts"));
    const quint8 apwmethodint = s.value(QStringLiteral("pwmethod"), SK_DEF_ACC_PWMETHOD).value<quint8>();
    if (apwmethodint > static_cast<quint8>(SK_MAX_ACC_PWMETHOD)) {
        return configErrorWithKey(tr("Invalid value for the account password encryption method."), QStringLiteral("Accounts/pwmethod"));
    }
    const Password::Method apwmethod = static_cast<Password::Method>(apwmethodint);

    const quint8 apwalgoint = s.value(QStringLiteral("pwalgorithm"), SK_DEF_ACC_PWALGORITHM).value<quint8>();
    const Password::Algorithm apwalgo = static_cast<Password::Algorithm>(apwalgoint);

    if (apwmethod == Password::Crypt && apwalgo > Password::CryptBcrypt) {
        return configErrorWithKey(tr("Invalid value for the account password hashing algorithm."), QStringLiteral("Accounts/pwalgorithm"));
    }
    if (apwmethod == Password::MySQL && (apwalgo != Password::Default && apwalgo != Password::MySQLNew && apwalgo != Password::MySQLOld)) {
        return configErrorWithKey(tr("Invalid value for the account password hashing algorithm."), QStringLiteral("Accounts/pwalgorithm"));
    }
    if (apwmethod == Password::Crypt || apwmethod == Password::MySQL) {
        if (apwmethod == Password::Crypt && (apwalgo == Password::CryptBcrypt || apwalgo == Password::CryptSHA256 || apwalgo == Password::CryptSHA512)) {
            const int apwrounds = s.value(QStringLiteral("pwrounds"), SK_DEF_ACC_PWROUNDS).toInt();
            if (apwalgo == Password::CryptBcrypt) {
                if (apwrounds < 4 || apwrounds > 31) {
                    return configErrorWithKey(tr("Invalid value for the account password hashing rounds."), QStringLiteral("Accounts/pwrounds"));
                }
            } else {
                if (apwrounds < 1000 || apwrounds > 999999999) {
                    return configErrorWithKey(tr("Invalid value for the password hashing rounds."), QStringLiteral("Accounts/pwrounds"));
                }
            }
        }
    }

#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
    const int apwthreshold = s.value(QStringLiteral("pwthreshold"), SK_DEF_ACC_PWTHRESHOLD).toInt();
    if (apwthreshold < 0 || apwthreshold > 100) {
         return configErrorWithKey(tr("Invalid value for the account password quality threshold."), QStringLiteral("Accounts/pwthreshold"));
    }
#else
    const uint apwminlength = s.value(QStringLiteral("pwminlength"), SK_DEF_ACC_PWMINLENGTH).toUInt();
    if (apwminlength > static_cast<uint>(255)) {
        return configErrorWithKey(tr("Invalid value for the administrator password minimum length."), QStringLiteral("Accounts/pwminlength"));
    }
#endif
    s.endGroup();



    s.beginGroup(QStringLiteral("IMAP"));
    const QString imapHost = s.value(QStringLiteral("host"), QStringLiteral("localhost")).toString();
    if (imapHost.isEmpty()) {
        return configErrorWithKey(tr("Empty IMAP host name."), QStringLiteral("IMAP/host"));
    }
    QHostAddress imapHostAddress(imapHost);
    if (!(!imapHostAddress.isNull() || Cutelyst::ValidatorDomain::validate(imapHost, false) || imapHost == QLatin1String("localhost"))) {
        return configErrorWithKey(tr("IMAP host is not a valid domain name or IP address."), QStringLiteral("IMAP/host"));
    }
    const uint imapPort = s.value(QStringLiteral("port"), 143).toUInt();
    if (imapPort < 1 || imapPort > static_cast<uint>(65535)) {
        return configErrorWithKey(tr("IMAP port is not valid."), QStringLiteral("IMAP/port"));
    }
    const int imapProtocol = s.value(QStringLiteral("protocol"), SK_DEF_IMAP_PROTOCOL).toInt();
    if (imapProtocol < 0 || imapProtocol > SK_MAX_IMAP_PROTOCOL) {
        return configErrorWithKey(tr("IMAP network protocol is not valid."), QStringLiteral("IMAP/protocol"));
    }
    const quint8 imapEncryption = s.value(QStringLiteral("encryption"), SK_DEF_IMAP_ENCRYPTION).value<quint8>();
    if (imapEncryption > SK_MAX_IMAP_ENCRYPTION) {
        return configErrorWithKey(tr("IMAP connection encryption type is not valid."), QStringLiteral("IMAP/encryption"));
    }
    const quint8 imapAuthMech = s.value(QStringLiteral("authmech"), SK_DEF_IMAP_AUTHMECH).value<quint8>();
    if (imapAuthMech > static_cast<quint8>(SK_MAX_IMAP_AUTHMECH)) {
        return configErrorWithKey(tr("IMAP authentication mechanism is not valid."), QStringLiteral("IMAP/authmech"));
    }
    if (s.value(QStringLiteral("user")).toString().isEmpty()) {
        return configErrorWithKey(tr("Empty IMAP administrator user name."), QStringLiteral("IMAP/user"));
    }
    if (s.value(QStringLiteral("password")).toString().isEmpty()) {
        return configErrorWithKey(tr("Empty IMAP administrator password."), QStringLiteral("IMAP/password"));
    }
    const quint8 imapCreateMailbox = s.value(QStringLiteral("createmailbox"), SK_DEF_IMAP_CREATEMAILBOX).value<quint8>();
    if (imapCreateMailbox > SK_MAX_IMAP_CREATEMAILBOX) {
        return configErrorWithKey(tr("Invalid value for IMAP mailbox creation strategy."), QStringLiteral("IMAP/createmailbox"));
    }
    s.endGroup();

    printDatabaseSettings(s);
    printAdminSettings(s);
    printAccountSettings(s);
    printImapSettings(s);

    return 0;
}

int ConfigChecker::configErrorWithKey(const QString &message, const QString &key) const
{
    //: %1 will be a sentence that will contain a description, %2 will contain the affected configuration key
    return configError(tr("%1 Configuration key: %2", "configuration error with key").arg(message, key));
}
