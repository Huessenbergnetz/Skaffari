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
#include <Cutelyst/Plugins/Utils/validatorpwquality.h>
#include <QCryptographicHash>
#include <QStringList>
#include <QTimeZone>

#include "database.h"
#include "imap.h"
#include "../common/password.h"
#include "../common/config.h"
#include "../common/global.h"

Setup::Setup(const QString &confFile, bool quiet) :
    ConfigInput(quiet), m_confFile(confFile, true, true, quiet)
{

}


int Setup::exec() const
{
    printMessage(tr("Start to configure Skaffari."));

    bool configExists = m_confFile.exists();

    const int configCheck = m_confFile.checkConfigFile();
    if (configCheck > 0) {
        return configCheck;
    }

    QSettings os(m_confFile.absoluteFilePath(), QSettings::IniFormat);

    QVariantHash dbparams;
    os.beginGroup(QStringLiteral("Database"));
    for (const QString &key : os.childKeys()) {
        dbparams.insert(key, os.value(key));
    }
    os.endGroup();

    insertParamsDefault(dbparams, QStringLiteral("host"), QStringLiteral("localhost"));
    insertParamsDefault(dbparams, QStringLiteral("type"), QStringLiteral("QMYSQL"));
    insertParamsDefault(dbparams, QStringLiteral("port"), 3306);

    Database db;

    bool dbaccess = false;
    if (configExists && !dbparams.value(QStringLiteral("password")).toString().isEmpty()) {
        printTable({
                       {tr("Type"), dbparams.value(QStringLiteral("type")).toString()},
                       {tr("Host"), dbparams.value(QStringLiteral("host")).toString()},
                       {tr("Port"), dbparams.value(QStringLiteral("port")).toString()},
                       {tr("Name"), dbparams.value(QStringLiteral("name")).toString()},
                       {tr("User"), dbparams.value(QStringLiteral("user")).toString()},
                       {tr("Password"), QStringLiteral("********")}
                   }, tr("Database settings"));
        printStatus(tr("Establishing database connection"));
        dbaccess = db.open(dbparams);
        if (dbaccess) {
            printDone();
        } else {
            printFailed();
        }
    }

    while (!dbaccess) {
        printDesc(tr("Please enter the data to connect to your database system."));

        dbparams = askDatabaseConfig(dbparams);

        printStatus(tr("Establishing database connection"));

        if (!db.open(dbparams)) {
            printFailed();
        } else {
            printDone();
            dbaccess = true;
        }
    }

    os.beginGroup(QStringLiteral("Database"));
    QVariantHash::const_iterator i = dbparams.constBegin();
    while (i != dbparams.constEnd()) {
        os.setValue(i.key(), i.value());
        ++i;
    }
    os.endGroup();
    os.sync();


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

    printStatus(tr("Searching for available administrator accounts"));
    const uint adminCount = db.checkAdmin();
    if (adminCount > 0) {
        //: %1 will be the number of found administartors
        printDone(tr("Found %1").arg(adminCount));
    } else {
        //: no administrators have been found
        printFailed(tr("None"));
    }

    QVariantHash adminsParams;

    os.beginGroup(QStringLiteral("Admins"));
    adminsParams.insert(QStringLiteral("pwalgorithm"), os.value(QStringLiteral("pwalgorithm"), SK_DEF_ADM_PWALGORITHM));
    adminsParams.insert(QStringLiteral("pwrounds"), os.value(QStringLiteral("pwrounds"), SK_DEF_ADM_PWROUNDS));
#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
    adminsParams.insert(QStringLiteral("pwthreshold"), os.value(QStringLiteral("pwthreshold"), SK_DEF_ADM_PWTHRESHOLD));
    adminsParams.insert(QStringLiteral("pwsettingsfile"), os.value(QStringLiteral("pwsettingsfile")));
#else
    adminsParams.insert(QStringLiteral("pwminlength"), os.value(QStringLiteral("pwminlength"), SK_DEF_ADM_PWMINLENGTH));
#endif
    os.endGroup();

    if (adminCount == 0) {
        printDesc(tr("Please configure your administrator password settings and create a new administrator user."));
        printDesc(QString());

        adminsParams = askPbkdf2Config(adminsParams);

        printMessage(QString());

        printDesc(tr("Create an administrator account to login into your Skaffari installation. This account can create further administrators and domain managers."));

        const QString adminUser = readString(tr("User name"), QStringLiteral("admin"));
        bool adminPassValid = false;
        QString adminPass;
        while (!adminPassValid) {
            adminPass = readString(tr("Password"), QString());
#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
            printStatus(tr("Checking administrator password quality"));
            if (Cutelyst::ValidatorPwQuality::validate(adminPass, adminsParams.value(QStringLiteral("pwsettingsfile")), QString(), adminUser) >= adminsParams.value(QStringLiteral("pwthreshold")).toInt()) {
                printDone();
                adminPassValid = true;
            } else {
                printFailed();
            }
#else
            printStatus(tr("Checking administrator password length"));
            if (adminPass.length() >= adminsParams.value(QStringLiteral("pwminlength")).toInt()) {
                printDone();
                adminPassValid = true;
            } else {
                printFailed();
            }
#endif
        }

        printStatus(tr("Encrypting administrator password"));
        const QByteArray pw = Cutelyst::CredentialPassword::createPassword(adminPass.toUtf8(),
                                                                           static_cast<QCryptographicHash::Algorithm>(adminsParams.value(QStringLiteral("pwalgorithm")).value<quint8>()),
                                                                           adminsParams.value(QStringLiteral("pwrounds")).toInt(),
                                                                           24,
                                                                           27);
        if (!pw.isEmpty()) {
            printDone();
        } else {
            printFailed();
            return error(tr("Failed to encrypt administrator password. Encrypted password was empty."), 6);
        }

        printStatus(tr("Creating new administrator account in database"));
        if (!db.setAdmin(adminUser, pw)) {
            printFailed();
            return dbError(db.lastDbError());
        }

        printDone();

        os.beginGroup(QStringLiteral("Admins"));
        QVariantHash::const_iterator i = adminsParams.constBegin();
        while (i != adminsParams.constEnd()) {
            os.setValue(i.key(), i.value());
            ++i;
        }
        os.endGroup();
        os.sync();

    } else if (readBool(tr("Do you want to set the administrator password settings?"), false)) {

        adminsParams = askPbkdf2Config(adminsParams);

        os.beginGroup(QStringLiteral("Admins"));
        QVariantHash::const_iterator i = adminsParams.constBegin();
        while (i != adminsParams.constEnd()) {
            os.setValue(i.key(), i.value());
            ++i;
        }
        os.endGroup();
        os.sync();
    }

    QVariantHash accPwParams;

    os.beginGroup(QStringLiteral("Accounts"));
    accPwParams.insert(QStringLiteral("pwmethod"), os.value(QStringLiteral("pwmethod"), SK_DEF_ACC_PWMETHOD));
    accPwParams.insert(QStringLiteral("pwalgorithm"), os.value(QStringLiteral("pwalgorithm"), SK_DEF_ACC_PWALGORITHM));
    accPwParams.insert(QStringLiteral("pwrounds"), os.value(QStringLiteral("pwrounds"), SK_DEF_ACC_PWROUNDS));
#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
    accPwParams.insert(QStringLiteral("pwthreshold"), os.value(QStringLiteral("pwthreshold"), SK_DEF_ACC_PWTHRESHOLD));
    accPwParams.insert(QStringLiteral("pwsettingsfile"), os.value(QStringLiteral("pwsettingsfile")));
#else
    accPwParams.insert(QStringLiteral("pwminlength"), os.value(QStringLiteral("pwminlength"), SK_DEF_ACC_PWMINLENGTH).value<quint8>());
#endif
    os.endGroup();

    if (!configExists || readBool(tr("Do you want to set the user password settings?"), false)) {

        accPwParams = askPamPwConfig(accPwParams);

        os.beginGroup(QStringLiteral("Accounts"));
        QVariantHash::const_iterator i = accPwParams.constBegin();
        while (i != accPwParams.constEnd()) {
            os.setValue(i.key(), i.value());
            ++i;
        }
        os.endGroup();
        os.sync();
    }

    printStatus(tr("Checking for IMAP administrator account"));
    QString cyrusAdmin = db.checkCyrusAdmin();
    QString cyrusAdminPass;
    if (!cyrusAdmin.isEmpty()) {
        printDone(cyrusAdmin);
    } else {
        //: status for not finding any IMAP admin users
        printFailed(tr("None"));

        printDesc(tr("The administrator user for the IMAP server is defined in the imapd.conf in the admins: key. The user name that you enter here must also be specified in the imapd.conf. The administrator is used to perform various tasks on the IMAP server, such as setting storage quotas and creating/deleting mailboxes and folders. The user is created in the database used for Skaffari."));

        cyrusAdmin = readString(tr("IMAP administrator user"), QStringLiteral("cyrus"));

        bool cyrusAdminPassValid = false;
        while (!cyrusAdminPassValid) {
            cyrusAdminPass = readString(tr("IMAP administrator password"), QString());
#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
            printStatus(tr("Checking IMAP administrator password quality"));
            if (Cutelyst::ValidatorPwQuality::validate(cyrusAdminPass, accPwParams.value(QStringLiteral("pwsettingsfile")), QString(), cyrusAdmin) >= accPwParams.value(QStringLiteral("pwthreshold")).toInt()) {
                printDone();
                cyrusAdminPassValid = true;
            } else {
                printFailed();
            }
#else
            printStatus(tr("Checking IMAP administrator password length"));
            if (cyrusAdminPass.length() >= accPwParams.value(QStringLiteral("pwminlength")).toInt()) {
                printDone();
                cyrusAdminPassValid = true;
            } else {
                printFailed();
            }
#endif
        }

        printStatus(tr("Encrypting IMAP administrator password"));
        const Password cyrusAdminPw(cyrusAdminPass);
        const Password::Method accountsPwMethod = accPwParams.value(QStringLiteral("pwmethod"), SK_DEF_ACC_PWMETHOD).value<Password::Method>();
        const Password::Algorithm accountsPwAlgo = accPwParams.value(QStringLiteral("pwalgorithm"), SK_DEF_ACC_PWALGORITHM).value<Password::Algorithm>();
        const quint32 accountsPwRounds = accPwParams.value(QStringLiteral("pwrounds"), SK_DEF_ACC_PWROUNDS).value<quint32>();
        const QByteArray cyrusAdminPwEnc = cyrusAdminPw.encrypt(accountsPwMethod, accountsPwAlgo, accountsPwRounds);
        if (cyrusAdminPwEnc.isEmpty()) {
            printFailed();
            return error(tr("Failed to encrypt Cyrus administrator password."), 6);
        } else {
            printDone();
        }

        printStatus(tr("Creating IMAP administrator in database"));

        if (!db.setCryusAdmin(cyrusAdmin, cyrusAdminPwEnc)) {
            printFailed();
            return dbError(db.lastDbError());
        } else {
            printDone();
        }
    }

    QVariantHash imapParams;

    os.beginGroup(QStringLiteral("IMAP"));
    for (const QString &key : os.childKeys()) {
        imapParams.insert(key, os.value(key));
    }
    os.endGroup();

    insertParamsDefault(imapParams, QStringLiteral("host"), QStringLiteral("localhost"));
    insertParamsDefault(imapParams, QStringLiteral("port"), 143);
    insertParamsDefault(imapParams, QStringLiteral("protocol"), SK_DEF_IMAP_PROTOCOL);
    insertParamsDefault(imapParams, QStringLiteral("encryption"), SK_DEF_IMAP_ENCRYPTION);

    bool imapaccess = false;
    Imap imap;

    if (configExists && !imapParams.value(QStringLiteral("password")).toString().isEmpty()) {
        printTable({
                       {tr("Host"), imapParams.value(QStringLiteral("host")).toString()},
                       {tr("Port"), imapParams.value(QStringLiteral("port")).toString()},
                       {tr("Protocol"), Imap::networkProtocolToString(imapParams.value(QStringLiteral("protocol")).value<quint8>())},
                       {tr("Encryption"), Imap::encryptionTypeToString(imapParams.value(QStringLiteral("encryption")).value<quint8>())},
                       {tr("User"), imapParams.value(QStringLiteral("user")).toString()},
                       {tr("Password"), QStringLiteral("********")},
                       {tr("Peer name"), imapParams.value(QStringLiteral("peername")).toString()}
                   }, tr("IMAP settings"));

        printStatus(tr("Establishing IMAP connection"));
        imap.setParams(imapParams);
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

    if (imapParams.value(QStringLiteral("password")).toString().isEmpty() || !imapaccess || readBool(tr("Do you want to set the IMAP connection settings?"), false)) {

        if (!imapaccess) {
            printDesc(tr("Connection to your IMAP server failed. Please reenter your connection data."));
        } else {
            printDesc(tr("Please enter the data to connect to your IMAP server."));
            imapaccess = false;
        }
        printDesc(QString());
        printDesc(tr("Connection to your IMAP server as administrator is used to perform tasks like setting quotas and creating/deleting mailboxes and folders. The user account has to be defined as administrator in the imapd.conf file in the admins: key."));
        printDesc(QString());

        while (!imapaccess) {
            imapParams = askImapConfig(imapParams);
            printStatus(tr("Establishing IMAP connection"));
            imap.setParams(imapParams);
            if (imap.login()) {
                if (imap.logout()) {
                    printDone();
                    imapaccess = true;
                } else {
                    printFailed();
                    return imapError(imap.lastError());
                }
            } else {
                printFailed();
                printError(imap.lastError());
            }
        }

        os.beginGroup(QStringLiteral("IMAP"));
        QVariantHash::const_iterator i = imapParams.constBegin();
        while (i != imapParams.constEnd()) {
            os.setValue(i.key(), i.value());
            ++i;
        }
        os.endGroup();
        os.sync();

    }

    if (!configExists || readBool(tr("Do you want to set the IMAP behavior settings?"), false)) {

        printDesc(tr("Depending on the settings of your IMAP server, you can set different behavior of Skaffari when managing your email accounts."));
        printDesc(QString());

        insertParamsDefault(imapParams, QStringLiteral("unixhierarchysep"), SK_DEF_IMAP_UNIXHIERARCHYSEP);
        insertParamsDefault(imapParams, QStringLiteral("domainasprefix"), SK_DEF_IMAP_DOMAINASPREFIX);
        insertParamsDefault(imapParams, QStringLiteral("fqun"), SK_DEF_IMAP_FQUN);
        insertParamsDefault(imapParams, QStringLiteral("createmailbox"), SK_DEF_IMAP_CREATEMAILBOX);

        bool unixHierarchySep   = imapParams.value(QStringLiteral("unixhierarchysep")).toBool();
        bool domainAsPrefix     = imapParams.value(QStringLiteral("domainasprefix")).toBool();
        bool fqun               = imapParams.value(QStringLiteral("fqun")).toBool();
        quint8 createmailbox    = imapParams.value(QStringLiteral("createmailbox")).value<quint8>();

        unixHierarchySep = readBool(tr("UNIX hierarchy separator"),
                                    unixHierarchySep,
                                    QStringList({
                                                    tr("This setting should correspond to the value of the same setting in your imapd.conf(5) file and indicates that your imap server uses the UNIX separator character '/' for delimiting levels of mailbox hierarchy instead of the netnews separator character '.'. Up to Cyrus-IMAP 2.5.x the default value for this value in the IMAP server configuration is off, beginning with version 3.0.0 of Cyrus-IMAP the default has changed to on.")
                                                })
                                    );
        imapParams.insert(QStringLiteral("unixhierarchysep"), unixHierarchySep);

        if (unixHierarchySep) {

            domainAsPrefix = readBool(tr("Domain as prefix"),
                                      domainAsPrefix,
                                      QStringList({
                                                      tr("If enabled, usernames will be composed from the email local part and the domain name, separated by a dot instead of an @ sign. Like user.example.com. If you want to use real email addresses (fully qualified user names aka. fqun) like user@example.com as user names, you also have to set fqun to true in the next step."),
                                                      tr("For domains, the prefix will automatically be the same as the domain name when enabling this option."),
                                                      tr("NOTE: you have to set the following line in your imapd.conf file:"),
                                                      QStringLiteral("unixhierarchysep: yes")
                                                  })
                                      );
            imapParams.insert(QStringLiteral("domainasprefix"), domainAsPrefix);
        } else {
            imapParams.insert(QStringLiteral("domainasprefix"), false);
        }

        if (unixHierarchySep && domainAsPrefix) {

            fqun = readBool(tr("Fully qualified user name"),
                            fqun,
                            QStringList({
                                            tr("If you wish to use user names like email addresses (aka. fully qualified user name) you can activate this option."),
                                            tr("NOTE: you also have to add this lines to your imapd.conf file:"),
                                            QStringLiteral("unixhierarchysep: yes"),
                                            QStringLiteral("virtdomains: yes")
                                        })
                            );
            imapParams.insert(QStringLiteral("fqun"), fqun);
        } else {
            imapParams.insert(QStringLiteral("fqun"), false);
        }

        createmailbox = readChar(tr("Create mailboxes"),
                                 createmailbox,
                                 QStringList({
                                                 tr("Skaffari can create the mailboxes and all default folders on the IMAP server after creating a new user account. Alternatively the IMAP server can create default folders and account quotas on the first user login or first incoming email for the new account (has to be configured in your imapd.conf file). Skaffari is more flexible on creating different default folders for different domains."),
                                                 tr("Available behavior:"),
                                                 tr("0: Skaffari does nothing - all will be created by the IMAP server on first login or first incoming email"),
                                                 tr("1: Login after creation - Skaffari relies on the IMAP server to create folders and quotas, but will perform a login after account creation to initiate the creation by the IMAP server"),
                                                 tr("2: Only set quota - Skaffari will login to the new account after creation to let the IMAP server create the mailbox and will then set the quota"),
                                                 tr("3: Create by Skaffari - Skaffari will create the new mailbox and the default folders and will set the account quota after adding a new account")
                                             }),
                                 QList<quint8>({0,1,2,3})
                                 );
        imapParams.insert(QStringLiteral("createmailbox"), createmailbox);

        os.beginGroup(QStringLiteral("IMAP"));
        QVariantHash::const_iterator i = imapParams.constBegin();
        while (i != imapParams.constEnd()) {
            os.setValue(i.key(), i.value());
            ++i;
        }
        os.endGroup();
        os.sync();
    }

    printSuccess(tr("Skaffari setup was successful."));

    return 0;
}

void Setup::insertParamsDefault(QVariantHash &params, const QString &key, const QVariant &defVal)
{
    if (!params.value(key).isValid()) {
        params.insert(key, defVal);
    }
}
