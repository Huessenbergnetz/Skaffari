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

#include <unicode/ucnv_err.h>
#include <unicode/uenum.h>
#include <unicode/localpointer.h>
#include <unicode/ucnv.h>

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
        setPeerVerifyName(SkaffariConfig::imapPeername());
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

        setPeerVerifyName(SkaffariConfig::imapPeername());

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

bool Imap::login()
{
    return login(SkaffariConfig::imapUser(), SkaffariConfig::imapPassword());
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

QString getCreateFolderSpecialUse(Imap::SpecialUse specialUse)
{
    switch (specialUse) {
    case Imap::SpecialUse::Archive:
        return QStringLiteral(R"( (USE (\Archive)))");
    case Imap::SpecialUse::Drafts:
        return QStringLiteral(R"( (USE (\Drafts)))");
    case Imap::SpecialUse::Junk:
        return QStringLiteral(R"( (USE (\Junk)))");
    case Imap::SpecialUse::Sent:
        return QStringLiteral(R"( (USE (\Sent)))");
    case Imap::SpecialUse::Trash:
        return QStringLiteral(R"( (USE (\Trash)))");
    default:
        return {};
    }
}

bool Imap::createFolder(const QString &user, const QString &folder, SpecialUse specialUse)
{
    Q_ASSERT_X(!folder.isEmpty(), "create folder", "empty folder name");
    Q_ASSERT_X(!user.isEmpty(), "create folder", "empty user name");

    m_lastError.clear();

    const QString _folder = Imap::toUtf7Imap(folder);

    if (Q_UNLIKELY(!_folder.isEmpty())) {
        m_lastError = ImapError{ImapError::InternalError, m_c->translate("SkaffariIMAP", "Failed to convert folder name into UTF-7-IMAP.")};
        return false;
    }

    const QString tag = getTag();
    const QString delimeter = getDelimeter(NamespaceType::Others);
    QString cmd = QLatin1String(R"(CREATE ")") + getUserMailboxName(user, false) + delimeter + _folder + QLatin1Char('"');

    if (hasCapability(QStringLiteral("CREATE-SPECIAL-USE"))) {
        cmd += getCreateFolderSpecialUse(specialUse);
    }

    if (Q_UNLIKELY(!sendCommand(tag, cmd))) {
        return false;
    }

    if (Q_UNLIKELY(!waitForResponse(true))) {
        return false;
    }

    const ImapResponse r = checkResponse(readAll(), tag);

    if (!r) {
        m_lastError = r.error();
        return false;
    }

    return true;
}

bool Imap::createFolder(const QString &folder, Imap::SpecialUse specialUse)
{
    Q_ASSERT_X(!folder.isEmpty(), "create folder", "empty folder name");

    const QString _folder = Imap::toUtf7Imap(folder);

    if (Q_UNLIKELY(!_folder.isEmpty())) {
        m_lastError = ImapError{ImapError::InternalError, m_c->translate("SkaffariIMAP", "Failed to convert folder name into UTF-7-IMAP.")};
        return false;
    }

    const QString tag = getTag();
    const NsList nsList = getNamespace(NamespaceType::Personal);
    QString nsname;
    if (!nsList.empty()) {
        if (const NsData ns = nsList.constFirst(); !ns.first.isNull()) {
            nsname = ns.first;
        }
    }
    if (nsname.isNull()) {
        nsname = QStringLiteral("INBOX.");
    }

    QString cmd = QLatin1String(R"(CREATE ")") + nsname + _folder + QLatin1Char('"');

    if (hasCapability(QStringLiteral("CREATE-SPECIAL-USE"))) {
        cmd += getCreateFolderSpecialUse(specialUse);
    }

    if (Q_UNLIKELY(!sendCommand(tag, cmd))) {
        return false;
    }

    if (Q_UNLIKELY(!waitForResponse(true))) {
        return false;
    }

    const ImapResponse r = checkResponse(readAll(), tag);

    if (!r) {
        m_lastError = r.error();
        return false;
    }

    return true;
}

bool Imap::createMailbox(const QString &user)
{
    m_lastError.clear();

    const QString tag = getTag();
    const QString cmd = QLatin1String("CREATE ") + getUserMailboxName(user);

    if (Q_UNLIKELY(!sendCommand(tag, cmd))) {
        return false;
    }

    if (Q_UNLIKELY(!waitForResponse(true))) {
        return false;
    }

    const ImapResponse r = checkResponse(readAll(), tag);
    if (!r) {
        m_lastError = r.error();
        return false;
    }

    return true;
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
    const NsList nsList = getNamespace(nsType);
    if (!nsList.empty()) {
        const NsData ns = nsList.constFirst();
        if (!ns.second.isNull()) {
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

QStringList Imap::getMailboxes()
{
    m_lastError.clear();

    QString nsName;
    QString delimeter;
    const NsList nsList = getNamespace(NamespaceType::Others);
    if (nsList.isEmpty() || nsList.constFirst().first.isNull()) {
        delimeter = getDelimeter(NamespaceType::Others);
        nsName = QStringLiteral("user") + delimeter;
    } else {
        nsName = nsList.constFirst().first;
        delimeter = nsList.constFirst().second;
    }

    const QString tag = getTag();
    const QString cmd = QLatin1String(R"(LIST ")") + nsName + QLatin1String(R"(" %)");

    if (Q_UNLIKELY(!sendCommand(tag, cmd))) {
        return {};
    }

    if (Q_UNLIKELY(!waitForResponse(true))) {
        return {};
    }

    const ImapResponse r = checkResponse(readAll(), tag);

    if (!r) {
        m_lastError = r.error();
        return {};
    }

    const QStringList lines = r.lines();
    if (lines.empty()) {
        m_lastError = ImapError{ImapError::ResponseError, m_c->translate("SkaffariIMAP", "Failed to get mailboxes from IMAP server: empty response")};
        return {};
    }

    ImapParser parser;
    QStringList lst;
    for (const QString &l : lines) {
        QString _l = l;
        _l.remove(QLatin1String("LIST "));
        const QVariantList parsed = parser.parse(_l);
        if (parsed.size() != 3) {
            return {};
        }
        QString mb = parsed.at(2).toString();
        mb.remove(nsName);
        lst << mb;
    }

    return lst;
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

bool Imap::setQuota(const QString &user, quota_size_t quota)
{
    m_lastError.clear();

    const QString tag = getTag();
    const QString cmd = QLatin1String("SETQUOTA ") + getUserMailboxName(user) + QLatin1String("(STORAGE ") + QString::number(quota) + QLatin1Char(')');

    if (Q_UNLIKELY(!sendCommand(tag, cmd))) {
        return false;
    }

    if (Q_UNLIKELY(!waitForResponse(true))) {
        return false;
    }

    const ImapResponse r = checkResponse(readAll(), tag);

    if (!r) {
        m_lastError = r.error();
        return false;
    }

    return true;
}

bool Imap::setSpecialUse(const QString &folder, Imap::SpecialUse specialUse)
{
    Q_ASSERT_X(!folder.isEmpty(), "set special use", "empty folder name");

    m_lastError.clear();

    const QString _folder = Imap::toUtf7Imap(folder);
    if (Q_UNLIKELY(!_folder.isEmpty())) {
        m_lastError = ImapError{ImapError::InternalError, m_c->translate("SkaffariIMAP", "Failed to convert folder name into UTF-7-IMAP.")};
        return false;
    }

    QString cmd = QLatin1String("SETMETADATA ") + getInboxFolder({folder}) + QLatin1String(" (/private/specialuse ");

    switch(specialUse) {
    case SpecialUse::Archive:
        cmd += QLatin1String(R"("\\Archive")");
        break;
    case SpecialUse::Drafts:
        cmd += QLatin1String(R"("\\Drafts")");
        break;
    case SpecialUse::Junk:
        cmd += QLatin1String(R"("\\Junk")");
        break;
    case SpecialUse::Sent:
        cmd += QLatin1String(R"("\\Sent")");
        break;
    case SpecialUse::Trash:
        cmd += QLatin1String(R"("\\Trash")");
        break;
    case SpecialUse::None:
        cmd += QLatin1String("NIL");
        break;
    default:
        Q_ASSERT_X(false, "set special use", "invalid special use type");
        m_lastError = ImapError{ImapError::InternalError, m_c->translate("SkaffariIMAP", "Invalid special use type.")};
        return false;
    }

    cmd += QLatin1Char(')');
    const QString tag = getTag();

    if (Q_UNLIKELY(!sendCommand(tag, cmd))) {
        return false;
    }

    if (Q_UNLIKELY(!waitForResponse(true))) {
        return false;
    }

    const ImapResponse r = checkResponse(readAll(), tag);

    if (!r) {
        m_lastError = r.error();
        return false;
    }

    return true;
}

bool Imap::subscribeFolder(const QString &folder)
{
    Q_ASSERT_X(!folder.isEmpty(), "set special use", "empty folder name");

    m_lastError.clear();

    QString _folder;
    if (!folder.isEmpty()) {
        _folder = Imap::toUtf7Imap(folder);
        if (Q_UNLIKELY(!_folder.isEmpty())) {
            m_lastError = ImapError{ImapError::InternalError, m_c->translate("SkaffariIMAP", "Failed to convert folder name into UTF-7-IMAP.")};
            return false;
        }
    }

    if (_folder.isEmpty()) {
        _folder = QStringLiteral("INBOX");
    } else {
        _folder = getInboxFolder({_folder}, false);
    }

    const QString tag = getTag();
    const QString cmd = QLatin1String(R"(SUBSCRIBE ")") + _folder + QLatin1Char('"');

    if (Q_UNLIKELY(!sendCommand(tag, cmd))) {
        return false;
    }

    if (Q_UNLIKELY(!waitForResponse(true))) {
        return false;
    }

    const ImapResponse r = checkResponse(readAll(), tag);

    if (!r) {
        m_lastError = r.error();
        return false;
    }

    return true;
}

QString Imap::toUtf7Imap(const QString &str)
{
    if (str.isEmpty()) {
        return str;
    }

    const QByteArray utf8 = str.toUtf8();

    const int32_t bufSize = utf8.size() * 3;
    auto buf = static_cast<char*>(malloc(sizeof(char) * bufSize));

    if (Q_UNLIKELY(!buf)) {
        qCCritical(SK_IMAP) << "Failed to convert UTF-8 string" << str << "to UTF7-IMAP (RFC2060 5.1.3): failed to allocate buffer of size" << bufSize;
        return {};
    }

    UErrorCode uec = U_ZERO_ERROR;

    const int32_t size = ucnv_convert("imap-mailbox-name", "utf-8", buf, bufSize, utf8.constData(), utf8.size(), &uec);

    QString utf7;
    if ((size > 0) && (uec == U_ZERO_ERROR)) {
        utf7 = QString::fromLatin1(buf, size);
    } else {
        qCCritical(SK_IMAP) << "Failed to convert UTF-8 string" << str << "to UTF7-IMAP (RFC2060 5.1.3):" << u_errorName(uec);
    }

    free(buf);

    return utf7;
}

QString Imap::fromUtf7Imap(const QString &str)
{
    if (str.isEmpty()) {
        return str;
    }

    const QByteArray utf7 = str.toLatin1();

    const int32_t bufSize = utf7.size() + 1;
    auto buf = static_cast<char*>(malloc(sizeof(char) * bufSize));

    if (Q_UNLIKELY(!buf)) {
        qCCritical(SK_IMAP) << "Failed to convert UTF7-IMAP (RFC2060 5.1.3) string" << str << "to UTF-8: failed to allocate buffer of size" << bufSize;
        return {};
    }

    UErrorCode uec = U_ZERO_ERROR;

    const int32_t size = ucnv_convert("utf-8", "imap-mailbox-name", buf, bufSize, utf7.constData(), utf7.size(), &uec);

    QString utf8;
    if ((size > 0) && (uec == U_ZERO_ERROR)) {
        utf8 = QString::fromUtf8(buf, size);
    } else {
        qCCritical(SK_IMAP) << "Failed to convert UTF7-IMAP (RFC2060 5.1.3) string" << str << "to UTF-8:" << u_errorName(uec);
    }

    free(buf);

    return utf8;
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

Imap::NsList Imap::getNamespace(Imap::NamespaceType type)
{
    if (!m_namespaceQueried) {
        getNamespaces();
    }

    return m_namespaces.value(static_cast<int>(type));
}

quota_pair Imap::getQuota(const QString &user)
{
    quota_pair quota{0, 0};

    m_lastError.clear();

    const QString tag = getTag();
    const QString cmd = QLatin1String("GETQUOTA ") + getUserMailboxName(user);

    if (Q_UNLIKELY(!sendCommand(tag, cmd))) {
        return quota;
    }

    if (Q_UNLIKELY(!waitForResponse(true))) {
        return quota;
    }

    const ImapResponse r = checkResponse(readAll(), tag);
    if (!r) {
        disconnectOnError(r.error());
        return quota;
    }

    if (Q_UNLIKELY(r.lines().empty())) {
        qCCritical(SK_IMAP) << "Failed to request storage quota for user" << user << ": invalid response";
        m_lastError = ImapError{ImapError::ResponseError, m_c->translate("SkaffariIMAP", "Failed to request storage quota for user %1: invalid response").arg(user)};
        return quota;
    }

    QString line = r.lines().constFirst();
    line.remove(QLatin1String("QUOTA "));

    ImapParser parser;
    const QVariantList parsed = parser.parse(line);
    if (Q_UNLIKELY(parsed.size() < 2)) {
        qCCritical(SK_IMAP) << "Failed to request storage quota for user" << user << ": invalid response";
        m_lastError = ImapError{ImapError::ResponseError, m_c->translate("SkaffariIMAP", "Failed to request storage quota for user %1: invalid response").arg(user)};
        return quota;
    }

    const QVariantList quotaLst = parsed.at(1).toList();
    if (quotaLst.empty() || quotaLst.size() % 3 != 0) {
        qCCritical(SK_IMAP) << "Failed to request storage quota for user" << user << ": invalid response";
        m_lastError = ImapError{ImapError::ResponseError, m_c->translate("SkaffariIMAP", "Failed to request storage quota for user %1: invalid response").arg(user)};
        return quota;
    }

    for (int i = 0; i < quotaLst.size(); ++i) {
        if (quotaLst.at(i).toString().compare(QLatin1String("STORAGE"), Qt::CaseInsensitive) == 0) {
            if ((i + 2) < quotaLst.size()) {
                bool sOk = false;
                bool qOk = false;
                const auto s = quotaLst.at(i + 1).toString().toULongLong(&sOk);
                const auto q = quotaLst.at(i + 2).toString().toULongLong(&qOk);
                if (!sOk || !qOk) {
                    qCCritical(SK_IMAP) << "Failed to request storage quota for user" << user << ": invalid response";
                    m_lastError = ImapError{ImapError::ResponseError, m_c->translate("SkaffariIMAP", "Failed to request storage quota for user %1: invalid response").arg(user)};
                    return quota;
                }
                quota.first = s;
                quota.second = q;
                return quota;

            } else {
                qCCritical(SK_IMAP) << "Failed to request storage quota for user" << user << ": invalid response";
                m_lastError = ImapError{ImapError::ResponseError, m_c->translate("SkaffariIMAP", "Failed to request storage quota for user %1: invalid response").arg(user)};
                return quota;
            }
        }
    }

    qCCritical(SK_IMAP) << "Failed to request storage quota for user" << user << ": invalid response";
    m_lastError = ImapError{ImapError::ResponseError, m_c->translate("SkaffariIMAP", "Failed to request storage quota for user %1: invalid response").arg(user)};
    return quota;
}

void Imap::getNamespaces()
{
    m_lastError.clear();

    const QString tag = getTag();

    if (Q_UNLIKELY(!sendCommand(tag, QStringLiteral("NAMESPACE")))) {
        m_lastError.clear();
        return;
    }

    if (Q_UNLIKELY(!waitForResponse())) {
        m_lastError.clear();
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

    QList<NsList> namespaces;

    for (const QVariant &v : nsList) {
        if (v.type() == QMetaType::QString) {
            const auto str = v.toString();
            if (str.isEmpty()) {
                namespaces << NsList();
            } else {
                // single string can only be NIL, if it is
                // not parsed as empty string, the response is invalid
                // or the parser is shit...
                qCWarning(SK_IMAP) << "Failed to request namespaces from the IMAP server: invalid response, failed to parse";
                return;
            }
        } else if (v.type() == QMetaType::QVariantList) {
            const auto lst = v.toList();
            NsList tmpNs;
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

QString Imap::getUserMailboxName(const QString &user, bool quoted)
{
    const NsList nsList = getNamespace(Imap::NamespaceType::Others);
    if (!nsList.empty()) {
        if (!nsList.constFirst().first.isNull()) {
            if (quoted) {
                return QLatin1Char('"') + nsList.constFirst().first + user + QLatin1Char('"');
            } else {
                return nsList.constFirst().first + user;
            }
        }
    }
    const QString delimeter = getDelimeter(Imap::NamespaceType::Others);
    if (quoted) {
        return QLatin1String(R"("user)") + delimeter + user + QLatin1Char('"');
    } else {
        return QLatin1String("user") + delimeter + user;
    }
}

QString Imap::getInboxFolder(const QStringList &folders, bool quoted)
{
    const NsList nsList = getNamespace(Imap::NamespaceType::Personal);
    if (!nsList.empty()) {
        if (const NsData &ns = nsList.constFirst(); ns.first.isNull()) {
            if (quoted) {
                return QLatin1Char('"') + ns.first + folders.join(ns.second) + QLatin1Char('"');
            } else {
                return ns.first + folders.join(ns.second);
            }
        }
    }
    const QString delimeter = getDelimeter(Imap::NamespaceType::Personal);
    if (quoted) {
        return QLatin1String(R"("INBOX)") + delimeter + folders.join(delimeter) + QLatin1Char('"');
    } else {
        return QLatin1String(R"(INBOX)") + delimeter + folders.join(delimeter);
    }
}

#include "moc_imap.cpp"
