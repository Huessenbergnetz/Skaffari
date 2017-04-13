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

Q_LOGGING_CATEGORY(SK_IMAP, "skaffari.imap")

SkaffariIMAP::SkaffariIMAP(QObject *parent): QSslSocket(parent)
{

}


SkaffariIMAP::SkaffariIMAP (const QString& user, const QString& password, const QString& host, quint16 port, QAbstractSocket::NetworkLayerProtocol protocol, EncryptionType conType, QObject* parent ) : QSslSocket(parent), m_user(user), m_password(password), m_host(host), m_port(port), m_protocol(protocol), m_encType(conType)
{

}


SkaffariIMAP::~SkaffariIMAP()
{

}



bool SkaffariIMAP::login()
{
	if (this->state() == QAbstractSocket::UnconnectedState) {
		if (m_encType != IMAPS) {
			this->connectToHost(m_host, m_port, ReadWrite, m_protocol);
		} else {
			this->connectToHostEncrypted(m_host, m_port, ReadWrite, m_protocol);
		}
	}

	if (m_encType != IMAPS) {
		if (!this->waitForConnected(15000)) {
			return connectionTimeOut();
		}
	} else {
		if (!this->waitForEncrypted(15000)) {
			return connectionTimeOut();
		}
	}

	if (!this->waitForReadyRead(5000)) {
		return connectionTimeOut();
	}

	if (!checkResponse(this->readAll())) {
		return false;
	}

	if (m_encType == StartTLS) {

        if (m_response.split(QStringLiteral(" ")).contains(QLatin1String("STARTTLS", 8))) {

            static const QString startTLSCommand(QStringLiteral(". STARTTLS\n"));
			this->write(startTLSCommand.toLatin1());

			if (!this->waitForReadyRead(5000)) {
				return connectionTimeOut();
			}

			if (!checkResponse(this->readAll())) {
				return false;
			}

			this->startClientEncryption();

		} else {
            qCWarning(SK_IMAP)<< "STARTTLS not supported, continuing unencrypted.";
		}
	}

	// build login command
    QString loginData(QStringLiteral(". LOGIN "));
    loginData.append(m_user).append(QLatin1String(" ", 1)).append(m_password).append(QStringLiteral("\n"));

	this->write(loginData.toLatin1());

	if (!this->waitForReadyRead(5000)) {
		return connectionTimeOut();
	}

	if (!this->checkResponse(this->readAll())) {
		return false;
	}

	return true;
}


bool SkaffariIMAP::logout()
{
	if (this->state() == UnconnectedState) {
		return true;
	}

	if (this->state() == ClosingState) {
		return true;
	}

    static const QString logoutCommand(QStringLiteral(". LOGGOUT\n"));
    this->write(logoutCommand.toLatin1());

	if (this->waitForReadyRead(5000)) {
		this->disconnect();
		return true;
	} else {
        qCWarning(SK_IMAP) << "No response on logout.";
		this->disconnect();
		return false;
	}
}


bool SkaffariIMAP::capabilities()
{
    static const QString capabilitiesCommand(QStringLiteral(". CAPABILITY\n"));

	this->write(capabilitiesCommand.toLatin1());

	if (!this->waitForReadyRead(5000)) {
		return connectionTimeOut();
	}

	if (!this->checkResponse(this->readAll())) {
		return false;
	}

	m_capabilities.clear();

	m_response.remove(QRegularExpression(QStringLiteral("\\s*.* (OK|BAD|WARN) .*")));

	QStringList resp = m_response.trimmed().split(" ");

	if (!resp.isEmpty()) {
		resp.removeFirst();
		resp.removeFirst();
		m_capabilities = resp;
	} else {
		m_imapError = SkaffariIMAPError(SkaffariIMAPError::EmptyResponse, tr("The IMAP response was empty or could not get extracted."));
		return false;
	}

	setNoError();
	return true;
}


QStringList SkaffariIMAP::getCapabilities()
{
	return m_capabilities;
}



QPair<quint32, quint32> SkaffariIMAP::getQuota(const QString& user)
{
    QPair<qint32,qint32> qp(0, 0);

    QString getQuotaCommand(QStringLiteral(". GETQUOTA user."));
    getQuotaCommand.append(user);
	getQuotaCommand.append("\n");

	this->write(getQuotaCommand.toLatin1());

	if (this->waitForReadyRead(5000)) {
		QString response(this->readAll());
		QRegularExpression re(QStringLiteral("^\\* QUOTA \\S* \\(STORAGE (\\d+) (\\d+)"));
		QRegularExpressionMatch match = re.match(response);
		if (match.hasMatch()) {
// 			qDebug() << "User:" << user << "Quota:" << match.captured(1).toInt() << "/" << match.captured(2).toInt();
            qp.first = static_cast<quint32>(match.captured(1).toULong());
            qp.second = static_cast<quint32>(match.captured(2).toULong());
		} else {
            qCWarning(SK_IMAP) << "No quota found for user" << user;
			m_imapError = SkaffariIMAPError(SkaffariIMAPError::UndefinedResponse, tr("No quota found for user %1.").arg(user));
		}
	} else {
        qCWarning(SK_IMAP) << "Connection to IMAP server timed out.";
        m_imapError = SkaffariIMAPError(SkaffariIMAPError::ConnectionTimeout, tr("Could not request quota for user %1, connection to IMAP server timed out.").arg(user));
	}

    return qp;
}




QString SkaffariIMAP::errorText() const
{
	return m_errorText;
}


bool SkaffariIMAP::connectionTimeOut()
{
    qCWarning(SK_IMAP) << "Connection to IMAP server timed out.";
	m_imapError = SkaffariIMAPError(SkaffariIMAPError::ConnectionTimeout, tr("Connection to IMAP server timed out."));
	this->disconnect();
	return false;
}


bool SkaffariIMAP::checkResponse(const QByteArray &resp, const QString &tag)
{
	m_response = QString(resp);

	if (m_response.isEmpty()) {
		m_imapError = SkaffariIMAPError(SkaffariIMAPError::UndefinedResponse, tr("The IMAP response is undefined."));
		return false;
	}

	if (tag.isEmpty()) {
		m_responseRegex.setPattern(QStringLiteral("\\s*.* (OK|BAD|WARN) "));
	} else {
        m_responseRegex.setPattern(QStringLiteral("\\s*%1 (OK|BAD|WARN) ").arg(tag));
	}

	m_responseRegexMatch = m_responseRegex.match(m_response);

	if (!m_responseRegexMatch.hasMatch()) {
		m_imapError = SkaffariIMAPError(SkaffariIMAPError::UndefinedResponse, tr("The IMAP response is undefined."));
		return false;
	}

	QString respType = m_responseRegexMatch.captured(1);

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
