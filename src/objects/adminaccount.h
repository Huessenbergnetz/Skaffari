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

#ifndef SKAFFARI_ADMINACCOUNT_H
#define SKAFFARI_ADMINACCOUNT_H

#include "domain.h"
#include "../../common/global.h"
#include <Cutelyst/ParamsMultiMap>
#include <QString>
#include <QSharedDataPointer>
#include <QVariant>
#include <QCryptographicHash>
#include <QJsonObject>
#include <vector>

namespace Cutelyst {
class Context;
class AuthenticationUser;
}

class AdminAccountData;
class SkaffariError;

/*!
 * \ingroup skaffaricore
 * \brief Represents an administrator account.
 *
 * The %AdminAccount class represents an administrator account object.
 * There are currently three types of admins, super users, administrators and domain managers.
 *
 * Superusers can modify anything and can also create new domains and deleting existing
 * ones, as well as adding and removing new admin accounts.
 *
 * Administratos can create new domains and domain managers, but can not create other
 * administrators or super users.
 *
 * Domain managers can only modify accounts in domains they are responsible for.
 */
class AdminAccount
{
    Q_GADGET
    /*!
     * \brief %Database ID of the administrator account.
     */
    Q_PROPERTY(dbid_t id READ id CONSTANT)
    /*!
     * \brief %Username of the administrator account.
     */
    Q_PROPERTY(QString username READ username CONSTANT)
    /*!
     * \brief List of %domain %database IDs this admin account is responsible for.
     *
     * Only used if the admin account is of type DomainMaster.
     */
    Q_PROPERTY(QList<dbid_t> domains READ domains CONSTANT)
    /*!
     * \brief Type of this admin account.
     */
    Q_PROPERTY(AdminAccount::AdminAccountType type READ type CONSTANT)
    /*!
     * \brief Type of this admin account as stringified integer value.
     */
    Q_PROPERTY(QString typeStr READ typeStr CONSTANT)
    /*!
     * \brief \c true if this admin account is a super user account.
     */
    Q_PROPERTY(bool isSuperUser READ isSuperUser CONSTANT)
    /*!
     * \brief \c true if this admin accout seems to be valid.
     *
     * Note that this does not do a real database check. It simply checks
     * if the database id is greater than \c 0 and if the username is not empty.
     */
    Q_PROPERTY(bool isValid READ isValid CONSTANT)
    /*!
     * \brief The language selected by this admin account as string accepted by QLocale.
     */
    Q_PROPERTY(QString lang READ lang CONSTANT)
    /*!
     * \brief The time zone selected by this admin accout as IANA time zone ID.
     */
    Q_PROPERTY(QString tz READ tz CONSTANT)
    /*!
     * \brief The maximum number of items to display per page set by this admin.
     *
     * Used on pages where results - especially lists - are distributed on multiple pages.
     */
    Q_PROPERTY(quint8 maxDisplay READ maxDisplay CONSTANT)
    /*!
     * \brief The warn level set by this admin.
     *
     * Used on element that display a usage ratio like quotas.
     */
    Q_PROPERTY(quint8 warnLevel READ warnLevel CONSTANT)
    /*!
     * \brief Date and time this admin account has been created.
     */
    Q_PROPERTY(QDateTime created READ created CONSTANT)
    /*!
     * \brief Date and time thia dmin account has been updated the last time.
     */
    Q_PROPERTY(QDateTime updated READ updated CONSTANT)
public:
    /*!
     * \brief This enum describes the type of the admin account.
     */
    enum AdminAccountType : quint8 {
        Disabled        = 0,    /**< A disabled admin account. */
        DomainMaster    = 127,  /**< A domain manager that can only modify certain domains */
        Administrator   = 254,  /**< An administartor account that can create new domain masters and domains, but not other administrators or super users */
        SuperUser       = 255   /**< A super user account, that has all rights, especially that can create all other admin accounts */
    };
    Q_ENUM(AdminAccountType)

    /*!
     * \brief Constructs an empty administrator account object.
     *
     * isValid() will return \c false for this default constructed object.
     */
    AdminAccount();

    /*!
     * \brief Constructs a new administrator account object from the given parameters.
     *
     * \param id        The database id of the admin account.
     * \param username  The user name of the administrator
     * \param type      The admin account type.
     * \param domains   List of domain %database IDs this admin is responsible for.
     */
    AdminAccount(dbid_t id, const QString &username, AdminAccountType type, const QList<dbid_t> &domains);

    /*!
     * \brief Constructs a new administrator account object from the given parameters.
     *
     * \param id        The database id of the admin account.
     * \param username  The user name of the administrator
     * \param type      The admin account type as integer value.
     * \param domains   List of domain %database IDs this admin is responsible for.
     */
    AdminAccount(dbid_t id, const QString& username, quint8 type, const QList<dbid_t> &domains);

    /*!
     * \brief Constructs a new administrator account object from \a user.
     */
    AdminAccount(const Cutelyst::AuthenticationUser &user);

    /*!
     * \brief Constucts a new administrator account object from the given parameters.
     *
     * \param id            The database ID of the admin account.
     * \param username      The user name of the administrator.
     * \param type          The admin account type.
     * \param domains       List of domain database IDs this admin is responsible for.
     * \param tz            The IANA time zone ID string selected by this admin.
     * \param lang          The language code used by QLocale selected by this admin.
     * \param tmpl          The template selected by this admin.
     * \param maxDisplay    The maximum items to display per page for this admin.
     * \param warnLevel     The warn level selected by this admin.
     * \param created       Date and time this admin account has been created.
     * \param updated       Date and time this admin account was updated the last time.
     */
    AdminAccount(dbid_t id, const QString &username, AdminAccountType type, const QList<dbid_t> &domains, const QString &tz, const QString &lang, const QString &tmpl, quint8 maxDisplay, quint8 warnLevel, const QDateTime &created, const QDateTime &updated);

    /*!
     * \brief Constructs a copy of other.
     */
    AdminAccount(const AdminAccount &other);

    /*!
     * \brief Move-constructs an %AdminAccount instance, making it point at the same object that \a other pointing to.
     */
    AdminAccount(AdminAccount &&other) noexcept;

    /*!
     * \brief Assigns other to this %AdminAccount and returns a reference to this account.
     */
    AdminAccount &operator=(const AdminAccount &other);

    /*!
     * \brief Move-assigns \a other to this %AdminAccount instance.
     */
    AdminAccount &operator=(AdminAccount &&other) noexcept;

    /*!
     * \brief Destroys the object
     */
    ~AdminAccount();

    /*!
     * \brief Swaps this %AdminAccount instance with \a other.
     */
    void swap(AdminAccount &other) noexcept;

    /*!
     * \brief Returns the database ID.
     */
    dbid_t id() const;

    /*!
     * \brief Returns the username of this admin account.
     */
    QString username() const;

    /*!
     * \brief Returns a string that contains the username and the database ID.
     *
     * This will mostly be used in log messages and returns a string like
     * <code>"bigadmin (ID: 123)"</code>.
     */
    QString nameIdString() const;

    /*!
     * \brief Returns the domain IDs this admin is responsible for.
     */
    QList<dbid_t> domains() const;

    /*!
     * \brief Returns the type of this admin account.
     * \sa typeStr(), typeInt()
     */
    AdminAccountType type() const;
    /*!
     * \brief Returns the humand readable name of the account type.
     * The context \a c is required for translations.
     */
    QString typeName(Cutelyst::Context *c) const;
    /*!
     * \brief Returns the integer representation of the currently set account type.
     * \sa type()
     */
    quint8 typeInt() const;
    /*!
     * \brief Returns the type as string containing the number value.
     * \sa type()
     */
    QString typeStr() const;

    /*!
     * \brief Returns \c true if this admin is of type SuperUser.
     */
    bool isSuperUser() const;

    /*!
     * \brief Returns the language set for this admin.
     */
    QString lang() const;

    /*!
     * \brief Returns the time zone ID for this admin.
     */
    QString tz() const;

    /*!
     * \brief Returns the date and time this admin account has been created.
     */
    QDateTime created() const;

    /*!
     * \brief Returns the date and tim this admin account has been created.
     */
    QDateTime updated() const;

    /*!
     * \brief Returns the maximum number of list entries to display per page.
     */
    quint8 maxDisplay() const;

    /*!
     * \brief Returns the warn level in percent for displaying limits like quota usage.
     */
    quint8 warnLevel() const;

    /*!
     * \brief Returns the template name to use.
     * Currently not in use.
     */
    QString getTemplate() const;

    /*!
     * \brief Returns \c true if the account seems to be valid.
     *
     * There is no test against the databse. It only checks if username
     * and id are correctly set.
     */
    bool isValid() const;

    /*!
     * \brief Returns the data as JSON object.
     *
     * The returned JSON object will contain all properties of the %AdminAccount object
     * except typeStr and type will be stored as integer value.
     *
     * If the %AdminAccount is not valid (isValid() returns \c false), the returned
     * JSON object will be empty.
     */
    QJsonObject toJson() const;

    /*!
     * \brief Creates a new account with the given \a params and returns it.
     *
     * The returned account might be invalid if an error occurred.
     *
     * \par Keys for the params
     * Key          | Converted Type | Description
     * -------------|----------------|------------------------------
     * username     | QString        | the user name for the new administrator account, will be trimmed
     * password     | QString        | the password for the new administrator account, will be stored with PBKDF2
     * type         | quint8         | the type of the new administrator account, see AdminAccount::AdminAccountType
     * assocdomains | QStringList    | list of domains this account will be associated to if the type is AdminAccount::DomainMaster
     *
     * \param c         pointer to the current context, used for translating strings
     * \param params    parameters for the new accounts
     * \param error     object taking error information
     */
    static AdminAccount create(Cutelyst::Context *c, const QVariantHash &params, SkaffariError &error);

    /*!
     * \brief Returns a list of all administrator accounts from the database.
     * \param c     pointer to the current context, used for translating strings
     * \param error object taking error information
     */
    static std::vector<AdminAccount> list(Cutelyst::Context *c, SkaffariError &error);

    /*!
     * \brief Returns a single administrator account from the database identified by the database \a id.
     * \param c     pointer to the current context, used for translating strings
     * \param e     object taking error information
     * \param id    database id of the administrator account to get
     */
    static AdminAccount get(Cutelyst::Context *c, SkaffariError &e, dbid_t id);

    /*!
     * \brief Updates the administrator account with the new \a params and returns \c true on success.
     *
     * \par Keys for the params
     * Key          | Converted Type | Description
     * -------------|----------------|---------------------------------------
     * password     | QString        | new password, will be stored with PBKDF2, if empty, password will not be changed
     * type         | quint8         | new type, see AdminAccount::AdminAccountType
     * assocdomains | QStringList    | list of domains this account will be associated to if the type is AdminAccount::DomainMaster
     *
     * \param c         pointer to the current context, used for translating strings
     * \param e         object taking error information
     * \param params    parameters used to update the account
     * \return \c true on success
     */
    bool update(Cutelyst::Context *c, SkaffariError &e, const QVariantHash &params);

    /*!
     * \brief Updates the account of the currently authenticated administrator \a u with the new parameters \a p and returns \c true on success.
     *
     * \par Keys for the params
     * Key          | Converted Type | Description
     * -------------|----------------|---------------------------------------
     * password     | QString        | new password, will be stored with PBKDF2, if empty, password will not be changed
     * maxdisplay   | quint8         | new value for maximum numbuer of items to display in lists
     * warnlevel    | quint8         | new warn level in percent for things like quota constraints
     * lang         | QString        | new desplay language code
     * tz           | QString        | new time zone value
     *
     * \param c pointer to the current context, used for translating strings
     * \param e object taking error information
     * \param p parameters used to update the account
     * \return \c true on success
     */
    bool updateOwn(Cutelyst::Context *c, SkaffariError &e, const QVariantHash &p);

    /*!
     * \brief Removes the administrator account and returns \c true on success.
     * \param c pointer to the current context, used for translating strings
     * \param e object taking error information
     * \return \c true on success
     */
    bool remove(Cutelyst::Context *c, SkaffariError &e);

    /*!
     * \brief Puts the admin account identified by \a adminId into the stash of the current context \a c.
     * \param c         pointer to the current context
     * \param adminId   the database id of the admin account
     */
    static void toStash(Cutelyst::Context *c, dbid_t adminId);

    /*!
     * \brief Puts \a adminAccount into the stash.
     * \param c             pointer to the current context
     * \param adminAccount  the admin account to put into the stash
     */
    static void toStash(Cutelyst::Context *c, const AdminAccount &adminAccount);

    /*!
     * \brief Returns the admin account from current context.
     * \param c pointer to the current context
     */
    static AdminAccount fromStash(Cutelyst::Context *c);

    /*!
     * \brief Returns the admin user type associated with the integer \a type.
     */
    static AdminAccount::AdminAccountType getUserType(quint8 type);

    /*!
     * \brief Returns the type of the \a user.
     */
    static AdminAccount::AdminAccountType getUserType(const Cutelyst::AuthenticationUser &user);

    /*!
     * \brief Returns the type of the currently logged in user.
     * \param c pointer to the current context
     */
    static AdminAccount::AdminAccountType getUserType(Cutelyst::Context *c);

    /*!
     * \brief Returns the type of the %AdminAccount stored in the \a user variant.
     */
    static AdminAccount::AdminAccountType getUserType(const QVariant &user);

    /*!
     * \brief Returns the currenlty logged in user.
     * \param c pointer to the current context
     */
    static AdminAccount getUser(Cutelyst::Context *c);

    /*!
     * \brief Returns the database ID of the currently logged in admin user.
     * \param c pointer to the current context
     */
    static dbid_t getUserId(Cutelyst::Context *c);

    /*!
     * \brief Returns the user name of the currently logged in database user.
     * \param c pointer to the current context
     */
    static QString getUserName(Cutelyst::Context *c);

    /*!
     * \brief Returns a string that contains database ID and user name of the currently logged in user.
     * \param c pointer to the current context
     */
    static QString getUserNameIdString(Cutelyst::Context *c);

    /*!
     * \brief Returns a translated name for the user \a type.
     * \param type  the type to get the name for
     * \param c     pointer to the current context, used for translation
     */
    static QString typeToName(AdminAccount::AdminAccountType type, Cutelyst::Context *c);

    /*!
     * \brief Returns a list of integer strings that show the allowed types a user with \a userType can create.
     */
    static QStringList allowedTypes(AdminAccount::AdminAccountType userType);

    /*!
     * \brief Returns a list of integer strings that show the types the currently logged in user can create.
     * \param c pointer to the current context
     */
    static QStringList allowedTypes(Cutelyst::Context *c);

    /*!
     * \brief Returns the maximum allowed type a user with \a userType can create.
     */
    static AdminAccount::AdminAccountType maxAllowedType(AdminAccount::AdminAccountType userType);

    /*!
     * \brief Returns the maximum allowed type the current logged in user can created.
     * \param c pointer to the current context
     */
    static AdminAccount::AdminAccountType maxAllowedType(Cutelyst::Context *c);

protected:
    QSharedDataPointer<AdminAccountData> d;

    friend QDataStream &operator>>(QDataStream &stream, AdminAccount &account);
    friend QDataStream &operator<<(QDataStream &stream, const AdminAccount &account);
};

Q_DECLARE_METATYPE(AdminAccount)
Q_DECLARE_TYPEINFO(AdminAccount, Q_MOVABLE_TYPE);

/*!
 * \relates AdminAccount
 * \brief Writes the admin \a account to the \a dbg stream and returns the stream.
 */
QDebug operator<<(QDebug dbg, const AdminAccount &account);

/*!
 * \relates AdminAccount
 * \brief Reads an %AdminAccount from the given \a stream and stores it in the given \a account.
 */
QDataStream &operator>>(QDataStream &stream, AdminAccount &account);

/*!
 * \relates AdminAccount
 * \brief Writes the given \a account to the given \a stream.
 */
QDataStream &operator<<(QDataStream &stream, const AdminAccount &account);

#endif // SKAFFARI_ADMINACCOUNT_H
