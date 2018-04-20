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

#include "configinput.h"
#include "../common/config.h"
#include "../common/password.h"

ConfigInput::ConfigInput(bool quiet) :
    ConsoleOutput(quiet)
{

}

QVariantHash ConfigInput::askDatabaseConfig(const QVariantHash &defaults) const
{
    QVariantHash conf;

    const QString dbtype = readString(tr("DB Type"),
                                      defaults.value(QStringLiteral("type"), QStringLiteral("QMYSQL")).toString(),
                                      QStringList({
                                                      tr("The database type you use, identified by the Qt driver name."),
                                                      //: %1 will be substituded by a link to the Qt documentation
                                                      tr("At %1 you will find a list of the database drivers supported by Qt.").arg(QStringLiteral("http://doc.qt.io/qt-5/sql-driver.html")),
                                                      tr("Currently supported by Skaffari: %1").arg(QStringLiteral("QMYSQL"))}),
                                      QStringList({QStringLiteral("QMYSQL")}));
    conf.insert(QStringLiteral("type"), dbtype);

    const QString dbhost = readString(tr("DB Host"),
                                      defaults.value(QStringLiteral("host"), QStringLiteral("localhost")).toString(),
                                      QStringList({
                                                      tr("The Host that runs your database server. By default, this is the local host."),
                                                      tr("You can use localhost, a remote host identified by hostname or IP address, or an absolute path to a local socket file.")
                                                  }));
    conf.insert(QStringLiteral("host"), dbhost);

    if (dbhost[0] != QLatin1Char('/')) {
        const quint16 dbport = readPort(tr("DB Port"), defaults.value(QStringLiteral("port"), 3306).value<quint16>(), QStringList({tr("The port your database server is listening on.")}));
        conf.insert(QStringLiteral("port"), dbport);
    } else {
        conf.insert(QStringLiteral("port"), 3306);
    }

    const QString dbname = readString(tr("DB Name"), defaults.value(QStringLiteral("name")).toString(), QStringList({tr("The name of the database.")}));
    conf.insert(QStringLiteral("name"), dbname);

    const QString dbuser = readString(tr("DB User"), defaults.value(QStringLiteral("user")).toString(), QStringList({tr("The name of the database user that has read and write access to the database defined in the previous step.")}));
    conf.insert(QStringLiteral("user"), dbuser);

    const QString dbpass = readString(tr("DB Password"), QString(), QStringList({tr("The password of the database user defined in the previous step.")}));
    conf.insert(QStringLiteral("password"), dbpass);

    return conf;
}

QVariantHash ConfigInput::askImapConfig(const QVariantHash &defaults) const
{
    QVariantHash conf;

    const QString imaphost = readString(tr("IMAP Host"), defaults.value(QStringLiteral("host"), QStringLiteral("localhost")).toString(), QStringList(tr("The Host that runs your IMAP server.")));
    conf.insert(QStringLiteral("host"), imaphost);

    const quint16 imapport = readPort(tr("IMAP Port"), defaults.value(QStringLiteral("port"), 143).value<quint16>(), QStringList(tr("The port your IMAP server is listening on. The default port for IMAP without encryption or with STARTTLS encryption is 143, the default port for IMAPS is 993.")));
    conf.insert(QStringLiteral("port"), imapport);

    const QString imapuser = readString(tr("IMAP User"), defaults.value(QStringLiteral("user")).toString(), QStringList(tr("The user name of the IMAP administrator user.")));
    conf.insert(QStringLiteral("user"), imapuser);

    const QString imappass = readString(tr("IMAP Password"), QString(), QStringList(tr("Password for the IMAP administrator user.")));
    conf.insert(QStringLiteral("password"), imappass);

    const quint8 imapprotocol = readChar(tr("IMAP Protocol"),
                                         defaults.value(QStringLiteral("protocol"), 2).value<quint8>(),
                                         QStringList({
                                                         tr("The network layer protocol to connect to the IMAP server."),
                                                         tr("Available protocols:"),
                                                         QStringLiteral("0: IPv4"),
                                                         QStringLiteral("1: IPv6"),
                                                         tr("2: Either IPv4 or IPv6")
                                                     }),
                                         QList<quint8>({0,1,2}));
    conf.insert(QStringLiteral("protocol"), imapprotocol);

    const quint8 imapencryption = readChar(tr("IMAP Encryption"),
                                           defaults.value(QStringLiteral("encryption"), 1).value<quint8>(),
                                           QStringList({
                                                           tr("The method for encryption of the connection to the IMAP server."),
                                                           tr("Available methods:"),
                                                           tr("0: unsecured"),
                                                           QStringLiteral("1: StartTLS"),
                                                           QStringLiteral("2: IMAPS")
                                                       }),
                                           QList<quint8>({0,1,2}));
    conf.insert(QStringLiteral("encryption"), imapencryption);

    if (imapencryption > 0) {
        const QString peerName = readString(tr("Peer name"),
                                            defaults.value(QStringLiteral("peername"), QStringLiteral("none")).toString(),
                                            QStringList({
                                                            tr("If you use a different host name to connect to your IMAP server than the one used in the certificate of the IMAP server, you can define this different name used in the certificate here. This can for example be used to establish an encrypted connection to an IMAP server running on your local host."),
                                                            QString(),
                                                            tr("Enter the keyword \"none\" to disable the peer name.")
                                                        })
                                            );
        if (peerName != QLatin1String("none")) {
            conf.insert(QStringLiteral("peername"), peerName);
        } else {
            conf.insert(QStringLiteral("peername"), QString());
        }
    }

    const quint8 imapauthmech = readChar(tr("Authentication mechanism"),
                                         defaults.value(QStringLiteral("authmech"), imapencryption ? 1 : 2).value<quint8>(),
                                         QStringList{
                                             tr("The authentication mechanism used by your IMAP server. Skaffari has not negotiated the available mechanisms with your IMAP server. As the administrator, you should know for yourself which mechanisms are supported by your server."),
                                             tr("Mechanisms supported by Skaffari:"),
                                             tr("0: Clear Text"),
                                             QStringLiteral("1: LOGIN"),
                                             QStringLiteral("2: PLAIN"),
                                             QStringLiteral("3: CRAM-MD5")
                                         },
                                         QList<quint8>{0,1,2,3});
    conf.insert(QStringLiteral("authmech"), imapauthmech);

    return conf;
}

QVariantHash ConfigInput::askPbkdf2Config(const QVariantHash &defaults) const
{
    QVariantHash conf;


    //: %1 will be substituted by a link to pbkdf2test GitHub repo, %2 will be substituded by an URL to a Wikipedia page about PBKDF2
    printDesc(tr("Skaffari uses PBKDF2 to secure administrator passwords. PBKDF2 can use different hash functions and multiple rounds to generate a derived key and increase the cost of derivation. To better protect your passwords, you should choose settings that cause your system to encrypt a password for about half a second. This should be a good compromise between security and user experience. You can use %1 to test different settings with the PBKDF2 implementation of Cutelyst/Skaffari. Learn more about PBKDF2 here %2.").arg(QStringLiteral("https://github.com/Buschtrommel/pbkdf2test"), tr("https://en.wikipedia.org/wiki/PBKDF2")));

    printDesc(QString());

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


    quint8 method = readChar(tr("PBKDF2 algorithm"), defaults.value(QStringLiteral("pwalgorithm"), SK_DEF_ADM_PWALGORITHM).value<quint8>(), pbkdf2AlgoDesc, QList<quint8>({3,4,5,6,7,8,9,10}));
    quint32 rounds = readInt(tr("PBKDF2 rounds"), defaults.value(QStringLiteral("pwrounds"), SK_DEF_ADM_PWROUNDS).value<quint32>(), QStringList(tr("The iteration count is used to increase the cost for deriving the key from the password.")));
#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
    quint8 threshold = readChar(tr("Password quality threshold"), defaults.value(QStringLiteral("pwthreshold"), SK_DEF_ADM_PWTHRESHOLD).value<quint8>(), QStringList(tr("Threshold for the password quality. Skaffari uses libpwquality to calculate a quality score for passwords. Passwords with scores below this threshold will not be accepted.")));
    QString settingsFile = readFilePath(tr("Password quality settings file"), defaults.value(QStringLiteral("pwsettingsfile")).toString(), QStringList(tr("Absolute path to the configuration file that contains the settings for libpwquality. If the path is empty, the default configuration file will be used. Use man 5 pwquality.conf to learn more about the configuration options.")), true);
#else
    quint8 minLength = readChar(tr("Password minimum length"), defaults.value(QStringLiteral("pwminlength"), SK_DEF_ADM_PWMINLENGTH).value<quint8>(), QStringList(tr("Required minimum length for administrator passwords.")));
#endif

    conf.insert(QStringLiteral("pwalgorithm"), method);
    conf.insert(QStringLiteral("pwrounds"), rounds);
#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
    conf.insert(QStringLiteral("pwthreshold"), threshold);
    conf.insert(QStringLiteral("pwsettingsfile"), settingsFile);
#else
    conf.insert(QStringLiteral("pwminlength"), minLength);
#endif

    return conf;
}

QVariantHash ConfigInput::askPamPwConfig(const QVariantHash &defaults) const
{
    QVariantHash conf;

    printDesc(tr("Skaffari is designed to work together with pam_mysql. For this reason, passwords for user accounts are stored in a different format than those for administrator accounts. The format of the user passwords must be compatible with the methods supported by pam_mysql. You can find out more about the supported methods in the README of your pam_mysql installation."));

    printDesc(QString());

    const QStringList methodDesc({
                                     tr("The basic method for encryption of user passwords. Some methods support additional settings and different algorithms which can be set in the next step. If possible, you should use crypt(3) because this method supports modern hash functions together with salts and an extensible storage format. The other methods are available for compatibility reasons."),
                                     tr("Supported methods:"),
                                     QString(),
                                     tr("0: no encryption - highly discouraged"),
                                     tr("1: crypt(3) function - recommended"),
                                     tr("2: MySQL password function"),
                                     tr("3: plain hex MD5 - not recommended"),
                                     tr("4: plain hex SHA1 - not recommended")
                                 });

    Password::Method method = static_cast<Password::Method>(readChar(tr("Encryption method"),
                                                                     defaults.value(QStringLiteral("pwmethod"), SK_DEF_ACC_PWMETHOD).value<quint8>(),
                                                                     methodDesc,
                                                                     QList<quint8>({0,1,2,3,4})));

    Password::Algorithm algo = Password::Default;

    if (method == Password::Crypt) {
        const QStringList cryptAlgoDesc({
                                            tr("The method crypt(3) supports different algorithms to derive a key from a password. To find out which algorithms are supported on your system, use man crypt. Especially bcrypt, which uses Blowfish, is not available on all systems because it is not part of the standard distribution of crypt(3). The non-recommended algorithms are available for compatibility reasons and to store passwords across operating system boundaries."),
                                            QString(),
                                            tr("Supported algorithms:"),
                                            tr("0: Default - points to SHA-256"),
                                            tr("1: Traditional DES-based - not recommended"),
                                            tr("2: FreeBSD-style MD5-based - not recommended"),
                                            tr("3: SHA-256 based"),
                                            tr("4: SHA-512 based"),
                                            tr("5: OpenBSD-style Blowfish-based (bcrypt) - not supported everywhere")
                                        });

        algo = static_cast<Password::Algorithm>(readChar(tr("Encryption algorithm"),
                                                         defaults.value(QStringLiteral("pwalgorithm"), SK_DEF_ACC_PWALGORITHM).value<quint8>(),
                                                         cryptAlgoDesc,
                                                         QList<quint8>({0,1,2,3,4,5})));
    } else  if (method == Password::MySQL) {
        const QStringList mysqlAlgoDesc({
                                            tr("MySQL supports two different hash functions, a new and an old one. If possible, you should use the new function. The old function is provided for compatibility reasons."),
                                            QString(),
                                            tr("Supported algorithms:"),
                                            tr("0: default - points to MySQL new"),
                                            tr("6: MySQL new"),
                                            tr("7: MySQL old")
                                        });

        algo = static_cast<Password::Algorithm>(readChar(tr("Encryption algorithm"),
                                                         defaults.value(QStringLiteral("pwalgorithm"), SK_DEF_ACC_PWALGORITHM).value<quint8>(),
                                                         mysqlAlgoDesc,
                                                         QList<quint8>({0,6,7})));
    }

    quint32 rounds = defaults.value(QStringLiteral("pwrounds"), SK_DEF_ACC_PWROUNDS).value<quint32>();
    if (method == Password::Crypt) {
        if ((algo == Password::CryptSHA256) || (algo == Password::CryptSHA512) || (algo == Password::CryptBcrypt)) {
            const QStringList roundsDesc = (algo == Password::CryptBcrypt) ? QStringList(tr("Crypt(3) with bcrypt supports an iteration count to increase the time cost for creating the derived key. The iteration count passed to the crypt function is the base-2 logarithm of the actual iteration count. Supported values are between 4 and 31.")) : QStringList(tr("Crypt(3) with SHA-256 and SHA-512 supports an iteration count from 1000 to 999999999. The iterations are used to increase the time cost for creating the derived key."));

            if ((algo == Password::CryptBcrypt) && ((rounds < 4) || (rounds > 31))) {
                rounds = 12;
            }
            if ((algo != Password::CryptBcrypt) && ((rounds < 1000) || (rounds > 999999999))) {
                rounds = 5000;
            }

            rounds = readInt(tr("Encryption rounds"), rounds, roundsDesc);
        }
    }

#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
    quint8 threshold = readChar(tr("Password quality threshold"), defaults.value(QStringLiteral("pwthreshold"), SK_DEF_ADM_PWTHRESHOLD).value<quint8>(), QStringList(tr("Threshold for the password quality. Skaffari uses libpwquality to calculate a quality score for passwords. Passwords with scores below this threshold will not be accepted.")));
    QString settingsFile = readFilePath(tr("Password quality settings file"), defaults.value(QStringLiteral("pwsettingsfile")).toString(), QStringList(tr("Absolute path to the configuration file that contains the settings for libpwquality. If the path is empty, the default configuration file will be used. Use man 5 pwquality.conf to learn more about the configuration options.")), true);
#else
    quint8 minLength = readChar(tr("Password minimum length"), defaults.value(QStringLiteral("pwminlength"), SK_DEF_ADM_PWMINLENGTH).value<quint8>(), QStringList(tr("Required minimum length for administrator passwords.")));
#endif

    conf.insert(QStringLiteral("pwmethod"), method);
    conf.insert(QStringLiteral("pwalgorithm"), algo);
    conf.insert(QStringLiteral("pwrounds"), rounds);
#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
    conf.insert(QStringLiteral("pwthreshold"), threshold);
    conf.insert(QStringLiteral("pwsettingsfile"), settingsFile);
#else
    conf.insert(QStringLiteral("pwminlength"), minLength);
#endif

    return conf;
}
