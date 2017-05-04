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

QStringList Imap::m_capabilities = QStringList();

Imap::Imap(QObject *parent) : QSslSocket(parent)
{

}


Imap::Imap(const QString &user, const QString &password, const QString &host, quint16 port, NetworkLayerProtocol protocol, EncryptionType conType, QChar hierarchysep, QObject *parent) :
    QSslSocket(parent), m_user(user), m_password(password), m_host(host), m_port(port), m_protocol(protocol), m_encType(conType), m_hierarchysep(hierarchysep)
{

}


Imap::~Imap()
{
    logout();
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

    QList<QByteArray> response;
    if (Q_UNLIKELY(!checkResponse(this->readAll(), QLatin1String("*"), &response))) {
        this->disconnectFromHost();
        this->waitForDisconnected();
        return false;
    }

    const QByteArray respLine = response.first();

    if (m_encType == StartTLS) {

        if (respLine.contains(QByteArrayLiteral("STARTTLS"))) {

            const QString tag = getTag();
            const QString command = tag + QLatin1String(" STARTTLS\r\n");
            this->write(command.toLatin1());

            if (Q_UNLIKELY(!this->waitForReadyRead())) {
                m_lastError = tr("Connection to IMAP server timed out while wating for response to STARTTLS.");
                this->abort();
                return false;
            }

            if (Q_UNLIKELY(!checkResponse(this->readAll(), tag))) {
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
    const QString tag2 = getTag();
    const QString loginCommand = tag2 + QLatin1String(" LOGIN ") + m_user + QChar(QChar::Space) + m_password + QChar(QChar::CarriageReturn) + QChar(QChar::LineFeed);

    this->write(loginCommand.toLatin1());

    if (Q_UNLIKELY(!this->waitForReadyRead())) {
        m_lastError = tr("Connection to IMAP server timed out while waiting for response to login data.");
        this->abort();
        return false;
    }

    if (Q_UNLIKELY(!this->checkResponse(this->readAll(), tag2, &response))) {
        this->disconnectFromHost();
        this->waitForDisconnected();
        return false;
    }

    if (Imap::m_capabilities.isEmpty()) {
        if (response.size() == 1) {
            const QByteArray loginRespLine = response.first();
            int start = loginRespLine.indexOf(QByteArrayLiteral("[CAPABILITY"));
            if (start > -1) {
                // advancing start 12 positions to be at the start of the capability list
                // 12 is the length of "[CAPABILITY" + 1
                start += 12;
                int end = loginRespLine.indexOf(QByteArrayLiteral("]"), start);
                if (end > -1) {
                    const QString capstring = QString::fromLatin1(loginRespLine.mid(start, end - start));
                    Imap::m_capabilities = capstring.split(QChar(QChar::Space), QString::SkipEmptyParts);
                }
            }
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

    const QString tag = getTag();
    const QString command = tag + QLatin1String(" LOGOUT\r\n");
    this->write(command.toLatin1());

    this->waitForReadyRead();

    if (Q_UNLIKELY(!this->checkResponse(this->readAll()))) {
        this->disconnectFromHost();
        m_lastError = tr("Failed to successfully log out from IMAP server.");
        if (Q_UNLIKELY(!this->waitForDisconnected())) {
            m_lastError = tr("Connection to IMAP server timed out while waiting for disconnection.");
            m_loggedIn = false;
            m_tagSequence = 0;
            this->abort();
        }
        return false;
    }

    m_loggedIn = false;
    m_tagSequence = 0;

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

        const QString tag = getTag();

        const QString command = tag + QLatin1String(" CAPABILITY\r\n");

        this->write(command.toLatin1());

        if (Q_UNLIKELY(!this->waitForReadyRead())) {
            m_lastError = tr("Connection to IMAP server timed out.");
            return Imap::m_capabilities;
        }

        QList<QByteArray> response;
        if (Q_UNLIKELY(!this->checkResponse(this->readAll(), tag, &response))) {
            return Imap::m_capabilities;
        }

        if (response.isEmpty()) {
            m_lastError = tr("Failed to request capabilities from the IMAP server. Aborting.");
            return Imap::m_capabilities;
        }

        // 13 is the length of "* CAPABILITY " + 1
        const QString respString = QString::fromLatin1(response.first().mid(13));

        if (!respString.isEmpty()) {
            Imap::m_capabilities = respString.split(QChar(QChar::Space), QString::SkipEmptyParts);
        }

        if (Q_UNLIKELY(Imap::m_capabilities.isEmpty())) {
             m_lastError = tr("Failed to request capabilities from the IMAP server. Aborting.");
        }
    }

    return Imap::m_capabilities;
}



std::pair<quint32,quint32> Imap::getQuota(const QString &user)
{
    std::pair<quint32,quint32> quota(0, 0);

    const QString tag = getTag();
    const QString command = tag + QLatin1String(" GETQUOTA user") + m_hierarchysep + user + QChar(QChar::CarriageReturn) + QChar(QChar::LineFeed);

    this->write(command.toLatin1());

    if (Q_LIKELY(this->waitForReadyRead())) {
        QList<QByteArray> response;
        if (checkResponse(this->readAll(), tag, &response)) {
            if (response.empty()) {
                m_lastError = tr("Can not get quota.");
                return quota;
            }
            const QByteArray respLine = response.first();
            int startUsage = respLine.indexOf(QByteArrayLiteral("STORAGE"));
            if (startUsage > -1) {
                // 8 is the length of "STORAGE" + 1
                startUsage += 8;
                int startQuota = respLine.indexOf(' ', startUsage);
                quota.first = respLine.mid(startUsage, startQuota - (startUsage)).toULong();
                // advancing 1 to be at the start of the quota value
                startQuota++;
                int endQuota = respLine.indexOf(' ', startQuota);
                if (endQuota < 0) {
                    endQuota = respLine.indexOf(')', startQuota);
                }
                quota.second = respLine.mid(startQuota, endQuota - startQuota).toULong();
            }
        }
    }

    return quota;
}



bool Imap::checkResponse(const QByteArray &data, const QString &tag, QList<QByteArray> *response)
{
    bool ret = false;

    if (Q_UNLIKELY(data.isEmpty())) {
        m_lastError = tr("The IMAP response is empty");
        return ret;
    }

    const QList<QByteArray> lines = data.split('\n');
    if (Q_UNLIKELY(lines.isEmpty())) {
        m_lastError = tr("The IMAP response is empty");
        return ret;
    }

    const QByteArray tagLatin1 = tag.toLatin1();

    QByteArray statusLine;
    QList<QByteArray> trimmedList;
    for (const QByteArray &ba : lines) {
        if (!ba.isEmpty()) {
            QByteArray baTrimmed = ba.trimmed();
            if (!baTrimmed.isEmpty()) {
                if (baTrimmed.startsWith(tagLatin1)) {
                    statusLine = baTrimmed;
                } else {
                    trimmedList.append(baTrimmed);
                }
            }
        }
    }

    if (Q_UNLIKELY(statusLine.isEmpty() && (trimmedList.size() == 1))) {
        statusLine = trimmedList.last();
    }

    if (Q_UNLIKELY(statusLine.isEmpty())) {
        m_lastError = tr("The IMAP response is undefined");
        return ret;
    }

    if (trimmedList.isEmpty()) {
        trimmedList.append(statusLine);
    }

    const QByteArray status = statusLine.mid(tagLatin1.size()+1);

    if (status.startsWith(QByteArrayLiteral("OK"))) {
        ret = true;
        if (response) {
            response->swap(trimmedList);
        }
    } else if (status.startsWith(QByteArrayLiteral("BAD"))) {
        m_lastError = tr("We received a BAD response from the IMAP server: %1").arg(QString::fromLatin1(status.mid(4)));
    } else if (status.startsWith(QByteArrayLiteral("NO"))) {
        m_lastError = tr("We received a NO response from the IMAP server: %1").arg(QString::fromLatin1(status.mid(3)));
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


void Imap::setHierarchySeparator(QChar separator)
{
    m_hierarchysep = separator;
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


QString Imap::getTag()
{
    return QStringLiteral("a%1").arg(++m_tagSequence, 6, 10, QLatin1Char('0'));
}
