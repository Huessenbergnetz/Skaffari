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

#include "setupimporter.h"
#include <QSettings>
#include <QVersionNumber>

#include <Cutelyst/Plugins/Authentication/credentialpassword.h>
#include <Cutelyst/Plugins/Utils/validatorpwquality.h>
#include <Cutelyst/Plugins/Utils/validatoralphadash.h>

#include "database.h"
#include "imap.h"
#include "../common/password.h"
#include "../common/config.h"
#include "../common/global.h"

SetupImporter::SetupImporter(const QString &confFile, const QString &importFile, bool quiet) :
    ConfigInput(quiet), m_confFile(confFile, true, true, quiet), m_importFile(importFile, quiet)
{

}

int SetupImporter::exec() const
{
    printMessage(tr("Start importing configuration."));

    const int importFileCheck = m_importFile.exec();
    if (importFileCheck > 0) {
        return importFileCheck;
    }

    QSettings is(m_importFile.absolutePathToConfigFile(), QSettings::IniFormat);
    if (is.status() != QSettings::NoError) {
        return fileError(tr("Can not read configuration from %1.").arg(m_importFile.absolutePathToConfigFile()));
    }

    if (m_confFile.exists()) {
        if (!readBool(tr("The configuration file %1 already exists. Would you like to overwrite it?").arg(m_confFile.absoluteFilePath()), false)) {
            return 0;
        }
    }

    const int configFileCheck = m_confFile.checkConfigFile();
    if (configFileCheck > 0) {
        return configFileCheck;
    }

    QSettings os(m_confFile.absoluteFilePath(), QSettings::IniFormat);
    if (os.status() != QSettings::NoError) {
        return fileError(tr("Can not open configuration file %1.").arg(m_confFile.absoluteFilePath()));
    }

    QVariantHash dbopts;
    is.beginGroup(QStringLiteral("Database"));
    const auto dbkeys = is.childKeys();
    for (const QString &key : dbkeys) {
        dbopts.insert(key, is.value(key));
    }
    is.endGroup();

    insertParamsDefault(dbopts, QStringLiteral("host"), QStringLiteral("localhost"));
    insertParamsDefault(dbopts, QStringLiteral("type"), QStringLiteral("QMYSQL"));
    insertParamsDefault(dbopts, QStringLiteral("port"), 3306);

    printTable({
                   {tr("Type"), dbopts.value(QStringLiteral("type")).toString()},
                   {tr("Host"), dbopts.value(QStringLiteral("host")).toString()},
                   {tr("Port"), dbopts.value(QStringLiteral("port")).toString()},
                   {tr("Name"), dbopts.value(QStringLiteral("name")).toString()},
                   {tr("User"), dbopts.value(QStringLiteral("user")).toString()},
                   {tr("Password"), QStringLiteral("********")}
               }, tr("Database settings"));
    printStatus(tr("Establishing database connection"));

    Database db;

    if (!db.open(dbopts)) {
        printFailed();
        return dbError(db.lastDbError());
    }
    printDone();

    if (!db.empty()) {
        if (!readBool(tr("The database is not empty. Do you want to continue and drop all tables and views?"), false)) {
            return 0;
        }
        printStatus(tr("Dropping all tables and views from the database"));
        if (!db.clear()) {
            printFailed();
            return dbError(db.lastDbError());
        }
        printDone();
    }

    printStatus(tr("Performing database installation"));
    if (!db.installDatabase()) {
        printFailed();
        return dbError(db.lastDbError());
    }
    printDone();

    QVariantHash adminOpts;
    is.beginGroup(QStringLiteral("Admins"));
    adminOpts.insert(QStringLiteral("pwalgorithm"), is.value(QStringLiteral("pwalgorithm"), SK_DEF_ADM_PWALGORITHM));
    adminOpts.insert(QStringLiteral("pwrounds"), is.value(QStringLiteral("pwrounds"), SK_DEF_ADM_PWROUNDS));
#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
    adminOpts.insert(QStringLiteral("pwthreshold"), is.value(QStringLiteral("pwthreshold"), SK_DEF_ADM_PWTHRESHOLD));
    adminOpts.insert(QStringLiteral("pwsettingsfile"), is.value(QStringLiteral("pwsettingsfile")));
#else
    adminOpts.insert(QStringLiteral("pwminlength"), os.value(QStringLiteral("pwminlength"), SK_DEF_ADM_PWMINLENGTH));
#endif
    const QString adminUser = is.value(QStringLiteral("user")).toString();
    const QString adminPass = is.value(QStringLiteral("password")).toString();
    is.endGroup();

    const quint8 admpwalgo = adminOpts.value(QStringLiteral("pwalgorithm")).value<quint8>();
    if (admpwalgo < SK_MIN_ADM_PWALGORITHM || admpwalgo > SK_MAX_ADM_PWALGORITHM) {
        return configError(tr("Invalid value for admin password encryption algorithm. Configuration key: %1").arg(QStringLiteral("Admins/pwalgorithm")));
    }

#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
    const int admpwthresh = adminOpts.value(QStringLiteral("pwthreshold")).toInt();
    if (admpwthresh > 100 || admpwthresh < 0) {
        return configError(tr("Invalid value for admin password quality threshold. Configuration key: %1").arg(QStringLiteral("Admins/pwthreshold")));
    }
#else
    const uint admpwmin = adminOpts.value(QStringLiteral("pwminlength")).toUInt();
    if (admpwmin > static_cast<uint>(255)) {
        return configError(tr("Invalid value for admin password minimum length. Configuration key: %1").arg(QStringLiteral("Admins/pwminlength")));
    }
#endif

    printStatus(tr("Checking administrator user name"));
    if (adminUser.isEmpty() || !Cutelyst::ValidatorAlphaDash::validate(adminUser)) {
        printFailed();
        return configError(tr("Invalid administrator user name."));
    }
    printDone();

    printStatus(tr("Checking administrator password quality"));
#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
    if (Cutelyst::ValidatorPwQuality::validate(adminPass, adminOpts.value(QStringLiteral("pwsettingsfile")), QString(), adminUser) < admpwthresh) {
        printFailed();
        return configError(tr("The administrator password does not comply with the security requirements."));
    }
#else
    if (adminPass.length() < adminOpts.value(QStringLiteral("pwminlength")).toInt()) {
        printFailed();
        return configError(tr("The administrator password does not comply with the security requirements."));
    }
#endif
    printDone();

    printStatus(tr("Encrypting administrator password"));
    const QByteArray adminPassEnc = Cutelyst::CredentialPassword::createPassword(adminPass.toUtf8(),
                                                                                 static_cast<QCryptographicHash::Algorithm>(admpwalgo),
                                                                                 adminOpts.value(QStringLiteral("pwrounds")).toInt(),
                                                                                 24,
                                                                                 27);
    if (adminPassEnc.isEmpty()) {
        printFailed();
        return error(tr("Failed to encrypt administrator password. Encrypted password was empty."), 6);
    }
    printDone();

    printStatus(tr("Creating new administrator account in database"));
    if (!db.setAdmin(adminUser, adminPassEnc)) {
        printFailed();
        return dbError(db.lastDbError());
    }
    printDone();

    QVariantHash accPwParams;

    is.beginGroup(QStringLiteral("Accounts"));
    accPwParams.insert(QStringLiteral("pwmethod"), std::min(is.value(QStringLiteral("pwmethod"), SK_DEF_ACC_PWMETHOD).toUInt(), static_cast<uint>(SK_MAX_ACC_PWMETHOD)));
    accPwParams.insert(QStringLiteral("pwalgorithm"), is.value(QStringLiteral("pwalgorithm"), SK_DEF_ACC_PWALGORITHM).toUInt());
    accPwParams.insert(QStringLiteral("pwrounds"), is.value(QStringLiteral("pwrounds"), SK_DEF_ACC_PWROUNDS));
#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
    accPwParams.insert(QStringLiteral("pwthreshold"), is.value(QStringLiteral("pwthreshold"), SK_DEF_ACC_PWTHRESHOLD));
    accPwParams.insert(QStringLiteral("pwsettingsfile"), is.value(QStringLiteral("pwsettingsfile")));
#else
    accPwParams.insert(QStringLiteral("pwminlength"), is.value(QStringLiteral("pwminlength"), SK_DEF_ACC_PWMINLENGTH).value<quint8>());
#endif
    is.endGroup();

    const quint8 accpwmethodint = accPwParams.value(QStringLiteral("pwmethod")).value<quint8>();
    if (accpwmethodint > SK_MAX_ACC_PWMETHOD) {
        return configError(tr("Invalid value for account password encryption emthod. Configuration key: %1").arg(QStringLiteral("Accounts/pwmethod")));
    }

#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
    const int accpwthreshold = accPwParams.value(QStringLiteral("pwthreshold")).value<quint8>();
    if (accpwthreshold < 0 || accpwthreshold > 100) {
        return configError(tr("Invalid value for account password quality threshold. Configuration key: %1").arg(QStringLiteral("Accounts/pwthreshold")));
    }
#else
    const uint accpwminlength = accPwParams.value(QStringLiteral("pwminlength")).toUInt();
    if (accpwminlength > static_cast<uint>(255)) {
        return configError(tr("INvalid value for account password minimum length. Configuration key: %1").arg(QStringLiteral("Accounts/pwminlength")));
    }
#endif

    std::vector<std::pair<QString,QString>> accountsTable;
    const Password::Method pwmethod = static_cast<Password::Method>(accpwmethodint);
    accountsTable.push_back(std::make_pair(tr("Password encryption method"), Password::methodToString(pwmethod)));
    if (pwmethod == Password::Crypt || pwmethod == Password::MySQL) {
        const quint8 accpwalogint = accPwParams.value(QStringLiteral("pwalgorithm")).value<quint8>();
        if (accpwalogint > SK_MAX_ADM_PWALGORITHM) {
            return configError(tr("Invalid value for account password encryption algorithm. Configuration key: %1").arg(QStringLiteral("Accounts/pwalgorithm")));
        }
        const Password::Algorithm pwalgorithm = static_cast<Password::Algorithm>(accpwalogint);
        accountsTable.push_back(std::make_pair(tr("Password encryption algorithm"), Password::algorithmToString(pwalgorithm)));
        if (pwmethod == Password::Crypt) {
            if (pwalgorithm == Password::CryptBcrypt || pwalgorithm == Password::CryptSHA256 || pwalgorithm == Password::CryptSHA512) {
                accountsTable.push_back(std::make_pair(tr("Password encryption round"), accPwParams.value(QStringLiteral("pwrounds")).toString()));
            }
        }
    }
#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
    accountsTable.push_back(std::make_pair(tr("Password quality threshold"), accPwParams.value(QStringLiteral("pwthreshold")).toString()));
    accountsTable.push_back(std::make_pair(tr("Password quality configuration"), accPwParams.value(QStringLiteral("pwsettingsfile")).toString()));
#else
    accountsTable.push_back(std::make_pair(tr("Password minimum length"), QLocale::toString(accPwParams.value(QStringLiteral("pwminlength")).toUInt())));
#endif

    printTable(accountsTable, tr("Accounts settings"));

    QVariantHash imapParams;
    is.beginGroup(QStringLiteral("IMAP"));
    imapParams.insert(QStringLiteral("host"), is.value(QStringLiteral("host"), QStringLiteral("localhost")));
    imapParams.insert(QStringLiteral("port"), is.value(QStringLiteral("port"), 143));
    imapParams.insert(QStringLiteral("protocol"), std::min(is.value(QStringLiteral("protocol"), SK_DEF_IMAP_PROTOCOL).toUInt(), static_cast<uint>(SK_MAX_IMAP_PROTOCOL)));
    imapParams.insert(QStringLiteral("encryption"), std::min(is.value(QStringLiteral("encryption"), SK_DEF_IMAP_ENCRYPTION).toUInt(), static_cast<uint>(SK_MAX_IMAP_ENCRYPTION)));
    imapParams.insert(QStringLiteral("authmech"), std::min(is.value(QStringLiteral("authmech"), SK_DEF_IMAP_AUTHMECH).toUInt(), static_cast<uint>(SK_MAX_IMAP_AUTHMECH)));
    imapParams.insert(QStringLiteral("unixhierarchysep"), is.value(QStringLiteral("unixhierarchysep"), SK_DEF_IMAP_UNIXHIERARCHYSEP));
    imapParams.insert(QStringLiteral("domainasprefix"), is.value(QStringLiteral("domainasprefix"), SK_DEF_IMAP_DOMAINASPREFIX));
    imapParams.insert(QStringLiteral("fqun"), is.value(QStringLiteral("fqun"), SK_DEF_IMAP_FQUN));
    imapParams.insert(QStringLiteral("createmailbox"), std::min(is.value(QStringLiteral("createmailbox"), SK_DEF_IMAP_CREATEMAILBOX).toUInt(), static_cast<uint>(SK_MAX_IMAP_CREATEMAILBOX)));
    is.endGroup();

    QString createMailBoxString;
    switch(imapParams.value(QStringLiteral("createmailbox")).toInt()) {
    case 0:
        //: do not create mailboxes by Skaffari
        createMailBoxString = tr("do not create");
        break;
    case 1:
        createMailBoxString = tr("login after creation");
        break;
    case 2:
        createMailBoxString = tr("only set quota");
        break;
    case 3:
        createMailBoxString = tr("create by Skaffari");
        break;
    default:
        return configError(tr("Invalid value for IMAP mailbox creation strategy. Configuration key: %1").arg(QStringLiteral("IMAP/createmailbox")));
    }

    printTable({
                   {tr("Host"), imapParams.value(QStringLiteral("host")).toString()},
                   {tr("Port"), imapParams.value(QStringLiteral("port")).toString()},
                   {tr("Protocol"), Imap::networkProtocolToString(imapParams.value(QStringLiteral("protocol")).value<quint8>())},
                   {tr("Encryption"), Imap::encryptionTypeToString(imapParams.value(QStringLiteral("encryption")).value<quint8>())},
                   {tr("Authentication"), Imap::authMechToString(imapParams.value(QStringLiteral("authmech")).value<quint8>())},
                   {tr("User"), imapParams.value(QStringLiteral("user")).toString()},
                   {tr("Password"), QStringLiteral("********")},
                   {tr("Peer name"), imapParams.value(QStringLiteral("peername")).toString()},
                   {tr("Unix hierarchy seperator"), imapParams.value(QStringLiteral("unixhierarchysep")).toBool() ? QStringLiteral("true") : QStringLiteral("false")},
                   {tr("Domain as prefix"), imapParams.value(QStringLiteral("domainasprefix")).toBool() ? QStringLiteral("true") : QStringLiteral("false")},
                   {tr("Fully qualified username"), imapParams.value(QStringLiteral("fqun")).toBool() ? QStringLiteral("true") : QStringLiteral("false")},
                   {tr("Create mailbox"), createMailBoxString}
               }, tr("IMAP settings"));



    return 0;
}

void SetupImporter::insertParamsDefault(QVariantHash &params, const QString &key, const QVariant &defVal) const
{
    if (!params.value(key).isValid()) {
        params.insert(key, defVal);
    }
}
