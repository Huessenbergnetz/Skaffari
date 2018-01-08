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

#ifndef DOMAIN_H
#define DOMAIN_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QSharedDataPointer>
#include <QDateTime>
#include <QVector>
#include <grantlee5/grantlee/metatype.h>
#include <Cutelyst/ParamsMultiMap>
#include <math.h>
#include <QLoggingCategory>

#include "simpleadmin.h"
#include "simpledomain.h"
#include "folder.h"
#include "../../common/global.h"

Q_DECLARE_LOGGING_CATEGORY(SK_DOMAIN)

namespace Cutelyst {
class Context;
class AuthenticationUser;
}

class DomainData;
class SkaffariError;

/*!
 * \ingroup skaffaricore
 * \brief Represents a domain
 */
class Domain
{
public:
    /*!
     * \brief Constructs a new Domain object with empty data.
     *
     * isValid() will return \c false.
     */
    Domain();

    /*!
     * \brief Constructs a new Domain object with the given parameters.
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
     * \param folders           default folders for new accounts in this domain
     * \param accounts          current number of accounts in this domain
     * \param created           date and time this domain has been created in UTC time zone
     * \param updated           date and time this domain has been updated in UTC time zone
     */
    Domain(dbid_t id, const QString &name, const QString &prefix, const QString &transport, quota_size_t quota, quint32 maxAccounts, quota_size_t domainQuota, quota_size_t domainQuotaUsed, bool freeNames, bool freeAddress, const QVector<Folder> &folders, quint32 accounts, const QDateTime &created, const QDateTime &updated);

    /*!
     * \brief Constructs a copy of \a other.
     */
	Domain(const Domain &other);

    /*!
     * \brief Assigns \a other to this domain and returns a reference to this domain.
     */
	Domain& operator=(const Domain &other);

    /*!
     * Destroys the domain.
     */
    ~Domain();

    /*!
     * \brief Returns the database ID of the domain.
     * \sa setId()
     */
    dbid_t id() const;

    /*!
     * \brief Returns the domain name.
     * \sa setName()
     */
    QString getName() const;

    /*!
     * \brief Returns the prefix used for the domain.
     * \sa setPrefix()
     */
    QString getPrefix() const;

    /*!
     * \brief Returns the transport method used for this domain.
     * \sa setTransport()
     */
    QString getTransport() const;

    /*!
     * \brief Returns the default quota for new accounts in this domain.
     * \sa setQuota()
     */
    quota_size_t getQuota() const;

    /*!
     * \brief Returns the number of maximum allowed accounts in this domain.
     *
     * If this returns \c 0,  there is no limit.
     *
     * \sa setMaxAccounts()
     */
    quint32 getMaxAccounts() const;

    /*!
     * \brief Returns the overall quota for all accounts in this domain in KiB.
     *
     * If this returns \c 0, there is no limit.
     *
     * \sa setDomainQuota()
     */
    quota_size_t getDomainQuota() const;

    /*!
     * \brief Returns the overall used quota for all accounts in this domain in KiB.
     *
     * \sa setDomainQuotaUsed()
     */
    quota_size_t getDomainQuotaUsed() const;

    /*!
     * \brief Returns \c true if free account user names are allowed for this domain.
     * \sa setFreeNamesEnabled()
     */
    bool isFreeNamesEnabled() const;

    /*!
     * \brief Returns \c true if free email addresses are allowed for this domain.
     * \sa setFreeAddressEnabled()
     */
    bool isFreeAddressEnabled() const;

    /*!
     * \brief Returns the list of default folders that will be created for new accounts in this domain.
     * \sa setFolders()
     */
    QVector<Folder> getFolders() const;

    /*!
     * \brief Returns the number of accounts in this domain.
     * \sa setAccounts()
     */
    quint32 getAccounts() const;

    /*!
     * \brief Returns a list of administrators that are responsible for this domain.
     *
     * The admins in this list are admin users of type AdminAccount::DomainMaster that are
     * associated to this domain.
     *
     * \sa setAdmins()
     */
    QVector<SimpleAdmin> getAdmins() const;
    /*!
     * \brief Returns the date and time this domain has been created.
     * \sa setCreated()
     */
    QDateTime created() const;
    /*!
     * \brief Returns the date and time this domain has been updated.
     * \sa setUpdated()
     */
    QDateTime updated() const;
    /*!
     * \brief Returns information about the parent domain, if any has been set.
     * \sa setParent()
     */
    SimpleDomain parent() const;
    /*!
     * \brief Returns a list of child domains if any.
     * \sa setChildren()
     */
    QVector<SimpleDomain> children() const;
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
     * \brief Sets the database ID of this doain.
     * \sa id()
     */
    void setId(dbid_t id);
    /*!
     * \brief Sets the domain name.
     * \sa getName()
     */
    void setName(const QString &nName);
    /*!
     * \brief Sets the prefix used for this domain.
     * \sa getPrefix()
     */
    void setPrefix(const QString &nPrefix);
    /*!
     * \brief Sets the transport method used for this domain.
     * \sa getPrefix()
     */
    void setTransport(const QString &nTransport);
    /*!
     * \brief Sets the default quota for new accounts in this domain.
     * \sa getQuota()
     */
    void setQuota(quota_size_t nQuota);
    /*!
     * \brief Sets the maximum number of allowed accounts in this domain.
     * \sa getMaxAccounts()
     */
    void setMaxAccounts(quint32 nMaxAccounts);
    /*!
     * \brief Sets the allowed overall account quota for this domain.
     *
     * If this is set to \c 0, there will be no limit.
     *
     * \sa getDomainQuota()
     */
    void setDomainQuota(quota_size_t nDomainQuota);
    /*!
     * \brief Sets the overall used quota of accounts in this domain.
     * \sa getDomainQuotaUsed()
     */
    void setDomainQuotaUsed(quota_size_t nDomainQuotaUsed);
    /*!
     * \brief Set this to \c true if free account user names are allowed in this domain.
     * \sa isFreeNamesEnabled()
     */
    void setFreeNamesEnabled(bool nFreeNames);
    /*!
     * \brief Set this to \c true if free email addresses are allowed in this domain.
     * \sa isFreeAddressEnabled()
     */
    void setFreeAddressEnabled(bool nFreeAddress);
    /*!
     * \brief Sets the default folders that will be created for new accounts in this domain.
     * \sa getFolders()
     */
    void setFolders(const QVector<Folder> &nFolders);
    /*!
     * \brief Sets the number of accounts available in this domain.
     * \sa getAccounts()
     */
    void setAccounts(quint32 nAccounts);
    /*!
     * \brief Sets the list of admins that are responsible for this domain.
     *
     * The admins in this list should be admins of type AdminAccount::DomainMaster that are
     * associated to this domain.
     *
     * \sa getAdmins()
     */
    void setAdmins(const QVector<SimpleAdmin> &adminList);
    /*!
     * \brief Sets the date and time this domain has been created.
     * \sa created()
     */
    void setCreated(const QDateTime &dt);
    /*!
     * \brief Sets the date and time this domain has been updated.
     * \sa updated()
     */
    void setUpdated(const QDateTime &dt);
    /*!
     * \brief Sets the parent domain information.
     * \sa parent()
     */
    void setParent(const SimpleDomain &parent);
    /*!
     * \brief Sets the list of child domains.
     * \sa children()
     */
    void setChildren(const QVector<SimpleDomain> &children);

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
     * \brief Creates a new domain and returns it.
     *
     * The returned domain might be invalid if the creation was not successful. Use isValid() to check
     * for validity.
     *
     * \par Keys in the params
     * Key               | Converted Type | Description
     * ------------------|----------------|----------------------------------------------------------------
     * domainName        | QString        | the name for the domain, will be trimmed and converted to lower case
     * prefix            | QString        | the prefix for the domain, will be trimmed and converted to lower case
     * humanQuota        | quota_size_t   | default quota for new accounts as string that can be converted by Utils::humanToIntSize()
     * quota             | quota_size_t   | default quota for new accounts as string that can be converted into integer
     * humanDomainQuota  | quota_size_t   | overall allowed account quota for the domain as string that can be convertedn by Utils::humanToIntSize()
     * domainQuota       | quota_size_t   | overall allowed account quota for the domain as string that can be converted into integer
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
     * \param errorData pointer to an object taking error information
     */
    static Domain create(Cutelyst::Context *c, const Cutelyst::ParamsMultiMap &params, SkaffariError *errorData);

    /*!
     * \brief Returns the domain identified by \a domId from the database.
     * \param c         pointer to the current context, used for translating strings
     * \param domId     database ID of the domain to query
     * \param errorData pointer to an object taking error information
     */
    static Domain get(Cutelyst::Context *c, dbid_t domId, SkaffariError *errorData);

    /*!
     * \brief Returns a list of domains from the database.
     * \param c         pointer to the current context, used for translating strings
     * \param errorData pointer to an object taking error information
     * \param user      the currently authenticated user, used to either query all or only associated domains
     * \param orderBy   the database column to order the list by
     * \param sort      the sorting direction
     * \param limit     optional limit for the result, if \c 0, there will be no limit
     */
    static std::vector<Domain> list(Cutelyst::Context *c, SkaffariError *errorData, const Cutelyst::AuthenticationUser &user, const QString orderBy = QLatin1String("domain_name"), const QString sort = QLatin1String("ASC"), quint32 limit = 0);

    /*!
     * \brief Returns \c true if \a domainName is part of the database.
     * \param domainName The domain name to check.
     * \return True if domain name is available.
     */
    static bool isAvailable(const QString &domainName);

    /*!
     * \brief Removes the \a domain and all of their accounts from the database and the IMAP server.
     * \todo Remove accounts from IMAP server.
     * \param c                 Pointer to the current context, used for translations.
     * \param domain            Pointer to the domain to remove.
     * \param error             Pointer to an error object to give feedback on database and IMAP errors.
     * \param newParent         Database ID of the new parent domain if the domain to remove has child domains.
     * \param deleteChildren    If \c true, child domains will be removed too.
     * \return          True on success.
     */
    static bool remove(Cutelyst::Context *c, Domain *domain, SkaffariError *error, dbid_t newParentId, bool deleteChildren);

    /*!
     * \brief Updates domain \a d in the database.
     * \param c Pointer to the current context, used for translations.
     * \param p Parameters from the HTML form to update in the database.
     * \param e Pointer to an error object to give feedback on database errors.
     * \param d The domain to update.
     * \param u The user that wants to update the domain, used to decide which parameters could be updated.
     * \return  True on success.
     */
    static bool update(Cutelyst::Context *c, const Cutelyst::ParamsMultiMap &p, SkaffariError *e, Domain *d, const Cutelyst::AuthenticationUser &u);

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
     * \brief Returns \c true if access has been granted to the domain.
     *
     * Before you can use this, use Domain::checkAccess().
     */
    static bool accessGranted(Cutelyst::Context *c);

    /*!
     * \brief Trims every string in the \a list.
     * \param list  The list to trim
     * \return A list of trimmed strings.
     */
    static QStringList trimStringList(const QStringList &list);

    /*!
     * \brief Returns the user name of the account used to catch all emails without recipient.
     * \param c The current context, used for translations.
     * \param e Pointer to an object taking error information.
     * \return User name of the catch all account.
     */
    QString getCatchAllAccount(Cutelyst::Context *c, SkaffariError *e) const;

private:
    QSharedDataPointer<DomainData> d;
};

Q_DECLARE_METATYPE(Domain)
Q_DECLARE_TYPEINFO(Domain, Q_MOVABLE_TYPE);

GRANTLEE_BEGIN_LOOKUP(Domain)
QVariant var;
if (property == QLatin1String("id")) {
    var.setValue(object.id());
} else if (property == QLatin1String("name")) {
    var.setValue(object.getName());
} else if (property == QLatin1String("prefix")) {
    var.setValue(object.getPrefix());
} else if (property == QLatin1String("transport")) {
    var.setValue(object.getTransport());
} else if (property == QLatin1String("quota")) {
    var.setValue(object.getQuota());
} else if (property == QLatin1String("maxAccounts")) {
    var.setValue(object.getMaxAccounts());
} else if (property == QLatin1String("domainQuota")) {
    var.setValue(object.getDomainQuota());
} else if (property == QLatin1String("domainQuotaUsed")) {
    var.setValue(object.getDomainQuotaUsed());
} else if (property == QLatin1String("domainQuotaUsagePercent")) {
    var.setValue(object.domainQuotaUsagePercent());
}  else if (property == QLatin1String("freeNames")) {
    var.setValue(object.isFreeNamesEnabled());
} else if (property == QLatin1String("freeAddress")) {
    var.setValue(object.isFreeAddressEnabled());
} else if (property == QLatin1String("folders")) {
    var.setValue(object.getFolders());
} else if (property == QLatin1String("accounts")) {
    var.setValue(object.getAccounts());
} else if (property == QLatin1String("admins")) {
    var.setValue(object.getAdmins());
} else if (property == QLatin1String("isValid")) {
    var.setValue(object.isValid());
} else if (property == QLatin1String("created")) {
    var.setValue(object.created());
} else if (property == QLatin1String("updated")) {
    var.setValue(object.updated());
} else if (property == QLatin1String("parent")) {
    var.setValue(object.parent());
} else if (property == QLatin1String("children")) {
    var.setValue(object.children());
} else if (property == QLatin1String("accountUsagePercent")) {
    var.setValue(object.accountUsagePercent());
}
return var;
GRANTLEE_END_LOOKUP

#endif // DOMAIN_H
