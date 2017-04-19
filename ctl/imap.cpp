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
#include <cstdio>

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
    if (this->state() == QAbstractSocket::UnconnectedState) {
        if (m_encType != IMAPS) {
            this->connectToHost(m_host, m_port, ReadWrite, m_protocol);
        } else {
            this->connectToHostEncrypted(m_host, m_port, ReadWrite, m_protocol);
        }
    }

    if (m_encType != IMAPS) {
        if (!this->waitForConnected(15000)) {
            printf("%s\n", qUtf8Printable(tr("Connection to IMAP server timed out.")));
            return false;
        }
    } else {
        if (!this->waitForEncrypted(15000)) {
            printf("%s\n", qUtf8Printable(tr("Connection to IMAP server timed out.")));
            return false;
        }
    }

    if (!this->waitForReadyRead(5000)) {
        printf("%s\n", qUtf8Printable(tr("Connection to IMAP server timed out.")));
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
                printf("%s\n", qUtf8Printable(tr("Connection to IMAP server timed out.")));
                return false;
            }

            if (!checkResponse(this->readAll())) {
                return false;
            }

            this->startClientEncryption();

        } else {
            printf("%s\n", qUtf8Printable(tr("STARTLS is not supported. Aborting.")));
            return false;
        }
    }

    // build login command
    QString loginData(QStringLiteral(". LOGIN "));
    loginData.append(m_user).append(QLatin1String(" ", 1)).append(m_password).append(QStringLiteral("\n"));

    this->write(loginData.toLatin1());

    if (!this->waitForReadyRead(5000)) {
        printf("%s\n", qUtf8Printable(tr("Connection to IMAP server timed out.")));
        return false;
    }

    if (!this->checkResponse(this->readAll())) {
        return false;
    }

    return true;
}


bool Imap::logout()
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
        this->disconnect();
        return false;
    }
}


QStringList Imap::getCapabilities()
{
    QStringList resp;

    static const QString capabilitiesCommand(QStringLiteral(". CAPABILITY\n"));

    this->write(capabilitiesCommand.toLatin1());

    if (!this->waitForReadyRead(5000)) {
        printf("%s\n", qUtf8Printable(tr("Connection to IMAP server timed out.")));
        return resp;
    }

    if (!this->checkResponse(this->readAll())) {
        return resp;
    }

    m_response.remove(QRegularExpression(QStringLiteral("\\s*.* (OK|BAD|WARN) .*")));

    resp = m_response.trimmed().split(" ");

    if (!resp.empty()) {
        resp.removeFirst();
        resp.removeFirst();
    } else {
        printf("%s\n", qUtf8Printable(tr("Failed to request capabilities from the IMAP server. Aborting.")));
    }

    return resp;
}



bool Imap::checkResponse(const QByteArray &resp, const QString &tag)
{
    m_response = QString(resp);

    if (m_response.isEmpty()) {
        printf("%s\n", qUtf8Printable(tr("The IMAP response is undefined.")));
        return false;
    }

    if (tag.isEmpty()) {
        m_responseRegex.setPattern(QStringLiteral("\\s*.* (OK|BAD|WARN) "));
    } else {
        m_responseRegex.setPattern(QStringLiteral("\\s*%1 (OK|BAD|WARN) ").arg(tag));
    }

    m_responseRegexMatch = m_responseRegex.match(m_response);

    if (!m_responseRegexMatch.hasMatch()) {
        printf("%s\n", qUtf8Printable(tr("The IMAP response is undefined.")));
        return false;
    }

    const QString respType = m_responseRegexMatch.captured(1);

    if (respType == QLatin1String("OK")) {
        return true;
    } else if (respType == QLatin1String("NO")) {
        printf("%s\n", qUtf8Printable(tr("We received a NO response from the IMAP server.")));
        return false;
    } else if (respType == QLatin1String("BAD")) {
        printf("%s\n", qUtf8Printable(tr("We received a BAD response from the IMAP server, probably because of a malformed request.")));
        return false;
    } else {
        printf("%s\n", qUtf8Printable(tr("The IMAP response is undefined.")));
        return false;
    }
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
