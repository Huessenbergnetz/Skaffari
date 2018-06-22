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

#include "tester.h"
#include <QSettings>
#include <QVersionNumber>
#include "database.h"
#include "imap.h"

Tester::Tester(const QString &confFile) :
    m_confFile(confFile)
{

}


int Tester::exec() const
{
    if (!m_confFile.exists()) {
        //: %1 will contain the absoulute file path
        return fileError(tr("Configuration file not found at %1").arg(m_confFile.absoluteFilePath()));
    }

    if (!m_confFile.isReadable()) {
        //: %1 will contain the absoulute file path
        return fileError(tr("Configuration file at %1 not readable.").arg(m_confFile.absoluteFilePath()));
    }

    QSettings conf(m_confFile.absoluteFilePath(), QSettings::IniFormat);

    conf.beginGroup(QStringLiteral("Database"));
    const QString dbhost = conf.value(QStringLiteral("host"), QStringLiteral("localhost")).toString();
    const QString dbname = conf.value(QStringLiteral("name")).toString();
    const QString dbpass = conf.value(QStringLiteral("password")).toString();
    const QString dbtype = conf.value(QStringLiteral("type")).toString();
    const QString dbuser = conf.value(QStringLiteral("user")).toString();
    const quint16 dbport = conf.value(QStringLiteral("port"), 3306).value<quint16>();
    conf.endGroup();

    Database db(dbtype, dbhost, dbport, dbname, dbuser, dbpass);
    if (!db.open()) {
        return dbError(db.lastDbError());
    }

    const QVersionNumber installedVersion = db.installedVersion();
    if (installedVersion.isNull()) {
        return error(tr("Database layout not installed."));
    }

    const uint adminCount = db.checkAdmin();
    if (adminCount == 0) {
        return error(tr("No administrator account available."));
    }

    conf.beginGroup(QStringLiteral("IMAP"));
    const QString imapuser = conf.value(QStringLiteral("user")).toString();
    const QString imappass = conf.value(QStringLiteral("password")).toString();
    const QString imaphost = conf.value(QStringLiteral("host"), QStringLiteral("localhost")).toString();
    const quint16 imapport = conf.value(QStringLiteral("port"), 143).value<quint16>();
    const quint8 imapprotocol = conf.value(QStringLiteral("protocol"), 2).value<quint8>();
    const quint8 imapencryption = conf.value(QStringLiteral("encryption"), 1).value<quint8>();
    const QString imappeername = conf.value(QStringLiteral("peername")).toString();
    const quint8 imapauthmech = conf.value(QStringLiteral("authmech")).value<quint8>();
    conf.endGroup();

    Imap imap(imapuser, imappass, static_cast<Imap::AuthMech>(imapauthmech), imaphost, imapport, static_cast<QAbstractSocket::NetworkLayerProtocol>(imapprotocol), static_cast<Imap::EncryptionType>(imapencryption), QLatin1Char('.'), imappeername);
    if (!imap.login()) {
        return imapError(imap.lastError());
    }

    imap.logout();

    return 0;
}
