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

#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QSharedDataPointer>
#include <QString>
#include <QStringList>
#include <grantlee5/grantlee/metatype.h>
#include <Cutelyst/ParamsMultiMap>
#include <Cutelyst/Plugins/Utils/Pagination>
#include <QLoggingCategory>
#include <QDateTime>
#include <math.h>
#include <QJsonObject>
#include "../../common/global.h"
#include "domain.h"

Q_DECLARE_LOGGING_CATEGORY(SK_ACCOUNT)

namespace Cutelyst {
class Context;
}

class AccountData;
class SkaffariError;

/*!
 * \ingroup skaffaricore
 * \brief Represents an user account
 *
 * An user account contains information about a single email user account, inlcuding associated addresses and forwards.
 */
class Account
{
public:
    /*!
     * \brief Creates an empty, invalid %Account object.
     */
    Account();

    /*!
     * \brief Creates a new %Account object with the given parameters.
     * \param id            Database ID
     * \param domainId      Database ID of the domain the account belongs to
     * \param username      Account user name
     * \param prefix        Prefix of the domain the account belongs to
     * \param domainName    Name of the domain the account belongs to
     * \param imap          \c true if IMAP access is enabled
     * \param pop           \c true if POP3 access is enabled
     * \param sieve         \c true if access to Sieve is enabled
     * \param smtpauth      \c true if access to sending emails via SMTP is enabled
     * \param addresses     List of email addresses for this account
     * \param forwards      List of email forwards for this account
     * \param quota         Quota set for this account in KiB
     * \param usage         Quota used by this account in KiB
     * \param created       Date and time this account has been created
     * \param updated       Date and time this account has been updated the last time
     * \param validUntil    Date and time until this account is valid
     * \param pwdExpiration Date and time until the password for this account is valid
     * \param keepLocal     \c true if forwaded emails should be kept local too
     * \param catchAll      \c true if this is the catch-all account for the \a domainName
     * \param status        Status value saved in the database.
     */
    Account(dbid_t id, dbid_t domainId, const QString &username, const QString &prefix, const QString &domainName, bool imap, bool pop, bool sieve, bool smtpauth, const QStringList &addresses, const QStringList &forwards, quota_size_t quota, quota_size_t usage, const QDateTime &created, const QDateTime &updated, const QDateTime &validUntil, const QDateTime &pwdExpiration, bool keepLocal, bool catchAll, quint8 status);

    /*!
     * \brief Creates a copy of \a other.
     */
    Account(const Account &other);

    /*!
     * \brief Assigns \a other to this %Account.
     */
    Account& operator=(const Account &other);

    /*!
     * \brief Destroys the %Account object and frees any allocated resources.
     */
    ~Account();

    /*!
     * \brief Strategy for mailbox creation on IMAP server.
     */
    enum CreateMailbox : quint8 {
        DoNotCreate         = 0,    /**< Skaffari will not create anything on the IMAP server. */
        LoginAfterCreation  = 1,    /**< Skaffari will login to the new account after creation to let the IMAP server create the mailbox and set the quota. */
        OnlySetQuota        = 2,    /**< Skaffari will login to the new account after creation to let the IMAP server create the mailbox and will than set the account quota. */
        CreateBySkaffari    = 3     /**< Skaffari will create the account, including the domain's default folders and will set the quota. */
    };

    /*!
     * \brief Returns the account database ID.
     *
     * Access from Grantlee: id
     *
     * \sa setId()
     */
    dbid_t id() const;
    /*!
     * \brief Returns the database ID of the domain the account belongs to.
     *
     * Access from Grantlee: domainId
     *
     * \sa setDomainId()
     */
    dbid_t domainId() const;
    /*!
     * \brief Returns the username of the account.
     *
     * Access from Grantlee: username
     *
     * \sa setUsername()
     */
    QString username() const;
    /*!
     * \brief Returns the prefix of the domain the account belongs to.
     *
     * Access from Grantlee: prefix
     *
     * \sa setPrefix()
     */
    QString prefix() const;
    /*!
     * \brief Returns the name of the domain the account belongs to.
     *
     * Access from Grantlee: domainName
     *
     * \sa setDomainName()
     */
    QString domainName() const;
    /*!
     * \brief Returns \c true if IMAP access is enabled for this account.
     *
     * Access from Grantlee: imap
     *
     * \sa setImapEnabled()
     */
    bool isImapEnabled() const;
    /*!
     * \brief Returns \c true if POP3 access is enabled for this account.
     *
     * Access from Grantlee: pop
     *
     * \sa setPopEnabled()
     */
    bool isPopEnabled() const;
    /*!
     * \brief Returns \c true if Sieve access is enabled for this account.
     *
     * Access from Grantlee: sieve
     *
     * \sa setSieveEnabled()
     */
    bool isSieveEnabled() const;
    /*!
     * \brief Returns \c true if sending emails via SMTP is enabled for this account.
     *
     * Access from Grantlee: smtpauth
     *
     * \sa setSmtpauthEnabled()
     */
    bool isSmtpauthEnabled() const;
    /*!
     * \brief Returns the email addresses that are connected to this account.
     *
     * Access from Grantlee: addresses
     *
     * \sa setAddresses()
     */
    QStringList addresses() const;
    /*!
     * \brief Returns the emal forwards that are defined for this account.
     *
     * Access from Grantlee: forwards
     *
     * \sa setForwards()
     */
    QStringList forwards() const;
    /*!
     * \brief Returns the quota defined for this account in KiB.
     *
     * Access from Grantlee: quota
     *
     * \sa setQuota()
     */
    quota_size_t quota() const;
    /*!
     * \brief Returns the quota used by this account in KiB.
     *
     * Access from Grantlee: usage
     *
     * \sa setUsage()
     */
    quota_size_t usage() const;
    /*!
     * \brief Returns a percentage value of the used quota.
     *
     * Access from Grantlee: usagePercent
     *
     * \sa quota(), usage(), setUsagePercent()
     */
    float usagePercent() const;
    /*!
     * \brief Returns \c true if this account is valid.
     *
     * An account is valid if the account's database ID, returned via getId(), and the
     * domain database ID both return values greater than \c 0.
     *
     * Access from Grantlee: isValid
     */
    bool isValid() const;
    /*!
     * \brief Returns the date and time this account has been created.
     *
     * Access from Grantlee: created
     *
     * \sa setCreated()
     */
    QDateTime created() const;
    /*!
     * \brief Returns the date and time this account has been updated the last time.
     *
     * Access from Grantlee: updated
     *
     * \sa setUpdated()
     */
    QDateTime updated() const;
    /*!
     * \brief Returns the date and time until this account is valid.
     *
     * Access from Grantlee: validUntil
     *
     * \sa setValidUntil()
     */
    QDateTime validUntil() const;
    /*!
     * \brief Returns \c true if forwarded emails should be kept local too.
     *
     * Access from Grantlee: keepLocal
     *
     * \sa setKeepLocal()
     */
    bool keepLocal() const;
    /*!
     * \brief Returns \c true if this account is for catch-all.
     *
     * Access from Grantlee: catchAll
     *
     * \sa setCatchAll()
     */
    bool catchAll() const;
    /*!
     * \brief Returns the date and time when the current password of this user expires.
     *
     * Access from Grantlee: passwordExpires
     *
     * \sa setPasswordExpires()
     */
    QDateTime passwordExpires() const;
    /*!
     * \brief Returns \c true if the password of this user has been expired.
     *
     * Access from Grantlee: passwordExpired
     *
     * \sa passwordExpires()
     */
    bool passwordExpired() const;
    /*!
     * \brief Returns \c true if this account has been expired.
     *
     * Access from Grantlee: expired
     *
     * \sa validUntil()
     */
    bool expired() const;
    /*!
     * \brief Returns the status as it is saved in the database.
     * \sa setStatus()
     */
    quint8 status() const;



    /*!
     * \brief Sets the database ID of this account.
     * \sa id()
     */
    void setId(dbid_t nId);
    /*!
     * \brief Sets the databsae ID of the domain this account belongs to.
     * \sa domainId()
     */
    void setDomainId(dbid_t nDomainId);
    /*!
     * \brief Sets the username of this account.
     * \sa username()
     */
    void setUsername(const QString &nUsername);
    /*!
     * \brief Sets the prefix of the domain this account belongs to.
     * \sa prefix()
     */
    void setPrefix(const QString &nPrefix);
    /*!
     * \brief Sets the name of the domain this account belongs to.
     * \sa domainName()
     */
    void setDomainName(const QString &nDomainName);
    /*!
     * \brief Set this to \c true if IMAP access is enabled for this account.
     * \sa isImapEnabled()
     */
    void setImapEnabled(bool nImap);
    /*!
     * \brief Set this to \c true if POP3 access is enabled for this account.
     * \sa isPopEnabled()
     */
    void setPopEnabled(bool nPop);
    /*!
     * \brief Set this to \c true if Sieve access is enabeld for this account.
     * \sa isSieveEnabled()
     */
    void setSieveEnabled(bool nSieve);
    /*!
     * \brief Set this to \c true if sending via SMTP is enabled for this account.
     * \sa isSmtpauthEnabled()
     */
    void setSmtpauthEnabled(bool nSmtpauth);
    /*!
     * \brief Sets the list of email addresses that are connected to this account.
     * \sa addresses()
     */
    void setAddresses(const QStringList &nAddresses);
    /*!
     * \brief Sets the list of email forwards that are defined for this account.
     * \sa forwards()
     */
    void setForwards(const QStringList &nForwards);
    /*!
     * \brief Sets the quota in KiB that is defined for this account.
     * \sa quota()
     */
    void setQuota(quota_size_t nQuota);
    /*!
     * \brief Sets the amount of quota used by this account in KiB.
     * \sa usage()
     */
    void setUsage(quota_size_t nUsage);
    /*!
     * \brief Sets the date and time this account has been created.
     * \sa created()
     */
    void setCreated(const QDateTime &created);
    /*!
     * \brief Sets the date and time this account has been updated the last time.
     * \sa updated()
     */
    void setUpdated(const QDateTime &updated);
    /*!
     * \brief Sets the date and time until this account is valid.
     * \sa getValidUntil()
     */
    void setValidUntil(const QDateTime &validUntil);
    /*!
     * \brief Set this to \c true if forwarded emails should be kept local too.
     * \sa keepLocal()
     */
    void setKeepLocal(bool nKeepLocal);
    /*!
     * \brief Set this to \c true if this is a catch-all account.
     * \sa catchAll()
     */
    void setCatchAll(bool nCatchAll);
    /*!
     * \brief Sets the date and time when the password for this account expires.
     * \sa passwordExpires()
     */
    void setPasswordExpires(const QDateTime &expirationDate);
    /*!
     * \brief Sets the status as it is saved in the database.
     * \sa status()
     */
    void setStatus(quint8 status);

    /*!
     * \brief Converts the account data into a JSON object.
     */
    QJsonObject toJson() const;

    /*!
     * \brief Creates a new user account in the database and on the IMAP server.
     *
     * The new account will be created in the database at first, if that succeeds, the mailbox will be
     * created on the IMAP server according to the setting of SkaffariConfig::imapCreatemailbox().
     *
     * You should use isValid() on the returned Account object to check if the account has been created
     * successfully. If the returned account object is not valid, the SkaffariError object pointed to by \a e
     * will contain information about the error.
     *
     * \param c Pointer to the current context, used for string translation and user authentication.
     * \param e Pointer to an object taking information about occurring errors.
     * \param p Input parameters containting information about the new account.
     * \param d The domain the account should be created in.
     * \return A valid Account object on success.
     */
    static Account create(Cutelyst::Context *c, SkaffariError *e, const QVariantHash &p, const Domain &d, const QStringList &selectedKids);

    /*!
     * \brief Removes the account from the database and the IMAP server.
     *
     * Will at first try to delete the account's mailbox from the IMAP server, if that succeeds, the account will
     * be deleted from the database.
     *
     * If deletion fails, the SkaffariError object pointed to by \a e will contain information about the error.
     *
     * \param c         Pointer to the current context, used for string translation and user authentication.
     * \param e         Pointer to an object taking information about occurring errors.
     * \param domain    The domain the account to delete belongs to.
     * \return \c true on success.
     */
    bool remove(Cutelyst::Context *c, SkaffariError *e) const;

    /*!
     * \brief Lists all accounts belonging to the domain \a d.
     * \param c             Pointer to the current context, used for string translation and user authentication.
     * \param e             Pointer to an object taking information about occurring errors.
     * \param d             The domain you want to list the accounts from.
     * \param p             Contains information about the pagination.
     * \param sortBy        Column to sort the accounts by.
     * \param sortOrder     Order to sort the accounts by, valid values: ASC, DESC
     * \param searchRole    The column to search in for the \a searchString.
     * \param searchString  The string to search for in the column defined by \a searchRole.
     * \return A pagination object containing information about the pagination and the list of accounts.
     */
    static Cutelyst::Pagination list(Cutelyst::Context *c, SkaffariError *e, const Domain &d, const Cutelyst::Pagination &p, const QString &sortBy = QStringLiteral("username"), const QString &sortOrder = QStringLiteral("ASC"), const QString &searchRole = QStringLiteral("username"), const QString &searchString = QString());

    /*!
     * \brief Gets the account defined by database ID \a id from the database.
     *
     * You should use isValid() to check if the request succeeded. If not, the SkaffariError object point to
     * by \a e will contain information about occurred errors.
     *
     * \param c     Pointer to the current context, used for string translation, user authentication and to set the stash.
     * \param e     Pointer to an object taking information about occurring errors.
     * \param id    The database ID of the account.
     * \return      Account object containing the account data.
     */
    static Account get(Cutelyst::Context *c, SkaffariError *e, dbid_t id);

    /*!
     * \brief Requests information about the account defined by \a accountId from the database and adds it to the stash.
     * \param c         Pointer to the current context, used for string translation and user authentication.
     * \param accountId The database ID of the account.
     * \sa fromStash()
     */
    static void toStash(Cutelyst::Context *c, dbid_t accountId);

    /*!
     * \brief Puts the account \a into the context statsh.
     * \param c         Pointer to the current context.
     * \param a         Account to put into the context stash.
     */
    static void toStash(Cutelyst::Context *c, const Account &a);

    /*!
     * \brief Returns the current Account object from the stash.
     * \param c Pointer to the current context, used for string translation, user authentication and to get the stash.
     * \return Account object of the current account in the stash.
     */
    static Account fromStash(Cutelyst::Context *c);

    /*!
     * \brief Updates the account pointed to by \a a in the database and on the IMAP server.
     *
     * Will update data like account password and quota.
     *
     * If the update fails, the SkaffariError object pointed to by \a e will contain information about occurred errors.
     *
     * \param c Pointer to the current context, used for string translation and user authentication.
     * \param e Pointer to an object taking information about occurring errors.
     * \param dom Pointer to an Domain object that will have it's used domain quota updated.
     * \param p Updated parameters for this account.
     * \return \c true on success.
     */
    bool update(Cutelyst::Context *c, SkaffariError *e, Domain *dom, const QVariantHash &p);

    /*!
     * \brief Checks if all account data is available and creates missing data.
     *
     * This will for example check for missing mailbox on the IMAP server and wrong or missing storage quotas.
     *
     * \param c         Pointer to the current context, used for string translation and user authentication.
     * \param e         Pointer to an object taking information about occurring errors.
     * \param domain    Object containing information about the domain the account belongs to.
     * \param p         Man of input parameters containing checks to perform.
     * \return List of actions performed for this account.
     */
    QStringList check(Cutelyst::Context *c, SkaffariError *e, const Domain &domain = Domain(), const Cutelyst::ParamsMultiMap &p = Cutelyst::ParamsMultiMap());

    /*!
     * \brief Updates a single email address connected to the account pointed to by \a a.
     *
     * If the update fails, the SkaffariError object pointed to by \a e will contain information about occurred errors.
     *
     * \param c             Pointer to the current context, used for string translation and user authentication.
     * \param e             Pointer to an object taking information about occurring errors.
     * \param a             Pointer to the Account object the email address should be updated for.
     * \param p             Input parameters containing the updated email address.
     * \param oldAddress    The old email address.
     * \return \c updated email address
     */
    QString updateEmail(Cutelyst::Context *c, SkaffariError *e, const QVariantHash &p, const QString &oldAddress);

    /*!
     * \brief Adds a new email address to the account pointed to by \a a.
     *
     * If adding the email address fails, the SkaffariError object pointed to by \a e will contain information
     * about occurred errors.
     *
     * \param c Pointer to the current context, used for string translation and user authentication.
     * \param e Pointer to an object taking information about occurring errors.
     * \param p Input parameters containing the new email address.
     * \return \c added email address
     */
    QString addEmail(Cutelyst::Context *c, SkaffariError *e, const QVariantHash &p);

    /*!
     * \brief Removes an email address from the account pointed to by \a a.
     * \param c         Pointer to the current context, used for string translation and user authentication.
     * \param e         Pointer to an object taking information about occurring errors.
     * \param address   The address that should be removed.
     * \return \c true on success.
     */
    bool removeEmail(Cutelyst::Context *c, SkaffariError *e, const QString &address);

    /*!
     * \brief Adds a new forward address to the account pointed to by \a a.
     *
     * If adding the forward email address fails, the SkaffariError object pointed to by \a e will contain information
     * about occurred errors.
     *
     * \param c Pointer to the current context, used for string translation and user authentication.
     * \param e Pointer to an object taking information about occurring errors.
     * \param a Pointer to the Account object the new forward address should be added to.
     * \param p INput parameters containing the new forward email address.
     * \return \c true on success.
     */
    static bool addForward(Cutelyst::Context *c, SkaffariError *e, Account *a, const Cutelyst::ParamsMultiMap &p);

    /*!
     * \brief Removes an forward email address from the account pointed to by \a a.
     * \param c         Pointer to the current context, used for string translation and user authentication.
     * \param e         Pointer to an object taking information about occurring errors.
     * \param a         Pointer to the Account object the forward address should be deleted from.
     * \param forward   The forward address that should be removed.
     * \return \c true on success
     */
    static bool removeForward(Cutelyst::Context *c, SkaffariError *e, Account *a, const QString &forward);

    /*!
     * \brief Edits a forward address on the account pointer to by \a a.
     * \param c             Pointer to the current context, used for string translation and user authentication.
     * \param e             Pointer to an object taking information about occurring errors.
     * \param a             Pointer to the Account object the forward address should edited on.
     * \param oldForward    Old forward address that should be changed.
     * \param newForward    New forward address the old one should be changed to.
     * \return              \c true on succes
     */
    static bool editForward(Cutelyst::Context *c, SkaffariError *e, Account *a, const QString &oldForward, const QString &newForward);

    /*!
     * \brief Changes the keeping of forwarded mails of account \a to \a keepLocal.
     * \param c         Pointer to the current context, used for string translation and user authentication.
     * \param e         Pointer to an object taking information about occurring errors.
     * \param a         Pointer to the Account object the keepLocal setting should changed on.
     * \param keepLocal Set to \c true if forwarded mails should be kept in local mail box, to \c false otherwise.
     * \return          \c true on success
     */
    static bool changeKeepLocal(Cutelyst::Context *c, SkaffariError *e, Account *a, bool keepLocal);

    /*!
     * \brief Converts an email address from ACE into UTF-8.
     */
    static QString addressFromACE(const QString &address);

    /*!
     * \brief Converts an email address from UTF-8 to ACE.
     */
    static QString addressToACE(const QString &address);

    /*!
     * \brief Calculates the account status based on the expiration times.
     * \param validUntil    account expiration date and time
     * \param pwExpires     password expiration date and time
     * \return              status flags as integer value
     */
    static quint8 calcStatus(const QDateTime validUntil, const QDateTime pwExpires);

    /*!
     * \brief Splits an email address into local and domain part.
     * \param address The email address to split.
     * \return String pair where first contains the local part and second the domain part.
     */
    static std::pair<QString,QString> addressParts(const QString &address);

protected:
    QSharedDataPointer<AccountData> d;
};

Q_DECLARE_METATYPE(Account)
Q_DECLARE_TYPEINFO(Account, Q_MOVABLE_TYPE);

GRANTLEE_BEGIN_LOOKUP(Account)
QVariant var;
if (property == QLatin1String("id")) {
    var.setValue(object.id());
} else if (property == QLatin1String("domainId")) {
    var.setValue(object.domainId());
} else if (property == QLatin1String("username")) {
    var.setValue(object.username());
} else if (property == QLatin1String("prefix")) {
    var.setValue(object.prefix());
} else if (property == QLatin1String("domainName")) {
    var.setValue(object.domainName());
} else if (property == QLatin1String("imap")) {
    var.setValue(object.isImapEnabled());
} else if (property == QLatin1String("pop")) {
    var.setValue(object.isPopEnabled());
} else if (property == QLatin1String("sieve")) {
    var.setValue(object.isSieveEnabled());
} else if (property == QLatin1String("smtpauth")) {
    var.setValue(object.isSmtpauthEnabled());
} else if (property == QLatin1String("addresses")) {
    var.setValue(object.addresses());
} else if (property == QLatin1String("forwards")) {
    var.setValue(object.forwards());
} else if (property == QLatin1String("quota")) {
    var.setValue(object.quota());
} else if (property == QLatin1String("usage")) {
    var.setValue(object.usage());
} else if (property == QLatin1String("usagePercent")) {
    var.setValue(object.usagePercent());
} else if (property == QLatin1String("isValid")) {
    var.setValue(object.isValid());
} else if (property == QLatin1String("created")) {
    var.setValue(object.created());
} else if (property == QLatin1String("updated")) {
    var.setValue(object.updated());
} else if (property == QLatin1String("validUntil")) {
    var.setValue(object.validUntil());
} else if (property == QLatin1String("keepLocal")) {
    var.setValue(object.keepLocal());
} else if (property == QLatin1String("catchAll")) {
    var.setValue(object.catchAll());
} else if (property == QLatin1String("passwordExpires")) {
    var.setValue(object.passwordExpires());
} else if (property == QLatin1String("passwordExpired")) {
    var.setValue(object.passwordExpired());
} else if (property == QLatin1String("expired")) {
    var.setValue(object.expired());
}
return var;
GRANTLEE_END_LOOKUP

#endif // ACCOUNT_H
