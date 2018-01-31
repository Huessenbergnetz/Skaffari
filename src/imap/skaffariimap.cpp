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
#include "../utils/skaffariconfig.h"
#include <unicode/ucnv_err.h>
#include <unicode/uenum.h>
#include <unicode/localpointer.h>
#include <unicode/ucnv.h>
#include <QVariantMap>
#include <QSslError>
#include <Cutelyst/Context>

Q_LOGGING_CATEGORY(SK_IMAP, "skaffari.imap")

QStringList SkaffariIMAP::m_capabilities = QStringList();
const QString SkaffariIMAP::m_allAcl = QStringLiteral("lrswipkxtecda");

SkaffariIMAP::SkaffariIMAP(Cutelyst::Context *context, QObject *parent) :
    QSslSocket(parent),
    m_user(SkaffariConfig::imapUser()),
    m_password(SkaffariConfig::imapPassword()),
    m_host(SkaffariConfig::imapHost()),
    m_c(context),
    m_port(SkaffariConfig::imapPort()),
    m_protocol(SkaffariConfig::imapProtocol()),
    m_encType(SkaffariConfig::imapEncryption())
{
    Q_ASSERT_X(m_c, "Skaffari IMAP", "invalid context");

    if (SkaffariConfig::imapUnixhierarchysep()) {
        m_hierarchysep = QLatin1Char('/');
    }

    setPeerVerifyName(SkaffariConfig::imapPeername());
}


SkaffariIMAP::~SkaffariIMAP()
{
    logout();
}


bool SkaffariIMAP::login()
{
    setNoError();

    if (m_loggedIn) {
        return true;
    }

    if (m_encType != IMAPS) {
        connectToHost(m_host, m_port, ReadWrite, m_protocol);
    } else {
        connectToHostEncrypted(m_host, m_port, ReadWrite, m_protocol);
    }

    if (m_encType != IMAPS) {
        if (Q_UNLIKELY(!waitForConnected())) {
            abort();
            return connectionTimeOut();
        }
    } else {
        if (Q_UNLIKELY(!waitForEncrypted())) {
            const QList<QSslError> sslErrs = sslErrors();
            if (!sslErrs.empty()) {
                m_imapError = SkaffariIMAPError(sslErrs.first());
                abort();
                return false;
            } else {
                abort();
                return connectionTimeOut();
            }
        }
    }

    if (Q_UNLIKELY(!waitForReadyRead())) {
        abort();
        return connectionTimeOut();
    }

    QVector<QByteArray> response;
    if (Q_UNLIKELY(!checkResponse(readAll(), QLatin1String("*"), &response))) {
        disconnectFromHost();
        waitForDisconnected();
        return false;
    }

    const QByteArray respLine = response.first();

    if (m_encType == StartTLS) {

        if (respLine.contains(QByteArrayLiteral("STARTTLS"))) {

            const QString tag = getTag();
            const QString command = tag + QLatin1String(" STARTTLS\r\n");
            write(command.toLatin1());

            if (Q_UNLIKELY(!waitForReadyRead())) {
                abort();
                return connectionTimeOut();
            }

            if (Q_UNLIKELY(!checkResponse(readAll(), tag))) {
                disconnectFromHost();
                waitForDisconnected();
                return false;
            }

            startClientEncryption();

            waitForEncrypted();

            if ((mode() != QSslSocket::SslClientMode || !isEncrypted())) {
                QString sslErrorString;
                if (!sslErrors().empty()) {
                    sslErrorString = sslErrors().constFirst().errorString();
                }
                m_imapError = SkaffariIMAPError(SkaffariIMAPError::EncryptionError, m_c->translate("SkaffariIMAP", "Failed to initiate STARTTLS: %1").arg(sslErrorString));
                abort();
                return false;
            }

        } else {
            m_imapError = SkaffariIMAPError(SkaffariIMAPError::EncryptionError, m_c->translate("SkaffariIMAP", "STARTTLS is not supported."));
            disconnectFromHost();
            waitForDisconnected();
            return false;
        }
    }

    const QString tag2 = getTag();
    const QString loginCommand = tag2 + QLatin1String(" LOGIN \"") + m_user + QLatin1String("\" \"") + m_password + QLatin1String("\"\r\n");

    if (Q_UNLIKELY(write(loginCommand.toLatin1()) < 0)) {
        m_imapError = SkaffariIMAPError(SkaffariIMAPError::SocketError, m_c->translate("SkaffariIMAP", "Failed to send command to IMAP server: %1").arg(errorString()));
        disconnectFromHost();
        waitForDisconnected();
        return false;
    }

    if (Q_UNLIKELY(!waitForReadyRead())) {
        abort();
        return connectionTimeOut();
    }

    if (Q_UNLIKELY(!checkResponse(readAll(), tag2, &response))) {
        disconnectFromHost();
        waitForDisconnected();
        return false;
    }

    m_loggedIn = true;

    if (SkaffariIMAP::m_capabilities.empty()) {
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
                    SkaffariIMAP::m_capabilities = capstring.split(QChar(QChar::Space), QString::SkipEmptyParts);
                }
            }
            if (SkaffariIMAP::m_capabilities.empty()) {
                SkaffariIMAP::m_capabilities = getCapabilities();
                if (SkaffariIMAP::m_capabilities.empty()) {
                    logout();
                    return false;
                }
            }
        }
    }

    return true;
}


bool SkaffariIMAP::logout()
{
    setNoError();

    if (!m_loggedIn) {
        return true;
    }

    if (state() == UnconnectedState) {
        m_loggedIn = false;
        return true;
    }

    if (state() == ClosingState) {
        m_loggedIn = false;
        return true;
    }

    const QString tag = getTag();
    const QString command = tag + QLatin1String(" LOGOUT\r\n");

    if (Q_UNLIKELY(write(command.toLatin1()) < 0)) {
        m_imapError = SkaffariIMAPError(SkaffariIMAPError::SocketError, m_c->translate("SkaffariIMAP", "Failed to send command to IMAP server: %1").arg(errorString()));
        disconnectFromHost();
        waitForDisconnected(3000);
        m_loggedIn = false;
        m_tagSequence = 0;
        return false;
    }

    waitForReadyRead();

    if (Q_UNLIKELY(!checkResponse(readAll(), tag))) {
        disconnectFromHost();
        if (Q_UNLIKELY(!waitForDisconnected())) {
            m_loggedIn = false;
            m_tagSequence = 0;
            abort();
        }
        return false;
    }

    m_loggedIn = false;
    m_tagSequence = 0;

    disconnectFromHost();

    if (Q_LIKELY(waitForDisconnected())) {
        return true;
    } else {
        return connectionTimeOut();
    }
}


QStringList SkaffariIMAP::getCapabilities(bool forceReload)
{
    setNoError();

    if (SkaffariIMAP::m_capabilities.empty() || forceReload) {

        SkaffariIMAP::m_capabilities.clear();

        const QString tag = getTag();

        const QString command = tag + QLatin1String(" CAPABILITY\r\n");

        if (Q_UNLIKELY(!sendCommand(command))) {
            return SkaffariIMAP::m_capabilities;
        }

        if (Q_UNLIKELY(!waitForReadyRead())) {
            connectionTimeOut();
            return SkaffariIMAP::m_capabilities;
        }

        QVector<QByteArray> response;
        if (Q_UNLIKELY(!checkResponse(readAll(), tag, &response))) {
            return SkaffariIMAP::m_capabilities;
        }

        if (response.isEmpty()) {
            m_imapError = SkaffariIMAPError(SkaffariIMAPError::ResponseError, m_c->translate("SkaffariIMAP", "Failed to request capabilities from the IMAP server."));
            return SkaffariIMAP::m_capabilities;
        }

        // 13 is the length of "* CAPABILITY " + 1
        const QString respString = QString::fromLatin1(response.first().mid(13));

        if (!respString.isEmpty()) {
            SkaffariIMAP::m_capabilities = respString.split(QChar(QChar::Space), QString::SkipEmptyParts);
        }

        if (Q_UNLIKELY(SkaffariIMAP::m_capabilities.empty())) {
            m_imapError = SkaffariIMAPError(SkaffariIMAPError::ResponseError, m_c->translate("SkaffariIMAP", "Failed to request capabilities from the IMAP server."));
        }
    }

    return SkaffariIMAP::m_capabilities;
}


quota_pair SkaffariIMAP::getQuota(const QString &user)
{
    quota_pair quota(0, 0);

    setNoError();

    const QString tag = getTag();
    const QString command = tag + QLatin1String(" GETQUOTA \"user") + m_hierarchysep + user + QLatin1Char('"') + QChar(QChar::CarriageReturn) + QChar(QChar::LineFeed);

    write(command.toLatin1());

    if (Q_LIKELY(waitForReadyRead())) {
        QVector<QByteArray> response;
        if (Q_LIKELY(checkResponse(readAll(), tag, &response))) {
            if (Q_UNLIKELY(response.empty())) {
                qCCritical(SK_IMAP, "Failed to request storage quota for user %s.", user.toUtf8().constData());
                m_imapError = SkaffariIMAPError(SkaffariIMAPError::ResponseError, m_c->translate("SkaffariIMAP", "Failed to request storage quota."));
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
            } else {
                qCWarning(SK_IMAP, "Can not extract storage quota values for user %s from IMAP server response.", user.toUtf8().constData());
            }
        } else {
            qCCritical(SK_IMAP, "Failed to get quota for user %s.", user.toUtf8().constData());
        }
    } else {
        connectionTimeOut();
    }

    return quota;
}


bool SkaffariIMAP::setQuota(const QString &user, quota_size_t quota)
{
    bool ok = false;

    setNoError();

    const QString tag = getTag();
    const QString command = tag + QLatin1String(" SETQUOTA \"user") + m_hierarchysep + user + QLatin1String("\" (STORAGE ") + QString::number(quota) + QLatin1String(")\r\n");

    if (Q_UNLIKELY(!sendCommand(command))) {
        return ok;
    }

    if (Q_UNLIKELY(!waitForReadyRead())) {
        ok = connectionTimeOut();
        return ok;
    }

    ok = checkResponse(readAll(), tag);

    if (Q_UNLIKELY(!ok)) {
        qCCritical(SK_IMAP, "Failed to set quota value of %llu for user %s.", quota, qUtf8Printable(user));
    }

    return ok;
}


bool SkaffariIMAP::createMailbox(const QString &user)
{
    bool ok = false;

    Q_ASSERT_X(!user.isEmpty(), "create mailbox", "empty username");

    setNoError();

    const QString tag = getTag();
    const QString command = tag + QLatin1String(" CREATE \"user") + m_hierarchysep + user + QLatin1String("\"\r\n");

    if (Q_UNLIKELY(!sendCommand(command))) {
        return ok;
    }

    if (Q_UNLIKELY(!waitForReadyRead())) {
        ok = connectionTimeOut();
        return ok;
    }

    ok = checkResponse(readAll(), tag);
    if (Q_UNLIKELY(!ok)) {
        qCCritical(SK_IMAP, "Failed to create mailbox for user %s.", user.toUtf8().constData());
    }

    return ok;
}


bool SkaffariIMAP::deleteMailbox(const QString &user)
{
    Q_ASSERT_X(!user.isEmpty(), "delete mailbox", "empty username");

    setNoError();

    const QString tag = getTag();
    const QString command = tag + QLatin1String(" DELETE \"user") + m_hierarchysep + user + QLatin1String("\"\r\n");

    if (Q_UNLIKELY(!sendCommand(command))) {
        return false;
    }

    if (Q_UNLIKELY(!waitForReadyRead())) {
        return false;
    }

    return checkResponse(readAll(), tag);
}


bool SkaffariIMAP::createFolder(const QString &folder)
{
    Q_ASSERT_X(!folder.isEmpty(), "create folder", "empty folder name");

    setNoError();

    const QString _folder = toUTF7Imap(folder.trimmed());

    if (Q_UNLIKELY(_folder.isEmpty())) {
        m_imapError = SkaffariIMAPError(SkaffariIMAPError::InternalError, m_c->translate("SkaffariIMAP", "Failed to convert folder name into UTF-7-IMAP."));
        return false;
    }

    const QString tag1 = getTag();
    const QString command1 = tag1 + QLatin1String(" CREATE \"INBOX") + m_hierarchysep + _folder + QLatin1String("\"\r\n");

    if (Q_UNLIKELY(!sendCommand(command1))) {
        return false;
    }

    if (Q_UNLIKELY(!waitForReadyRead())) {
        return connectionTimeOut();
    }

    if (Q_UNLIKELY(!checkResponse(readAll(), tag1))) {
        return false;
    }

    const QString tag2 = getTag();
    const QString command2 = tag2 + QLatin1String(" SUBSCRIBE \"INBOX") + m_hierarchysep + _folder + QLatin1String("\"\r\n");

    if (Q_UNLIKELY(!sendCommand(command2))) {
        return false;
    }

    if (Q_UNLIKELY(!waitForReadyRead())) {
        return connectionTimeOut();
    }

    return checkResponse(readAll(), tag2);
}


bool SkaffariIMAP::setAcl(const QString &mailbox, const QString &user, const QString &acl)
{
    setNoError();
    QString _acl;
    if (acl.isEmpty()) {
        _acl = m_allAcl;
    }

    const QString tag = getTag();
    const QString command = tag + QLatin1String(" SETACL \"user") + m_hierarchysep + mailbox + QLatin1String("\" \"") + user + QLatin1String("\" ") + _acl + QChar(QChar::CarriageReturn) + QChar(QChar::LineFeed);

    if (Q_UNLIKELY(!sendCommand(command))) {
        return false;
    }

    if (Q_UNLIKELY(!waitForReadyRead())) {
        return connectionTimeOut();
    }

    return checkResponse(readAll(), tag);
}


bool SkaffariIMAP::deleteAcl(const QString &mailbox, const QString &user)
{
    setNoError();

    Q_ASSERT_X(!mailbox.isEmpty(), "delete acl", "empty mailbox name");
    Q_ASSERT_X(!user.isEmpty(), "delete acl", "empty user name");

    const QString tag = getTag();
    const QString command = tag + QLatin1String(" DELETEACL \"user") + m_hierarchysep + mailbox + QLatin1String("\" \"") + user + QLatin1String("\"\r\n");

    if (Q_UNLIKELY(!sendCommand(command))) {
        return false;
    }

    if (Q_UNLIKELY(!waitForReadyRead())) {
        return false;
    }

    return checkResponse(readAll(), tag);
}


QStringList SkaffariIMAP::getMailboxes()
{
    QStringList list;

    setNoError();

    const QString tag = getTag();
    const QString command = tag + QLatin1String(" LIST \"user.\" %\r\n");

    if (Q_UNLIKELY(!sendCommand(command))) {
        return list;
    }

    if (Q_UNLIKELY(!waitForReadyRead())) {
        return list;
    }

    QVector<QByteArray> respLines;
    if (Q_UNLIKELY(!checkResponse(readAll(), tag, &respLines))) {
        return list;
    }

    for (auto i = respLines.cbegin(); i != respLines.cend(); ++i) {
        const QByteArray line = *i;
        const int idx = line.lastIndexOf(QByteArrayLiteral("user."));
        if (idx > -1) {
            const QByteArray mbox = line.mid(idx + 5);
            list.push_back(QString::fromLatin1(mbox));
        }
    }

    return list;
}


bool SkaffariIMAP::connectionTimeOut()
{
    qCWarning(SK_IMAP) << "Connection to IMAP server timed out.";
    m_imapError = SkaffariIMAPError(SkaffariIMAPError::ConnectionTimeout, m_c->translate("SkaffariIMAP", "Connection to IMAP server timed out."));
    abort();
    return false;
}


bool SkaffariIMAP::checkResponse(const QByteArray &data, const QString &tag, QVector<QByteArray> *response)
{
    bool ret = false;

    if (Q_UNLIKELY(data.isEmpty())) {
        qCWarning(SK_IMAP) << "The IMAP response is undefined.";
        m_imapError = SkaffariIMAPError(SkaffariIMAPError::UndefinedResponse, m_c->translate("SkaffariIMAP", "The IMAP response is undefined."));
        return ret;
    }

    const QList<QByteArray> lines = data.split('\n');
    if (Q_UNLIKELY(lines.empty())) {
        qCWarning(SK_IMAP) << "The IMAP response is undefined.";
        m_imapError = SkaffariIMAPError(SkaffariIMAPError::UndefinedResponse, m_c->translate("SkaffariIMAP", "The IMAP response is undefined."));
        return ret;
    }

    const QByteArray tagLatin1 = tag.toLatin1();

    QByteArray statusLine;
    QVector<QByteArray> trimmedList;
    for (const QByteArray &ba : lines) {
        if (!ba.isEmpty()) {
            const QByteArray baTrimmed = ba.trimmed();
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
        m_imapError = SkaffariIMAPError(SkaffariIMAPError::UndefinedResponse, m_c->translate("SkaffariIMAP", "The IMAP response is undefined."));
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
        m_imapError = SkaffariIMAPError(SkaffariIMAPError::BadResponse, m_c->translate("SkaffariIMAP", "We received a BAD response from the IMAP server: %1").arg(QString::fromLatin1(status.mid(4))));;
        qCCritical(SK_IMAP) << "We received a BAD response from the IMAP server:" << QString::fromLatin1(status.mid(4));
    } else if (status.startsWith(QByteArrayLiteral("NO"))) {
        m_imapError = SkaffariIMAPError(SkaffariIMAPError::NoResponse, m_c->translate("SkaffariIMAP", "We received a NO response from the IMAP server: %1").arg(QString::fromLatin1(status.mid(3))));
        qCCritical(SK_IMAP) << "We received a NO response from the IMAP server:" << QString::fromLatin1(status.mid(3));
    } else {
        m_imapError = SkaffariIMAPError(SkaffariIMAPError::UndefinedResponse, m_c->translate("SkaffariIMAP", "The IMAP response is undefined."));
        qCCritical(SK_IMAP) << "The IMAP response is undefined.";
    }

    return ret;
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



QString SkaffariIMAP::toUTF7Imap(const QString &str)
{
    QString utf7Imap;
    QByteArray ba;

    if (str.isEmpty()) {
        return utf7Imap;
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

    utf7Imap = QString::fromLatin1(ba);

    return utf7Imap;
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


QString SkaffariIMAP::getTag()
{
    return QStringLiteral("a%1").arg(++m_tagSequence, 6, 10, QLatin1Char('0'));
}


bool SkaffariIMAP::sendCommand(const QString &command)
{
    qCDebug(SK_IMAP) << "Sending command:" << command;

    if (Q_UNLIKELY(write(command.toLatin1()) < 0)) {
        qCCritical(SK_IMAP, "Failed to send command \"%s\" to the IMAP server: %s", qUtf8Printable(command), qUtf8Printable(errorString()));
        m_imapError = SkaffariIMAPError(SkaffariIMAPError::SocketError, m_c->translate("SkaffariIMAP", "Failed to send command to IMAP server: %1").arg(errorString()));
        return false;
    }

    return true;
}

#include "moc_skaffariimap.cpp"
