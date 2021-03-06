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

#include "imap.h"
#include <QSslError>
#include <QMessageAuthenticationCode>

QStringList Imap::m_capabilities = QStringList();

Imap::Imap(QObject *parent) : QSslSocket(parent)
{

}


Imap::Imap(const QString &user, const QString &password, AuthMech mech, const QString &host, quint16 port, NetworkLayerProtocol protocol, EncryptionType conType, QChar hierarchysep, const QString &peerName, QObject *parent) :
    QSslSocket(parent), m_user(user), m_password(password), m_host(host), m_port(port), m_protocol(protocol), m_encType(conType), m_hierarchysep(hierarchysep), m_authMech(mech)
{
    setPeerVerifyName(peerName);
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
            m_lastError = tr("Connection to the IMAP server timed out while waiting for connection.");
            this->abort();
            return false;
        }
    } else {
        if (Q_UNLIKELY(!this->waitForEncrypted())) {
            const QList<QSslError> ssles = sslErrors();
            if (!ssles.empty()) {
                m_lastError = tr("Failed to establish encrypted connection to the IMAP server: %1").arg(ssles.first().errorString());
                this->abort();
                return false;
            } else {
                m_lastError = tr("Connection to the IMAP server timed out while waiting for SSL handshake to complete.");
                this->abort();
                return false;
            }
        }
    }

    if (Q_UNLIKELY(!this->waitForReadyRead())) {
        m_lastError = tr("Connection to the IMAP server timed out while waiting for first response on login.");
        this->abort();
        return false;
    }

    QList<QByteArray> response;
    if (Q_UNLIKELY(!checkResponse(this->readAll(), QStringLiteral("*"), &response))) {
        this->disconnectFromHost();
        if (state() != QSslSocket::UnconnectedState) {
            this->waitForDisconnected();
        }
        return false;
    }

    const QByteArray respLine = response.first();

    if (m_encType == StartTLS) {

        if (respLine.contains(QByteArrayLiteral("STARTTLS"))) {

            const QString tag = getTag();
            const QString command = tag + QLatin1String(" STARTTLS\r\n"); // clazy:exclude=qstring-allocations
            this->write(command.toLatin1());

            if (Q_UNLIKELY(!this->waitForReadyRead())) {
                m_lastError = tr("Connection to the IMAP server timed out while wating for response to STARTTLS.");
                this->abort();
                return false;
            }

            if (Q_UNLIKELY(!checkResponse(this->readAll(), tag))) {
                this->disconnectFromHost();
                if (state() != QSslSocket::UnconnectedState) {
                    this->waitForDisconnected();
                }
                return false;
            }

            this->startClientEncryption();

            this->waitForEncrypted();

            if (mode() != QSslSocket::SslClientMode || !isEncrypted()) {
                qDebug() << sslErrors();
                this->abort();
                return false;
            }

        } else {
            m_lastError = tr("STARTTLS is not supported. Aborting.");
            this->disconnectFromHost();
            if (state() != QSslSocket::UnconnectedState) {
                this->waitForDisconnected();
            }
            return false;
        }
    }

    if (m_authMech == CLEAR) {

        const QString tag2 = getTag();
        QString cmd = tag2 + QLatin1String(" LOGIN \"") + m_user + QLatin1String("\" \"") + m_password + QLatin1String("\"\r\n"); // clazy:exclude=qstring-allocations
        QByteArray cmdBa = cmd.toUtf8();
        if (Q_UNLIKELY(write(cmdBa) != cmdBa.length())) {
            return disconnectOnError(tr("Failed to send %1 command to the IMAP server: %2").arg(QStringLiteral("LOGIN"), errorString()));
        }

        if (Q_UNLIKELY(!waitForRespsonse(true, tr("Connection to the IMAP server timed out while waiting for a response to %1.").arg(QStringLiteral("LOGIN"))))) {
            return false;
        }

        if (Q_UNLIKELY(!checkResponse(readAll(), tag2, &response))) {
            return disconnectOnError();
        }

    } else if (m_authMech == LOGIN) {
        const QString tag2 = getTag();
        QString cmd = tag2 + QLatin1String(" AUTHENTICATE LOGIN") + QChar(QChar::CarriageReturn) + QChar(QChar::LineFeed); // clazy:exclude=qstring-allocations
        QByteArray cmdBa = cmd.toLatin1();
        if (Q_UNLIKELY(write(cmdBa) != cmdBa.length())) {
            return disconnectOnError(tr("Failed to send %1 command to the IMAP server: %2").arg(QStringLiteral("AUTHENTICATE LOGIN"), errorString()));
        }

        if (Q_UNLIKELY(!waitForRespsonse(true, tr("Connection to the IMAP server timed out while waiting for a response to %1.").arg(QStringLiteral("AUTHENTICATE LOGIN"))))) {
            return false;
        }

        if (Q_UNLIKELY(!readAll().startsWith('+'))) {
            return disconnectOnError(tr("Invalid response from the IMAP server to %1.").arg(QStringLiteral("AUTHENTICATE LOGIN")));
        }

        cmd = QString::fromLatin1(m_user.toUtf8().toBase64()) + QChar(QChar::CarriageReturn) + QChar(QChar::LineFeed);
        cmdBa = cmd.toLatin1();
        if (Q_UNLIKELY(write(cmdBa) != cmdBa.length())) {
            return disconnectOnError(tr("Failed to send user name to IMAP server: %1").arg(errorString()));
        }

        if (Q_UNLIKELY(!waitForRespsonse(true, tr("Connection to the IMAP server timed out while waiting for a response to the sent user name.")))) {
            return false;
        }

        if (Q_UNLIKELY(!readAll().startsWith('+'))) {
            return disconnectOnError(tr("Invalid response from the IMAP server to the sent user name."));
        }

        cmd = QString::fromLatin1(m_password.toUtf8().toBase64()) + QChar(QChar::CarriageReturn) + QChar(QChar::LineFeed);
        cmdBa = cmd.toLatin1();
        if (Q_UNLIKELY(write(cmdBa) != cmdBa.length())) {
            return disconnectOnError(tr("Failed to send password to IMAP server: %1").arg(errorString()));
        }

        if (Q_UNLIKELY(!waitForRespsonse(true, tr("Connection to the IMAP server timed out while waiting for a response to the sent password.")))) {
            return false;
        }

        if (Q_UNLIKELY(!checkResponse(readAll(), tag2, &response))) {
            return disconnectOnError();
        }

    } else if (m_authMech == PLAIN) {

        const QString tag2 = getTag();
        QString cmd = tag2 + QLatin1String(" AUTHENTICATE PLAIN") + QChar(QChar::CarriageReturn) + QChar(QChar::LineFeed); // clazy:exclude=qstring-allocations
        QByteArray cmdBa = cmd.toLatin1();
        if (Q_UNLIKELY(write(cmdBa) != cmdBa.length())) {
            return disconnectOnError(tr("Failed to send %1 command to the IMAP server: %2").arg(QStringLiteral("AUTHENTICATE PLAIN"), errorString()));
        }

        if (Q_UNLIKELY(!waitForRespsonse(true, tr("Connection to the IMAP server timed out while waiting for a response to %1.").arg(QStringLiteral("AUTHENTICATE LOGIN"))))) {
            return false;
        }

        if (Q_UNLIKELY(!readAll().startsWith('+'))) {
            return disconnectOnError(tr("Invalid response from the IMAP server to %1.").arg(QStringLiteral("AUTHENTICATE PLAIN")));
        }

        cmdBa = QByteArrayLiteral("\0") + m_user.toUtf8() + QByteArrayLiteral("\0") + m_password.toUtf8() + QByteArrayLiteral("\r\n");
        if (Q_UNLIKELY(write(cmdBa) != cmdBa.size())) {
            return disconnectOnError(tr("Failed to send authentication credentials to the IMAP server: %1").arg(errorString()));
        }

        if (Q_UNLIKELY(!waitForRespsonse(true, tr("Connection to the IMAP server timed out while waiting for a response after sending authentication credentials.")))) {
            return false;
        }

        if (!Q_UNLIKELY(!checkResponse(readAll(), tag2, &response))) {
            return disconnectOnError();
        }

    } else if (m_authMech == CRAMMD5) {

        const QString tag2 = getTag();
        QString cmd = tag2 + QLatin1String(" AUTHENTICATE CRAM-MD5") + QChar(QChar::CarriageReturn) + QChar(QChar::LineFeed); // clazy:exclude=qstring-allocations
        QByteArray cmdBa = cmd.toLatin1();
        if (Q_UNLIKELY(write(cmdBa) != cmdBa.length())) {
            return disconnectOnError(tr("Failed to send %1 command to the IMAP server: %2").arg(QStringLiteral("AUTHENTICATE CRAM-MD5"), errorString()));
        }

        if (Q_UNLIKELY(!waitForRespsonse(true, tr("Connection to the IMAP server timed out while waiting for a response to %1.").arg(QStringLiteral("AUTHENTICATE CRAM-MD5"))))) {
            return false;
        }

        QByteArray challenge = readAll();
        if (Q_UNLIKELY(!challenge.startsWith('+'))) {
            return disconnectOnError(tr("Invalid response from the IMAP server to %1.").arg(QStringLiteral("AUTHENTICATE CRAM-MD5")));
        }

        challenge = challenge.mid(2);
        challenge = QByteArray::fromBase64(challenge);

        if (Q_UNLIKELY(!(challenge.startsWith('<') && challenge.endsWith('>')))) {
            return disconnectOnError(tr("Invalid challenge format for CRAM-MD5 authentication mechanism."));
        }

        const QByteArray challengeAnswer = QMessageAuthenticationCode::hash(challenge, m_password.toUtf8(), QCryptographicHash::Md5).toHex().toLower().toBase64() + QByteArrayLiteral("\r\n");

        if (Q_UNLIKELY(write(challengeAnswer) != challengeAnswer.size())) {
            return disconnectOnError(tr("Failed to send challenge response for CRAM-MD5 to the IMAP server: %1").arg(errorString()));
        }

        if (Q_UNLIKELY(!waitForRespsonse(true, tr("Connection to the IMAP server timed out while waiting for a response to the CRAM-MD5 challenge response.")))) {
            return false;
        }

        if (!Q_UNLIKELY(!checkResponse(readAll(), tag2, &response))) {
            return disconnectOnError();
        }
    }

    if (Imap::m_capabilities.empty()) {
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
            if (Imap::m_capabilities.empty()) {
                Imap::m_capabilities = getCapabilities();
                if (Imap::m_capabilities.empty()) {
                    m_lastError = tr("Failed to get capabilities from the IMAP server.");
                    this->disconnectFromHost();
                    if (state() != QSslSocket::UnconnectedState) {
                        this->waitForDisconnected();
                    }
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
    const QString command = tag + QLatin1String(" LOGOUT\r\n"); // clazy:exclude=qstring-allocations
    this->write(command.toLatin1());

    if ((this->state() == ClosingState) || (this->state() == UnconnectedState)) {
        m_loggedIn = false;
        m_tagSequence = 0;
        return true;
    }

    this->waitForReadyRead();

    if (Q_UNLIKELY(!this->checkResponse(this->readAll(), tag))) {
        this->disconnectFromHost();
        m_lastError = tr("Failed to successfully log out from IMAP server.");
        if (Q_UNLIKELY(!this->waitForDisconnected())) {
            m_lastError = tr("Connection to the IMAP server timed out while waiting for disconnection.");
            m_loggedIn = false;
            m_tagSequence = 0;
            this->abort();
        }
        return false;
    }

    m_loggedIn = false;
    m_tagSequence = 0;

    this->disconnectFromHost();

    if ((this->state() != ClosingState) && (this->state() != UnconnectedState)) {
        if (Q_LIKELY(this->waitForDisconnected())) {
            return true;
        } else {
            m_lastError = tr("Connection to the IMAP server timed out while waiting for disconnection.");
            this->abort();
            return false;
        }
    } else {
        return true;
    }
}


QStringList Imap::getCapabilities(bool forceReload)
{
    if (Imap::m_capabilities.empty() || forceReload) {

        Imap::m_capabilities.clear();

        const QString tag = getTag();

        const QString command = tag + QLatin1String(" CAPABILITY\r\n"); // clazy:exclude=qstring-allocations

        this->write(command.toLatin1());

        if (Q_UNLIKELY(!this->waitForReadyRead())) {
            m_lastError = tr("Connection to the IMAP server timed out.");
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

        if (Q_UNLIKELY(Imap::m_capabilities.empty())) {
             m_lastError = tr("Failed to request capabilities from the IMAP server. Aborting.");
        }
    }

    return Imap::m_capabilities;
}



quota_pair Imap::getQuota(const QString &user)
{
    quota_pair quota(0, 0);

    const QString tag = getTag();
    const QString command = tag + QLatin1String(" GETQUOTA user") + m_hierarchysep + user + QChar(QChar::CarriageReturn) + QChar(QChar::LineFeed); // clazy:exclude=qstring-allocations

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
                quota.first = respLine.mid(startUsage, startQuota - (startUsage)).toULongLong();
                // advancing 1 to be at the start of the quota value
                startQuota++;
                int endQuota = respLine.indexOf(' ', startQuota);
                if (endQuota < 0) {
                    endQuota = respLine.indexOf(')', startQuota);
                }
                quota.second = respLine.mid(startQuota, endQuota - startQuota).toULongLong();
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
    if (Q_UNLIKELY(lines.empty())) {
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
                    trimmedList.push_back(baTrimmed);
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

    if (trimmedList.empty()) {
        trimmedList.push_back(statusLine);
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


void Imap::setPort ( const quint16 port )
{
    m_port = port;
}


void Imap::setProtocol ( QAbstractSocket::NetworkLayerProtocol protocol )
{
    m_protocol = protocol;
}


void Imap::setEncryptionType(Imap::EncryptionType type)
{
    m_encType = type;
}


void Imap::setParams(const QVariantHash &parameters)
{
    m_host = parameters.value(QStringLiteral("host")).toString();
    m_port = parameters.value(QStringLiteral("port")).value<quint16>();
    m_user = parameters.value(QStringLiteral("user")).toString();
    m_password = parameters.value(QStringLiteral("password")).toString();
    m_protocol = static_cast<QAbstractSocket::NetworkLayerProtocol>(parameters.value(QStringLiteral("protocol")).toInt());
    m_encType = static_cast<EncryptionType>(parameters.value(QStringLiteral("encryption")).value<quint8>());
    m_authMech = static_cast<AuthMech>(parameters.value(QStringLiteral("authmech")).value<quint8>());
    setPeerVerifyName(parameters.value(QStringLiteral("peername")).toString());
}

void Imap::setHierarchySeparator(QChar separator)
{
    m_hierarchysep = separator;
}

void Imap::setAuthMech(AuthMech mech)
{
    m_authMech = mech;
}

QString Imap::encryptionTypeToString(EncryptionType type)
{
    QString str;

    switch(type) {
    case Unsecured:
        str = tr("Unsecured");
        break;
    case StartTLS:
        str = QStringLiteral("StartTLS");
        break;
    case IMAPS:
        str = QStringLiteral("IMAPS");
        break;
    }

    return str;
}


QString Imap::encryptionTypeToString(quint8 type)
{
    return encryptionTypeToString(static_cast<EncryptionType>(type));
}


QString Imap::networkProtocolToString(QAbstractSocket::NetworkLayerProtocol protocol)
{
    QString str;

    switch(protocol) {
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

QString Imap::networkProtocolToString(quint8 protocol)
{
    return networkProtocolToString(static_cast<QAbstractSocket::NetworkLayerProtocol>(protocol));
}

QString Imap::authMechToString(AuthMech mechanism)
{
    switch (mechanism) {
    case CLEAR:
        return QStringLiteral("CLEAR");
    case LOGIN:
        return QStringLiteral("LOGIN");
    case PLAIN:
        return QStringLiteral("PLAIN");
    case CRAMMD5:
        return QStringLiteral("CRAM-MD5");
    default:
        Q_UNREACHABLE();
        break;
    }
}

QString Imap::authMechToString(quint8 mechanism)
{
    return Imap::authMechToString(static_cast<Imap::AuthMech>(mechanism));
}

QString Imap::lastError() const
{
    return m_lastError;
}

QString Imap::getTag()
{
    return QStringLiteral("a%1").arg(++m_tagSequence, 6, 10, QLatin1Char('0'));
}

bool Imap::disconnectOnError(const QString &error)
{
    if (!error.isEmpty()) {
        m_lastError = error;
    }
    disconnectFromHost();
    if (state() != QSslSocket::UnconnectedState) {
        if (Q_UNLIKELY(!waitForDisconnected())) {
            abort();
        }
    }
    return false;
}

bool Imap::waitForRespsonse(bool _abort, const QString &error, int msecs)
{
    if (Q_UNLIKELY(!waitForReadyRead(msecs))) {
        const QString _error = !error.isEmpty() ? error : tr("Connection to the IMAP server timed out.");
        if (_abort) {
            m_lastError = _error;
            abort();
        } else {
            disconnectOnError(_error);
        }
        return false;
    } else {
        return true;
    }
}
