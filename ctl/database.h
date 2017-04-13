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

class Database
{
    Q_DECLARE_TR_FUNCTIONS(Database)
public:
    Database();
    Database(const QString &type, const QString &host, quint16 port, const QString &name, const QString &user, const QString &password);

    bool open();
    bool open(const QString &type, const QString &host, quint16 port, const QString &name, const QString &user, const QString &password);

    QVersionNumber installedVersion() const;
    QVersionNumber sqlFilesVersion() const;

    bool installDatabase() const;
    bool setAdmin(const QString &adminUser, const QByteArray &adminPassword) const;
    bool checkAdmin() const;

    QSqlError lastDbError() const;

private:
    QSqlDatabase m_db;

    QFileInfoList getSqlFiles() const;

    int setAdminAccount(const QString &user, const QByteArray &pass) const;
    bool rollbackAdminAccount(int adminId) const;

    bool setAdminSettings(int adminId) const;
    bool rollbackAdminSettings(int adminId) const;

    bool setAdminDomains(int adminId) const;
    bool rollbackAdminDomains(int adminId) const;
};

#endif // DATABASE_H
