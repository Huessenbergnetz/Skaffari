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

#include "skaffariengine.h"
#include "imap/skaffariimap.h"
#include <Cutelyst/Plugins/Utils/Sql>
#include <QVersionNumber>
#include <QSqlQuery>
#include <QSqlError>
#include <QAbstractSocket>

Q_LOGGING_CATEGORY(SK_ENGINE, "weihwazo.engine")

SkaffariEngine::SkaffariEngine(QObject* parent) : QObject(parent)
{
}


SkaffariEngine::~SkaffariEngine()
{

}



bool SkaffariEngine::init(const QVariantMap &adminsConfig, const QVariantMap &defaults, const QVariantMap &accountsConfig, const QVariantMap &imapConfig)
{
    m_adminsConfig = adminsConfig;
    m_defaultValues = defaults;
    m_accountsConfig = accountsConfig;
    m_imapConfig = imapConfig;

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT val FROM systeminfo WHERE name = 'skaffari_db_version'"));
    if (!q.exec()) {
        qCCritical(SK_ENGINE) << "Failed to query installed database layout version from database!" << q.lastError().text();
        return false;
    }

    if (!q.next()) {
        qCCritical(SK_ENGINE) << "Failed to query installed database layout version from database!";
    }

    m_updateRequired = (QVersionNumber::fromString(q.value(0).toString()) < QVersionNumber(0, 0, 1));

    return initImap();
}


QVariant SkaffariEngine::adminConfig(const QString &key, const QVariant &defaultValue) const
{
    QVariant val = defaultValue;

    QMap<QString, QVariant>::const_iterator it = m_adminsConfig.constFind(key);
    if (it != m_adminsConfig.constEnd()) {
        val = it.value();
    }

    return val;
}


QVariant SkaffariEngine::imapConfig(const QString &key, const QVariant &defaultValue) const
{
    QVariant val = defaultValue;

    QMap<QString, QVariant>::const_iterator it = m_imapConfig.constFind(key);
    if (it != m_imapConfig.constEnd()) {
        val = it.value();
    }

    return val;
}


QVariantMap SkaffariEngine::imapConfig() const
{
    return m_imapConfig;
}



QVariant SkaffariEngine::defaultValue(const QString &key, const QVariant &defaultValue) const
{
    QVariant val = defaultValue;

    QMap<QString, QVariant>::const_iterator it = m_defaultValues.constFind(key);
    if (it != m_defaultValues.constEnd()) {
        val = it.value();
    }

    return val;
}


QVariant SkaffariEngine::accountConfig(const QString &key, const QVariant &defaultValue) const
{
    QVariant val = defaultValue;

    QMap<QString, QVariant>::const_iterator it = m_accountsConfig.constFind(key);
    if (it != m_accountsConfig.constEnd()) {
        val = it.value();
    }

    return val;
}



bool SkaffariEngine::initImap()
{
    const QString user = imapConfig(QStringLiteral("user")).toString();
    if (user.isEmpty()) {
        qCCritical(SK_ENGINE) << "No valid IMAP user defined.";
        return false;
    }

    const QString password = imapConfig(QStringLiteral("password")).toString();
    if (password.isEmpty()) {
        qCCritical(SK_ENGINE) << "No valid IMAP password defined.";
        return false;
    }

    const QString host = imapConfig(QStringLiteral("host"), QStringLiteral("localhost")).toString();
    if (host.isEmpty()) {
        qCCritical(SK_ENGINE) << "No valid IMAP host defined.";
        return false;
    }

    const quint16 port = imapConfig(QStringLiteral("port"), 143).value<quint16>();
    if (!port) {
        qCCritical(SK_ENGINE) << "No valid IMAP port defined.";
        return false;
    }

//    const QAbstractSocket::NetworkLayerProtocol protocol = static_cast<QAbstractSocket::NetworkLayerProtocol>(imapConfig(QStringLiteral("protocol"), 2).toInt());
//    const SkaffariIMAP::EncryptionType encryption = static_cast<SkaffariIMAP::EncryptionType>(imapConfig(QStringLiteral("encryption"), 1).value<quint8>());

    return true;
}




bool SkaffariEngine::testImapConnection(const QString& user, const QString& password, const QString& host, quint16 port, quint8 protocol, quint8 encType)
{
	QAbstractSocket::NetworkLayerProtocol prot = static_cast<QAbstractSocket::NetworkLayerProtocol>(protocol);
	SkaffariIMAP::EncryptionType et = static_cast<SkaffariIMAP::EncryptionType>(encType);

	SkaffariIMAP imap(user, password, host, port, prot, et);

	if (!imap.login()) {
		return false;
	}

	if (!imap.capabilities()) {
		return false;
	}

	imap.logout();

	return true;
}


