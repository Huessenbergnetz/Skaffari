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

#ifndef SKAFFARIIMAP_H
#define SKAFFARIIMAP_H

#include <QSslSocket>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include "skaffariimaperror.h"

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(SK_IMAP)

class SkaffariIMAP : public QSslSocket
{
	Q_OBJECT
public:
	/*!
	 * \brief Availbale methods of connection encryptions.
	 */
    enum EncryptionType : quint8 {
		Unsecured	= 0,	/**< 0: no encryption, mostly on port 143 */
		StartTLS	= 1,	/**< 1: use <A HREF="https://en.wikipedia.org/wiki/STARTTLS">StartTLS</A>, mostly on port 143 */
		IMAPS		= 2, 	/**< 2: use <A HREF="https://en.wikipedia.org/wiki/IMAPS">IMAPS</A>, mostly on port 993 */
	};
    Q_ENUM(EncryptionType)
	
    enum ResponseType : quint8 {
		OK			= 0,
		NO			= 1,
		BAD			= 2,
		Undefined	= 3
	};

    explicit SkaffariIMAP(QObject *parent = nullptr);
    ~SkaffariIMAP();

	bool login();
	bool logout();
    bool isLoggedIn() const;

    QStringList getCapabilities(bool forceReload = false);
    std::pair<quint32,quint32> getQuota(const QString &user);

	QString errorText() const;
	SkaffariIMAPError lastError() const;

	void setUser(const QString &user);
	void setPassword(const QString &password);
	void setHost(const QString &host);
	void setPort(const quint16 &port);
	void setProtocol(NetworkLayerProtocol protocol);
	void setEncryptionType(EncryptionType encType);

    static QByteArray toUTF7Imap(const QString &str);
    static QString fromUTF7Imap(const QByteArray &ba);

private:
	bool connectionTimeOut();
    bool checkResponse(const QByteArray &resp, const QString &tag = QString());
	QString m_user;
	QString m_password;
	QString m_host;
    quint16 m_port = 143;
    NetworkLayerProtocol m_protocol = QAbstractSocket::AnyIPProtocol;
    EncryptionType m_encType = StartTLS;
    QChar m_hierarchysep = QLatin1Char('.');
	QString m_response;
	SkaffariIMAPError m_imapError;
    bool m_loggedIn = false;
    static QStringList m_capabilities;
	void setNoError();

    Q_DISABLE_COPY(SkaffariIMAP)
};

#endif // SKAFFARIIMAP_H
