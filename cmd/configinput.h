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

#ifndef CONFIGINPUT_H
#define CONFIGINPUT_H

#include <QString>
#include <QCoreApplication>
#include <QVariantHash>

#include "consoleoutput.h"

/*!
 * \ingroup skaffaricmd
 * \brief Bundles methods to query the user for different config options on the command line.
 */
class ConfigInput : public ConsoleOutput
{
    Q_DECLARE_TR_FUNCTIONS(ConfigInput)
public:
    /*!
     * \brief Constructs a new ConfigInput object.
     */
    ConfigInput();

    /*!
     * \brief Asks the user for database connection parameters on the command line.
     *
     * Can take a set of defaults that will be used for the questions.
     *
     * \par Keys in the hash (defaults and return value)
     * Key      | Type    | Description
     * ---------|---------|----------------------------------------------------------------------------------------
     * type     | QString | the database type as QSqlDatabase driver name like QMYSQL
     * host     | QString | the database host, either an IP address/host name or absolute path to local socket file
     * port     | quint16 | he database port if no socket file will be used
     * name     | QString | the database name
     * user     | QString | the database user
     * password | QString | the database password
     */
    QVariantHash askDatabaseConfig(const QVariantHash &defaults = QVariantHash()) const;

    /*!
     * \brief Asks the user for IMAP server connection parameters on the command line.
     *
     * Can take a set of defaults that will be used for the questions.
     *
     * \par Keys in the hash (defaults and return value)
     * Key        | Type    | Description
     * -----------|---------|------------------------------------------------------
     * host       | QString | the IMAP server host, either an IP address or a host name
     * port       | quint16 | the IMAP server port
     * user       | QString | the IMAP server admin user name
     * password   | QString | the IMAP server admin user password
     * protocol   | quint8  | QAbstractSocket::NetworkLayerProtocol
     * encryption | quint8  | Imap::EncryptionType
     * peername   | QString | SSL/TLS peer name
     */
    QVariantHash askImapConfig(const QVariantHash &defaults = QVariantHash()) const;

    /*!
     * \brief Asks the user for PBKDF2 admin user password encryption options.
     *
     * Can take a set of defaults that will be used for the questions.
     *
     * \par Keys in the hash (defaults and return value)
     * Key       | Type    | Description
     * ----------|---------|-------------
     * method    | quint8  | QCryptographicHash::Algorithm
     * rounds    | quint32 | number of rounds used for the hashing
     * minlength | quint8  | minimum password length
     */
    QVariantHash askPbkdf2Config(const QVariantHash &defaults = QVariantHash()) const;
};

#endif // CONFIGINPUT_H
