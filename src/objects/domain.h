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

#ifndef DOMAIN_H
#define DOMAIN_H

#include "../../common/global.h"
#include "simpleadmin.h"
#include "simpledomain.h"
#include "folder.h"

#include <Cutelyst/ParamsMultiMap>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QSharedDataPointer>
#include <QDateTime>
#include <QLoggingCategory>

#include <math.h>
#include <vector>

Q_DECLARE_LOGGING_CATEGORY(SK_DOMAIN)

namespace Cutelyst {
class Context;
class AuthenticationUser;
}

class DomainData;
class SkaffariError;

/*!
 * \ingroup skaffaricore
 * \brief Represents a single domain that is managed by %Skaffari.
 */
class Domain
{
    Q_GADGET
    Q_PROPERTY(dbid_t id READ id CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString prefix READ prefix CONSTANT)
    Q_PROPERTY(QString transport READ transport CONSTANT)
    Q_PROPERTY(quota_size_t quota READ quota CONSTANT)
    Q_PROPERTY(quint32 maxAccounts READ maxAccounts CONSTANT)
    Q_PROPERTY(quota_size_t domainQuota READ domainQuota CONSTANT)
    Q_PROPERTY(quota_size_t domainQuotaUsed READ domainQuotaUsed CONSTANT)
    Q_PROPERTY(float domainQuotaUsagePercent READ domainQuotaUsagePercent CONSTANT)
    Q_PROPERTY(bool freeNames READ isFreeNamesEnabled CONSTANT)
    Q_PROPERTY(bool freeAddress READ isFreeAddressEnabled CONSTANT)
    Q_PROPERTY(std::vector<Folder> folders READ folders CONSTANT)
    Q_PROPERTY(Folder draftsFolder READ draftsFolder CONSTANT)
    Q_PROPERTY(Folder junkFolder READ junkFolder CONSTANT)
    Q_PROPERTY(Folder sentFolder READ sentFolder CONSTANT)
    Q_PROPERTY(Folder trashFolder READ trashFolder CONSTANT)
    Q_PROPERTY(Folder archiveFolder READ archiveFolder CONSTANT)
    Q_PROPERTY(Folder otherFolders READ otherFolders CONSTANT)
    Q_PROPERTY(quint32 accounts READ accounts CONSTANT)
    Q_PROPERTY(float accountUsagePercent READ accountUsagePercent CONSTANT)
    Q_PROPERTY(std::vector<SimpleAdmin> admins READ admins CONSTANT)
    Q_PROPERTY(bool isValid READ isValid CONSTANT)
    Q_PROPERTY(QDateTime created READ created CONSTANT)
    Q_PROPERTY(QDateTime updated READ updated CONSTANT)
    Q_PROPERTY(Domain::AutoconfigStrategy autoconfig READ autoconfig CONSTANT)
    Q_PROPERTY(SimpleDomain parent READ parent CONSTANT)
    Q_PROPERTY(std::vector<SimpleDomain> children READ children CONSTANT)
    Q_PROPERTY(bool isIdn READ isIdn CONSTANT)
    Q_PROPERTY(QDateTime validUntil READ validUntil CONSTANT)
public:
    /*!
     * \brief How to handle autoconfig for this domain.
     */
    enum AutoconfigStrategy : qint8 {
        AutoconfigDisabled  = 0,    /**< Autoconfig is disabled for this domain. */
        UseGlobalAutoconfig = 1,    /**< Use the global servers for this domain. */
        UseCustomAutoconfig = 2     /**< Use custom servers for this domain. */
    };
    Q_ENUM(AutoconfigStrategy)

    /*!
     * \brief Constructs a new %Domain object with empty data.
     *
     * isValid() will return \c false.
     */
    Domain();

    /*!
     * \brief Constructs a new %Domain object with the given parameters.
     * \param id                database ID of the domain
     * \param name              the domain name
     * \param prefix            the prefix used for this domain
     * \param transport         the transport method used for this domain
     * \param quota             the default quota for new accounts in this domain
     * \param maxAccounts       the maximum number of accounts allowed for this domain
     * \param domainQuota       the overall quota for all accounts in this domain
     * \param domainQuotaUsed   the overall used quota for all accounts in this domain
     * \param freeNames         \c true if free user names are allowed for this domain
     * \param freeAddress       \c true if free email addresses are allowed for this domain
     * \param accounts          current number of accounts in this domain
     * \param created           date and time this domain has been created in UTC time zone
     * \param updated           date and time this domain has been updated in UTC time zone
     * \param validUntil        date and time until accounts in this domain can be valid
     * \param autoconfig        the AutoconfigStrategy to use for this domain
     */
    Domain(dbid_t id, dbid_t aceId, const QString &name, const QString &prefix, const QString &transport, quota_size_t quota, quint32 maxAccounts, quota_size_t domainQuota, quota_size_t domainQuotaUsed, bool freeNames, bool freeAddress, quint32 accounts, const QDateTime &created, const QDateTime &updated, const QDateTime &validUntil, AutoconfigStrategy autoconfig, const SimpleDomain &parent, const std::vector<SimpleDomain> &children, const std::vector<SimpleAdmin> &admins, const std::vector<Folder> &folders);

    /*!
     * \brief Constructs a copy of \a other.
     */
    Domain(const Domain &other);

    /*!
     * \brief Move-constructs a %Domain instance, making it point at the same object that \a other was pointing to.
     */
    Domain(Domain &&other) noexcept;

    /*!
     * \brief Assigns \a other to this domain and returns a reference to this domain.
     */
    Domain& operator=(const Domain &other);

    /*!
     * \brief Move-assigns \a other to this %Domain instance.
     */
    Domain& operator=(Domain &&other) noexcept;

    /*!
     * Destroys the domain.
     */
    ~Domain();

    /*!
     * \brief Swaps this %Domain instance with \a other.
     */
    void swap(Domain &other) noexcept;

    /*!
     * \brief Returns the database ID of the domain.
     */
    dbid_t id() const;

    /*!
     * \brief Returns the domain name.
     * \sa aceName()
     */
    QString name() const;

    /*!
     * \brief Returns the ACE version of the domain name.
     * If this is not an IDN it will return the same as name().
     * \sa name()
     */
    QString aceName() const;

    /*!
     * \brief Returns a string that contains the username and the database ID.
     *
     * This will mostly be used in log messages and returns a string like
     * <code>"example.com (ID: 123)"</code>.
     */
    QString nameIdString() const;

    /*!
     * \brief Returns the prefix used for the domain.
     */
    QString prefix() const;

    /*!
     * \brief Returns the transport method used for this domain.
     */
    QString transport() const;

    /*!
     * \brief Returns the default quota for new accounts in this domain.
     */
    quota_size_t quota() const;

    /*!
     * \brief Returns the number of maximum allowed accounts in this domain.
     *
     * If this returns \c 0, there is no limit.
     */
    quint32 maxAccounts() const;

    /*!
     * \brief Returns the overall quota for all accounts in this domain in KiB.
     *
     * If this returns \c 0, there is no limit.
     *
     * \sa setDomainQuota()
     */
    quota_size_t domainQuota() const;

    /*!
     * \brief Returns the overall used quota for all accounts in this domain in KiB.
     *
     * \sa setDomainQuotaUsed()
     */
    quota_size_t domainQuotaUsed() const;
    /*!
     * \brief Sets the overall used quota of accounts in this domain.
     * \sa domainQuotaUsed()
     */
    void setDomainQuotaUsed(quota_size_t nDomainQuotaUsed);

    /*!
     * \brief Returns \c true if free account user names are allowed for this domain.
     */
    bool isFreeNamesEnabled() const;

    /*!
     * \brief Returns \c true if free email addresses are allowed for this domain.
     */
    bool isFreeAddressEnabled() const;

    /*!
     * \brief Returns the list of default folders that will be created for new accounts in this domain.
     */
    std::vector<Folder> folders() const;

    /*!
     * \brief Returns the folder for the \a specialUse.
     */
    Folder folder(SkaffariIMAP::SpecialUse specialUse) const;

    /*!
     * \brief Returns the folder to create for drafts.
     */
    Folder draftsFolder() const;

    /*!
     * \brief Returns the folder to create for junk.
     */
    Folder junkFolder() const;

    /*!
     * \brief Returns the folder to create for sent messages.
     */
    Folder sentFolder() const;

    /*!
     * \brief Returns the folder to create as trash.
     */
    Folder trashFolder() const;

    /*!
     * \brief Returns the folder to create as archive.
     */
    Folder archiveFolder() const;

    /*!
     * \brief Returns a folder that has a name as comma separated list of other folders.
     */
    Folder otherFolders() const;

    /*!
     * \brief Returns the number of accounts in this domain.
     */
    quint32 accounts() const;

    /*!
     * \brief Returns a list of administrators that are responsible for this domain.
     *
     * The admins in this list are admin users of type AdminAccount::DomainMaster that are
     * associated to this domain.
     */
    std::vector<SimpleAdmin> admins() const;

    /*!
     * \brief Returns the date and time this domain has been created.
     */
    QDateTime created() const;

    /*!
     * \brief Returns the date and time this domain has been updated.
     */
    QDateTime updated() const;

    /*!
     * \brief Returns the date and time until accounts in this domain can be valid.
     */
    QDateTime validUntil() const;

    /*!
     * \brief Returns the AutoconfigStrategy for this domain.
     */
    Domain::AutoconfigStrategy autoconfig() const;

    /*!
     * \brief Returns information about the parent domain, if any has been set.
     */
    SimpleDomain parent() const;

    /*!
     * \brief Returns a list of child domains if any.
     */
    std::vector<SimpleDomain> children() const;

    /*!
     * \brief Returns \c true if this domain has an internationalized domain name (IDN).
     * \sa aceId()
     */
    bool isIdn() const;

    /*!
     * \brief Returns the dabase ID of the corresponding ACE version of the domain name.
     */
    dbid_t aceId() const;

    /*!
     * \brief Returns the percental value of used domain quota if any quota is set.
     *
     * If there is no domain quota set (domain quota == 0), this will return \c 0.
     */
    float domainQuotaUsagePercent() const;

    /*!
     * \brief Returns the percental value of used maximum accounts if maximum account value is set.
     *
     * If there is no maximum account value set (maximum accounts == 0), this will return \c 0.
     */
    float accountUsagePercent() const;

    /*!
     * \brief Returns \c true if this domain is valid.
     *
     * A domain will be valid if the database ID is greater than \c 0 and if
     * prefix and domain name are not empty.
     */
    bool isValid() const;

    /*!
     * \brief Returns \c true if the administrator in the current context \a c has access to this domain.
     */
    bool hasAccess(Cutelyst::Context *c) const;

    /*!
     * \brief Returns \c true if this domain is valid.
     *
     * A domain will be valid if the database ID is greater than \c 0 and if
     * prefix and domain name are not empty.
     *
     * \sa isValid()
     */
    explicit operator bool() const {
        return isValid();
    }

    /*!
     * \brief Returns a SimpleDomain object containing information about this domain.
     */
    SimpleDomain toSimple() const;

    /*!
     * \brief Creates a new domain and returns it.
     *
     * The returned domain might be invalid if the creation was not successful. Use isValid() to check
     * for validity.
     *
     * \par Keys in the params
     * Key               | Converted Type | Description
     * ------------------|----------------|------------
     * domainName        | QString        | the name for the domain, will be trimmed and converted to lower case
     * prefix            | QString        | the prefix for the domain, will be trimmed and converted to lower case
     * quota             | quota_size_t   | default quota for new accounts
     * domainQuota       | quota_size_t   | overall allowed account quota for the domain
     * maxAccounts       | quint32        | maximum allowed accounts in this domain
     * freeNames         | bool           | if free account user names are allowed or not
     * freeAddress       | bool           | if free email addresses are allowed or not
     * folders           | QStringList    | comma separated list of default folder names for new accounts in this domain
     * transport         | QString        | transport method to use for this domain
     * parent            | dbid_t         | database id of the parent domain
     * abuseAccount      | dbid_t         | database id of the account to get the abuse@domain.name address for the new domain
     * nocAccount        | dbid_t         | database id of the account to get the noc@domain.name address for the new domain
     * securityAccount   | dbid_t         | database id of the account to get the security@domain.name address for the new domain
     * postmasterAccount | dbid_t         | database id of the account to get the postmaster@domain.name address for the new domain
     * hostmasterAccount | dbid_t         | database id of the account to get the hostmaster@domain.name address for the new domain
     * webmasterAccount  | dbid_t         | database id of the acocunt to get the webmaster@domain.name address for the new domain
     *
     * \param c         pointer to the current context, used for translating strings
     * \param params    parameters used to create the new domain
     * \param errorData object taking error information
     */
    static Domain create(Cutelyst::Context *c, const QVariantHash &params, SkaffariError &errorData);

    /*!
     * \brief Returns the domain identified by \a domId from the database.
     * \param c         pointer to the current context, used for translating strings
     * \param domId     database ID of the domain to query
     * \param errorData object taking error information
     */
    static Domain get(Cutelyst::Context *c, dbid_t domId, SkaffariError &errorData);

    /*!
     * \brief Returns a list of domains from the database.
     * \param c         pointer to the current context, used for translating strings
     * \param errorData object taking error information
     * \param user      the currently authenticated user, used to either query all or only associated domains
     * \param orderBy   the database column to order the list by
     * \param sort      the sorting direction
     * \param limit     optional limit for the result, if \c 0, there will be no limit
     */
    static std::vector<Domain> list(Cutelyst::Context *c, SkaffariError &errorData, const Cutelyst::AuthenticationUser &user, const QString &orderBy = QStringLiteral("domain_name"), const QString &sort = QStringLiteral("ASC"), quint32 limit = 0);

    /*!
     * \brief Returns \c true if \a domainName is part of the database.
     * \param domainName The domain name to check.
     * \return True if domain name is available.
     */
    static bool isAvailable(const QString &domainName);

    /*!
     * \brief Removes the %Domain and all of their accounts from the database and the IMAP server.
     *
     * \param c                 Pointer to the current context, used for translations.
     * \param error             Object to give feedback on database and IMAP errors.
     * \param newParent         Database ID of the new parent domain if the domain to remove has child domains.
     * \param deleteChildren    If \c true, child domains will be removed too.
     * \return \c true on success
     */
    bool remove(Cutelyst::Context *c, SkaffariError &error, dbid_t newParentId, bool deleteChildren);

    /*!
     * \brief Updates domain \a d in the database.
     * \param c Pointer to the current context, used for translations.
     * \param p Parameters from the HTML form to update in the database.
     * \param e Object to give feedback on database errors.
     * \return \c true on success
     */
    bool update(Cutelyst::Context *c, const QVariantHash &p, SkaffariError &e);

    /*!
     * \brief Puts this domain into the current context stash.
     * If the domain it not valid, a 404 error will be returned and the processing
     * will be detached to the Root::error() method.
     * \param c Pointer to the current context.
     * \return \c true if the domain was valid, otherwise \c false.
     */
    bool toStash(Cutelyst::Context *c) const;

    /*!
     * \brief Loads the domain identified by \a domainId into the stash of context \a c.
     *
     * \param c The current context.
     * \param domainId Database ID of the domain.
     * \param e Pointer to an object taking error information.
     * \return True on success.
     */
    static void toStash(Cutelyst::Context *c, dbid_t domainId);

    /*!
     * \brief Returns the current domain from the context stash that has been set by toStash().
     *
     * \param c The current context.
     * \return Valid Domain object on success.
     */
    static Domain fromStash(Cutelyst::Context *c);

    /*!
     * \brief Returns \c true if the currently authenticated user in the context \a c has access to the domain with \a domainId.
     */
    static bool checkAccess(Cutelyst::Context *c, dbid_t domainId = 0);

    /*!
     * \brief Returns the user name of the account used to catch-all emails without recipient.
     * \param c The current context, used for translations.
     * \param e Object taking error information.
     * \return User name of the catch-all account.
     */
    QString getCatchAllAccount(Cutelyst::Context *c, SkaffariError &e) const;

private:
    QSharedDataPointer<DomainData> d;

    friend QDataStream &operator>>(QDataStream &stream, Domain &domain);
    friend QDataStream &operator<<(QDataStream &stream, const Domain &domain);
};

Q_DECLARE_METATYPE(Domain)
Q_DECLARE_TYPEINFO(Domain, Q_MOVABLE_TYPE);

/*!
 * \relates Domain
 * \brief Writes the \a domain to the \a dbg stream  and returns the stream.
 */
QDebug operator<<(QDebug dbg, const Domain &domain);

/*!
 * \relates Domain
 * \brief Reads a %Domain from the given \a stream and stores it in the given \a domain.
 */
QDataStream &operator>>(QDataStream &stream, Domain &domain);

/*!
 * \relates Domain
 * \brief Writes the given \a domain to the given \a stream.
 */
QDataStream &operator<<(QDataStream &stream, const Domain &domain);

#endif // DOMAIN_H
