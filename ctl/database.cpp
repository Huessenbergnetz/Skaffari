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

#include "database.h"
#include <QDir>
#include <QSqlQuery>
#include <QFile>
#include <QTextStream>

#include <QDebug>

Database::Database()
{

}

Database::Database(const QString &type, const QString &host, quint16 port, const QString &name, const QString &user, const QString &password)
{
    m_db = QSqlDatabase::addDatabase(type);
    m_db.setDatabaseName(name);
    m_db.setUserName(user);
    m_db.setPassword(password);

    if (host[0] == QChar('/')) {
        m_db.setConnectOptions(QStringLiteral("UNIX_SOCKET=%1").arg(host));
    } else {
        m_db.setHostName(host);
        m_db.setPort(port);
    }
}


bool Database::open()
{
    return m_db.open();
}


bool Database::open(const QString &type, const QString &host, quint16 port, const QString &name, const QString &user, const QString &password)
{
    m_db = QSqlDatabase::addDatabase(type);
    m_db.setDatabaseName(name);
    m_db.setUserName(user);
    m_db.setPassword(password);

    if (host[0] == QChar('/')) {
        m_db.setConnectOptions(QStringLiteral("UNIX_SOCKET=%1").arg(host));
    } else {
        m_db.setHostName(host);
        m_db.setPort(port);
    }

    return m_db.open();
}


QSqlError Database::lastDbError() const
{
    return m_db.lastError();
}


QVersionNumber Database::installedVersion() const
{
    QVersionNumber version;

    QSqlQuery q;

    q.exec(QStringLiteral("SELECT val FROM systeminfo WHERE name = 'skaffari_db_version'"));

    if (q.next()) {
        version = QVersionNumber::fromString(q.value(0).toString());
    }

    return version;
}


QVersionNumber Database::sqlFilesVersion() const
{
    QVersionNumber version;

    QFileInfoList fil = getSqlFiles();
    if (!fil.empty()) {
        QString fn = fil.last().fileName();
        fn.chop(4);
        version = QVersionNumber::fromString(fn);
    }

    return version;
}


bool Database::installDatabase() const
{
    bool success = false;

    const QFileInfoList fil = getSqlFiles();
    if (fil.empty()) {
        printf("%s\n", qUtf8Printable(tr("Empty SQL file list. Aborting.")));
        return success;
    }

    for (const QFileInfo &fi : fil) {
        QFile f(fi.absoluteFilePath());
        if (!f.open(QIODevice::ReadOnly|QIODevice::Text)) {
            printf("%s\n", qUtf8Printable(tr("Failed to open file %1 for reading. Aborting.").arg(fi.absoluteFilePath())));
            return success;
        }
        f.close();
    }

    QSqlQuery q(m_db);

    for (const QFileInfo &fi : fil) {
        printf("%s\n", qUtf8Printable(tr("Applying SQL statements from %1.").arg(fi.absoluteFilePath())));
        QFile f(fi.absoluteFilePath());
        f.open(QIODevice::ReadOnly|QIODevice::Text);
        QTextStream in(&f);
        const QString sql = in.readAll();
        if (!q.exec(sql)) {
            printf("%s\n", qUtf8Printable(q.lastError().text()));
            printf("%s\n", qUtf8Printable(tr("Failed to apply SQL statements from %1. Aborting.").arg(fi.absoluteFilePath())));
            return success;
        }
    }

    success = true;

    return success;
}

bool Database::setAdmin(const QString &adminUser, const QByteArray &adminPassword) const
{
    bool ret = false;

    const int id = setAdminAccount(adminUser, adminPassword);
    if (id <= 0) {
        return ret;
    }

    if (!setAdminSettings(id)) {
        rollbackAdminAccount(id);
        return ret;
    }

    if (!setAdminDomains(id)) {
        rollbackAdminSettings(id);
        rollbackAdminAccount(id);
        return ret;
    }

    ret = true;

    return ret;
}


int Database::setAdminAccount(const QString &user, const QByteArray &pass) const
{
    int id = 0;

    QSqlQuery q(m_db);
    if (!q.prepare(QStringLiteral("INSERT INTO adminuser (username, password) VALUES (?, ?)"))) {
        printf("%s\n", qUtf8Printable(q.lastError().text()));
        printf("%s\n", qUtf8Printable(tr("Failed to create admin account in database. Aborting.")));
        return id;
    }

    q.addBindValue(user);
    q.addBindValue(pass);

    if (!q.exec()) {
        printf("%s\n", qUtf8Printable(q.lastError().text()));
        printf("%s\n", qUtf8Printable(tr("Failed to create admin account in database. Aborting.")));
        return id;
    }

    id = q.lastInsertId().toInt();

    return id;
}


bool Database::rollbackAdminAccount(int adminId) const
{
    bool ret = false;

    QSqlQuery q(m_db);
    if (!q.prepare(QStringLiteral("DELETE FROM adminuser WHERE id = ?"))) {
        printf("%s\n", qUtf8Printable(q.lastError().text()));
        printf("%s\n", qUtf8Printable(tr("Failed to revert admin account changes in database. Aborting.")));
        return ret;
    }

    q.addBindValue(adminId);

    if (!q.exec()) {
        printf("%s\n", qUtf8Printable(q.lastError().text()));
        printf("%s\n", qUtf8Printable(tr("Failed to revert admin account changes in database. Aborting.")));
        return ret;
    }

    ret = true;

    return ret;
}


bool Database::setAdminSettings(int adminId) const
{
    bool ret = false;

    QSqlQuery q(m_db);
    if (!q.prepare(QStringLiteral("INSERT INTO settings (admin_id) VALUES (?)"))) {
        printf("%s\n", qUtf8Printable(q.lastError().text()));
        printf("%s\n", qUtf8Printable(tr("Failed to insert admin settings into database. Aborting.")));
        return ret;
    }

    q.addBindValue(adminId);

    if (!q.exec()) {
        printf("%s\n", qUtf8Printable(q.lastError().text()));
        printf("%s\n", qUtf8Printable(tr("Failed to insert admin settings into database. Aborting.")));
        return ret;
    }

    ret = true;

    return ret;
}


bool Database::rollbackAdminSettings(int adminId) const
{
    bool ret = false;

    QSqlQuery q(m_db);
    if (!q.prepare(QStringLiteral("DELETE FROM settings WHERE admin_id = ?"))) {
        printf("%s\n", qUtf8Printable(q.lastError().text()));
        printf("%s\n", qUtf8Printable(tr("Failed to revert admin settings in database. Aborting.")));
        return ret;
    }

    q.addBindValue(adminId);

    if (!q.exec()) {
        printf("%s\n", qUtf8Printable(q.lastError().text()));
        printf("%s\n", qUtf8Printable(tr("Failed to revert admin settings in database. Aborting.")));
        return ret;
    }

    ret = true;

    return ret;
}


bool Database::setAdminDomains(int adminId) const
{
    bool ret = false;

    QSqlQuery q(m_db);
    if (!q.prepare(QStringLiteral("INSERT INTO domainadmin (domain_id, admin_id) VALUES (0, ?)"))) {
        printf("%s\n", qUtf8Printable(q.lastError().text()));
        printf("%s\n", qUtf8Printable(tr("Failed to insert admin to domain connection into database. Aborting.")));
        return ret;
    }

    q.addBindValue(adminId);

    if (!q.exec()) {
        printf("%s\n", qUtf8Printable(q.lastError().text()));
        printf("%s\n", qUtf8Printable(tr("Failed to insert admin to domain connection into database. Aborting.")));
        return ret;
    }

    ret = true;

    return ret;
}


bool Database::rollbackAdminDomains(int adminId) const
{
    bool ret = false;

    QSqlQuery q(m_db);
    if (!q.prepare(QStringLiteral("DELETE FROM domainadmin WHERE admin_id = ?"))) {
        printf("%s\n", qUtf8Printable(q.lastError().text()));
        printf("%s\n", qUtf8Printable(tr("Failed to revert admin to domain connections in database. Aborting.")));
        return ret;
    }

    q.addBindValue(adminId);

    if (!q.exec()) {
        printf("%s\n", qUtf8Printable(q.lastError().text()));
        printf("%s\n", qUtf8Printable(tr("Failed to revert admin to domain connections in database. Aborting.")));
        return ret;
    }

    ret = true;

    return ret;
}


bool Database::checkAdmin() const
{
    bool success = false;

    QSqlQuery q(m_db);

    if (!q.exec(QStringLiteral("SELECT username FROM adminuser WHERE type = 0"))) {
        printf("%s\n", qUtf8Printable(tr("Failed to query admin users from database.")));
    }

    success = (q.size() > 0);
    printf("%s\n", qUtf8Printable(tr("Found %n admin user(s) in database.", "", q.size())));

    return success;
}


QFileInfoList Database::getSqlFiles() const
{
    QFileInfoList fil;

    QDir sqlDir(QStringLiteral(SKAFFARI_SQLDIR) + QLatin1String("/") + m_db.driverName());

    fil = sqlDir.entryInfoList(QStringList(QStringLiteral("*.sql")), QDir::Files, QDir::Name);

    return fil;
}
