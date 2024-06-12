/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef SKAFFARI_IMAP_H
#define SKAFFARI_IMAP_H

#include <QSslSocket>

#include "imap/imapresult.h"

namespace Cutelyst {
class Context;
}

class Imap : public QSslSocket
{
    Q_OBJECT
public:
    enum EncryptionType : int {
        Unsecured = 0,
        StartTLS = 1,
        IMAPS = 2
    };
    Q_ENUM(EncryptionType)

    explicit Imap(Cutelyst::Context *, QObject *parent = nullptr);

    ~Imap() override = default;

    [[nodiscard]] ImapError lastError() const noexcept;

    [[nodiscard]] bool login(const QString &user, const QString &password);

    void logout();

    [[nodiscard]] QStringList getCapabilities(bool reload = false);

    void getNamespaces();

    [[nodiscard]] bool hasCapability(const QString &capability, bool reload = false);

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

    void sendId();

    ImapError m_lastError;
    std::pair<QString,QChar> personalNamespace;
    std::pair<QString,QChar> usersNamespace;
    std::pair<QString,QChar> sharedNamespace;
    Cutelyst::Context *m_c{nullptr};
    quint32 m_tagSequence{0};
    QStringList m_capabilites;
    QChar delimeter;
    bool m_loggedIn{false};
    bool m_namespaceQueried{false};
};

#endif // SKAFFARI_IMAP_H
