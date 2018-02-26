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

#ifndef SKAFFARI_ADMINACCOUNT_H
#define SKAFFARI_ADMINACCOUNT_H

#include "domain.h"
#include "../../common/global.h"
#include <Cutelyst/ParamsMultiMap>
#include <grantlee5/grantlee/metatype.h>
#include <QString>
#include <QSharedDataPointer>
#include <QVariant>
#include <QCryptographicHash>
#include <QJsonObject>

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
 * There are currently two types of admins, administrators and domain managers.
 * Superusers can modify anything and can also create new domains and deleting existing
 * ones, as well as adding and removing new admin accounts.
 *
 * Domain admins can only modify accounts in domains they are responsible for.
 *
 * \par Grantlee accessors
 * Accessor    | Type             | Method
 * ------------|------------------|-----------------
 * created     | QDateTime        | created()
 * domains     | QList<dbid_t>    | domains()
 * id          | dbid_t           | id()
 * isSuperUser | bool             | isSuperUser()
 * isValid     | bool             | isValid()
 * lang        | QString          | lang()
 * maxDisplay  | quint8           | maxDisplay()
 * type        | quint8           | typeInt()
 * typeStr     | QString          | typeStr()
 * tz          | QByteArray       | tz()
 * updated     | QDateTime        | updated()
 * username    | QString          | username()
 * warnLevel   | quint8           | warnLevel()
 */
class AdminAccount
{
    Q_GADGET
public:
    /*!
     * \brief This enum describes the type of the admin account.
     *
     * AllAdmins is a special value, it is not defined to describe an account,
     * but to filter all admin accounts in specific functions.
     */
    enum AdminAccountType : quint8 {
        Disabled        = 0,
        DomainMaster    = 127,    /**< a domain manager, that can only modify certain domains */
        Administrator   = 254,  /**< an administartor account that can create new domain masters and domains, but not other administrators or super users */
        SuperUser       = 255     /**< a super user account, that has all rights, especially that can create all other admin accounts */
    };
    Q_ENUM(AdminAccountType)

    /*!
     * \brief Constructs an empty administrator account object.
     */
    AdminAccount();

    /*! \brief Constructs a new administrator account object.
     *
     * Constructs a new administrator account object from the given parameters.
     * Most important are the username and the password, by default it constructs
     * an account of type administrator. The type can be changed by the type parameter.
     * \param id the database id of the admin account
     * \param username the user name of the administrator
     * \param type the admin account type, can not be AllAdmins
     * \param domains list of domain IDs this admin is responsible for
     * \sa setUsername(), setType(), setDomains()
     */
    AdminAccount(dbid_t id, const QString &username, AdminAccountType type, const QList<dbid_t> &domains);

    /*!
     * \overload
     */
    AdminAccount(dbid_t id, const QString& username, quint8 type, const QList<dbid_t> &domains);

    /*!
     * \overload
     */
    AdminAccount(const Cutelyst::AuthenticationUser &user);

    /*!
     * \brief Constructs a copy of other.
     */
    AdminAccount(const AdminAccount &other);

    /*!
     * \brief Assigns other to this %AdminAccount and returns a reference to this account.
     */
    AdminAccount &operator=(const AdminAccount &other);

    /*!
     * \brief Destroys the object
     */
    ~AdminAccount();

    /*!
     * \brief Returns the database ID.
     * \sa setId()
     */
    dbid_t id() const;
    /*!
     * \brief Sets the database ID.
     * \sa id()
     */
    void setId(dbid_t id);

    /*!
     * \brief Returns the username of this admin account.
     * \sa setUsername()
     */
    QString username() const;
    /*!
     * \brief Sets the user name of this admin account.
     * \sa username()
     */
    void setUsername(const QString &nUsername);
    /*!
     * \brief Returns a string that contains the username and the database ID.
     *
     * This will mostly be used in log messages and returns a string like
     * <code>"bigadmin (ID: 123)"</code>.
     */
    QString nameIdString() const;

    /*!
     * \brief Returns the domain IDs this admin is responsible for.
     * \sa setDomains()
     */
    QList<dbid_t> domains() const;
    /*!
     * \brief Sets the list of domain IDs this admin is responsible for.
     * \sa domains()
     */
    void setDomains(const QList<dbid_t> &nDomains);

    /*!
     * \brief Returns the type of this admin account.
     * \sa setType()
     */
    AdminAccountType type() const;
    /*!
     * \brief Returns the humand readable name of the account type.
     * The context is required for translations.
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
     * \brief Sets the type of this admin account.
     * \sa type()
     */
    void setType(AdminAccountType nType);
    /*!
     * \brief Sets the type of this admin account.
     * Values that do not match an AdminAccountType value will
     * be set as Disabled.
     * \sa type()
     */
    void setType(quint8 nType);

    /*!
     * \brief Returns \c true if this admin is a super user (administrator, not domain manager).
     */
    bool isSuperUser() const;

    /*!
     * \brief Returns the language set for this admin.
     * \sa setLang()
     */
    QString lang() const;
    /*!
     * \brief Sets the language for this admin.
     * \sa lang()
     */
    void setLang(const QString &lang);

    /*!
     * \brief Returns the time zone ID for this admin.
     * \sa setTz()
     */
    QByteArray tz() const;
    /*!
     * \brief Sets the time zone ID for this admin.
     * \sa tz()
     */
    void setTz(const QByteArray &tz);

    /*!
     * \brief Returns the date and time this admin account has been created.
     * \sa setCreated()
     */
    QDateTime created() const;
    /*!
     * \brief Sets the date and time this admin account has been created.
     * \sa created()
     */
    void setCreated(const QDateTime &created);

    /*!
     * \brief Returns the date and tim this admin account has been created.
     * \sa setUpdated()
     */
    QDateTime updated() const;
    /*!
     * \brief Sets the date and time this admin account has been created.
     * \sa updated()
     */
    void setUpdated(const QDateTime &updated);

    /*!
     * \brief Returns the maximum number of list entries to display per page.
     * \sa setMaxDisplay()
     */
    quint8 maxDisplay() const;
    /*!
     * \brief Sets the maximum number of list entries to display per page.
     * \sa maxDisplay()
     */
    void setMaxDisplay(quint8 maxDisplay);

    /*!
     * \brief Returns the warn level in percent for displaying limits like quota usage.
     * \sa setWarnLevel()
     */
    quint8 warnLevel() const;
    /*!
     * \brief Sets the warn level in percent for displaying limits like quota usage.
     * \sa warnLevel()
     */
    void setWarnLevel(quint8 warnLevel);

    /*!
     * \brief Returns the template name to use.
     * Currently not in use.
     * \sa setTemplate()
     */
    QString getTemplate() const;
    /*!
     * \brief Sets the template name to use.
     * Currently not in use
     * \sa getTemplate()
     */
    void setTemplate(const QString &tmpl);

    /*!
     * \brief Returns \c true if the account seems to be valid.
     *
     * This function returns \c true, if the account seems to be valid. There is
     * no test against the databse. It only checks if user name and id are
     * correctly set.
     */
    bool isValid() const;

    /*!
     * \brief Returns the data as JSON object.
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
     * \param error     pointer to an object taking error information
     */
    static AdminAccount create(Cutelyst::Context *c, const QVariantHash &params, SkaffariError *error);

    /*!
     * \brief Returns a list of all administrator accounts from the database.
     * \param c     pointer to the current context, used for translating strings
     * \param error pointer to an object taking error information
     */
    static QVector<AdminAccount> list(Cutelyst::Context *c, SkaffariError *error);

    /*!
     * \brief Returns a single administrator account from the database identified by the database \a id.
     * \param c     pointer to the current context, used for translating strings
     * \param e     pointer to an object taking error information
     * \param id    database id of the administrator account to get
     */
    static AdminAccount get(Cutelyst::Context *c, SkaffariError *e, dbid_t id);

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
     * \param e         pointer to an object taking error information
     * \param params    parameters used to update the account
     * \return \c true on success
     */
    bool update(Cutelyst::Context *c, SkaffariError *e, const QVariantHash &params);

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
     * \param e pointer to an object taking error information
     * \param p parameters used to update the account
     * \return \c true on success
     */
    bool updateOwn(Cutelyst::Context *c, SkaffariError *e, const QVariantHash &p);

    /*!
     * \brief Removes the administrator account and returns \c true on success.
     * \param c pointer to the current context, used for translating strings
     * \param e pointer to an object taking error information
     * \return \c true on success
     */
    bool remove(Cutelyst::Context *c, SkaffariError *e);

    /*!
     * \brief Puts the admin account identified by \a adminId into the stash of the current context \a c.
     * \param c         pointer to the current context
     * \param adminId   the database id of the admin account
     */
    static void toStash(Cutelyst::Context *c, dbid_t adminId);

    static void toStash(Cutelyst::Context *c, const AdminAccount &adminAccount);

    /*!
     * \brief Returns the admin account from current context.
     * \param c pointer to the current context
     */
    static AdminAccount fromStash(Cutelyst::Context *c);

    static AdminAccount::AdminAccountType getUserType(quint8 type);

    static AdminAccount::AdminAccountType getUserType(const Cutelyst::AuthenticationUser &user);

    static AdminAccount::AdminAccountType getUserType(Cutelyst::Context *c);

    static AdminAccount::AdminAccountType getUserType(const QVariant &type);

    static dbid_t getUserId(Cutelyst::Context *c);

    static QString getUserName(Cutelyst::Context *c);

    static QString getUserNameIdString(Cutelyst::Context *c);

    static QString typeToName(AdminAccount::AdminAccountType type, Cutelyst::Context *c);

    static QStringList allowedTypes(Cutelyst::Context *c);

    static AdminAccount::AdminAccountType maxAllowedType(Cutelyst::Context *c);

protected:
    QSharedDataPointer<AdminAccountData> d;

private:
    static dbid_t setAdminAccount(Cutelyst::Context *c, SkaffariError *error, const QString &user, const QByteArray &pass, qint16 type);
    static bool rollbackAdminAccount(Cutelyst::Context *c, SkaffariError *error, dbid_t adminId);

    static bool setAdminSettings(Cutelyst::Context *c, SkaffariError *error, dbid_t adminId);
    static bool rollbackAdminSettings(Cutelyst::Context *c, SkaffariError *error, dbid_t adminId);

    static bool setAdminDomains(Cutelyst::Context *c, SkaffariError *error, dbid_t adminId, const QList<dbid_t> &domains);
    static bool rollbackAdminDomains(Cutelyst::Context *c, SkaffariError *error, dbid_t adminId);
};

Q_DECLARE_METATYPE(AdminAccount)
Q_DECLARE_TYPEINFO(AdminAccount, Q_MOVABLE_TYPE);

QDebug operator<<(QDebug dbg, const AdminAccount &account);

GRANTLEE_BEGIN_LOOKUP(AdminAccount)
QVariant var;
if (property == QLatin1String("id")) {
    var.setValue(object.id());
} else if (property == QLatin1String("username")) {
    var.setValue(object.username());
} else if (property == QLatin1String("domains")) {
    var.setValue(object.domains());
} else if (property == QLatin1String("type")) {
    var.setValue(object.typeInt());
} else if (property == QLatin1String("typeStr")) {
    var.setValue(object.typeStr());
} else if (property == QLatin1String("isValid")) {
    var.setValue(object.isValid());
} else if (property == QLatin1String("isSuperUser")) {
    var.setValue(object.isSuperUser());
} else if (property == QLatin1String("lang")) {
    var.setValue(object.lang());
} else if (property == QLatin1String("tz")) {
    var.setValue(object.tz());
} else if (property == QLatin1String("created")) {
    var.setValue(object.created());
} else if (property == QLatin1String("updated")) {
    var.setValue(object.updated());
} else if (property == QLatin1String("warnLevel")) {
    var.setValue(object.warnLevel());
} else if (property == QLatin1String("maxDisplay")) {
    var.setValue(object.maxDisplay());
}
return var;
GRANTLEE_END_LOOKUP

#endif // SKAFFARI_ADMINACCOUNT_H
