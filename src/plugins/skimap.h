/*
 * Skaffari - a mail account administration web interface based on Cutelyst
 * Copyright (C) 2017-2019 Matthias Fehring <mf@huessenbergnetz.de>
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

#ifndef SKIMAP_H
#define SKIMAP_H

#include <Cutelyst/Plugin>

#include <QLoggingCategory>

#include <KIMAP2/LoginJob>

#include "../../common/global.h"

Q_DECLARE_LOGGING_CATEGORY(SK_IMAP2)

namespace KIMAP2 {
    class Session;
}

class SkImap : public Cutelyst::Plugin
{
    Q_OBJECT
    Q_DISABLE_COPY(SkImap)
public:
    enum SpecialUse : uint8_t {
        None    = 0,
        All     = 1,
        Archive = 2,
        Drafts  = 3,
        Flagged = 4,
        Junk    = 5,
        Sent    = 6,
        Trash   = 7
    };

    explicit SkImap(const QString &hostName, quint16 port, Cutelyst::Application *parent);

    ~SkImap() override;

    bool setup(Cutelyst::Application *app) override;

    QString userName() const;
    void setUserName(const QString &userName);

    QString password() const;
    void setPassword(const QString &password);

    void setAuthenticationMode(KIMAP2::LoginJob::AuthenticationMode mode);

    void setEncryptionMode(QSsl::SslProtocol mode, bool startTls = false);

    void setUnixhierarchySeperator(bool unixhierarchySeperator);

    bool hasCapability(const QString &capability);

    static bool isConnected();

    static bool login();

    static bool login(const QString &userName, const QString &password);

    static bool logout();

    static QStringList getCapabilities(bool reload = false);

    static bool checkCapability(const QString &capability, bool reload = false);

    static quota_pair getQuota(const QString &user, const QByteArray &resource = QByteArrayLiteral("STORAGE"), bool *ok = nullptr);

    static bool setQuota(const QString &user, qint64 limit, const QByteArray &resource = QByteArrayLiteral("STORAGE"));

    static bool createMailbox(const QString &user);

    static bool createFolder(const QString &user, const QString &folder, SpecialUse specialUse = None);

    static bool setAcl(const QString &mailbox, const QString &user, const QString &acl = QStringLiteral("lrswipkxtecda"));

    static bool setMetadata(const QString &mailbox, const QString &name, const QString &value = QStringLiteral("NIL"));

//    static QString toUtf7Imap(const QString &str);

//    static QString fromUtf7Imap(const QByteArray &ba);

private:
    QString buildHierarchy(const QStringList &parts) const;
//    QByteArray buildUserHierarchy(const QStringList &parts) const;
    QString buildUserHierarchy(const QString &user) const;

    KIMAP2::Session *m_session = nullptr;
    QStringList m_capabilities;
    QString m_userName;
    QString m_password;
    KIMAP2::LoginJob::AuthenticationMode m_authenticationMode = KIMAP2::LoginJob::ClearText;
    QSsl::SslProtocol m_encryptionMode = QSsl::UnknownProtocol;
    QChar m_hierarchySeperator = QLatin1Char('.');
    bool m_startTls = false;
};

#endif // SKIMAP_H
