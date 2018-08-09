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

#ifndef PASSWORD_H
#define PASSWORD_H

#include <QString>
#include <QLoggingCategory>
#include <QCoreApplication>

Q_DECLARE_LOGGING_CATEGORY(SK_PASSWORD)

/*!
 * \ingroup skaffaricore
 * \brief Handles passwords encrypted with the crypt(3) function or MySQL password hashing.
 *
 * For more information about crypt(3) see man 3 crypt, for more information about MySQL encrpytion
 * see <A HREF="https://dev.mysql.com/doc/refman/5.7/en/password-hashing.html">Password Hasing in MySQL</A>.
 *
 * The methods and algorithms are used as they are used by <A HREF="https://github.com/NigelCunningham/pam-MySQL/blob/master/README">pam_mysql</A>.
 */
class Password
{
    Q_GADGET
    Q_DECLARE_TR_FUNCTIONS(Password)
public:
    /*!
     * \brief Different methods for password hashing.
     */
    enum Method : quint8 {
        PlainText       = 0,    /**< No encryption, passwords stored in plaintext. (HIGHLY DISCOURAGED) */
        Crypt           = 1,    /**< Use crypt(3) function. */
        MySQL           = 2,    /**< Use MySQL PASSWORD() function. */
        MD5             = 3,    /**< Use plain hex MD5. Not recommended.*/
        SHA1            = 4     /**< Use plain hex SHA1. */
    };
    Q_ENUM(Method)

    /*!
     * \brief Different algorithms for crpyt(3) and MySQL password hashing methods.
     */
    enum Algorithm : quint8 {
        Default         = 0,    /**< Uses the default algorithm for the specified method. (SHA256 for crypt(3) and the new style hashing for MySQL) */
        CryptDES        = 1,    /**< Traditional DES-based encryption of the crypt(3) function. No longer offering adequate security. */
        CryptMD5        = 2,    /**< FreeBSD-style MD5-based encryption of the crypt(3) function. Not recommended anymore. */
        CryptSHA256     = 3,    /**< SHA256-based encryption of the crypt(3) function. */
        CryptSHA512     = 4,    /**< SHA512-based encryption of the crypt(3) function. */
        CryptBcrypt     = 5,    /**< OpenBSD-style Blowfish-based (bcrypt) encryption of the crypt(3) function. This is not available on every platform/distribution. */
        MySQLNew        = 32,   /**< New hashing algorithm of the MySQL PASSWORD() function. */
        MySQLOld        = 33    /**< Old hashing algorithm of the MySQL PASSWORD() function. */
    };
    Q_ENUM(Algorithm)

    /*!
     * \brief Constructs a new Password object.
     * \param pw The unencrypted password.
     */
    explicit Password(const QString &pw);

    /*!
     * \brief Encrypt the password set in the constructor according to the parameters.
     *
     * If using Crypt as \a method and CryptSHA256, CryptSHA512 or CryptBcrypt as \a algo,
     * you can set the number of rounds used for encrytption. For CryptSHA256 and CryptSHA512 the
     * number of iterations can be between 1000 and 999999999, setting a value outside this range
     * it will fall back to the default of 5000 iterations. For CryptBcrypt the numer of iterations
     * is the base-2 logarithm of the \a rounds value. Current bcrypt implementations support values
     * between 4 and 31. If the value for \a rounds is out of bounds, it will be either set to the
     * lowest or highest supported value.
     *
     * \param method    The method to use for encrypting the password.
     * \param algo      The algorithm to use if the method supports different algorithms.
     * \param rounds    The number of encryption rounds to use if the algorithm supports it.
     * \return          Encrypted password.
     */
    QByteArray encrypt(Method method, Algorithm algo = Default, quint32 rounds = 0) const;

    /*!
     * \brief Checks if the unencrypted password in the constructor is equal to the saved password.
     * \todo Implement password checking. It currently only returns false.
     * \param savedPw   The encrypted saved password.
     * \return          True if both passwords are equal.
     */
    bool check(const QByteArray &savedPw);

    /*!
     * \brief Returns the human readable string of the encryption \a method.
     */
    static QString methodToString(Method method);

    /*!
     * \brief Returns the human readable string of the encryption \a method.
     */
    static QString methodToString(quint8 method);

    /*!
     * \brief Return the human readable string of the encryption \a algorithm.
     */
    static QString algorithmToString(Algorithm algorithm);

    /*!
     * \brief Return the human readable string of the encryption \a algorithm.
     */
    static QString algorithmToString(quint8 algorithm);

private:
    QString m_password;

    /*!
     * \brief Requests a salt value of given \a length and with the \a allowed characters.
     *
     * This uses \c /dev/urandom to request pseudo random numbers.
     *
     * \param length        Length of the salt value.
     * \param allowedChars  Array of allowed characters.
     * \return              Byte array that can be used as salt.
     */
    static QByteArray requestSalt(quint16 length, const QByteArray &allowedChars = QByteArray());
};

#endif // PASSWORD_H
