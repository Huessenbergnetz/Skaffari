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

#include "imap.h"
#include <QRegularExpression>
#include <QRegularExpressionMatch>

Imap::Imap(QObject *parent) : QSslSocket(parent)
{

}


Imap::Imap(const QString &user, const QString &password, const QString &host, quint16 port, NetworkLayerProtocol protocol, EncryptionType conType, QObject *parent) :
    QSslSocket(parent), m_user(user), m_password(password), m_host(host), m_port(port), m_protocol(protocol), m_encType(conType)
{

}


Imap::~Imap()
{

}


bool Imap::login()
{
    if (m_loggedIn) {
        return true;
    }

    if (this->state() == QAbstractSocket::UnconnectedState) {
        if (m_encType != IMAPS) {
            this->connectToHost(m_host, m_port, ReadWrite, m_protocol);
        } else {
            this->connectToHostEncrypted(m_host, m_port, ReadWrite, m_protocol);
        }
    }

    if (m_encType != IMAPS) {
        if (!this->waitForConnected(15000)) {
            m_lastError = tr("Connection to IMAP server timed out while waiting for connection.");
            return false;
        }
    } else {
        if (!this->waitForEncrypted(15000)) {
            m_lastError = tr("Connection to IMAP server timed out while waiting for SSL handshake to complete.");
            return false;
        }
    }

    if (!this->waitForReadyRead(15000)) {
        m_lastError = tr("Connection to IMAP server timed out while waiting for first response on login.");
        return false;
    }

    if (!checkResponse(this->readAll())) {
        return false;
    }

    if (m_encType == StartTLS) {

        if (m_response.split(QStringLiteral(" ")).contains(QLatin1String("STARTTLS", 8))) {

            static const QString startTLSCommand(QStringLiteral(". STARTTLS\n"));
            this->write(startTLSCommand.toLatin1());

            if (!this->waitForReadyRead(5000)) {
                m_lastError = tr("Connection to IMAP server timed out while wating for response to STARTTLS.");
                return false;
            }

            if (!checkResponse(this->readAll())) {
                return false;
            }

            this->startClientEncryption();

        } else {
            m_lastError = tr("STARTTLS is not supported. Aborting.");
            return false;
        }
    }

    // build login command
    QString loginData(QStringLiteral(". LOGIN "));
    loginData.append(m_user).append(QLatin1String(" ", 1)).append(m_password).append(QStringLiteral("\n"));

    this->write(loginData.toLatin1());

    if (!this->waitForReadyRead(5000)) {
        m_lastError = tr("Connection to IMAP server timed out while waiting for response to login data.");
        return false;
    }

    if (!this->checkResponse(this->readAll())) {
        return false;
    }

    static const QRegularExpression capAfterLoginRegEx(QStringLiteral(".*\\[CAPABILITY (.*)\\]"));
    const QRegularExpressionMatch match = capAfterLoginRegEx.match(m_response);
    const QStringList capabilities = match.captured(1).split(QChar(' '), QString::SkipEmptyParts);
    if (Q_LIKELY(!capabilities.empty())) {
        m_capabilities = capabilities;
    } else {
        m_capabilities = getCapabilities();
    }

    m_loggedIn = true;

    return true;
}


bool Imap::logout()
{
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

    static const QString logoutCommand(QStringLiteral(". LOGOUT\n"));
    this->write(logoutCommand.toLatin1());

    this->waitForReadyRead(5000);

    if (Q_UNLIKELY(!this->checkResponse(this->readAll()))) {
        this->disconnectFromHost();
        m_lastError = tr("Failed to successfully log out from IMAP server.");
        if (Q_UNLIKELY(!this->waitForDisconnected(15000))) {
            m_lastError = tr("Connection to IMAP server timed out while waiting for disconnection.");
        }
        return false;
    }

    m_loggedIn = false;

    this->disconnectFromHost();

    if (Q_LIKELY(this->waitForDisconnected(15000))) {
        return true;
    } else {
        m_lastError = tr("Connection to IMAP server timed out while waiting for disconnection.");
        return false;
    }
}


QStringList Imap::getCapabilities()
{
    QStringList resp;

    static const QString capabilitiesCommand(QStringLiteral(". CAPABILITY\n"));

    this->write(capabilitiesCommand.toLatin1());

    if (!this->waitForReadyRead(5000)) {
        m_lastError = tr("Connection to IMAP server timed out.");
        return resp;
    }

    if (!this->checkResponse(this->readAll())) {
        return resp;
    }

    static const QRegularExpression regex(QStringLiteral("^\\* CAPABILITY (.*)\\r\\n"));
    const QRegularExpressionMatch match = regex.match(m_response);

    resp = match.captured(1).split(QChar(' '), QString::SkipEmptyParts);

    if (Q_UNLIKELY(resp.empty())) {
        m_lastError = tr("Failed to request capabilities from the IMAP server. Aborting.");
    }

    return resp;
}



bool Imap::checkResponse(const QByteArray &resp, const QString &tag)
{
    bool ret = false;

    m_response = QString(resp);

    if (m_response.isEmpty()) {
        m_lastError = tr("The IMAP response is undefined.");
        return ret;
    }

    QRegularExpression regex;

    if (tag.isEmpty()) {
        regex.setPattern(QStringLiteral("\\s*.* (OK|BAD|WARN|NO) (.*)"));
    } else {
        regex.setPattern(QStringLiteral("\\s*%1 (OK|BAD|WARN|NO) (.*)").arg(tag));
    }

    const QRegularExpressionMatch match = regex.match(m_response);

    if (!match.hasMatch()) {
        m_lastError = tr("The IMAP response is undefined.");
        return ret;
    }

    const QString respType = match.captured(1);

    if (respType == QLatin1String("OK")) {
        ret = true;
    } else if (respType == QLatin1String("NO")) {
        m_lastError = tr("We received a NO response from the IMAP server: %1").arg(match.captured(2));
    } else if (respType == QLatin1String("BAD")) {
        m_lastError = tr("We received a BAD response from the IMAP server: %1").arg(match.captured(2));
    } else {
        m_lastError = tr("The IMAP response is undefined.");
    }

    return ret;
}



void Imap::setUser ( const QString& user )
{
    m_user = user;
}



void Imap::setPassword ( const QString& password )
{
    m_password = password;
}


void Imap::setHost ( const QString& host )
{
    m_host = host;
}


void Imap::setPort ( const quint16& port )
{
    m_port = port;
}


void Imap::setProtocol ( QAbstractSocket::NetworkLayerProtocol protocol )
{
    m_protocol = protocol;
}



void Imap::setEncryptionType(Imap::EncryptionType encType)
{
    m_encType = encType;
}


QString Imap::encryptionTypeToString(EncryptionType et)
{
    QString str;

    switch(et) {
    case Unsecured:
        str = tr("Unsecured");
        break;
    case StartTLS:
        str = tr("StartTLS");
        break;
    case IMAPS:
        str = tr("IMAPS");
        break;
    default:
        break;
    }

    return str;
}


QString Imap::encryptionTypeToString(quint8 et)
{
    return encryptionTypeToString(static_cast<EncryptionType>(et));
}


QString Imap::networkProtocolToString(QAbstractSocket::NetworkLayerProtocol nlp)
{
    QString str;

    switch(nlp) {
    case QAbstractSocket::IPv4Protocol:
        str = QStringLiteral("IPv4");
        break;
    case QAbstractSocket::IPv6Protocol:
        str = QStringLiteral("IPv6");
        break;
    case QAbstractSocket::AnyIPProtocol:
        str = tr("Either IPv4 or IPv6");
        break;
    default:
        str = tr("Other");
        break;
    }

    return str;
}



QString Imap::networkProtocolToString(quint8 nlp)
{
    return networkProtocolToString(static_cast<QAbstractSocket::NetworkLayerProtocol>(nlp));
}


QString Imap::lastError() const
{
    return m_lastError;
}
