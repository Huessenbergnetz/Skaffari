/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "imap.h"
#include "skaffariimap.h"
#include "../utils/skaffariconfig.h"

#include <Cutelyst/Context>

#include <QLoggingCategory>
#include <QMessageAuthenticationCode>

using namespace Skaffari;

Imap::Imap(Cutelyst::Context *c, QObject *parent)
    : QSslSocket{parent}
    , m_c{c}
{
}

ImapError Imap::lastError() const noexcept
{
    return m_lastError;
}

bool Imap::login(const QString &user, const QString &password)
{
    if (m_loggedIn) {
        return true;
    }

    m_lastError.clear();

    const EncryptionType encType = SkaffariConfig::imapEncryption2();

    if (encType != IMAPS) {
        connectToHost(SkaffariConfig::imapHost(), SkaffariConfig::imapPort(), ReadWrite, SkaffariConfig::imapProtocol());
        if (Q_UNLIKELY(!waitForConnected())) {
            connectionTimedOut();
            return false;
        }
    } else {
        connectToHostEncrypted(SkaffariConfig::imapHost(), SkaffariConfig::imapPort(), ReadWrite, SkaffariConfig::imapProtocol());
        if (Q_UNLIKELY(!waitForEncrypted())) {
            const QList<QSslError> sslErrors = sslHandshakeErrors();
            if (!sslErrors.empty()) {
                m_lastError = ImapError{sslErrors.first()};
                abort();
            } else {
                connectionTimedOut();
            }
            return false;
        }
    }

    if (Q_UNLIKELY(!waitForResponse(true))) {
        return false;
    }

    ImapResult r = checkResponse(readAll());

    if (!r) {
        disconnectOnError(r.error());
        return false;
    }

    if (encType == StartTLS) {
        if (!hasCapability(QStringLiteral("STARTTLS"), true)) {
            disconnectOnError(ImapError{ImapError::EncryptionError, m_c->translate("SkaffariIMAP", "STARTTLS is not supported.")});
            return false;
        }

        const QString tag = getTag();

        if (Q_UNLIKELY(!sendCommand(tag, QStringLiteral("STARTTLS")))) {
            disconnectOnError();
            return false;
        }

        if (Q_UNLIKELY(!waitForResponse(true))) {
            return false;
        }

        r = checkResponse(readAll(), tag);
        if (!r) {
            disconnectOnError(r.error());
            return false;
        }

        startClientEncryption();

        waitForEncrypted();

        if (mode() != QSslSocket::SslClientMode || !isEncrypted()) {
            const QList<QSslError> sslErrors = sslHandshakeErrors();
            const QString sslErrorString = sslErrors.empty() ? QString() : sslErrors.constFirst().errorString();
            m_lastError = ImapError{ImapError::EncryptionError, m_c->translate("SkaffariIMAP", "Failed to initiate STARTTLS: %1").arg(sslErrorString)};
            abort();
            return false;
        }
    }

    QStringList caps = getCapabilities(true);

    if (caps.contains(QStringLiteral("AUTH=CRAM-MD5"))) {
        if (!authCramMd5(user, password)) {
            return false;
        }
    } else if (caps.contains(QStringLiteral("AUTH=PLAIN"))) {
        if (!authPlain(user, password)) {
            return false;
        }
    } else if (caps.contains(QStringLiteral("AUTH=LOGIN"))) {
        if (!authLogin(user, password)) {
            return false;
        }
    } else { // use IMAP LOGIN as fallback
        const QString tag = getTag();
        const QString cmd = QLatin1String("LOGIN \"") + user + QLatin1String("\" \"") + password + QLatin1Char('"');

        if (Q_UNLIKELY(!sendCommand(tag, cmd))) {
            disconnectOnError();
            return false;
        }

        if (Q_UNLIKELY(!waitForResponse(true))) {
            return false;
        }

        r = checkResponse(readAll(), tag);
        if (!r) {
            disconnectOnError(r.error());
            return false;
        }
    }

    m_loggedIn = true;

    if (hasCapability(QStringLiteral("ID"), true)) {
        sendId();
    }

    if (hasCapability(QStringLiteral("NAMESPACE"))) {

    }

    return true;
}

void Imap::logout()
{
    if (!m_loggedIn) {
        return;
    }

    m_lastError.clear();
    m_loggedIn = false;
    m_tagSequence = 0;

    if (state() == UnconnectedState || state() == ClosingState) {
        return;
    }

    const QString tag = getTag();

    if (Q_UNLIKELY(!sendCommand(tag, QStringLiteral("LOGOUT")))) {
        disconnectOnError();
        return;
    }

    if (Q_UNLIKELY(!waitForResponse(true))) {
        return;
    }

    const ImapResult r = checkResponse(readAll(), tag);

    if (!r) {
        disconnectOnError();
        return;
    }

    disconnectFromHost();
    if (state() != QSslSocket::UnconnectedState && !waitForDisconnected()) {
        abort();
    }
}

QStringList Imap::getCapabilities(bool reload)
{
    if (reload || m_capabilites.empty()) {
        m_capabilites.clear();

        const QString tag = getTag();

        if (Q_UNLIKELY(!sendCommand(tag, QStringLiteral("CAPABILITY")))) {
            return m_capabilites;
        }

        if (Q_UNLIKELY(!waitForResponse())) {
            return m_capabilites;
        }

        ImapResult r = checkResponse(readAll(), tag);
        if (!r) {
            m_lastError = r.error();
            return m_capabilites;
        }

        if (r.lines().empty()) {
            m_lastError = ImapError{ImapError::ResponseError, m_c->translate("SkaffariIMAP", "Failed to request capabilities from the IMAP server.")};
            return m_capabilites;
        }

        const QStringList lines = r.lines();
        for (const auto &l : lines) {
            const QStringList caps = l.split(QChar(QChar::Space), Qt::SkipEmptyParts);
            for (const auto &cap : caps) {
                const auto c = cap.toUpper();
                if (c != QLatin1String("CAPABILITY")) {
                    m_capabilites << c;
                }
            }
        }
    }

    return m_capabilites;
}

bool Imap::hasCapability(const QString &capability, bool reload)
{
    if (reload) {
        const QStringList caps = getCapabilities(true);
        return caps.contains(capability);
    } else {
        return m_capabilites.contains(capability);
    }
}

QString Imap::getTag()
{
    return QStringLiteral("a%1").arg(++m_tagSequence, 6, 10, QLatin1Char('0'));
}

bool Imap::sendCommand(const QString &command)
{
    return sendCommand(command.toLatin1());
}

bool Imap::sendCommand(const QString &tag, const QString &command)
{
    const QString cmd = tag + QChar(QChar::Space) + command;
    return sendCommand(cmd.toLatin1());
}

bool Imap::sendCommand(const QByteArray &command)
{
    qCDebug(SK_IMAP) << "Sending command:" << command;

    const QByteArray cmd = command + QByteArrayLiteral("\r\n");

    if (Q_UNLIKELY(write(cmd) != cmd.size())) {
        qCCritical(SK_IMAP) << "Failed to send command" << command << "to the IMAP server:"
                            << errorString();
        m_lastError = ImapError(ImapError::SocketError, m_c->translate("SkaffariIMAP", "Failed to send command to IMAP server: %1").arg(errorString()));
        return false;
    }

    return true;
}

void Imap::disconnectOnError(const ImapError &error)
{
    if (error) {
        m_lastError = error;
    }
    disconnectFromHost();
    if (state() != QSslSocket::UnconnectedState) {
        if (Q_UNLIKELY(!waitForDisconnected())) {
            abort();
        }
    }
    m_loggedIn = false;
}

void Imap::connectionTimedOut()
{
    qCWarning(SK_IMAP) << "Connection to IMAP server timed out.";
    m_lastError = ImapError{ImapError::ConnectionTimeout, m_c->translate("SkaffariIMAP", "Connection to IMAP server timed out.")};
    abort();
}

bool Imap::waitForResponse(bool disCon, const QString &errorString, int msecs)
{
    if (Q_LIKELY(waitForReadyRead(msecs))) {
        return true;
    }

    ImapError e{ImapError::ConnectionTimeout, errorString.isEmpty() ? m_c->translate("SkaffariIMAP", "Connection to the IMAP server timed out.") : errorString};

    if (disCon) {
        disconnectOnError(e);
    } else {
        m_lastError = std::move(e);
    }

    return false;
}

ImapResult Imap::checkResponse(const QByteArray &data, const QString &tag)
{
    if (Q_UNLIKELY(data.isEmpty())) {
        qCWarning(SK_IMAP) << "The IMAP response is undefined.";
        return {ImapResult::Undefined, ImapError{ImapError::UndefinedResponse, m_c->translate("SkaffariIMAP", "The IMAP response is undefined.")}};
    }

    const QList<QByteArray> rawLines = data.split('\n');
    if (Q_UNLIKELY(rawLines.empty())) {
        qCWarning(SK_IMAP) << "The IMAP response is undefined.";
        return {ImapResult::Undefined, ImapError{ImapError::UndefinedResponse, m_c->translate("SkaffariIMAP", "The IMAP response is undefined.")}};
    }

    QString statusLine;
    QStringList lines;

    for (const auto &rawLine : rawLines) {
        QString line = QString::fromLatin1(rawLine.trimmed());
        if (!tag.isEmpty() && line.startsWith(tag)) {
            statusLine = statusLine.mid(tag.size() + 1);
        } else {
            lines.push_back(line.mid(2));
        }
    }

    if (statusLine.isEmpty() && !lines.empty()) {
        statusLine = lines.takeLast();
    }

    if (Q_UNLIKELY(statusLine.isEmpty())) {
        qCWarning(SK_IMAP) << "The IMAP response is undefined.";
        return {ImapResult::Undefined, ImapError{ImapError::UndefinedResponse, m_c->translate("SkaffariIMAP", "The IMAP response is undefined.")}};
    }

    if (statusLine.startsWith(QLatin1String("OK"), Qt::CaseInsensitive)) {
        return {ImapResult::OK, statusLine.mid(3), lines};
    } else if (statusLine.startsWith(QLatin1String("BAD"), Qt::CaseInsensitive)) {
        qCCritical(SK_IMAP) << "We received a BAD response from the IMAP server:" << statusLine.mid(4);
        return {ImapResult::BAD, ImapError{ImapError::BadResponse, m_c->translate("SkaffariIMAP", "We received a BAD response from the IMAP server: %1").arg(statusLine.mid(4))}};
    } else if (statusLine.startsWith(QLatin1String("NO"), Qt::CaseInsensitive)) {
        qCCritical(SK_IMAP) << "We received a NO response from the IMAP server:" << statusLine.mid(3);
        return {ImapResult::NO, ImapError{ImapError::NoResponse, m_c->translate("SkaffariIMAP", "We received a NO response from the IMAP server: %1").arg(statusLine.mid(3))}};
    } else {
        qCCritical(SK_IMAP) << "The IMAP response is undefined";
        return {ImapResult::Undefined, ImapError{ImapError::UndefinedResponse, m_c->translate("SkaffariIMAP", "The IMAP response is undefined.")}};
    }
}

bool Imap::authLogin(const QString &user, const QString &password)
{
    const QString tag = getTag();

    if (Q_UNLIKELY(!sendCommand(tag, QStringLiteral("AUTHENTICATE LOGIN")))) {
        disconnectOnError();
        return false;
    }

    if (Q_UNLIKELY(!waitForResponse(true))) {
        return false;
    }

    if (Q_UNLIKELY(!readAll().startsWith('+'))) {
        disconnectOnError(ImapError{ImapError::ResponseError, m_c->translate("SkaffariIMAP", "Invalid response after AUTHENTICATE LOGIN.")});
        return false;
    }

    if (Q_UNLIKELY(!sendCommand(user.toUtf8().toBase64()))) {
        disconnectOnError();
        return false;
    }

    if (Q_UNLIKELY(!waitForResponse(true))) {
        return false;
    }

    if (Q_UNLIKELY(!readAll().startsWith('+'))) {
        disconnectOnError(ImapError{ImapError::ResponseError, m_c->translate("SkaffariIMAP", "Invalid response after AUTHENTICATE LOGIN.")});
        return false;
    }

    if (Q_UNLIKELY(!sendCommand(password.toUtf8().toBase64()))) {
        disconnectOnError();
        return false;
    }

    if (Q_UNLIKELY(!waitForResponse(true))) {
        return false;
    }

    const ImapResult r = checkResponse(readAll(), tag);
    if (!r) {
        disconnectOnError(r.error());
        return false;
    }

    return true;
}

bool Imap::authPlain(const QString &user, const QString &password)
{
    const QString tag = getTag();

    if (Q_UNLIKELY(!sendCommand(tag, QStringLiteral("AUTHENTICATE PLAIN")))) {
        disconnectOnError();
        return false;
    }

    if (Q_UNLIKELY(!waitForResponse(true))) {
        return false;
    }

    if (Q_UNLIKELY(!readAll().startsWith('+'))) {
        disconnectOnError(ImapError{ImapError::ResponseError, m_c->translate("SkaffariIMAP", "Invalid response after AUTHENTICATE PLAIN.")});
        return false;
    }

    const QByteArray cmd = QByteArrayLiteral("\0") + user.toUtf8() + QByteArrayLiteral("\0") + password.toUtf8();
    if (Q_UNLIKELY(!sendCommand(cmd))) {
        disconnectOnError();
        return false;
    }

    if (Q_UNLIKELY(!waitForResponse(true))) {
        return false;
    }

    const ImapResult r = checkResponse(readAll(), tag);
    if (!r) {
        disconnectOnError(r.error());
        return false;
    }

    return true;
}

bool Imap::authCramMd5(const QString &user, const QString &password)
{
    const QString tag = getTag();

    if (Q_UNLIKELY(!sendCommand(tag, QStringLiteral("AUTHENTICATE CRAM-MD5")))) {
        disconnectOnError();
        return false;
    }

    if (Q_UNLIKELY(!waitForResponse(true))) {
        return false;
    }

    QByteArray challenge = readAll();
    if (Q_UNLIKELY(!challenge.startsWith('+'))) {
        disconnectOnError(ImapError{ImapError::ResponseError, m_c->translate("SkaffariIMAP", "Invalid response from the IMAP server to %1.").arg(QStringLiteral("AUTHENTICATE CRAM-MD5"))});
        return false;
    }

    challenge = challenge.mid(2);
    challenge = QByteArray::fromBase64(challenge);

    if (Q_UNLIKELY(!(challenge.startsWith('<') && challenge.endsWith('>')))) {
        disconnectOnError(ImapError{ImapError::ResponseError, m_c->translate("SkaffariIMAP", "Invalid challenge format for CRAM-MD5 authentication mechanism.")});
        return false;
    }

    challenge = QMessageAuthenticationCode::hash(challenge, password.toUtf8(), QCryptographicHash::Md5).toHex().toLower();
    const QByteArray cmd = user.toUtf8() + ' ' + challenge;

    if (Q_UNLIKELY(!sendCommand(cmd.toBase64()))) {
        disconnectOnError(ImapError{ImapError::ResponseError, m_c->translate("SkaffariIMAP", "Failed to send challenge response for CRAM-MD5 to the IMAP server: %1").arg(errorString())});
        return false;
    }

    if (Q_UNLIKELY(!waitForResponse(true))) {
        return false;
    }

    const ImapResult r = checkResponse(readAll(), tag);
    if (!r) {
        disconnectOnError(r.error());
        return false;
    }

    return true;
}

void Imap::sendId()
{
    if (!m_loggedIn) {
        return;
    }

    const QString tag = getTag();
    QString os = QSysInfo::productType();
    QString osVersion = QSysInfo::productVersion();
    if (os == QLatin1String("unknown")) {
        os = QSysInfo::kernelType();
        osVersion = QSysInfo::kernelVersion();
    } else {
        os = QSysInfo::prettyProductName();
    }

    const QString cmd = QStringLiteral("ID (\"name\" \"%1\" \"version\" \"%2\" \"os\" \"%3\" \"os-version\" \"%4\")").arg(QCoreApplication::applicationName(), QCoreApplication::applicationVersion(), os, osVersion);

    if (Q_UNLIKELY(!sendCommand(tag, cmd))) {
        return;
    }

    if (Q_UNLIKELY(!waitForResponse())) {
        return;
    }

    const ImapResult r = checkResponse(readAll(), tag);

    if (r.lines().empty()) {
        return;
    }

    const QString &line = r.lines().first();

    Q_UNUSED(line)
}

#include "moc_imap.cpp"
