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

QStringList Imap::m_capabilities = QStringList();

Imap::Imap(QObject *parent) : QSslSocket(parent)
{

}


Imap::Imap(const QString &user, const QString &password, const QString &host, quint16 port, NetworkLayerProtocol protocol, EncryptionType conType, bool unixhierarchysep, QObject *parent) :
    QSslSocket(parent), m_user(user), m_password(password), m_host(host), m_port(port), m_protocol(protocol), m_encType(conType), m_unixhierarchysep(unixhierarchysep)
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

    if (m_encType != IMAPS) {
        this->connectToHost(m_host, m_port, ReadWrite, m_protocol);
    } else {
        this->connectToHostEncrypted(m_host, m_port, ReadWrite, m_protocol);
    }

    if (m_encType != IMAPS) {
        if (Q_UNLIKELY(!this->waitForConnected())) {
            m_lastError = tr("Connection to IMAP server timed out while waiting for connection.");
            this->abort();
            return false;
        }
    } else {
        if (Q_UNLIKELY(!this->waitForEncrypted())) {
            m_lastError = tr("Connection to IMAP server timed out while waiting for SSL handshake to complete.");
            this->abort();
            return false;
        }
    }

    if (Q_UNLIKELY(!this->waitForReadyRead())) {
        m_lastError = tr("Connection to IMAP server timed out while waiting for first response on login.");
        this->abort();
        return false;
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
                m_lastError = tr("Connection to IMAP server timed out while wating for response to STARTTLS.");
                this->abort();
                return false;
            }

            if (Q_UNLIKELY(!checkResponse(this->readAll()))) {
                this->disconnectFromHost();
                this->waitForDisconnected();
                return false;
            }

            this->startClientEncryption();

        } else {
            m_lastError = tr("STARTTLS is not supported. Aborting.");
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
        m_lastError = tr("Connection to IMAP server timed out while waiting for response to login data.");
        this->abort();
        return false;
    }

    if (Q_UNLIKELY(!this->checkResponse(this->readAll()))) {
        this->disconnectFromHost();
        this->waitForDisconnected();
        return false;
    }

    if (Imap::m_capabilities.isEmpty()) {
        const QRegularExpression capAfterLoginRegEx(QStringLiteral(".*\\[CAPABILITY (.*)\\]"));
        const QRegularExpressionMatch match = capAfterLoginRegEx.match(m_response);
        Imap::m_capabilities = match.captured(1).split(QChar(QChar::Space), QString::SkipEmptyParts);
        if (Imap::m_capabilities.isEmpty()) {
            Imap::m_capabilities = getCapabilities();
            if (Imap::m_capabilities.isEmpty()) {
                m_lastError = tr("Failed to get capabilities from the IMAP server.");
                this->disconnectFromHost();
                this->waitForDisconnected();
                return false;
            }
        }
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

    this->write(QStringLiteral(". LOGOUT\n").toLatin1());

    this->waitForReadyRead();

    if (Q_UNLIKELY(!this->checkResponse(this->readAll()))) {
        this->disconnectFromHost();
        m_lastError = tr("Failed to successfully log out from IMAP server.");
        if (Q_UNLIKELY(!this->waitForDisconnected())) {
            m_lastError = tr("Connection to IMAP server timed out while waiting for disconnection.");
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
        m_lastError = tr("Connection to IMAP server timed out while waiting for disconnection.");
        this->abort();
        return false;
    }
}


QStringList Imap::getCapabilities(bool forceReload)
{
    if (Imap::m_capabilities.empty() || forceReload) {

        Imap::m_capabilities.clear();

        this->write(QStringLiteral(". CAPABILITY\n").toLatin1());

        if (Q_UNLIKELY(!this->waitForReadyRead())) {
            m_lastError = tr("Connection to IMAP server timed out.");
            return Imap::m_capabilities;
        }

        if (Q_UNLIKELY(!this->checkResponse(this->readAll()))) {
            return Imap::m_capabilities;
        }

        const QRegularExpression regex(QStringLiteral("^\\* CAPABILITY (.*)\\r\\n"));
        const QRegularExpressionMatch match = regex.match(m_response);

        Imap::m_capabilities = match.captured(1).split(QChar(QChar::Space), QString::SkipEmptyParts);

        if (Q_UNLIKELY(Imap::m_capabilities.empty())) {
            m_lastError = tr("Failed to request capabilities from the IMAP server. Aborting.");
        }

    }

    return Imap::m_capabilities;
}



std::pair<quint32,quint32> Imap::getQuota(const QString &user)
{
    std::pair<quint32,quint32> quota(0, 0);

    QString command(QStringLiteral(". GETQUOTA user"));
    if (m_unixhierarchysep) {
        command.append(QLatin1Char('/'));
    } else {
        command.append(QLatin1Char('.'));
    }
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



bool Imap::checkResponse(const QByteArray &resp, const QString &tag)
{
    bool ret = false;

    m_response = QString::fromLatin1(resp);

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

void Imap::setUnixHierarchySep(bool unixhierarchysep)
{
    m_unixhierarchysep = unixhierarchysep;
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
