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

#ifndef DATABASE_H
#define DATABASE_H

#include <QCoreApplication>
#include <QString>
#include <QSqlDatabase>
#include <QSqlError>
#include <QFileInfoList>
#include <QVersionNumber>
#include <QVariantHash>

/*!
 * \ingroup skaffaricmd
 * \brief Handles setup database operations.
 */
class Database
{
    Q_DECLARE_TR_FUNCTIONS(Database)
public:
    /*!
     * \brief Constructs a new Database object without defining a database connection.
     */
    Database();
    /*!
     * \brief Constructs a new Database object with the given connection parameters.
     * \param type      the type of the database, like QMYSQL
     * \param host      the host address or name or full path to local socket file
     * \param port      the database port
     * \param name      the name of the database
     * \param user      the name of the database user
     * \param password  the password of the database user
     * \param conName   the name used for this connection
     */
    Database(const QString &type, const QString &host, quint16 port, const QString &name, const QString &user, const QString &password, const QString &conName = QLatin1String(QSqlDatabase::defaultConnection));

    /*!
     * \brief Opens the database connection and returns \c true on success.
     */
    bool open();
    /*!
     * \brief Opens the database connection with the given parameters and returns \c true on success.
     * \param type      the type of the database, like QMYSQL
     * \param host      the host address or name or full path to local socket file
     * \param port      the database port
     * \param name      the name of the database
     * \param user      the name of the database user
     * \param password  the password of the database user
     * \param conName   the name used for this connection
     */
    bool open(const QString &type, const QString &host, quint16 port, const QString &name, const QString &user, const QString &password, const QString &conName = QLatin1String(QSqlDatabase::defaultConnection));

    /*!
     * \brief Opens the database connection the given \a params and \a conName and returns \c on success.
     * \param params    as returned by ConfigInput::askDatabaseConfig()
     * \param conName   the name used for this connection
     */
    bool open(const QVariantHash &params, const QString &conName = QLatin1String(QSqlDatabase::defaultConnection));

    /*!
     * \brief Returns the version of the installed database schema.
     */
    QVersionNumber installedVersion() const;
    /*!
     * \brief Returns the most recent version of the available database schema files.
     */
    QVersionNumber sqlFilesVersion() const;

    /*!
     * \brief Installs the database schema and returns \c true on success.
     *
     * This will use the SQL schema files for installation.
     */
    bool installDatabase();
    /*!
     * \brief Creates \a adminUser with the \a adminPassword in the database and returns \c true on success.
     *
     * This is the super user administrator for the web access.
     */
    bool setAdmin(const QString &adminUser, const QByteArray &adminPassword);
    /*!
     * \brief Returns the number of admin accounts in the database.
     */
    uint checkAdmin() const;

    /*!
     * \brief Create the \a cyrusAdmin with the \a cyrusPassword in the database and returns \c true on succes.
     *
     * This is the Cyrus IMAP admin user.
     */
    bool setCryusAdmin(const QString &cyrusAdmin, const QByteArray &cyrusPassword);
    /*!
     * \brief Returns the name of the Cyrus IMAP server admin user from the database.
     */
    QString checkCyrusAdmin() const;

    /*!
     * \brief Returns the last occurred database error.
     */
    QSqlError lastDbError() const;
    /*!
     * \brief Returns the internal QSqlDatabase object.
     */
    QSqlDatabase getDb() const;
    void deleteAll();

    /*!
     * \brief Returns the options and values from the \a options table.
     */
    QVariantHash loadOptions();

    /*!
     * \brief Saves the \a options to the options table.
     */
    void saveOptions(const QVariantHash &options);

private:
    QSqlDatabase m_db;
    QString m_conName;

    QFileInfoList getSqlFiles() const;

    int setAdminAccount(const QString &user, const QByteArray &pass);
    bool rollbackAdminAccount(int adminId) const;

    bool setAdminSettings(int adminId);
    bool rollbackAdminSettings(int adminId) const;

    bool setAdminDomains(int adminId);
    bool rollbackAdminDomains(int adminId) const;

    QSqlError m_lastError;
};

#endif // DATABASE_H
