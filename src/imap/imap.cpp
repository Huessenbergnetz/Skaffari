/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "imap.h"
#include "imapparser.h"
#include "skaffariimap.h"
#include "../utils/skaffariconfig.h"

#include <Cutelyst/Context>

#include <QLoggingCategory>
#include <QMessageAuthenticationCode>

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

    qCDebug(SK_IMAP) << "Start login to IMAP server" << SkaffariConfig::imapHost() << "on port"
                     << SkaffariConfig::imapPort() << "as user" << user;

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

    ImapResponse r = checkResponse(readAll());

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

        qCDebug(SK_IMAP) << "User" << user << "successfully logged in using LOGIN";
    }

    m_loggedIn = true;

    if (hasCapability(QStringLiteral("ID"), true)) {
        sendId();
    }

    if (hasCapability(QStringLiteral("NAMESPACE"))) {
        getNamespaces();
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
        m_lastError.clear();
        return;
    }

    const ImapResponse r = checkResponse(readAll(), tag);

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

        const QString tag = getTag();

        if (Q_UNLIKELY(!sendCommand(tag, QStringLiteral("CAPABILITY")))) {
            m_lastError.clear();
            return m_capabilites;
        }

        if (Q_UNLIKELY(!waitForResponse())) {
            m_lastError.clear();
            return m_capabilites;
        }

        ImapResponse r = checkResponse(readAll(), tag);
        if (!r) {
            return m_capabilites;
        }

        if (r.lines().empty()) {
            return m_capabilites;
        }

        m_capabilites.clear();

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

        qCDebug(SK_IMAP) << "Requested capabilities:" << m_capabilites;
    }

    return m_capabilites;
}

QString Imap::getDelimeter(NamespaceType nsType)
{
    const QList<std::pair<QString,QString>> nsList = getNamespace(nsType);
    if (!nsList.empty()) {
        const std::pair<QString,QString> ns = nsList.constFirst();
        if (!ns.second.isEmpty()) {
            return ns.second;
        }
    }

    if (!m_delimeter.isEmpty()) {
        return m_delimeter;
    }

    const QString tag = getTag();
    const QString defaultDelimeter = SkaffariConfig::imapUnixhierarchysep() ? QStringLiteral("/") : QStringLiteral(".");

    if (Q_UNLIKELY(!sendCommand(tag, QStringLiteral(R"(LIST "" "")")))) {
        m_lastError.clear();
        qCWarning(SK_IMAP) << "Failed to get delimeter character, using config default:" << defaultDelimeter;
        return defaultDelimeter;
    }

    if (Q_UNLIKELY(!waitForResponse())) {
        m_lastError.clear();
        qCWarning(SK_IMAP) << "Failed to get delimeter character, using config default:" << defaultDelimeter;
        return defaultDelimeter;
    }

    const ImapResponse r = checkResponse(readAll(), tag);

    if (!r) {
        qCWarning(SK_IMAP) << "Failed to get delimeter character, using config default:" << defaultDelimeter;
        return defaultDelimeter;
    }

    if (Q_UNLIKELY(r.lines().empty())) {
        qCWarning(SK_IMAP) << "Failed to get delimeter character, using config default:" << defaultDelimeter;
        return defaultDelimeter;
    }

    QString line = r.lines().constFirst();
    line.remove(QLatin1String("LIST "));
    if (Q_UNLIKELY(line.isEmpty())) {
        qCWarning(SK_IMAP) << "Failed to get delimeter character, using config default:" << defaultDelimeter;
        return defaultDelimeter;
    }

    ImapParser parser;
    const QVariantList parsed = parser.parse(line);
    if (parsed.size() != 3) {
        qCWarning(SK_IMAP) << "Failed to get delimeter character, using config default:" << defaultDelimeter;
        return defaultDelimeter;
    }

    m_delimeter = parsed.at(1).toString();
    if (m_delimeter.isEmpty()) {
        qCWarning(SK_IMAP) << "Failed to get delimeter character, using config default:" << defaultDelimeter;
        return defaultDelimeter;
    }

    return m_delimeter;
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

ImapResponse Imap::checkResponse(const QByteArray &data, const QString &tag)
{
    if (Q_UNLIKELY(data.isEmpty())) {
        qCWarning(SK_IMAP) << "The IMAP response is undefined.";
        return {ImapResponse::Undefined, ImapError{ImapError::UndefinedResponse, m_c->translate("SkaffariIMAP", "The IMAP response is undefined.")}};
    }

    const QList<QByteArray> rawLines = data.split('\n');
    if (Q_UNLIKELY(rawLines.empty())) {
        qCWarning(SK_IMAP) << "The IMAP response is undefined.";
        return {ImapResponse::Undefined, ImapError{ImapError::UndefinedResponse, m_c->translate("SkaffariIMAP", "The IMAP response is undefined.")}};
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
        return {ImapResponse::Undefined, ImapError{ImapError::UndefinedResponse, m_c->translate("SkaffariIMAP", "The IMAP response is undefined.")}};
    }

    if (statusLine.startsWith(QLatin1String("OK"), Qt::CaseInsensitive)) {
        return {ImapResponse::OK, statusLine.mid(3), lines};
    } else if (statusLine.startsWith(QLatin1String("BAD"), Qt::CaseInsensitive)) {
        qCCritical(SK_IMAP) << "We received a BAD response from the IMAP server:" << statusLine.mid(4);
        return {ImapResponse::BAD, ImapError{ImapError::BadResponse, m_c->translate("SkaffariIMAP", "We received a BAD response from the IMAP server: %1").arg(statusLine.mid(4))}};
    } else if (statusLine.startsWith(QLatin1String("NO"), Qt::CaseInsensitive)) {
        qCCritical(SK_IMAP) << "We received a NO response from the IMAP server:" << statusLine.mid(3);
        return {ImapResponse::NO, ImapError{ImapError::NoResponse, m_c->translate("SkaffariIMAP", "We received a NO response from the IMAP server: %1").arg(statusLine.mid(3))}};
    } else {
        qCCritical(SK_IMAP) << "The IMAP response is undefined";
        return {ImapResponse::Undefined, ImapError{ImapError::UndefinedResponse, m_c->translate("SkaffariIMAP", "The IMAP response is undefined.")}};
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

    const ImapResponse r = checkResponse(readAll(), tag);
    if (!r) {
        disconnectOnError(r.error());
        return false;
    }

    qCDebug(SK_IMAP) << "User" << user << "successfully logged in using AUTH=LOGIN";

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

    const ImapResponse r = checkResponse(readAll(), tag);
    if (!r) {
        disconnectOnError(r.error());
        return false;
    }

    qCDebug(SK_IMAP) << "User" << user << "successfully logged in using AUTH=PLAIN";

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

    const ImapResponse r = checkResponse(readAll(), tag);
    if (!r) {
        disconnectOnError(r.error());
        return false;
    }

    qCDebug(SK_IMAP) << "User" << user << "successfully logged in using AUTH=CRAM-MD5";

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
        m_lastError.clear();
        return;
    }

    if (Q_UNLIKELY(!waitForResponse())) {
        m_lastError.clear();
        return;
    }

    const ImapResponse r = checkResponse(readAll(), tag);

    if (r.lines().empty()) {
        return;
    }

    ImapParser parser;
    const QVariantList responseList = parser.parse(r.lines().constFirst());
    if (responseList.empty()) {
        qCWarning(SK_IMAP) << "Failed to parse ID response";
        return;
    }
    const QVariantList idList = responseList.constFirst().toList();
    if (idList.empty()) {
        qCWarning(SK_IMAP) << "Failed to parse ID response";
        return;
    }

    if (idList.size() %2 != 0) {
        qCWarning(SK_IMAP) << "ID response has an odd size";
        return;
    }

    m_serverId.clear();

    int i = 0;
    const int size = idList.size();
    while(i < size) {
        const auto key = idList.at(i).toString();
        ++i;
        const auto value = idList.at(i).toString();
        ++i;
        m_serverId.insert(key, value);
    }

    qDebug(SK_IMAP) << "IMAP Server ID:" << m_serverId;
}

QList<std::pair<QString,QString>> Imap::getNamespace(Imap::NamespaceType type)
{
    if (!m_namespaceQueried) {
        getNamespaces();
    }

    return m_namespaces.value(static_cast<int>(type));
}

void Imap::getNamespaces()
{
    m_lastError.clear();

    const QString tag = getTag();

    if (Q_UNLIKELY(!sendCommand(tag, QStringLiteral("NAMESPACE")))) {
        return;
    }

    if (Q_UNLIKELY(!waitForResponse())) {
        return;
    }

    const ImapResponse r = checkResponse(readAll(), tag);
    if (!r) {
        qCWarning(SK_IMAP) << "Failed to request namespaces from the IMAP server";
        return;
    }

    if (r.lines().empty()) {
        qCWarning(SK_IMAP) << "Failed to request namespaces from the IMAP server: empty response";
        return;
    }

    QString nsLine = r.lines().constFirst();
    if (Q_UNLIKELY(!nsLine.startsWith(QLatin1String("NAMESPACE"), Qt::CaseInsensitive))) {
        qCWarning(SK_IMAP) << "Failed to request namespaces from the IMAP server: invalid response";
        return;
    }

    nsLine.remove(QLatin1String("NAMESPACE "), Qt::CaseInsensitive);

    ImapParser parser;
    const QVariantList nsList = parser.parse(nsLine);
    if (nsList.size() != 3) {
        qCWarning(SK_IMAP) << "Failed to request namespaces from the IMAP server: invalid response";
        return;
    }

    QList<QList<std::pair<QString,QString>>> namespaces;

    for (const QVariant &v : nsList) {
        if (v.type() == QMetaType::QString) {
            const auto str = v.toString();
            if (str.isEmpty()) {
                namespaces << QList<std::pair<QString,QString>>();
            } else {
                // single string can only be NIL, if it is
                // not parsed as empty string, the response is invalid
                // or the parser is shit...
                qCWarning(SK_IMAP) << "Failed to request namespaces from the IMAP server: invalid response, failed to parse";
                return;
            }
        } else if (v.type() == QMetaType::QVariantList) {
            const auto lst = v.toList();
            QList<std::pair<QString,QString>> tmpNs;
            for (const auto &li : lst) {
                const QVariantList tmp = parser.parse(li.toString());
                for (const auto &t : tmp) {
                    const QVariantList lst2 = t.toList();
                    tmpNs << std::make_pair(lst2.at(0).toString(), lst2.at(1).toString());
                }
            }
            namespaces << tmpNs;
        }
    }

    if (namespaces.size() != 3) {
        qCWarning(SK_IMAP) << "Failed to request namespaces from the IMAP server: invalid response, failed to parse";
        return;
    }

    m_namespaceQueried = true;

    m_namespaces = namespaces;
}

#include "moc_imap.cpp"
