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

#ifndef PASSWORD_H
#define PASSWORD_H

#include <QString>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(SK_PASSWORD)

class Password
{
public:
    enum Method : quint8 {
        PlainText       = 0,
        Crypt           = 1,
        MySQL           = 2,
        MD5             = 3,
        SHA1            = 4
    };

    enum Algorithm : quint8 {
        Default         = 0,
        CryptDES        = 1,
        CryptMD5        = 2,
        CryptSHA256     = 3,
        CryptSHA512     = 4,
        CryptBcrypt     = 5,
        MySQLNew        = 6,
        MySQLOld        = 7
    };

    explicit Password(const QString &pw);

    QByteArray encrypt(Method type, Algorithm method = Default, quint32 rounds = 0);
    bool check(const QByteArray &savedPw);

private:
    QString m_password;

    static QByteArray requestSalt(quint16 length, const QByteArray allowedChars = QByteArray());
};

#endif // PASSWORD_H
