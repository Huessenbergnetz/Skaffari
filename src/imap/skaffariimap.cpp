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

#include "skaffariimap.h"
#include <unicode/ucnv_err.h>
#include <unicode/uenum.h>
#include <unicode/localpointer.h>
#include <unicode/ucnv.h>
#include "../utils/skaffariconfig.h"
#include <QVariantMap>

Q_LOGGING_CATEGORY(SK_IMAP, "skaffari.imap")

QStringList SkaffariIMAP::m_capabilities = QStringList();

SkaffariIMAP::SkaffariIMAP(QObject *parent) :
    QSslSocket(parent),
    m_user(SkaffariConfig::imapUser()),
    m_password(SkaffariConfig::imapPassword()),
    m_host(SkaffariConfig::imapHost()),
    m_port(SkaffariConfig::imapPort()),
    m_protocol(SkaffariConfig::imapProtocol()),
    m_encType(SkaffariConfig::imapEncryption())
{
    if (SkaffariConfig::imapDomainasprefix() || SkaffariConfig::imapFqun()) {
        m_hierarchysep = QLatin1Char('/');
    }

    setPeerVerifyName(SkaffariConfig::imapPeername());
}


SkaffariIMAP::~SkaffariIMAP()
{

}


bool SkaffariIMAP::login()
{
    setNoError();

    if (m_loggedIn) {
        return true;
    }

    if (m_encType != IMAPS) {
        this->connectToHost(m_host, m_port, ReadWrite, m_protocol);
    } else {
        this->connectToHostEncrypted(m_host, m_port, ReadWrite, m_protocol);
    }

    if (m_encType != IMAPS) {
        if (Q_UNLIKELY(!this->waitForConnected())) {
            return connectionTimeOut();
        }
    } else {
        if (Q_UNLIKELY(!this->waitForEncrypted())) {
            return connectionTimeOut();
        }
    }

    if (Q_UNLIKELY(!this->waitForReadyRead())) {
        return connectionTimeOut();
    }

    if (Q_UNLIKELY(!checkResponse(this->readAll()))) {
        this->disconnectFromHost();
        this->waitForDisconnected();
        return false;
    }

    if (m_encType == StartTLS) {

        if (m_response.split(QChar(QChar::Space)).contains(QLatin1String("STARTTLS", 8))) {

            this->write(QStringLiteral(". STARTTLS\n").toLatin1());

            if (Q_UNLIKELY(!this->waitForReadyRead())) {
                return connectionTimeOut();
            }

            if (Q_UNLIKELY(!checkResponse(this->readAll()))) {
                this->disconnectFromHost();
                this->waitForDisconnected();
                return false;
            }

            this->startClientEncryption();

        } else {
            m_imapError = SkaffariIMAPError(SkaffariIMAPError::EncryptionError, tr("STARTTLS is not supported."));
            this->disconnectFromHost();
            this->waitForDisconnected();
            return false;
        }
    }

    // build login command
    QString loginData(QStringLiteral(". LOGIN "));
    loginData.append(m_user).append(QChar(QChar::Space)).append(m_password).append(QChar(QChar::LineFeed));

    this->write(loginData.toLatin1());

    if (Q_UNLIKELY(!this->waitForReadyRead())) {
        return connectionTimeOut();
    }

    if (Q_UNLIKELY(!this->checkResponse(this->readAll()))) {
        this->disconnectFromHost();
        this->waitForDisconnected();
        return false;
    }

    if (SkaffariIMAP::m_capabilities.isEmpty()) {
        const QRegularExpression capAfterLoginRegEx(QStringLiteral(".*\\[CAPABILITY (.*)\\]"));
        const QRegularExpressionMatch match = capAfterLoginRegEx.match(m_response);
        SkaffariIMAP::m_capabilities = match.captured(1).split(QChar(QChar::Space), QString::SkipEmptyParts);
        if (SkaffariIMAP::m_capabilities.isEmpty()) {
            SkaffariIMAP::m_capabilities = getCapabilities();
            if (SkaffariIMAP::m_capabilities.isEmpty()) {
                this->disconnectFromHost();
                this->waitForDisconnected();
                return false;
            }
        }
    }

    m_loggedIn = true;

    return true;
}


bool SkaffariIMAP::logout()
{
    setNoError();

    if (!m_loggedIn) {
        return true;
    }

    if (this->state() == UnconnectedState) {
        m_loggedIn = false;
        return true;
    }

    if (this->state() == ClosingState) {
        m_loggedIn = false;
        return true;
    }

    this->write(QStringLiteral(". LOGOUT\n").toLatin1());

    this->waitForReadyRead();

    if (Q_UNLIKELY(!this->checkResponse(this->readAll()))) {
        this->disconnectFromHost();
        if (Q_UNLIKELY(!this->waitForDisconnected())) {
            m_loggedIn = false;
            this->abort();
        }
        return false;
    }

    m_loggedIn = false;

    this->disconnectFromHost();

    if (Q_LIKELY(this->waitForDisconnected())) {
        return true;
    } else {
        return connectionTimeOut();
    }
}


QStringList SkaffariIMAP::getCapabilities(bool forceReload)
{
    if (SkaffariIMAP::m_capabilities.empty() || forceReload) {

        SkaffariIMAP::m_capabilities.clear();

        this->write(QStringLiteral(". CAPABILITY\n").toLatin1());

        if (Q_UNLIKELY(!this->waitForReadyRead())) {
            connectionTimeOut();
            return SkaffariIMAP::m_capabilities;
        }

        if (Q_UNLIKELY(!this->checkResponse(this->readAll()))) {
            return SkaffariIMAP::m_capabilities;
        }

        const QRegularExpression regex(QStringLiteral("^\\* CAPABILITY (.*)\\r\\n"));
        const QRegularExpressionMatch match = regex.match(m_response);

        SkaffariIMAP::m_capabilities = match.captured(1).split(QChar(QChar::Space), QString::SkipEmptyParts);

        if (Q_UNLIKELY(SkaffariIMAP::m_capabilities.empty())) {
            m_imapError = SkaffariIMAPError(SkaffariIMAPError::Unknown, tr("Failed to request capabilities from the IMAP server."));
        }

    }

    return SkaffariIMAP::m_capabilities;
}


std::pair<quint32,quint32> SkaffariIMAP::getQuota(const QString &user)
{
    std::pair<quint32,quint32> quota(0, 0);

    QString command(QStringLiteral(". GETQUOTA user"));
    command.append(m_hierarchysep);
    command.append(user);
    command.append(QChar(QChar::LineFeed));

    this->write(command.toLatin1());

    if (Q_LIKELY(this->waitForReadyRead())) {
        const QString response = QString::fromLatin1(this->readAll());
        static QRegularExpression re(QStringLiteral("^\\* QUOTA \\S* \\(STORAGE (\\d+) (\\d+)"));
        const QRegularExpressionMatch match = re.match(response);
        if (match.hasMatch()) {
            quota.first = static_cast<quint32>(match.captured(1).toULong());
            quota.second = static_cast<quint32>(match.captured(2).toULong());
        }
    }

    return quota;
}


bool SkaffariIMAP::connectionTimeOut()
{
    qCWarning(SK_IMAP) << "Connection to IMAP server timed out.";
	m_imapError = SkaffariIMAPError(SkaffariIMAPError::ConnectionTimeout, tr("Connection to IMAP server timed out."));
    this->abort();
	return false;
}


bool SkaffariIMAP::checkResponse(const QByteArray &resp, const QString &tag)
{
    m_response = QString::fromLatin1(resp);

	if (m_response.isEmpty()) {
		m_imapError = SkaffariIMAPError(SkaffariIMAPError::UndefinedResponse, tr("The IMAP response is undefined."));
		return false;
	}

    QRegularExpression regex;

    if (tag.isEmpty()) {
        regex.setPattern(QStringLiteral("\\s*.* (OK|BAD|WARN|NO) (.*)"));
    } else {
        regex.setPattern(QStringLiteral("\\s*%1 (OK|BAD|WARN|NO) (.*)").arg(tag));
    }

    const QRegularExpressionMatch match = regex.match(m_response);

    if (!match.hasMatch()) {
        m_imapError = SkaffariIMAPError(SkaffariIMAPError::UndefinedResponse, tr("The IMAP response is undefined."));
        return false;
    }

    const QString respType = match.captured(1);

    if (respType == QLatin1String("OK")) {
		setNoError();
		return true;
    } else if (respType == QLatin1String("NO")) {
		m_imapError = SkaffariIMAPError(SkaffariIMAPError::NoResponse, tr("We received a NO response from the IMAP server."));
		return false;
    } else if (respType == QLatin1String("BAD")) {
        m_imapError = SkaffariIMAPError(SkaffariIMAPError::BadResponse, tr("We received a BAD response from the IMAP server, probably because of a malformed request."));
		return false;
	} else {
		m_imapError = SkaffariIMAPError(SkaffariIMAPError::UndefinedResponse, tr("The IMAP response is undefined."));
		return false;
	}
}




void SkaffariIMAP::setUser ( const QString& user )
{
	m_user = user;
}



void SkaffariIMAP::setPassword ( const QString& password )
{
	m_password = password;
}


void SkaffariIMAP::setHost ( const QString& host )
{
	m_host = host;
}


void SkaffariIMAP::setPort ( const quint16& port )
{
	m_port = port;
}


void SkaffariIMAP::setProtocol ( QAbstractSocket::NetworkLayerProtocol protocol )
{
	m_protocol = protocol;
}



void SkaffariIMAP::setEncryptionType(SkaffariIMAP::EncryptionType encType)
{
	m_encType = encType;
}


void SkaffariIMAP::setNoError()
{
	if (m_imapError.type() != SkaffariIMAPError::NoError) {
		m_imapError = SkaffariIMAPError();
	}
}


SkaffariIMAPError SkaffariIMAP::lastError() const
{
	return m_imapError;
}



QByteArray SkaffariIMAP::toUTF7Imap(const QString &str)
{
    QByteArray ba;

    if (str.isEmpty()) {
        return ba;
    }

    UErrorCode uec = U_ZERO_ERROR;

    const QByteArray utf8 = str.toUtf8();

    char *out;
    out = (char *) malloc(sizeof(char) * utf8.size() * 3);

    int32_t size = ucnv_convert("imap-mailbox-name", "utf-8", out, utf8.size() * 3, utf8.constData(), utf8.size(), &uec);

    if ((size > 0) && (uec == U_ZERO_ERROR)) {
        ba = QByteArray(out, size);
    } else {
        qCDebug(SK_IMAP) << "Failed to convert UTF-8 string" << str << "to UTF7-IMAP (RFC2060 5.1.3) with error" << u_errorName(uec);
    }

    free(out);

    return ba;
}



QString SkaffariIMAP::fromUTF7Imap(const QByteArray &ba)
{
    QString str;

    if (ba.isEmpty()) {
        return str;
    }

    UErrorCode uec = U_ZERO_ERROR;
    char *out;
    out = (char *) malloc(sizeof(char) * (ba.size() + 1));

    int32_t size = ucnv_convert("utf-8", "imap-mailbox-name", out, ba.size() + 1, ba.constData(), ba.size(), &uec);

    if ((size > 0) && (uec == U_ZERO_ERROR)) {
        str = QString::fromUtf8(out, size);
    } else {
        qCDebug(SK_IMAP) << "Failed to convert UTF7-IMAP (RFC2060 5.1.3) string" << str << "to UTF-8 with error" << u_errorName(uec);
    }

    free(out);

    return str;
}



bool SkaffariIMAP::isLoggedIn() const
{
    return m_loggedIn;
}
