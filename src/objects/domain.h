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
 * \brief Represents a domain
 */
class Domain
{
public:
    Domain();
    Domain(dbid_t id, const QString &name, const QString &prefix, const QString &transport, quota_size_t quota, quint32 maxAccounts, quota_size_t domainQuota, quota_size_t domainQuotaUsed, bool freeNames, bool freeAddress, const QVector<Folder> &folders, quint32 accounts, const QDateTime &created, const QDateTime &updated);
	Domain(const Domain &other);
	Domain& operator=(const Domain &other);
    ~Domain();

    dbid_t id() const;
    QString getName() const;
    QString getPrefix() const;
    QString getTransport() const;
    quota_size_t getQuota() const;
    QString getHumanQuota() const;
    quint32 getMaxAccounts() const;
    quota_size_t getDomainQuota() const;
    QString getHumanDomainQuota() const;
    quota_size_t getDomainQuotaUsed() const;
    QString getHumanDomainQuotaUsed() const;
    bool isFreeNamesEnabled() const;
    bool isFreeAddressEnabled() const;
    QVector<Folder> getFolders() const;
    quint32 getAccounts() const;
    QVector<SimpleAdmin> getAdmins() const;
    /*!
     * \brief Returns the date and time this domain has been created.
     */
    QDateTime created() const;
    /*!
     * \brief Returns the date and time this domain has been updated.
     */
    QDateTime updated() const;
    /*!
     * \brief Returns the percental value of used domain quota if any quota is set.
     *
     * If there is no domain quota set (domain quota == 0), this will return \c 0.
     */
    float domainQuotaUsagePercent() const;


    void setId(dbid_t id);
    void setName(const QString &nName);
    void setPrefix(const QString &nPrefix);
    void setTransport(const QString &nTransport);
    void setQuota(quota_size_t nQuota);
    void setHumanQuota(const QString &nHumanQuota);
    void setMaxAccounts(quint32 nMaxAccounts);
    void setDomainQuota(quota_size_t nDomainQuota);
    void setHumanDomainQuota(const QString &humanDomainQuota);
    void setDomainQuotaUsed(quota_size_t nDomainQuotaUsed);
    void setHumanDomainQuotaUsed(const QString &humanDomainQuotaUsed);
    void setFreeNamesEnabled(bool nFreeNames);
    void setFreeAddressEnabled(bool nFreeAddress);
    void setFolders(const QVector<Folder> &nFolders);
    void setAccounts(quint32 nAccounts);
    void setAdmins(const QVector<SimpleAdmin> &adminList);
    /*!
     * \brief Sets the date and time this domain has been created.
     */
    void setCreated(const QDateTime &dt);
    /*!
     * \brief Sets the date and time this domain has been updated.
     */
    void setUpdated(const QDateTime &dt);

    bool isValid() const;
    bool hasAccess(Cutelyst::Context *c) const;

    static Domain create(Cutelyst::Context *c, const Cutelyst::ParamsMultiMap &params, SkaffariError *errorData);
    static Domain get(Cutelyst::Context *c, dbid_t domId, SkaffariError *errorData);
    static std::vector<Domain> list(Cutelyst::Context *c, SkaffariError *errorData, const Cutelyst::AuthenticationUser &user);

    /*!
     * \brief Returns \c true if \a domainName is part of the database.
     * \param domainName The domain name to check.
     * \return True if domain name is available.
     */
    static bool isAvailable(const QString &domainName);

    /*!
     * \brief Removes the \a domain and all of their accounts from the database and the IMAP server.
     * \todo Remove accounts from IMAP server.
     * \param c         Pointer to the current context, used for translations.
     * \param domain    Pointer to the domain to remove.
     * \param error     Pointer to an error object to give feedback on database and IMAP errors.
     * \return          True on success.
     */
    static bool remove(Cutelyst::Context *c, Domain *domain, SkaffariError *error);

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

    static bool checkAccess(Cutelyst::Context *c, dbid_t domainId = 0);

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
} else if (property == QLatin1String("humanQuota")) {
    var.setValue(object.getHumanQuota());
} else if (property == QLatin1String("maxAccounts")) {
    var.setValue(object.getMaxAccounts());
} else if (property == QLatin1String("domainQuota")) {
    var.setValue(object.getDomainQuota());
} else if (property == QLatin1String("humanDomainQuota")) {
    var.setValue(object.getHumanDomainQuota());
} else if (property == QLatin1String("domainQuotaUsed")) {
    var.setValue(object.getDomainQuotaUsed());
} else if (property == QLatin1String("humanDomainQuotaUsed")) {
    var.setValue(object.getHumanDomainQuotaUsed());
} else if (property == QLatin1String("domainQuotaUsagePercent")) {
    var.setValue(object.domainQuotaUsagePercent());
} else if (property == QLatin1String("domainQuotaUsagePercentFlat")) {
    var.setValue(lround(object.domainQuotaUsagePercent()));
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
}
return var;
GRANTLEE_END_LOOKUP

#endif // DOMAIN_H
