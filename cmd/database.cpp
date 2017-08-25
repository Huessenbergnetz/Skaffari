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
#include <QDateTime>

#include "../common/config.h"

Database::Database()
{

}

Database::Database(const QString &type, const QString &host, quint16 port, const QString &name, const QString &user, const QString &password, const QString &conName) :
    m_conName(conName)
{
    m_db = QSqlDatabase::addDatabase(type, m_conName);
    m_db.setDatabaseName(name);
    m_db.setUserName(user);
    m_db.setPassword(password);

    if (host[0] == QLatin1Char('/')) {
        m_db.setConnectOptions(QStringLiteral("UNIX_SOCKET=%1").arg(host));
    } else {
        m_db.setHostName(host);
        m_db.setPort(port);
    }
}


bool Database::open()
{
    bool ret = false;
    ret = m_db.open();
    if (Q_LIKELY(ret)) {
        m_lastError = QSqlError();
    } else {
        m_lastError = m_db.lastError();
        m_db.close();
        if (QSqlDatabase::contains(m_conName)) {
            QSqlDatabase::removeDatabase(m_conName);
        }
    }
    return ret;
}


bool Database::open(const QString &type, const QString &host, quint16 port, const QString &name, const QString &user, const QString &password, const QString &conName)
{
    if (m_db.isOpen()) {
        m_db.close();
    }

    if (QSqlDatabase::contains(conName)) {
        QSqlDatabase::removeDatabase(conName);
    }

    m_db = QSqlDatabase::addDatabase(type, conName);
    m_db.setDatabaseName(name);
    m_db.setUserName(user);
    m_db.setPassword(password);

    if (host[0] == QLatin1Char('/')) {
        m_db.setConnectOptions(QStringLiteral("UNIX_SOCKET=%1").arg(host));
    } else {
        m_db.setHostName(host);
        m_db.setPort(port);
    }

    return open();
}


QSqlError Database::lastDbError() const
{
    return m_lastError;
}


QVersionNumber Database::installedVersion() const
{
    QVersionNumber version;

    QSqlQuery q;

    q.exec(QStringLiteral("SELECT val FROM systeminfo WHERE name = 'skaffari_db_version'"));

    if (Q_LIKELY(q.next())) {
        version = QVersionNumber::fromString(q.value(0).toString());
    }

    return version;
}


QVersionNumber Database::sqlFilesVersion() const
{
    QVersionNumber version;

    QFileInfoList fil = getSqlFiles();
    if (Q_LIKELY(!fil.empty())) {
        QString fn = fil.last().fileName();
        fn.chop(4);
        version = QVersionNumber::fromString(fn);
    }

    return version;
}


bool Database::installDatabase()
{
    bool success = false;

    const QFileInfoList fil = getSqlFiles();
    if (Q_UNLIKELY(fil.empty())) {
        m_lastError = QSqlError(tr("Empty SQL file list. Aborting."), QString(), QSqlError::UnknownError);
        return success;
    }

    for (const QFileInfo &fi : fil) {
        QFile f(fi.absoluteFilePath());
        if (Q_UNLIKELY(!f.open(QFile::ReadOnly|QFile::Text))) {
            m_lastError = QSqlError(tr("Failed to open file %1 for reading. Aborting.").arg(fi.absoluteFilePath()), QString(), QSqlError::UnknownError);
            return success;
        }
        f.close();
    }

    QSqlQuery q(m_db);

    for (const QFileInfo &fi : fil) {
//        printf("%s\n", qUtf8Printable(tr("Applying SQL statements from %1.").arg(fi.absoluteFilePath())));
        QFile f(fi.absoluteFilePath());
        f.open(QFile::ReadOnly|QFile::Text);
        QTextStream in(&f);
        const QString sql = in.readAll();
        if (Q_UNLIKELY(!q.exec(sql))) {
            m_lastError = q.lastError();
            m_lastError.setDriverText(tr("Failed to apply SQL statements from %1. Aborting.").arg(fi.absoluteFilePath()));
            return success;
        }
    }

    success = true;

    return success;
}

bool Database::setAdmin(const QString &adminUser, const QByteArray &adminPassword)
{
    bool ret = false;

    const int id = setAdminAccount(adminUser, adminPassword);
    if (Q_UNLIKELY(id <= 0)) {
        return ret;
    }

    if (Q_UNLIKELY(!setAdminSettings(id))) {
        rollbackAdminAccount(id);
        return ret;
    }

    if (Q_UNLIKELY(!setAdminDomains(id))) {
        rollbackAdminSettings(id);
        rollbackAdminAccount(id);
        return ret;
    }

    ret = true;

    return ret;
}


int Database::setAdminAccount(const QString &user, const QByteArray &pass)
{
    int id = 0;

    QSqlQuery q(m_db);
    if (Q_UNLIKELY(!q.prepare(QStringLiteral("INSERT INTO adminuser (username, password, type, created_at, updated_at) VALUES (?, ?, ?, ?, ?)")))) {
        m_lastError = q.lastError();
        return id;
    }

    QDateTime current = QDateTime::currentDateTimeUtc();
    q.addBindValue(user);
    q.addBindValue(pass);
    q.addBindValue(0);
    q.addBindValue(current);
    q.addBindValue(current);

    if (Q_UNLIKELY(!q.exec())) {
        m_lastError = q.lastError();
        return id;
    }

    id = q.lastInsertId().toInt();

    return id;
}


bool Database::rollbackAdminAccount(int adminId) const
{
    bool ret = false;

    QSqlQuery q(m_db);
    if (Q_UNLIKELY(!q.prepare(QStringLiteral("DELETE FROM adminuser WHERE id = ?")))) {
        return ret;
    }

    q.addBindValue(adminId);

    if (Q_UNLIKELY(!q.exec())) {
        return ret;
    }

    ret = true;

    return ret;
}


bool Database::setAdminSettings(int adminId)
{
    bool ret = false;

    QSqlQuery q(m_db);
    if (Q_UNLIKELY(!q.prepare(QStringLiteral("INSERT INTO settings (admin_id) VALUES (?)")))) {
        m_lastError = q.lastError();
        return ret;
    }

    q.addBindValue(adminId);

    if (Q_UNLIKELY(!q.exec())) {
        m_lastError = q.lastError();
        return ret;
    }

    ret = true;

    return ret;
}


bool Database::rollbackAdminSettings(int adminId) const
{
    bool ret = false;

    QSqlQuery q(m_db);
    if (Q_UNLIKELY(!q.prepare(QStringLiteral("DELETE FROM settings WHERE admin_id = ?")))) {
        return ret;
    }

    q.addBindValue(adminId);

    if (Q_UNLIKELY(!q.exec())) {
        return ret;
    }

    ret = true;

    return ret;
}


bool Database::setAdminDomains(int adminId)
{
    bool ret = false;

    QSqlQuery q(m_db);
    if (Q_UNLIKELY(!q.prepare(QStringLiteral("INSERT INTO domainadmin (domain_id, admin_id) VALUES (0, ?)")))) {
        m_lastError = q.lastError();
        return ret;
    }

    q.addBindValue(adminId);

    if (Q_UNLIKELY(!q.exec())) {
        m_lastError = q.lastError();
        return ret;
    }

    ret = true;

    return ret;
}


bool Database::rollbackAdminDomains(int adminId) const
{
    bool ret = false;

    QSqlQuery q(m_db);
    if (Q_UNLIKELY(!q.prepare(QStringLiteral("DELETE FROM domainadmin WHERE admin_id = ?")))) {
        return ret;
    }

    q.addBindValue(adminId);

    if (Q_UNLIKELY(!q.exec())) {
        return ret;
    }

    ret = true;

    return ret;
}


uint Database::checkAdmin() const
{
    uint adminCount = 0;

    QSqlQuery q(m_db);

    q.exec(QStringLiteral("SELECT COUNT(id) FROM adminuser WHERE type = 0"));

    if (Q_LIKELY(q.next())) {
        adminCount = q.value(0).toUInt();
    }

    return adminCount;
}


bool Database::setCryusAdmin(const QString &cyrusAdmin, const QByteArray &cyrusPassword)
{
    bool ret = false;

    QSqlQuery q(m_db);
    if (Q_UNLIKELY(!q.prepare(QStringLiteral("INSERT INTO accountuser (username, domain_id, password, prefix, domain_name, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?, ?)")))) {
        m_lastError = q.lastError();
        return ret;
    }

    QDateTime current = QDateTime::currentDateTimeUtc();

    q.addBindValue(cyrusAdmin);
    q.addBindValue(0);
    q.addBindValue(cyrusPassword);
    q.addBindValue(QStringLiteral(""));
    q.addBindValue(QStringLiteral(""));
    q.addBindValue(current);
    q.addBindValue(current);

    ret = q.exec();
    if (Q_UNLIKELY(!ret)) {
        m_lastError = q.lastError();
    }

    return ret;
}


QString Database::checkCyrusAdmin() const
{
    QString admin;

    QSqlQuery q(m_db);

    q.exec(QStringLiteral("SELECT username FROM accountuser WHERE prefix = '' AND domain_name = ''"));

    if (Q_LIKELY(q.next())) {
        admin = q.value(0).toString();
    }

    return admin;
}


QFileInfoList Database::getSqlFiles() const
{
    QFileInfoList fil;

    QDir sqlDir(QStringLiteral(SKAFFARI_SQLDIR) + QLatin1Char('/') + m_db.driverName());

    fil = sqlDir.entryInfoList(QStringList(QStringLiteral("*.sql")), QDir::Files, QDir::Name);

    return fil;
}



QSqlDatabase Database::getDb() const
{
    return m_db;
}


void Database::deleteAll()
{
    QSqlQuery q(m_db);
    const QStringList tables = m_db.tables();
    for (const QString &table : tables) {
        q.exec(QStringLiteral("DROP TABLE %1").arg(table));
    }
}


QVariantHash Database::loadOptions()
{
    QVariantHash options;

    QSqlQuery q(m_db);


    if (Q_LIKELY(q.exec(QStringLiteral("SELECT option_name, option_value FROM options")))) {
        while (q.next()) {
            options.insert(q.value(0).toString(), q.value(1));
        }
    } else {
        m_lastError = q.lastError();
    }

    return options;
}

void Database::saveOptions(const QVariantHash &options)
{
    if (!options.empty()) {
        QSqlQuery q(m_db);
        QHash<QString,QVariant>::const_iterator i = options.constBegin();
        while (i != options.constEnd()) {
            if (Q_UNLIKELY(!q.prepare(QStringLiteral("INSERT INTO options (option_name, option_value) "
                                                     "VALUES (:option_name, :option_value) "
                                                     "ON DUPLICATE KEY UPDATE "
                                                     "option_value = :option_value")))) {
                m_lastError = q.lastError();
                break;
            }
            q.bindValue(QStringLiteral(":option_name"), i.key());
            q.bindValue(QStringLiteral(":option_value"), i.value());
            if (Q_UNLIKELY(!q.exec())) {
                m_lastError = q.lastError();
                break;
            }
            ++i;
        }
    }
}
