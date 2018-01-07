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
#include <QString>
#include <QSharedDataPointer>
#include <grantlee5/grantlee/metatype.h>
#include <QVariant>
#include <Cutelyst/ParamsMultiMap>
#include <QCryptographicHash>
#include <QLoggingCategory>
#include "../../common/global.h"

Q_DECLARE_LOGGING_CATEGORY(SK_ADMIN)

namespace Cutelyst {
class Context;
}

class AdminAccountData;
class SkaffariError;

/*! \brief Represents an administrator account.
 *
 * The AdminAccount class represents an administrator account object.
 * There are currently two types of administrators, superusers and domain admins.
 * Superusers can modify anyting and can also create new domains and deleting existing
 * ones, as well as adding and removing new admin accounts.
 *
 * Domain admins can only modify accounts in domains they are responsible for.
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
    enum AdminAccountType : qint16 {
		SuperUser = 0,		/**< a super user account, that has all rights */
        DomainMaster = 1    /**< a domain account, that can only modify certain domains */
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
	 * an account of type superuser. The type can be changed by the type parameter.
     * \param id the database id of the admin account
	 * \param username the user name of the administrator
	 * \param type the admin account type, can not be AllAdmins
     * \param domains list of domain IDs this admin is responsible for
     * \sa setUsername(), setPassword(), setType(), setDomains()
	 */
    AdminAccount(dbid_t id, const QString& username, qint16 type, const QList<dbid_t> &domains);

	/*!
	 * \brief Constructs a copy of other.
	 */
    AdminAccount(const AdminAccount &other);

	/*!
	 * \brief Assigns other to this AdminAccount and returns a reference to this account.
	 */
	AdminAccount &operator=(const AdminAccount &other);

	/*! \brief Destroys the object
	 */
    ~AdminAccount();


    /*!
     * \brief Returns the database ID.
     *
     * \return the database ID
     * \sa setId()
     */
    dbid_t getId() const;
    /*!
     * \brief Sets the database ID:
     *
     * \param id the database ID
     * \sa getId()
     */
    void setId(dbid_t id);


	/*!
	 * \brief Returns the username of this admin account.
	 *
	 * \return the user name
     * \sa setUsername()
	 */
	QString getUsername() const;
	/*!
	 * \brief Sets the user name of this admin account.
	 *
	 * \param nUsername the new user name
     * \sa getUsername()
	 */
	void setUsername(const QString &nUsername);


	/*!
     * \brief Returns the domain IDs this admin is responsible for.
	 *
	 * \retun list of domains
     * \sa setDomains()
	 */
    QList<dbid_t> getDomains() const;
	/*!
     * \brief Sets the list of domain IDs this admin is responsible for.
	 *
	 * Super users normally are responsible for all domains, so this list should only contain "*".
	 *
     * \param nDomains list of domain IDs
     * \sa getDomains()
	 */
    void setDomains(const QList<dbid_t> &nDomains);


	/*!
	 * \brief Returns the type of this admin account.
	 *
	 * \return the account type
     * \sa setType()
     */
    qint16 getType() const;
	/*!
	 * \brief Sets the type of this admin account.
	 *
	 * Values lower than 0 (SuperUser) will be converted into DomainAdmin.
     * \sa getType()
     */
    void setType(qint16 nType);

    /*!
     * \brief Returns \c true if this admin is a super user.
     */
    bool isSuperUser() const;

    /*!
     * \brief Returns the language set for this admin.
     * \sa setLang()
     */
    QString getLang() const;
    /*!
     * \brief Sets the language for this admin.
     * \sa getLang()
     */
    void setLang(const QString &lang);

    /*!
     * \brief Returns the time zone ID for this admin.
     * \sa setTz()
     */
    QByteArray getTz() const;
    /*!
     * \brief Sets the time zone ID for this admin.
     * \sa getTz()
     */
    void setTz(const QByteArray &tz);

    /*!
     * \brief Returns the date and time this admin account has been created.
     * \sa setCreated()
     */
    QDateTime getCreated() const;
    /*!
     * \brief Sets the date and time this admin account has been created.
     * \sa getCreated()
     */
    void setCreated(const QDateTime &created);

    /*!
     * \brief Returns the date and tim this admin account has been created.
     * \sa setUpdated()
     */
    QDateTime getUpdated() const;
    /*!
     * \brief Sets the date and time this admin account has been created.
     * \sa getUpdated()
     */
    void setUpdated(const QDateTime &updated);

    /*!
     * \brief Returns the maximum number of list entries to display per page.
     * \sa setMaxDisplay()
     */
    quint8 getMaxDisplay() const;
    /*!
     * \brief Sets the maximum number of list entries to display per page.
     * \sa getMaxDisplay()
     */
    void setMaxDisplay(quint8 maxDisplay);

    /*!
     * \brief Returns the warn level in percent for displaying limits like quota usage.
     * \sa setWarnLevel()
     */
    quint8 getWarnLevel() const;
    /*!
     * \brief Sets the warn level in percent for displaying limits like quota usage.
     * \sa getWarnLevel()
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
     * \brief Creates a new account with the given \a params and returns it.
     *
     * The returned account might be invalid if an error occured.
     *
     * \par Keys for the params
     * Key          | Converted Type | Description
     * -------------|----------------|------------------------------
     * username     | QString        | the user name for the new administrator account, will be trimmed
     * password     | QString        | the password for the new administrator account, will be stored with PBKDF2
     * type         | qint16         | the type of the new administrator account, see AdminAccount::AdminAccountType
     * assocdomains | QStringList    | list of domains this account will be associated to if the type is AdminAccount::DomainMaster
     *
     * \param c         pointer to the current context, used for translating strings
     * \param params    parameters for the new accounts
     * \param error     pointer to an object taking error information
     */
    static AdminAccount create(Cutelyst::Context *c, const Cutelyst::ParamsMultiMap &params, SkaffariError *error);

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
     * \brief Updates the administrator account \a with the new \a params and returns \c true on success.
     *
     * \par Keys for the params
     * Key          | Converted Type | Description
     * -------------|----------------|---------------------------------------
     * password     | QString        | new password, will be stored with PBKDF2, if empty, password will not be changed
     * type         | qint16         | new type, see AdminAccount::AdminAccountType
     * assocdomains | QStringList    | list of domains this account will be associated to if the type is AdminAccount::DomainMaster
     *
     * \param c         pointer to the current context, used for translating strings
     * \param e         pointer to an object taking error information
     * \param a         pointer to the administrator account to update
     * \param params    parameters used to update the account
     * \return
     */
    static bool update(Cutelyst::Context *c, SkaffariError *e, AdminAccount *a, const Cutelyst::ParamsMultiMap &params);

    /*!
     * \brief Updates the administrator acocunt \a of the currently authenticated administrator \a u with the new parameters \a p and returns \c true on success.
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
     * \param a pointer to the administartor account to update
     * \param u pointer to the currently authenticated user account
     * \param p parameters used to update the acount
     */
    static bool update(Cutelyst::Context *c, SkaffariError *e, AdminAccount *a, Cutelyst::AuthenticationUser *u, const Cutelyst::ParamsMultiMap &p);

    /*!
     * \brief Removes the administrator account \a a and returns \c true on success.
     * \param c pointer to the current context, used for translating strings
     * \param e pointer to an object taking error information
     * \param a the account to remove
     */
    static bool remove(Cutelyst::Context *c, SkaffariError *e, const AdminAccount &a);

    /*!
     * \brief Puts the admin account identified by \a adminId into the stash of the current context \a c.
     * \param c         pointer to the current context
     * \param adminId   the database id of the admin account
     */
    static void toStash(Cutelyst::Context *c, dbid_t adminId);

    /*!
     * \brief Returns the admin account from current context.
     * \param c pointer to the current context
     */
    static AdminAccount fromStash(Cutelyst::Context *c);

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

GRANTLEE_BEGIN_LOOKUP(AdminAccount)
QVariant var;
if (property == QLatin1String("id")) {
    var.setValue(object.getId());
} else if (property == QLatin1String("username")) {
    var.setValue(object.getUsername());
} else if (property == QLatin1String("domains")) {
    var.setValue(object.getDomains());
} else if (property == QLatin1String("type")) {
    var.setValue(object.getType());
} else if (property == QLatin1String("isValid")) {
    var.setValue(object.isValid());
} else if (property == QLatin1String("isSuperUser")) {
    var.setValue(object.isSuperUser());
} else if (property == QLatin1String("lang")) {
    var.setValue(object.getLang());
} else if (property == QLatin1String("tz")) {
    var.setValue(object.getTz());
} else if (property == QLatin1String("created")) {
    var.setValue(object.getCreated());
} else if (property == QLatin1String("updated")) {
    var.setValue(object.getUpdated());
} else if (property == QLatin1String("warnLevel")) {
    var.setValue(object.getWarnLevel());
} else if (property == QLatin1String("maxDisplay")) {
    var.setValue(object.getMaxDisplay());
}
return var;
GRANTLEE_END_LOOKUP

#endif // SKAFFARI_ADMINACCOUNT_H
