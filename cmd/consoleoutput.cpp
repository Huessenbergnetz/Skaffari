/*
 * Skaffari - a mail account administration web interface based on Cutelyst
 * Copyright (C) 2017-2018 Matthias Fehring <mf@huessenbergnetz.de>
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

#include "consoleoutput.h"
#include "../common/config.h"
#include "../common/password.h"
#include "imap.h"
#include <cstdio>
#include <iostream>
#include <string>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>
#include <QStringList>
#include <QFileInfo>
#include <QSettings>

ConsoleOutput::ConsoleOutput(bool quiet) :
    m_quiet(quiet)
{

}

int ConsoleOutput::error(const QString &message, int code) const
{
    printError(message);
    return code;
}

int ConsoleOutput::inputError(const QString &message) const
{
    return error(message, 1);
}

int ConsoleOutput::configError(const QString &message) const
{
    return error(message, 2);
}

int ConsoleOutput::dbError(const QString &message) const
{
    return error(message, 3);
}

int ConsoleOutput::dbError(const QSqlDatabase &db) const
{
    return dbError(db.lastError().text());
}

int ConsoleOutput::dbError(const QSqlQuery &q) const
{
    return dbError(q.lastError().text());
}

int ConsoleOutput::dbError(const QSqlError &e) const
{
    return dbError(e.text());
}

int ConsoleOutput::fileError(const QString &message) const
{
    return error(message, 4);
}

int ConsoleOutput::imapError(const QString &message) const
{
    return error(message, 5);
}

void ConsoleOutput::printStatus(const QString &status) const
{
    if (!m_quiet) {
        printf("%-100s", status.toUtf8().constData());
    }
}

void ConsoleOutput::printDone(const QString &done) const
{
    if (!m_quiet) {
        if (done.isEmpty()) {
            printf("\x1b[32m%s\x1b[0m\n", QCoreApplication::translate("ConsoleOutput", "Done").toUtf8().constData());
        } else {
            printf("\x1b[32m%s\x1b[0m\n", done.toUtf8().constData());
        }
    }
}

void ConsoleOutput::printFailed(const QString &failed) const
{
    if (!m_quiet) {
        if (failed.isEmpty()) {
            printf("\x1b[31m%s\x1b[0m\n", QCoreApplication::translate("ConsoleOutput", "Failed").toUtf8().constData());
        } else {
            printf("\x1b[31m%s\x1b[0m\n", failed.toUtf8().constData());
        }
    }
}

void ConsoleOutput::printError(const QString &error) const
{
    if (!m_quiet) {
        printf("\x1b[33m%s\x1b[0m\n", error.toUtf8().constData());
    }
}

void ConsoleOutput::printError(const QStringList &errors) const
{
    if (!m_quiet) {
        if (!errors.empty()) {
            for (const QString &error : errors) {
                printError(error);
            }
        }
    }
}

void ConsoleOutput::printMessage(const QString &message) const
{
    if (!m_quiet) {
        printf("%s\n", message.toUtf8().constData());
    }
}

void ConsoleOutput::printSuccess(const QString &message) const
{
    if (!m_quiet) {
        printf("\x1b[32m%s\x1b[0m\n", message.toUtf8().constData());
    }
}

void ConsoleOutput::printDesc(const QString &desc) const
{
    if (!m_quiet) {
        if (desc.length() <= 95) {
            printf("# %s\n", desc.toUtf8().constData());
        } else {
            const QStringList parts = desc.split(QStringLiteral(" "));
            QStringList out;
            int outSize = 0;
            for (const QString &part : parts) {
                if ((outSize + part.length() + 1) <= 95) {
                    out << part;
                    outSize += (part.length() + 1);
                } else {
                    printf("# %s\n", out.join(QStringLiteral(" ")).toUtf8().constData());
                    out.clear();
                    out << part;
                    outSize = (part.length() + 1);
                }
            }
            if (!out.isEmpty()) {
                printf("# %s\n", out.join(QStringLiteral(" ")).toUtf8().constData());
            }
        }
    }
}

void ConsoleOutput::printDesc(const QStringList &descList) const
{
    if (!m_quiet) {
        if (!descList.empty()) {
            for (const QString &desc : descList) {
                printDesc(desc);
            }
        }
    }
}

void ConsoleOutput::printTable(std::initializer_list<std::pair<QString, QString> > table, const QString &header) const
{
    if (!m_quiet) {
        if (table.size() == 0) {
            return;
        }

        int maxLabelLength = 0;
        int maxValueLength = 0;

        for (const std::pair<QString,QString> &col : table) {
            if (maxLabelLength < col.first.length()) {
                maxLabelLength = col.first.length();
            }
            if (maxValueLength < col.second.length()) {
                maxValueLength = col.second.length();
            }
        }

        maxLabelLength += 5;
        maxValueLength += 5;

        int fullLength = maxLabelLength + maxValueLength;
        if ((fullLength % 2) > 0) {
            fullLength++;
        }

        QString devider(QLatin1Char('+'));
        for (int i = 0; i < fullLength; ++i) {
            devider.append(QLatin1Char('-'));
        }
        devider.append(QLatin1Char('+'));
        printf("%s\n", devider.toUtf8().constData());

        if (!header.isEmpty()) {
            int centerHeader = fullLength / 2 + header.length() / 2;
            printf("|%*s%*s\n", centerHeader, qUtf8Printable(header), fullLength-centerHeader+1, "|");
            printf("%s\n", qUtf8Printable(devider));
        }

        for (const std::pair<QString,QString> &col : table) {
            printf("| %-*s| %-*s|\n", maxLabelLength-1, qUtf8Printable(col.first), maxValueLength-1, qUtf8Printable(col.second));
        }

        printf("%s\n", qUtf8Printable(devider));
    }
}

void ConsoleOutput::printTable(const std::vector<std::pair<QString, QString>> &table, const QString &header) const
{
    if (!m_quiet) {
        if (table.size() == 0) {
            return;
        }

        int maxLabelLength = 0;
        int maxValueLength = 0;

        for (const std::pair<QString,QString> &col : table) {
            if (maxLabelLength < col.first.length()) {
                maxLabelLength = col.first.length();
            }
            if (maxValueLength < col.second.length()) {
                maxValueLength = col.second.length();
            }
        }

        maxLabelLength += 5;
        maxValueLength += 5;

        int fullLength = maxLabelLength + maxValueLength;
        if ((fullLength % 2) > 0) {
            fullLength++;
        }

        QString devider(QLatin1Char('+'));
        for (int i = 0; i < fullLength; ++i) {
            devider.append(QLatin1Char('-'));
        }
        devider.append(QLatin1Char('+'));
        printf("%s\n", devider.toUtf8().constData());

        if (!header.isEmpty()) {
            int centerHeader = fullLength / 2 + header.length() / 2;
            printf("|%*s%*s\n", centerHeader, qUtf8Printable(header), fullLength-centerHeader+1, "|");
            printf("%s\n", qUtf8Printable(devider));
        }

        for (const std::pair<QString,QString> &col : table) {
            printf("| %-*s| %-*s|\n", maxLabelLength-1, qUtf8Printable(col.first), maxValueLength-1, qUtf8Printable(col.second));
        }

        printf("%s\n", qUtf8Printable(devider));
    }
}

QString ConsoleOutput::readString(const QString &name, const QString &defaultVal, const QStringList &desc, const QStringList &acceptableInput) const
{
    QString inputVal;

    printDesc(desc);

    if (acceptableInput.empty()) {
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
    } else {
        while (!acceptableInput.contains(inputVal)) {
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
        }
    }

    return inputVal;
}

quint16 ConsoleOutput::readPort(const QString &name, quint16 defaultVal, const QStringList &desc) const
{
    quint16 inputVal = 0;

    printDesc(desc);

    bool ok = false;

    while (!ok) {
        printf("%s [%i]: ", qUtf8Printable(name), defaultVal);
        std::string in;
        std::getline(std::cin, in);
        QString _inputVal = QString::fromStdString(in);
        if (_inputVal.isEmpty()) {
            inputVal = defaultVal;
            ok = true;
        } else {
            inputVal = _inputVal.toUShort(&ok);
        }
    }

    return inputVal;
}

quint8 ConsoleOutput::readChar(const QString &name, quint8 defaultVal, const QStringList &desc, const QList<quint8> &acceptableInput) const
{
    quint8 inputVal = 255;

    printDesc(desc);

    if (acceptableInput.empty()) {
        bool ok = false;
        while (!ok) {
            printf("%s [%i]: ", qUtf8Printable(name), defaultVal);
            std::string in;
            std::getline(std::cin, in);
            QString _inputVal = QString::fromStdString(in);
            if (_inputVal.isEmpty()) {
                inputVal = defaultVal;
                ok = true;
            } else {
                const ushort _intermediate = _inputVal.toUShort(&ok);
                if (ok && (_intermediate < 256)) {
                    inputVal = static_cast<quint8>(_intermediate);
                } else {
                    ok = false;
                }
            }
        }
    } else {
        while(!acceptableInput.contains(inputVal)) {
            printf("%s [%i]: ", qUtf8Printable(name), defaultVal);
            std::string in;
            std::getline(std::cin, in);
            QString _inputVal = QString::fromStdString(in);
            if (_inputVal.isEmpty()) {
                inputVal = defaultVal;
            } else {
                bool ok = false;
                const ushort _intermediate = _inputVal.toUShort(&ok);
                if (ok && (_intermediate < 256)) {
                    inputVal = static_cast<quint8>(_intermediate);
                }
            }
        }
    }

    return inputVal;
}

quint32 ConsoleOutput::readInt(const QString &name, quint32 defaultVal, const QStringList &desc) const
{
    quint32 inputVal = 0;

    printDesc(desc);

    bool ok = false;
    while (!ok) {
        printf("%s [%i]: ", qUtf8Printable(name), defaultVal);
        std::string in;
        std::getline(std::cin, in);
        QString _inputVal = QString::fromStdString(in);
        if (_inputVal.isEmpty()) {
            inputVal = defaultVal;
            ok = true;
        } else {
            inputVal = _inputVal.toUInt(&ok);
        }
    }

    return inputVal;
}

bool ConsoleOutput::readBool(const QString &name, bool defaultVal, const QStringList &desc) const
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

QString ConsoleOutput::readFilePath(const QString &name, const QString &defaultVal, const QStringList &desc, bool canBeEmpty) const
{
    QString ret;

    printDesc(desc);

    QFileInfo fi;
    if (canBeEmpty) {
        QString inStr = QStringLiteral("asdfads");
        while (!inStr.isEmpty() && !fi.exists()) {
            printf("%s [%s]: ", qUtf8Printable(name), qUtf8Printable(defaultVal));
            std::string in;
            std::getline(std::cin, in);
            inStr = QString::fromStdString(in);
            if (inStr.isEmpty() && !defaultVal.isEmpty()) {
                inStr = defaultVal;
            }
            if (!inStr.isEmpty()) {
                fi.setFile(inStr);
                if (!fi.exists()) {
                    printFailed(tr("File %1 does not exist.").arg(inStr));
                } else if (!fi.isReadable()) {
                    printFailed(tr("File %1 is not readable.").arg(inStr));
                }
            }
        }
        ret = inStr;
    } else {
        QString inStr;
        while (!fi.exists()) {
            printf("%s [%s]: ", qUtf8Printable(name), qUtf8Printable(defaultVal));
            std::string in;
            std::getline(std::cin, in);
            inStr = QString::fromStdString(in);
            if (inStr.isEmpty() && !defaultVal.isEmpty()) {
                inStr = defaultVal;
            }
            if (!inStr.isEmpty()) {
                fi.setFile(inStr);
                if (!fi.exists()) {
                    printFailed(tr("File %1 does not exist.").arg(inStr));
                } else if (!fi.isReadable()) {
                    printFailed(tr("File %1 is not readable.").arg(inStr));
                }
            }
        }
        ret = inStr;
    }

    return ret;
}

QString ConsoleOutput::qtCryptoHashEnumToString(QCryptographicHash::Algorithm algo) const
{
    switch (algo) {
    case QCryptographicHash::Sha224:
        return QStringLiteral("SHA2-224");
    case QCryptographicHash::Sha256:
        return QStringLiteral("SHA2-256");
    case QCryptographicHash::Sha384:
        return QStringLiteral("SHA2-384");
    case QCryptographicHash::Sha512:
        return QStringLiteral("SHA2-512");
    case QCryptographicHash::Sha3_224:
        return QStringLiteral("SHA3-224");
    case QCryptographicHash::Sha3_256:
        return QStringLiteral("SHA3-256");
    case QCryptographicHash::Sha3_384:
        return QStringLiteral("SHA3-384");
    case QCryptographicHash::Sha3_512:
        return QStringLiteral("SHA3-512");
    default:
        return tr("invalid algorithm");
    }
}

void ConsoleOutput::printDatabaseSettings(const QVariantHash &s) const
{
    std::vector<std::pair<QString,QString>> t;
    t.push_back(std::make_pair(tr("Type"), s.value(QStringLiteral("type"), QStringLiteral("QMYSQL")).toString()));
    const QString host = s.value(QStringLiteral("host"), QStringLiteral("localhost")).toString();
    if (host.startsWith(QLatin1Char('/'))) {
        t.push_back(std::make_pair(tr("Socket"), host));
    } else {
        t.push_back(std::make_pair(tr("Host"), host));
        t.push_back(std::make_pair(tr("Port"), s.value(QStringLiteral("port"), 3306).toString()));
    }
    t.push_back(std::make_pair(tr("Name"), s.value(QStringLiteral("name")).toString()));
    t.push_back(std::make_pair(tr("User"), s.value(QStringLiteral("user")).toString()));
    const QString pw = s.value(QStringLiteral("password")).toString();
    if (!pw.isEmpty()) {
        t.push_back(std::make_pair(tr("Password"), QStringLiteral("*********")));
    }
    printTable(t, tr("Database settings"));
}

void ConsoleOutput::printDatabaseSettings(QSettings &s) const
{
    QVariantHash h;
    s.beginGroup(QStringLiteral("Database"));
    const QStringList keys = s.childKeys();
    for (const QString &key : keys) {
        h.insert(key, s.value(key));
    }
    s.endGroup();
}

void ConsoleOutput::printAdminSettings(const QVariantHash &s) const
{
    std::vector<std::pair<QString,QString>> t;
    t.push_back(std::make_pair(tr("Password hashing algorithm"), qtCryptoHashEnumToString(static_cast<QCryptographicHash::Algorithm>(s.value(QStringLiteral("pwalgorithm"), SK_DEF_ADM_PWALGORITHM).toInt()))));
    t.push_back(std::make_pair(tr("Password hashing rounds"), s.value(QStringLiteral("pwrounds"), SK_DEF_ADM_PWROUNDS).toString()));
#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
    t.push_back(std::make_pair(tr("Password quality threshold"), s.value(QStringLiteral("pwthreshold"), SK_DEF_ADM_PWTHRESHOLD).toString()));
    t.push_back(std::make_pair(tr("Password quality settings file"), s.value(QStringLiteral("pwsettingsfile")).toString()));
#else
    t.push_back(std::make_pair(tr("Password minimum length"), s.value(QStringLiteral("pwminlength"), SK_DEF_ADM_PWMINLENGTH).toString()));
#endif
    printTable(t, tr("Administrator settings"));
}

void ConsoleOutput::printAdminSettings(QSettings &s) const
{
    QVariantHash h;
    s.beginGroup(QStringLiteral("Admins"));
    const QStringList keys = s.childKeys();
    for (const QString &key : keys) {
        h.insert(key, s.value(key));
    }
    s.endGroup();
}

void ConsoleOutput::printAccountSettings(const QVariantHash &s) const
{
    std::vector<std::pair<QString,QString>> t;
    const Password::Method method = static_cast<Password::Method>(s.value(QStringLiteral("pwmethod"), SK_DEF_ACC_PWMETHOD).value<quint8>());
    t.push_back(std::make_pair(tr("Password encryption method"), Password::methodToString(method)));
    if (method == Password::Crypt || method == Password::MySQL) {
        const Password::Algorithm algo = static_cast<Password::Algorithm>(s.value(QStringLiteral("pwalgorithm"), SK_DEF_ACC_PWALGORITHM).value<quint8>());
        t.push_back(std::make_pair(tr("Password hashing algorithm"), Password::algorithmToString(algo)));
        if (method == Password::Crypt && (algo == Password::CryptBcrypt || algo == Password::CryptSHA256 || algo == Password::CryptSHA512)) {
            t.push_back(std::make_pair(tr("Password hashing rounds"), s.value(QStringLiteral("pwrounds"), SK_DEF_ACC_PWROUNDS).toString()));
        }
    }
#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
    t.push_back(std::make_pair(tr("Password quality threshold"), s.value(QStringLiteral("pwthreshold"), SK_DEF_ACC_PWTHRESHOLD).toString()));
    t.push_back(std::make_pair(tr("Password quality settings file"), s.value(QStringLiteral("pwsettingsfile")).toString()));
#else
    t.push_back(std::make_pair(tr("Password minimum length"), s.value(QStringLiteral("pwminlength"), SK_DEF_ACC_PWMINLENGTH).toString()));
#endif
    printTable(t, tr("User account settings"));
}

void ConsoleOutput::printAccountSettings(QSettings &s) const
{
    QVariantHash h;
    s.beginGroup(QStringLiteral("Accounts"));
    const QStringList keys = s.childKeys();
    for (const QString &key : keys) {
        h.insert(key, s.value(key));
    }
    s.endGroup();
}

void ConsoleOutput::printImapSettings(const QVariantHash &s) const
{
    std::vector<std::pair<QString,QString>> t;
    t.push_back(std::make_pair(tr("Host"), s.value(QStringLiteral("host"), QStringLiteral("localhost")).toString()));
    t.push_back(std::make_pair(tr("Port"), s.value(QStringLiteral("port"), 143).toString()));
    t.push_back(std::make_pair(tr("Protocol"), Imap::networkProtocolToString(s.value(QStringLiteral("protocol"), SK_DEF_IMAP_PROTOCOL).value<quint8>())));
    t.push_back(std::make_pair(tr("Encryption"), Imap::encryptionTypeToString(s.value(QStringLiteral("encryption"), SK_DEF_IMAP_ENCRYPTION).value<quint8>())));
    const QString peerName = s.value(QStringLiteral("peername")).toString();
    if (!peerName.isEmpty()) {
        t.push_back(std::make_pair(tr("Peer name"), peerName));
    }
    t.push_back(std::make_pair(tr("Authentication mechanism"), Imap::authMechToString(s.value(QStringLiteral("authmech"), SK_DEF_IMAP_AUTHMECH).value<quint8>())));
    t.push_back(std::make_pair(tr("Admin user"), s.value(QStringLiteral("user")).toString()));
    t.push_back(std::make_pair(tr("Password"), QStringLiteral("******")));
    t.push_back(std::make_pair(tr("Unix hierarchy seperator"), s.value(QStringLiteral("unixhierarchysep"), SK_DEF_IMAP_UNIXHIERARCHYSEP).toBool() ? tr("enabled") : tr("disabled")));
    t.push_back(std::make_pair(tr("Domain as prefix"), s.value(QStringLiteral("domainasprefix"), SK_DEF_IMAP_DOMAINASPREFIX).toBool() ? tr("enabled") : tr("disabled")));
    t.push_back(std::make_pair(tr("Fully qualified user name"), s.value(QStringLiteral("fqun"), SK_DEF_IMAP_FQUN).toBool() ? tr("enabled") : tr("disabled")));
    QString cm;
    switch(s.value(QStringLiteral("createmailbox"), SK_DEF_IMAP_CREATEMAILBOX).toInt()) {
    case 0:
        cm = tr("all by IMAP server");
        break;
    case 1:
        cm = tr("login after creation");
        break;
    case 2:
        cm = tr("only set quota");
        break;
    case 3:
        cm = tr("all by Skaffari");
        break;
    default:
        cm = tr("invalid value");
        break;
    }
    t.push_back(std::make_pair(tr("Create mailbox"), cm));
    printTable(t, tr("IMAP settings"));
}

void ConsoleOutput::printImapSettings(QSettings &s) const
{
    QVariantHash h;
    s.beginGroup(QStringLiteral("IMAP"));
    const QStringList keys = s.childKeys();
    for (const QString &key : keys) {
        h.insert(key, s.value(key));
    }
    s.endGroup();
}

void ConsoleOutput::printSkaffariSettings(const QVariantHash &s) const
{
    std::vector<std::pair<QString,QString>> t;
    QString lb = s.value(QStringLiteral("logging_backend"), QStringLiteral("stdout")).toString();
#ifdef WITH_SYSTEMD
    const QStringList supportedLoggingBackends{QStringLiteral("syslog"), QStringLiteral("stdout"), QStringLiteral("journald")};
#else
    const QStringList supportedLoggingBackends{QStringLiteral("syslog"), QStringLiteral("stdout")};
#endif
    if (!supportedLoggingBackends.contains(lb)) {
        lb = QStringLiteral("stdout");
    }
    t.push_back(std::make_pair(tr("Logging backend"), lb));
    const bool useMemcached =  s.value(QStringLiteral("usememcached"), false).toBool();
    t.push_back(std::make_pair(tr("Memcached"), useMemcached ? tr("enabled") : tr("disabled")));
    const bool useMemcachedSession = s.value(QStringLiteral("usememcachedsession"), false).toBool();
    t.push_back(std::make_pair(tr("Memcached session"), useMemcached && useMemcachedSession ? tr("enabled") : tr("disabled")));
    printTable(t, tr("Skaffari Settings"));
}

void ConsoleOutput::printSkaffariSettings(QSettings &s) const
{
    QVariantHash h;
    s.beginGroup(QStringLiteral("Skaffari"));
    const QStringList keys = s.childKeys();
    for (const QString &key : keys) {
        h.insert(key, s.value(key));
    }
    s.endGroup();
}
