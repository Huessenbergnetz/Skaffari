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

#include "skimap.h"

#include <Cutelyst/Application>

#include <KIMAP2/RfcCodecs>
#include <KIMAP2/Session>
#include <KIMAP2/LogoutJob>
#include <KIMAP2/IdJob>
#include <KIMAP2/CapabilitiesJob>
#include <KIMAP2/GetQuotaJob>
#include <KIMAP2/SetQuotaJob>
#include <KIMAP2/CreateJob>
#include <KIMAP2/SetMetaDataJob>
#include <KIMAP2/SetAclJob>
#include <KIMAP2/DeleteAclJob>
#include <KIMAP2/DeleteJob>
#include <KIMAP2/Acl>

#include <QCoreApplication>
#include <QSysInfo>

#include <unicode/ucnv_err.h>
#include <unicode/uenum.h>
#include <unicode/localpointer.h>
#include <unicode/ucnv.h>

Q_LOGGING_CATEGORY(SK_IMAP2, "skaffari.imap")

static thread_local SkImap *ski = nullptr;

SkImap::SkImap(const QString &hostName, quint16 port, Cutelyst::Application *parent)
    : Cutelyst::Plugin(parent), m_session(new KIMAP2::Session(hostName, port, this))
{

}

SkImap::~SkImap()
{

}

bool SkImap::setup(Cutelyst::Application *app)
{
    connect(app, &Cutelyst::Application::postForked, this, [=] () {
        ski = this;
    });

    return true;
}

QString SkImap::userName() const
{
    return m_userName;
}

void SkImap::setUserName(const QString &userName)
{
    m_userName = userName;
}

QString SkImap::password() const
{
    return m_password;
}

void SkImap::setPassword(const QString &password)
{
    m_password = password;
}

void SkImap::setAuthenticationMode(KIMAP2::LoginJob::AuthenticationMode mode)
{
    m_authenticationMode = mode;
}

void SkImap::setEncryptionMode(QSsl::SslProtocol mode, bool startTls)
{
    m_encryptionMode = mode;
    m_startTls = startTls;
}

void SkImap::setUnixhierarchySeperator(bool unixhierarchySeperator)
{
    m_hierarchySeperator = unixhierarchySeperator ? QLatin1Char('/') : QLatin1Char('.');
}

bool SkImap::hasCapability(const QString &capability)
{
    const QString _cap = capability.trimmed();
    if (_cap.isEmpty()) {
        qCWarning(SK_IMAP2, "Can not check for an empty capabilits.");
        return false;
    }

    if (m_capabilities.empty()) {
        SkImap::getCapabilities();
    }

    return m_capabilities.contains(capability, Qt::CaseInsensitive);
}

bool SkImap::isConnected()
{
    if (!ski) {
        qCCritical(SK_IMAP2, "%s", "SkImap plugin not registered!");
        return false;
    }

    return ski->m_session->isConnected();
}

bool SkImap::login()
{
    if (!ski) {
        qCCritical(SK_IMAP2, "%s", "SkImap plugin not registered!");
        return false;
    }

    return login(ski->userName(), ski->password());
}

bool SkImap::login(const QString &userName, const QString &password)
{
    if (!ski) {
        qCCritical(SK_IMAP2, "%s", "SkImap plugin not registered!");
        return false;
    }

    KIMAP2::LoginJob *lj = new KIMAP2::LoginJob(ski->m_session);
    lj->setUserName(userName);
    lj->setPassword(password);
    lj->setEncryptionMode(ski->m_encryptionMode, ski->m_startTls);
    lj->setAuthenticationMode(ski->m_authenticationMode);
    if (lj->exec()) {
        if (ski->m_session->isConnected()) {
            KIMAP2::IdJob *ij = new KIMAP2::IdJob(ski->m_session);
            ij->setField(QByteArrayLiteral("name"), QCoreApplication::applicationName().toLatin1());
            ij->setField(QByteArrayLiteral("version"), QCoreApplication::applicationVersion().toLatin1());
            QString os = QSysInfo::productType();
            QString osVersion = QSysInfo::productVersion();
            if (os == QLatin1String("unknown")) {
                os = QSysInfo::kernelType();
                osVersion = QSysInfo::kernelVersion();
            } else {
                os = QSysInfo::prettyProductName();
            }
            ij->setField(QByteArrayLiteral("os"), os.toLatin1());
            ij->setField(QByteArrayLiteral("os-version"), osVersion.toLatin1());
            if (Q_UNLIKELY(!ij->exec())) {
                qCWarning(SK_IMAP2, "%s", "Failed to send client ID to the IMAP server.");
            }

            if (ski->m_capabilities.empty()) {
                KIMAP2::CapabilitiesJob *cj = new KIMAP2::CapabilitiesJob(ski->m_session);
                if (Q_LIKELY(cj->exec())) {
                    ski->m_capabilities = cj->capabilities();
                } else {
                    qCWarning(SK_IMAP2, "%s", "Failed to request capabilities from the IMAP server.");
                }
            }

            return true;
        }
    }

    return false;
}

bool SkImap::logout()
{
    if (!ski) {
        qCCritical(SK_IMAP2, "%s", "SkImap plugin not registered!");
        return false;
    }

    KIMAP2::LogoutJob *lj = new KIMAP2::LogoutJob(ski->m_session);
    return lj->exec();
}

QStringList SkImap::getCapabilities(bool reload)
{
    QStringList caps;

    if (!ski) {
        qCCritical(SK_IMAP2, "%s", "SkImap plugin not registered!");
        return caps;
    }

    if (reload || ski->m_capabilities.empty()) {
        KIMAP2::CapabilitiesJob *cj = new KIMAP2::CapabilitiesJob(ski->m_session);
        if (Q_LIKELY(cj->exec())) {
            ski->m_capabilities = cj->capabilities();
        } else {
            qCCritical(SK_IMAP2, "%s", "Failed to request capabilities from the IMAP server.");
        }
    }

    caps = ski->m_capabilities;

    return caps;
}

quota_pair SkImap::getQuota(const QString &user, const QByteArray &resource, bool *ok)
{
    quota_pair quota(0, 0);

    if (!ski) {
        qCCritical(SK_IMAP2, "%s", "SkImap plugin not registered!");
        if (ok) {
            *ok = false;
        }
        return quota;
    }

    if (user.isEmpty()) {
        qCWarning(SK_IMAP2, "%s", "Can not get quota for an empty user.");
        if (ok) {
            *ok = false;
        }
        return quota;
    }

    KIMAP2::GetQuotaJob *qj = new KIMAP2::GetQuotaJob(ski->m_session);
    qj->setRoot(KIMAP2::encodeImapFolderName(ski->buildUserHierarchy(user).toUtf8()));
    if (Q_LIKELY(qj->exec())) {
        const qint64 usage = qj->usage(resource);
        const qint64 limit = qj->limit(resource);
        if (usage < 0 || limit < 0) {
            qCCritical(SK_IMAP2, "Failed to request %s quota for user %s.", resource.constData(), qUtf8Printable(user));
            if (ok) {
                *ok = false;
            }
        } else {
            quota.first = static_cast<quota_size_t>(usage);
            quota.second = static_cast<quota_size_t>(limit);
        }
    }

    if (ok) {
        *ok = true;
    }

    return quota;
}

bool SkImap::setQuota(const QString &user, qint64 limit, const QByteArray &resource)
{
    if (!ski) {
        qCCritical(SK_IMAP2, "%s", "SkImap plugin not registered!");
        return false;
    }

    if (user.isEmpty()) {
        qCWarning(SK_IMAP2, "%s", "Can not set quota for an empty user.");
        return false;
    }

    KIMAP2::SetQuotaJob *sqj = new KIMAP2::SetQuotaJob(ski->m_session);
    sqj->setRoot(KIMAP2::encodeImapFolderName(ski->buildUserHierarchy(user).toUtf8()));
    sqj->setQuota(resource, limit);
    if (Q_UNLIKELY(!sqj->exec())) {
        qCCritical(SK_IMAP2, "Failed to set %s quota for user %s.", resource.constData(), qUtf8Printable(user));
        return false;
    }

    return true;
}

bool SkImap::createMailbox(const QString &user)
{
    if (!ski) {
        qCCritical(SK_IMAP2, "%s", "SkImap plugin not registered!");
        return false;
    }

    if (user.isEmpty()) {
        qCWarning(SK_IMAP2, "%s", "Can not create a new mailbox for an empty user.");
        return false;
    }

    KIMAP2::CreateJob *cj = new KIMAP2::CreateJob(ski->m_session);
    cj->setMailBox(ski->buildUserHierarchy(user));
    if (Q_UNLIKELY(!cj->exec())) {
        qCCritical(SK_IMAP2, "Failed to create new mailbox for user %s.", qUtf8Printable(user));
        return false;
    }

    return true;
}

bool SkImap::createFolder(const QString &user, const QString &folder, SpecialUse specialUse)
{
    if (!ski) {
        qCCritical(SK_IMAP2, "%s", "SkImap plugin not registered!");
        return false;
    }

    if (user.isEmpty()) {
        qCWarning(SK_IMAP2, "%s", "Can not create a new for for an empty user name.");
        return false;
    }

    if (folder.isEmpty()) {
        qCWarning(SK_IMAP2, "%s", "Can not create ane empty folder name.");
        return false;
    }

    KIMAP2::CreateJob *cj = new KIMAP2::CreateJob(ski->m_session);
    cj->setMailBox(ski->buildHierarchy({QStringLiteral("user"), user, folder}));
    if (Q_UNLIKELY(!cj->exec())) {
        qCCritical(SK_IMAP2, "Failed to create new mailbox folder %s.", qUtf8Printable(folder));
        return false;
    }

    // TODO: when KIMAP2::CreateJob can set the special use flag, use it
    if (specialUse != None) {
        if (ski->hasCapability(QStringLiteral("SPECIAL-USE")) && ski->hasCapability(QStringLiteral("METADATA"))) {

        } else {
            qCWarning(SK_IMAP2, "Can not set SPECIAL USE flag for new folder \"%s\" as the IMAP server does not support SPECIAL-USE or METADATA.", qUtf8Printable(folder));
        }
    }

    return true;
}

bool SkImap::setAcl(const QString &mailbox, const QString &user, const QString &acl)
{
    if (!ski) {
        qCCritical(SK_IMAP2, "%s", "SkImap plugin not registered!");
        return false;
    }

    if (mailbox.isEmpty()) {
        qCWarning(SK_IMAP2, "%s", "Can not set ACL for empty mailbox.");
        return false;
    }

    if (user.isEmpty()) {
        qCWarning(SK_IMAP2, "%s", "Can not set ACL for empty user.");
        return false;
    }

    const QString _acl = acl.trimmed();

    if (_acl.isEmpty()) {
        KIMAP2::DeleteAclJob *daj = new KIMAP2::DeleteAclJob(ski->m_session);
        daj->setMailBox(ski->buildUserHierarchy(mailbox));
        daj->setIdentifier(user.toUtf8());
        return daj->exec();
    } else {
        KIMAP2::SetAclJob *saj = new KIMAP2::SetAclJob(ski->m_session);
        saj->setMailBox(ski->buildUserHierarchy(mailbox));
        saj->setIdentifier(user.toUtf8());
        saj->setRights(KIMAP2::AclJobBase::Change, KIMAP2::Acl::rightsFromString(acl.toLatin1()));
        return saj->exec();
    }
}

bool SkImap::setMetadata(const QString &mailbox, const QString &name, const QString &value)
{
    if (!ski) {
        qCCritical(SK_IMAP2, "%s", "SkImap plugin not registered!");
        return false;
    }

    if (!ski->hasCapability(QStringLiteral("METADATA"))) {
        qCWarning(SK_IMAP2, "%s", "The IMAP server does not support the METADATA extension.");
        return false;
    }

    if (mailbox.isEmpty()) {
        qCWarning(SK_IMAP2, "%s", "Can not set metadata for empty mailbox.");
        return false;
    }

    if (name.isEmpty()) {
        qCWarning(SK_IMAP2, "%s", "Can not set metadata for empty name.");
        return false;
    }

    if (value.isEmpty()) {
        qCWarning(SK_IMAP2, "%s", "Can not set metadata with empty value.");
        return false;
    }

    KIMAP2::SetMetaDataJob *smj = new KIMAP2::SetMetaDataJob(ski->m_session);
    smj->setServerCapability(KIMAP2::MetaDataJobBase::Metadata);
    smj->setMailBox(mailbox);
    smj->addMetaData(name.toUtf8(), value.toUtf8());
    return smj->exec();
}

QString SkImap::buildHierarchy(const QStringList &parts) const
{
    QString str;

    if (!parts.empty()) {
        str = parts.join(m_hierarchySeperator);
//        QStringList utf7Parts;
//        utf7Parts.reserve(parts.size());
//        for (const QString &part : parts) {
////            const QString utf7Part = SkImap::toUtf7Imap(part);
//            const QString utf7Part = KIMAP2::encodeImapFolderName(part);
//            if (utf7Part.isEmpty()) {
//                qCWarning(SK_IMAP2, "%s", "Can not build an IMAP hierarchy with empty parts.");
//                return data;
//            }
//            utf7Parts.push_back(utf7Part);
//        }
//        data = utf7Parts.join(m_hierarchySeperator).toLatin1();
    }

    return str;
}

//QByteArray SkImap::buildUserHierarchy(const QStringList &parts) const
//{
//    QByteArray data;

//    if (!parts.empty()) {
//        QStringList _parts{QStringLiteral("user")};
//        _parts.append(parts);
//        data = buildHierarchy(_parts);
//    }

//    return data;
//}

QString SkImap::buildUserHierarchy(const QString &user) const
{
    return buildHierarchy({QStringLiteral("user"), user});
}

//QString SkImap::toUtf7Imap(const QString &str)
//{
//    QString utf7Imap;
//    QByteArray ba;

//    if (str.isEmpty()) {
//        return utf7Imap;
//    }

//    UErrorCode uec = U_ZERO_ERROR;

//    const QByteArray utf8 = str.toUtf8();

//    char *out;
//    out = static_cast<char*>(malloc(sizeof(char) * static_cast<std::size_t>(utf8.size()) * 3));
////    out = (char *) malloc(sizeof(char) * utf8.size() * 3);

//    const int32_t size = ucnv_convert("imap-mailbox-name", "utf-8", out, utf8.size() * 3, utf8.constData(), utf8.size(), &uec);

//    if ((size > 0) && (uec == U_ZERO_ERROR)) {
//        ba = QByteArray(out, size);
//    } else {
//        qCWarning(SK_IMAP2, "Failed to concert UTF-8 string \"%s\" to UTF-7IMAP (RFC2060 5.1.3): %s", qUtf8Printable(str), u_errorName(uec));
//    }

//    free(out);

//    utf7Imap = QString::fromLatin1(ba);

//    return utf7Imap;
//}

//QString SkImap::fromUtf7Imap(const QByteArray &ba)
//{
//    QString str;

//    if (ba.isEmpty()) {
//        return str;
//    }

//    UErrorCode uec = U_ZERO_ERROR;
//    char *out;
//    out = static_cast<char*>(malloc(sizeof(char) * (static_cast<std::size_t>(ba.size()) + 1)));
////    out = (char *) malloc(sizeof(char) * (ba.size() + 1));

//    const int32_t size = ucnv_convert("utf-8", "imap-mailbox-name", out, ba.size() + 1, ba.constData(), ba.size(), &uec);

//    if ((size > 0) && (uec == U_ZERO_ERROR)) {
//        str = QString::fromUtf8(out, size);
//    } else {
//        qCWarning(SK_IMAP2, "Failed to convert UTF7_IMAP (RFC2060 5.1.3) string \"%s\" to UTF-8: %s", ba.constData(), u_errorName(uec));
//    }

//    free(out);

//    return str;
//}

#include "moc_skimap.cpp"
