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

ConfigInput::ConfigInput()
{

}


QVariantHash ConfigInput::askDatabaseConfig(const QVariantHash &defaults) const
{
    QVariantHash conf;

    const QString dbtype = readString(tr("DB Type"),
                                      defaults.value(QStringLiteral("type"), QStringLiteral("QMYSQL")).toString(),
                                      QStringList({
                                                      tr("The type of database you are using, identified by the Qt driver name."),
                                                      tr("See %1 for a list of drivers supported by Qt.").arg(QStringLiteral("http://doc.qt.io/qt-5/sql-driver.html")),
                                                      tr("Currently supported by Skaffari: %1").arg(QStringLiteral("QMYSQL"))}),
                                      QStringList({QStringLiteral("QMYSQL")}));
    conf.insert(QStringLiteral("type"), dbtype);

    const QString dbhost = readString(tr("DB Host"),
                                      defaults.value(QStringLiteral("host"), QStringLiteral("localhost")).toString(),
                                      QStringList({
                                                      tr("The host your database server is running on. By default this is the local host."),
                                                      tr("You can use localhost, a remote host identified by hostname or IP address or an absolute path to a local socket file.")
                                                  }));
    conf.insert(QStringLiteral("host"), dbhost);

    if (dbhost[0] != QLatin1Char('/')) {
        const quint16 dbport = readPort(tr("DB Port"), defaults.value(QStringLiteral("port"), 3306).value<quint16>(), QStringList({tr("The port your database server is listening on.")}));
        conf.insert(QStringLiteral("port"), dbport);
    } else {
        conf.insert(QStringLiteral("port"), 3306);
    }

    const QString dbname = readString(tr("DB Name"), defaults.value(QStringLiteral("name"), QStringLiteral("maildb")).toString(), QStringList({tr("The name of the database.")}));
    conf.insert(QStringLiteral("name"), dbname);

    const QString dbuser = readString(tr("DB User"), defaults.value(QStringLiteral("user"), QStringLiteral("mail")).toString(), QStringList({tr("The name of the database user that has read and write access to the database defined in the previous step.")}));
    conf.insert(QStringLiteral("user"), dbuser);

    const QString dbpass = readString(tr("DB Password"), defaults.value(QStringLiteral("pass")).toString(), QStringList({tr("The password of the database user defined in the previous step.")}));
    conf.insert(QStringLiteral("pass"), dbpass);

    return conf;
}



QVariantHash ConfigInput::askImapConfig(const QVariantHash &defaults) const
{
    QVariantHash conf;

    const QString imaphost = readString(tr("IMAP Host"), defaults.value(QStringLiteral("host"), QStringLiteral("localhost")).toString(), QStringList(tr("The host the IMAP server is running on.")));
    conf.insert(QStringLiteral("host"), imaphost);

    const quint16 imapport = readPort(tr("IMAP Port"), defaults.value(QStringLiteral("port"), 143).value<quint16>(), QStringList(tr("The port your IMAP server is listening on.")));
    conf.insert(QStringLiteral("port"), imapport);

    const QString imapuser = readString(tr("IMAP User"), defaults.value(QStringLiteral("user")).toString(), QStringList(tr("The user name of the IMAP admin user.")));
    conf.insert(QStringLiteral("user"), imapuser);

    const QString imappass = readString(tr("IMAP Password"), QString(), QStringList(tr("Password for the IMAP admin user.")));
    conf.insert(QStringLiteral("pass"), imappass);

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
                                                           tr("The method to encrypt the connection to the IMAP server."),
                                                           tr("Available methods:"),
                                                           tr("0: unsecured"),
                                                           QStringLiteral("1: StartTLS"),
                                                           QStringLiteral("2: IMAPS")
                                                       }),
                                           QList<quint8>({0,1,2}));
    conf.insert(QStringLiteral("encryption"), imapencryption);

    return conf;
}



QVariantHash ConfigInput::askPbkdf2Config(const QVariantHash &defaults) const
{
    QVariantHash conf;


    //: %1 will be substituted by a link to pbkdf2test GitHub repo, %2 will be substituded by an URL to a Wikipedia page about PBKDF2
    printDesc(tr("Skaffari uses PBKDF2 to secure the administrator passwords. PBKDF2 can use different hashing algorithms and iteration counts to produce a derived key and to increase the cost for the derivation. To better secure your administartor passwords you should use values that lead to a time consumption of around 0.5s on your system for creating the derived key. This might be a good compromise between security and user experience. To test different settings with the PBKDF2 implementation of Cutelyst/Skaffari you can use %1. See %2 to learn more about PBKDF2.").arg(QStringLiteral("https://github.com/Buschtrommel/pbkdf2test"), tr("https://en.wikipedia.org/wiki/PBKDF2")));

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


    quint8 method = readChar(tr("PBKDF2 algorithm"), 4, pbkdf2AlgoDesc, QList<quint8>({3,4,5,6,7,8,9,10}));
    quint32 rounds = readInt(tr("PBKDF2 iterations"), 32000, QStringList(tr("The iteration count is used to increase the cost for deriving the key from the password.")));
    quint8 minLength = readChar(tr("Password minimum length"), 8, QStringList(tr("Required minimum length for administrator passwords.")));

    conf.insert(QStringLiteral("method"), method);
    conf.insert(QStringLiteral("rounds"), rounds);
    conf.insert(QStringLiteral("minlength"), minLength);

    return conf;
}
