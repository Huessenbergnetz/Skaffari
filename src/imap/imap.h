/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef SKAFFARI_IMAP_H
#define SKAFFARI_IMAP_H

#include <QSslSocket>
#include <QLoggingCategory>

#include "imap/imapresponse.h"
#include "../../common/global.h"

Q_DECLARE_LOGGING_CATEGORY(SK_IMAP)

namespace Cutelyst {
class Context;
}

class Imap : public QSslSocket
{
    Q_OBJECT
public:
    using NsData = std::pair<QString,QString>;
    using NsList = QList<NsData>;

    enum EncryptionType : int {
        Unsecured = 0,
        StartTLS = 1,
        IMAPS = 2
    };
    Q_ENUM(EncryptionType)

    enum class NamespaceType : int {
        Personal = 0,
        Others = 1,
        Shared = 2
    };

    enum class SpecialUse : int {
        None = 0,
        All = 1,
        Archive = 2,
        Drafts = 3,
        Flagged = 4,
        Junk = 5,
        Sent = 6,
        Trash = 7,
        Other = 255
    };
    Q_ENUM(SpecialUse);

    explicit Imap(Cutelyst::Context *, QObject *parent = nullptr);

    ~Imap() override = default;

    [[nodiscard]] ImapError lastError() const noexcept;

    [[nodiscard]] bool login(const QString &user, const QString &password);

    [[nodiscard]] bool login();

    void logout();

    [[nodiscard]] bool isLoggedIn() const;

    [[nodiscard]] bool createFolder(const QString &user, const QString &folder, SpecialUse specialUse = SpecialUse::None);

    [[nodiscard]] bool createFolder(const QString &folder, SpecialUse specialUse = SpecialUse::None);

    [[nodiscard]] bool createMailbox(const QString &user);

    [[nodiscard]] bool deleteMailbox(const QString &user);

    [[nodiscard]] QStringList getCapabilities(bool reload = false);

    [[nodiscard]] QString getDelimeter(NamespaceType nsType);

    [[nodiscard]] QStringList getMailboxes();

    NsList getNamespace(NamespaceType type);

    [[nodiscard]] quota_pair getQuota(const QString &user);

    [[nodiscard]] bool hasCapability(const QString &capability, bool reload = false);

    [[nodiscard]] bool setAcl(const QString &mailbox, const QString &user, const QString &acl = QStringLiteral("lrswipkxtecda"));

    [[nodiscard]] bool deleteAcl(const QString &mailbox, const QString &user);

    [[nodiscard]] bool setQuota(const QString &user, quota_size_t quota);

    [[nodiscard]] bool setSpecialUse(const QString &folder, SpecialUse specialUse);

    [[nodiscard]] bool subscribeFolder(const QString &folder = {});

    static QString toUtf7Imap(const QString &str);

    static QString fromUtf7Imap(const QString &str);

private:
    QString getTag();

    bool sendCommand(const QString &command);

    bool sendCommand(const QString &tag, const QString &command);

    bool sendCommand(const QByteArray &command);

    bool sendCommand(const QByteArray &tag, const QByteArray &command);

    void disconnectOnError(const ImapError &error = {});

    void connectionTimedOut();

    bool waitForResponse(bool disCon = false, const QString &errorString = {}, int msecs = 30'000);

    ImapResponse checkResponse(const QByteArray &data, const QString &tag = {});

    bool authLogin(const QString &user, const QString &password);

    bool authPlain(const QString &user, const QString &password);

    bool authCramMd5(const QString &user, const QString &password);

    void getNamespaces();

    [[nodiscard]] QString getUserMailboxName(const QStringList &folders, bool quoted = true);

    [[nodiscard]] QString getInboxFolder(const QStringList &folders, bool quoted = true);

    [[nodiscard]] QList<std::pair<int,QString>> getUserFolders(const QString &user);

    void sendId();

    ImapError m_lastError;
    QList<NsList> m_namespaces;
    QMap<QString,QString> m_serverId;
    Cutelyst::Context *m_c{nullptr};
    quint32 m_tagSequence{0};
    QStringList m_capabilites;
    QString m_delimeter;
    bool m_loggedIn{false};
    bool m_namespaceQueried{false};
};

#endif // SKAFFARI_IMAP_H
