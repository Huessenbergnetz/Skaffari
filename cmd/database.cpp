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
    m_conName = conName;

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

bool Database::open(const QVariantHash &params, const QString &conName)
{
    return open(params.value(QStringLiteral("type")).toString(),
                params.value(QStringLiteral("host")).toString(),
                params.value(QStringLiteral("port")).value<quint16>(),
                params.value(QStringLiteral("name")).toString(),
                params.value(QStringLiteral("user")).toString(),
                params.value(QStringLiteral("password")).toString(),
                conName);
}

QSqlError Database::lastDbError() const
{
    return m_lastError;
}

QVersionNumber Database::installedVersion() const
{
    QVersionNumber version;

    QSqlQuery q(m_db);

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
        QFile f(fi.absoluteFilePath());
        f.open(QFile::ReadOnly|QFile::Text);
        QTextStream in(&f);
        const QString sql = in.readAll();
        if (Q_UNLIKELY(!q.exec(sql))) {
            m_lastError = q.lastError();
            m_lastError = QSqlError(tr("Failed to apply SQL statements from %1. Aborting.").arg(fi.absoluteFilePath()), q.lastError().databaseText(), q.lastError().type());
            return success;
        }
    }

    success = true;

    return success;
}

bool Database::setAdmin(const QString &adminUser, const QByteArray &adminPassword)
{
    bool ret = false;

    if (Q_LIKELY(m_db.transaction())) {

        QSqlQuery q(m_db);
        if (Q_LIKELY(q.prepare(QStringLiteral("INSERT INTO adminuser (username, password, type, created_at, updated_at) VALUES (?, ?, ?, ?, ?)")))) {
            QDateTime current = QDateTime::currentDateTimeUtc();
            q.addBindValue(adminUser);
            q.addBindValue(adminPassword);
            q.addBindValue(255);
            q.addBindValue(current);
            q.addBindValue(current);

            if (Q_LIKELY(q.exec())) {
                const uint id = q.lastInsertId().toUInt();

                if (id > 0) {
                    q.prepare(QStringLiteral("INSERT INTO settings (admin_id) VALUES (?)"));
                    q.addBindValue(id);
                    if (Q_LIKELY(q.exec())) {
                        if (Q_LIKELY(m_db.commit())) {
                            ret = true;
                        } else {
                            m_lastError = m_db.lastError();
                            m_db.rollback();
                        }

                    } else {
                        m_lastError = q.lastError();
                        m_db.rollback();
                    }

                } else {
                    m_db.rollback();
                }

            } else {
                m_lastError = q.lastError();
            }

        } else {
            m_lastError = q.lastError();
        }

    } else {
        m_lastError = m_db.lastError();
    }

    return ret;
}

uint Database::checkAdmin() const
{
    uint adminCount = 0;

    QSqlQuery q(m_db);

    q.exec(QStringLiteral("SELECT COUNT(id) FROM adminuser WHERE type = 255"));

    if (Q_LIKELY(q.next())) {
        adminCount = q.value(0).toUInt();
    }

    return adminCount;
}

bool Database::setCryusAdmin(const QString &cyrusAdmin, const QByteArray &cyrusPassword)
{
    bool ret = false;

    QSqlQuery q(m_db);
    if (Q_UNLIKELY(!q.prepare(QStringLiteral("INSERT INTO accountuser (username, domain_id, password, created_at, updated_at) VALUES (?, ?, ?, ?, ?)")))) {
        m_lastError = q.lastError();
        return ret;
    }

    const QDateTime current = QDateTime::currentDateTimeUtc();

    q.addBindValue(cyrusAdmin);
    q.addBindValue(0);
    q.addBindValue(cyrusPassword);
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

    q.exec(QStringLiteral("SELECT username FROM accountuser WHERE domain_id = 0"));

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

bool Database::empty() const
{
    return m_db.tables(QSql::Tables).empty() && m_db.tables(QSql::Views).empty();
}

bool Database::clear() const
{
    if (!empty()) {
        QSqlQuery q(m_db);

        const QStringList tables = m_db.tables(QSql::Tables);
        for (const QString &table : tables) {
            if (!q.exec(QStringLiteral("DROP TABLE %1").arg(table))) {
                return false;
            }
        }

        const QStringList views = m_db.tables(QSql::Views);
        for (const QString &view : views) {
            if (!q.exec(QStringLiteral("DROP VIEW %1").arg(view))) {
                return false;
            }
        }
    }

    return true;
}
