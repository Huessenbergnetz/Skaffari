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

#define _XOPEN_SOURCE
#include "password.h"
#include <unistd.h>
#include <QSqlQuery>
#include <QFile>
#include <Cutelyst/Plugins/Utils/Sql>
#include <QCryptographicHash>

Q_LOGGING_CATEGORY(SK_PASSWORD, "skaffari.password")

Password::Password(const QString &pw) :
    m_password(pw)
{

}


QByteArray Password::encrypt(Method method, Algorithm algo, quint32 rounds) const
{
    QByteArray pw;

    if (method == PlainText) {

        pw = m_password.toUtf8();
        qCWarning(SK_PASSWORD) << "Do not use unencrypted passwords!";

    } else if (method == Crypt) {

        QByteArray settings;
        uint pwSize = 0;

        if (algo == CryptDES) {

            settings = Password::requestSalt(2, QByteArrayLiteral("./0123456789ABCDEFGHIJKLMNJOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));
            pwSize = 13;
            qCWarning(SK_PASSWORD) << "Do not use weak hashing/encryption methods for passwords!";

        } else if (algo == CryptMD5) {

            settings = QByteArrayLiteral("$1$");
            settings.append(Password::requestSalt(8));
            settings.append(QByteArrayLiteral("$"));
            pwSize = (settings.size() + 22);
            qCWarning(SK_PASSWORD) << "Do not use weak hashing/encryption methods for passwords!";

        } else if ((algo == CryptSHA256) || (algo == CryptSHA512) || (algo == Default)) {

            if (algo == CryptSHA512) {
                settings = QByteArrayLiteral("$6$rounds=");
            } else {
                settings = QByteArrayLiteral("$5$rounds=");
            }

            if (rounds < 1000) {
                rounds = 5000;
            } else if (rounds > 999999999) {
                rounds = 5000;
            }

            settings.append(QByteArray::number(rounds));
            settings.append(QByteArrayLiteral("$"));
            settings.append(Password::requestSalt(16, QByteArrayLiteral("./0123456789ABCDEFGHIJKLMNJOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz")));
            settings.append(QByteArrayLiteral("$"));

            if (algo == CryptSHA512) {
                pwSize = (settings.size() + 86);
            } else {
                pwSize = (settings.size() + 43);
            }

        } else if (algo == CryptBcrypt) {

            pwSize = 60;
            settings = QByteArrayLiteral("$2y$");
            if (rounds < 4) {
                rounds = 4;
            }
            if (rounds > 31) {
                rounds = 31;
            }
            if (rounds < 10) {
                settings.append(QByteArrayLiteral("0"));
            }
            settings.append(QByteArray::number(rounds));
            settings.append(QByteArrayLiteral("$"));
            settings.append(Password::requestSalt(22, QByteArrayLiteral("./0123456789ABCDEFGHIJKLMNJOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz")));
            settings.append(QByteArrayLiteral("$"));

        } else {

            qCCritical(SK_PASSWORD) << "Password hashing method not supported!";
            return pw;

        }

        pw = QByteArray(crypt(m_password.toUtf8().constData(), settings.constData()), pwSize);

    } else if (method == MySQL) {

        QSqlQuery q;
        if (algo == MySQLOld) {
            q = CPreparedSqlQueryThread(QStringLiteral("SELECT OLD_PASSWORD(:pw)"));
        } else {
            q = CPreparedSqlQueryThread(QStringLiteral("SELECT PASSWORD(:pw)"));
        }
        q.bindValue(QStringLiteral(":pw"), m_password);
        if (q.exec() && q.next()) {
            pw = q.value(0).toString().toUtf8();
        } else {
            qCCritical(SK_PASSWORD) << "Failed to use MySQL password function.";
        }
        qCWarning(SK_PASSWORD) << "Do not use weak hashing/encryption methods for passwords!";

    } else if (method == MD5) {

        pw = QCryptographicHash::hash(m_password.toUtf8(), QCryptographicHash::Md5).toHex();
        qCWarning(SK_PASSWORD) << "Do not use weak hashing/encryption methods for passwords!";

    } else if (method == SHA1) {

        pw = QCryptographicHash::hash(m_password.toUtf8(), QCryptographicHash::Sha1).toHex();
        qCWarning(SK_PASSWORD) << "Do not use weak hashing/encryption methods for passwords!";

    } else {

        qCCritical(SK_PASSWORD) << "Password hashing method not supported!";

    }

    return pw;
}



bool Password::check(const QByteArray &savedPw)
{
    bool ret = false;

    Q_UNUSED(savedPw)

    return ret;
}

QString Password::methodToString(Method method)
{
    switch(method) {
    case PlainText:
        return tr("Clear text");
    case Crypt:
        return QStringLiteral("Crypt");
    case MySQL:
        return QStringLiteral("MySQL");
    case MD5:
        return QStringLiteral("MD5");
    case SHA1:
        return QStringLiteral("SHA1");
    default:
        return QString();
    }
}

QString Password::methodToString(quint8 method)
{
    return Password::methodToString(static_cast<Password::Method>(method));
}

QString Password::algorithmToString(Algorithm algorithm)
{
    switch(algorithm) {
    case Default:
        return tr("Default");
    case CryptDES:
        return QStringLiteral("DES");
    case CryptMD5:
        return QStringLiteral("MD5");
    case CryptSHA256:
        return QStringLiteral("SHA256");
    case CryptSHA512:
        return QStringLiteral("SHA512");
    case CryptBcrypt:
        return QStringLiteral("Bcrypt");
    case MySQLNew:
        //: new MySQL password encryption algorithm
        return tr("MySQL new");
    case MySQLOld:
        //: old MySQL password encryption algorithm
        return tr("MySQL old");
    default:
        return QString();
    }
}

QString Password::algorithmToString(quint8 algorithm)
{
    return Password::algorithmToString(static_cast<Password::Algorithm>(algorithm));
}

QByteArray Password::requestSalt(quint16 length, const QByteArray &allowedChars)
{
    QByteArray salt;

    QFile random(QStringLiteral("/dev/urandom"));
    if (Q_UNLIKELY(!random.open(QIODevice::ReadOnly))) {
    }

    if (allowedChars.isEmpty()) {
        salt = random.read(length).toBase64();
    } else {
        const QByteArray rand = random.read(4 * length).toBase64();
        int i = 0;
        while ((salt.size() < length) && (i < (4 * length))) {
            char part = rand.at(i);
            if (allowedChars.contains(part)) {
                salt.append(part);
            }
            ++i;
        }
    }

    if (salt.size() > length) {
        salt.chop(salt.size() - length);
    }

    return salt;
}
