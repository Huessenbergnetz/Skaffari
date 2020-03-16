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

#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "../../common/global.h"
#include "domain.h"
#include <Cutelyst/ParamsMultiMap>
#include <Cutelyst/Plugins/Utils/Pagination>
#include <QSharedDataPointer>
#include <QString>
#include <QStringList>
#include <QLoggingCategory>
#include <QDateTime>
#include <QJsonObject>
#include <utility>

Q_DECLARE_LOGGING_CATEGORY(SK_ACCOUNT)

namespace Cutelyst {
class Context;
}

class AccountData;
class SkaffariError;

/*!
 * \ingroup skaffariobjects
 * \brief Represents an user account
 *
 * An user account contains information about a single email user account, inlcuding associated addresses and forwards.
 */
class Account
{
    Q_GADGET
    /*!
     * \brief Database ID of the account.
     */
    Q_PROPERTY(dbid_t id READ id CONSTANT)
    /*!
     * \brief Database ID of the domain this account belongs to.
     */
    Q_PROPERTY(dbid_t domainId READ domainId CONSTANT)
    /*!
     * \brief User name for this account.
     */
    Q_PROPERTY(QString username READ username CONSTANT)
    /*!
     * \brief \c true if IMAP access for this account is enabled.
     */
    Q_PROPERTY(bool imap READ isImapEnabled CONSTANT)
    /*!
     * \brief \c true if POP3 access for this account is enabled.
     */
    Q_PROPERTY(bool pop READ isPopEnabled CONSTANT)
    /*!
     * \brief \c true if SIEVE access for this account is enabled.
     */
    Q_PROPERTY(bool sieve READ isSieveEnabled CONSTANT)
    /*!
     * \brief \c true if SMTP access for this account is enabled.
     */
    Q_PROPERTY(bool smtpauth READ isSmtpauthEnabled CONSTANT)
    /*!
     * \brief The email addresses that are connected to this account.
     */
    Q_PROPERTY(QStringList addresses READ addresses CONSTANT)
    /*!
     * \brief The email forwards that are defined for this account.
     */
    Q_PROPERTY(QStringList forwards READ forwards CONSTANT)
    /*!
     * \brief The quota defined for this account in KiB.
     */
    Q_PROPERTY(quota_size_t quota READ quota CONSTANT)
    /*!
     * \brief The quota used by this account in KiB.
     */
    Q_PROPERTY(quota_size_t usage READ usage CONSTANT)
    /*!
     * \brief Percentage value of the used quota.
     */
    Q_PROPERTY(float usagePercent READ usagePercent CONSTANT)
    /*!
     * \brief \c true if this account is valid.
     *
     * An account is valid if the account’s database ID, returned via getId(), and the
     * domain database ID both return values greater than \c 0.
     */
    Q_PROPERTY(bool isValid READ isValid CONSTANT)
    /*!
     * \brief UTC date and time this account has been created.
     */
    Q_PROPERTY(QDateTime created READ created CONSTANT)
    /*!
     * \brief UTC date and time this account has been updated the last time.
     */
    Q_PROPERTY(QDateTime updated READ updated CONSTANT)
    /*!
     * \brief UTC date and time until this account is valid.
     */
    Q_PROPERTY(QDateTime validUntil READ validUntil CONSTANT)
    /*!
     * \brief \c true if forwarded emails should be kept local too.
     */
    Q_PROPERTY(bool keepLocal READ keepLocal CONSTANT)
    /*!
     * \brief \c true if this account is used to catch all incoming emails for the domain.
     *
     * A catch-all account will receive all emails for a domain for that does not exist a
     * separate address.
     */
    Q_PROPERTY(bool catchAll READ catchAll CONSTANT)
    /*!
     * \brief UTC date and time when the current password of this user expires.
     */
    Q_PROPERTY(QDateTime passwordExpires READ passwordExpires CONSTANT)
    /*!
     * \brief \c true if the password for this account has been expired.
     *
     * A password has been expired if the date and time set in passwordExpires are after
     * the current date and time.
     */
    Q_PROPERTY(bool passwordExpired READ passwordExpired CONSTANT)
    /*!
     * \brief \c true if the validity of this account has been expired.
     *
     * An account has been expired if the date and time set in validUntil are after
     * the current date and time.
     */
    Q_PROPERTY(bool expired READ expired CONSTANT)
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
    Account(dbid_t id, dbid_t domainId, const QString &username, bool imap, bool pop, bool sieve, bool smtpauth, const QStringList &addresses, const QStringList &forwards, quota_size_t quota, quota_size_t usage, const QDateTime &created, const QDateTime &updated, const QDateTime &validUntil, const QDateTime &pwdExpiration, bool keepLocal, bool catchAll, quint8 status);

    /*!
     * \brief Creates a copy of \a other.
     */
    Account(const Account &other);

    /*!
     * \brief Move-constructs an %Account instance, making it point to the same object that \a other was pointing to.
     */
    Account(Account &&other) noexcept;

    /*!
     * \brief Assigns \a other to this %Account.
     */
    Account& operator=(const Account &other);

    /*!
     * \brief Move-assigns \a other to this %Account instance.
     */
    Account& operator=(Account &&other) noexcept;

    /*!
     * \brief Destroys the %Account object and frees any allocated resources.
     */
    ~Account();

    /*!
     * \brief Swaps this %Account instance with \a other.
     */
    void swap(Account &other) noexcept;

    /*!
     * \brief Strategy for mailbox creation on IMAP server.
     */
    enum CreateMailbox : quint8 {
        DoNotCreate         = 0,    /**< %Skaffari will not create anything on the IMAP server. */
        LoginAfterCreation  = 1,    /**< %Skaffari will login to the new account after creation to let the IMAP server create the mailbox and set the quota. */
        OnlySetQuota        = 2,    /**< %Skaffari will login to the new account after creation to let the IMAP server create the mailbox and will than set the account quota. */
        CreateBySkaffari    = 3     /**< %Skaffari will create the account, including the domain's default folders and will set the quota. */
    };

    /*!
     * \brief Returns the account database ID.
     *
     * Access from Grantlee: id
     */
    dbid_t id() const;

    /*!
     * \brief Returns the database ID of the domain the account belongs to.
     *
     * Access from Grantlee: domainId
     */
    dbid_t domainId() const;

    /*!
     * \brief Returns the username of the account.
     *
     * Access from Grantlee: username
     */
    QString username() const;

    /*!
     * \brief Returns a string that contains the username and the database ID.
     *
     * This will mostly be used in log messages and returns a string like
     * <code>"biguser (ID: 123)"</code>.
     */
    QString nameIdString() const;

    /*!
     * \brief Returns \c true if IMAP access is enabled for this account.
     *
     * Access from Grantlee: imap
     */
    bool isImapEnabled() const;

    /*!
     * \brief Returns \c true if POP3 access is enabled for this account.
     *
     * Access from Grantlee: pop
     */
    bool isPopEnabled() const;

    /*!
     * \brief Returns \c true if Sieve access is enabled for this account.
     *
     * Access from Grantlee: sieve
     */
    bool isSieveEnabled() const;

    /*!
     * \brief Returns \c true if sending emails via SMTP is enabled for this account.
     *
     * Access from Grantlee: smtpauth
     */
    bool isSmtpauthEnabled() const;

    /*!
     * \brief Returns the email addresses that are connected to this account.
     *
     * Access from Grantlee: addresses
     */
    QStringList addresses() const;

    /*!
     * \brief Returns the emal forwards that are defined for this account.
     *
     * Access from Grantlee: forwards
     */
    QStringList forwards() const;

    /*!
     * \brief Returns the quota defined for this account in KiB.
     *
     * Access from Grantlee: quota
     */
    quota_size_t quota() const;

    /*!
     * \brief Returns the quota used by this account in KiB.
     *
     * Access from Grantlee: usage
     */
    quota_size_t usage() const;

    /*!
     * \brief Returns a percentage value of the used quota.
     *
     * Access from Grantlee: usagePercent
     */
    float usagePercent() const;

    /*!
     * \brief Returns \c true if this account is valid.
     *
     * An account is valid if the account’s database ID, returned via getId(), and the
     * domain database ID both return values greater than \c 0.
     *
     * Access from Grantlee: isValid
     */
    bool isValid() const;

    /*!
     * \brief Returns the date and time this account has been created.
     *
     * Access from Grantlee: created
     */
    QDateTime created() const;

    /*!
     * \brief Returns the date and time this account has been updated the last time.
     *
     * Access from Grantlee: updated
     */
    QDateTime updated() const;

    /*!
     * \brief Returns the date and time until this account is valid.
     *
     * Access from Grantlee: validUntil
     */
    QDateTime validUntil() const;

    /*!
     * \brief Returns \c true if forwarded emails should be kept local too.
     *
     * Access from Grantlee: keepLocal
     */
    bool keepLocal() const;

    /*!
     * \brief Returns \c true if this account is for catch-all.
     *
     * Access from Grantlee: catchAll
     */
    bool catchAll() const;

    /*!
     * \brief Returns the date and time when the current password of this user expires.
     *
     * Access from Grantlee: passwordExpires
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
     */
    quint8 status() const;

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
     * \param e Object taking information about occurring errors.
     * \param p Input parameters containting information about the new account.
     * \param d The domain the account should be created in.
     * \return A valid Account object on success.
     */
    static Account create(Cutelyst::Context *c, SkaffariError &e, const QVariantHash &p, const Domain &d, const QStringList &selectedKids);

    /*!
     * \brief Removes the account from the database and the IMAP server.
     *
     * Will at first try to delete the account's mailbox from the IMAP server, if that succeeds, the account will
     * be deleted from the database.
     *
     * If deletion fails, the SkaffariError object pointed to by \a e will contain information about the error.
     *
     * \param c         Pointer to the current context, used for string translation and user authentication.
     * \param e         Object taking information about occurring errors.
     * \param domain    The domain the account to delete belongs to.
     * \return \c true on success.
     */
    bool remove(Cutelyst::Context *c, SkaffariError &e) const;

    /*!
     * \brief Lists all accounts belonging to the domain \a d.
     * \param c             Pointer to the current context, used for string translation and user authentication.
     * \param e             Object taking information about occurring errors.
     * \param d             The domain you want to list the accounts from.
     * \param p             Contains information about the pagination.
     * \param sortBy        Column to sort the accounts by.
     * \param sortOrder     Order to sort the accounts by, valid values: ASC, DESC
     * \param searchRole    The column to search in for the \a searchString.
     * \param searchString  The string to search for in the column defined by \a searchRole.
     * \return A pagination object containing information about the pagination and the list of accounts.
     */
    static Cutelyst::Pagination list(Cutelyst::Context *c, SkaffariError &e, const Domain &d, const Cutelyst::Pagination &p, const QString &sortBy = QStringLiteral("username"), const QString &sortOrder = QStringLiteral("ASC"), const QString &searchRole = QStringLiteral("username"), const QString &searchString = QString());

    /*!
     * \brief Gets the account defined by database ID \a id from the database.
     *
     * You should use isValid() to check if the request succeeded. If not, the SkaffariError object point to
     * by \a e will contain information about occurred errors.
     *
     * \param c     Pointer to the current context, used for string translation, user authentication and to set the stash.
     * \param e     Object taking information about occurring errors.
     * \param id    The database ID of the account.
     * \return      Account object containing the account data.
     */
    static Account get(Cutelyst::Context *c, SkaffariError &e, dbid_t id);

    /*!
     * \brief Requests information about the account defined by \a accountId from the database and adds it to the stash.
     * \param c         Pointer to the current context, used for string translation and user authentication.
     * \param accountId The database ID of the account.
     * \sa fromStash()
     */
    static void toStash(Cutelyst::Context *c, dbid_t accountId);

    /*!
     * \brief Puts this account into the context stash.
     * If the account it not valid, a 404 error will be returned and the processing
     * will be detached to the Root::error() method.
     * \param c Pointer to the current context.
     * \return \c true if the account was valid, otherwise \c false.
     */
    bool toStash(Cutelyst::Context *c) const;

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
     * \param e Object taking information about occurring errors.
     * \param dom Pointer to an Domain object that will have it's used domain quota updated.
     * \param p Updated parameters for this account.
     * \return \c true on success.
     */
    bool update(Cutelyst::Context *c, SkaffariError &e, Domain *dom, const QVariantHash &p);

    /*!
     * \brief Checks if all account data is available and creates missing data.
     *
     * This will for example check for missing mailbox on the IMAP server and wrong or missing storage quotas.
     *
     * \param c         Pointer to the current context, used for string translation and user authentication.
     * \param e         Object taking information about occurring errors.
     * \param domain    Object containing information about the domain the account belongs to.
     * \param p         Man of input parameters containing checks to perform.
     * \return List of actions performed for this account.
     */
    QStringList check(Cutelyst::Context *c, SkaffariError &e, const Domain &domain = Domain(), const Cutelyst::ParamsMultiMap &p = Cutelyst::ParamsMultiMap());

    /*!
     * \brief Updates a single email address connected to the account pointed to by \a a.
     *
     * If the update fails, the SkaffariError object pointed to by \a e will contain information about occurred errors.
     *
     * \param c             Pointer to the current context, used for string translation and user authentication.
     * \param e             Object taking information about occurring errors.
     * \param p             Input parameters containing the updated email address.
     * \param oldAddress    The old email address.
     * \return \c updated email address
     */
    QString updateEmail(Cutelyst::Context *c, SkaffariError &e, const QVariantHash &p, const QString &oldAddress);

    /*!
     * \brief Adds a new email address to the account.
     *
     * If adding the email address fails, the SkaffariError object pointed to by \a e will contain information
     * about occurred errors.
     *
     * \param c Pointer to the current context, used for string translation and user authentication.
     * \param e Object taking information about occurring errors.
     * \param p Input parameters containing the new email address.
     * \return \c added email address
     */
    QString addEmail(Cutelyst::Context *c, SkaffariError &e, const QVariantHash &p);

    /*!
     * \brief Removes an email address from the account pointed to by \a a.
     * \param c         Pointer to the current context, used for string translation and user authentication.
     * \param e         Object taking information about occurring errors.
     * \param address   The address that should be removed.
     * \return \c true on success.
     */
    bool removeEmail(Cutelyst::Context *c, SkaffariError &e, const QString &address);

    /*!
     * \brief Adds a new \a forward address to the account pointed to by \a a.
     *
     * If adding the forward email address fails, the SkaffariError object pointed to by \a e will contain information
     * about occurred errors.
     *
     * \param c Pointer to the current context, used for string translation and user authentication.
     * \param e Object taking information about occurring errors.
     * \param forward The new forward to add.
     * \return \c true on success.
     */
    bool addForward(Cutelyst::Context *c, SkaffariError &e, const QString &forward);

    /*!
     * \brief Removes a \a forward email address from the account.
     * \param c         Pointer to the current context, used for string translation and user authentication.
     * \param e         Object taking information about occurring errors.
     * \param forward   The forward address that should be removed.
     * \return \c true on success
     */
    bool removeForward(Cutelyst::Context *c, SkaffariError &e, const QString &forward);

    /*!
     * \brief Edits a forward address on the account.
     * \param c             Pointer to the current context, used for string translation and user authentication.
     * \param e             Object taking information about occurring errors.
     * \param oldForward    Old forward address that should be changed.
     * \param newForward    New forward address the old one should be changed to.
     * \return              \c true on succes
     */
    bool editForward(Cutelyst::Context *c, SkaffariError &e, const QString &oldForward, const QString &newForward);

    /*!
     * \brief Changes the keeping of forwarded mails of the account to \a keepLocal.
     * \param c         Pointer to the current context, used for string translation and user authentication.
     * \param e         Object taking information about occurring errors.
     * \param keepLocal Set to \c true if forwarded mails should be kept in local mail box, to \c false otherwise.
     * \return          \c true on success
     */
    bool changeKeepLocal(Cutelyst::Context *c, SkaffariError &e, bool keepLocal);

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
    static quint8 calcStatus(const QDateTime &validUntil, const QDateTime &pwExpires);

    /*!
     * \brief Splits an email address into local and domain part.
     * \param address The email address to split.
     * \return String pair where first contains the local part and second the domain part.
     */
    static std::pair<QString,QString> addressParts(const QString &address);

protected:
    QSharedDataPointer<AccountData> d;

private:
    /*!
     * \internal
     * \brief Sets the date and time the account has been last updated.
     */
    void markUpdated(Cutelyst::Context *c);

    friend QDataStream &operator>>(QDataStream &stream, Account &account);
    friend QDataStream &operator<<(QDataStream &stream, const Account &account);
};

Q_DECLARE_METATYPE(Account)
Q_DECLARE_TYPEINFO(Account, Q_MOVABLE_TYPE);

/*!
 * \relates Account
 * \brief Writes the \a account to the \a dbg stream and returns the stream.
 */
QDebug operator<<(QDebug dbg, const Account &account);

/*!
 * \relates Account
 * \brief Writes the given \a account to the given \a stream.
 */
QDataStream &operator<<(QDataStream &stream, const Account &account);

/*!
 * \relates Account
 * \brief Reads an %Account from the given \a stream and stores it in the given \a account.
 */
QDataStream &operator>>(QDataStream &stream, Account &account);

#endif // ACCOUNT_H
