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

class Database
{
    Q_DECLARE_TR_FUNCTIONS(Database)
public:
    Database();
    Database(const QString &type, const QString &host, quint16 port, const QString &name, const QString &user, const QString &password, const QString &conName = QLatin1String(QSqlDatabase::defaultConnection));

    bool open();
    bool open(const QString &type, const QString &host, quint16 port, const QString &name, const QString &user, const QString &password, const QString &conName = QLatin1String(QSqlDatabase::defaultConnection));

    QVersionNumber installedVersion() const;
    QVersionNumber sqlFilesVersion() const;

    bool installDatabase();
    bool setAdmin(const QString &adminUser, const QByteArray &adminPassword);
    uint checkAdmin() const;

    bool setCryusAdmin(const QString &cyrusAdmin, const QByteArray &cyrusPassword);
    QString checkCyrusAdmin() const;

    QSqlError lastDbError() const;
    QSqlDatabase getDb() const;
    void deleteAll();

    QVariantHash loadOptions();
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
