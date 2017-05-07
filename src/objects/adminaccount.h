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

#include <QDebug>

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
    AdminAccount(quint32 id, const QString& username, qint16 type, const QList<quint32> &domains);

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
    quint32 getId() const;

    /*!
     * \brief Sets the database ID:
     *
     * \param id the database ID
     * \sa getId()
     */
    void setId(quint32 id);


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
    QList<quint32> getDomains() const;

	/*!
     * \brief Sets the list of domain IDs this admin is responsible for.
	 *
	 * Super users normally are responsible for all domains, so this list should only contain "*".
	 *
     * \param nDomains list of domain IDs
     * \sa getDomains()
	 */
    void setDomains(const QList<quint32> &nDomains);


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
	 * \param nType the account type
     * \sa getType()
     */
    void setType(qint16 nType);

    /*!
     * \brief Returns \c true if this admin is a super user.
     */
    bool isSuperUser() const;

    /*!
     * \brief Returns the language set for this admin.
     */
    QString getLang() const;

    /*!
     * \brief Sets the language for this admin.
     */
    void setLang(const QString &lang);

    /*!
     * \brief Returns the time zone ID for this admin.
     */
    QByteArray getTz() const;

    /*!
     * \brief Sets the time zone ID for this admin.
     */
    void setTz(const QByteArray &tz);

    /*!
     * \brief Returns the date and time this admin account has been created.
     */
    QDateTime getCreated() const;

    /*!
     * \brief Sets the date and time this admin account has been created.
     */
    void setCreated(const QDateTime &created);

    /*!
     * \brief Returns the date and tim this admin account has been created.
     */
    QDateTime getUpdated() const;

    /*!
     * \brief Sets the date and time this admin account has been created.
     */
    void setUpdated(const QDateTime &updated);

    quint8 getMaxDisplay() const;
    void setMaxDisplay(quint8 maxDisplay);

    quint8 getWarnLevel() const;
    void setWarnLevel(quint8 warnLevel);

    QString getTemplate() const;
    void setTemplate(const QString &tmpl);


	/*!
     * \brief Returns true if the account seems to be valid.
	 *
	 * This function returns true, if the account seems to be valid. There is
     * no test against the databse. It only checks if user name and id are
	 * correctly set.
	 *
	 * \return true if account seems to be valid
	 */
    bool isValid() const;

    static AdminAccount create(Cutelyst::Context *c, const Cutelyst::ParamsMultiMap &params, SkaffariError *error);
    static QVector<AdminAccount> list(Cutelyst::Context *c, SkaffariError *error);
    static AdminAccount get(Cutelyst::Context *c, SkaffariError *e, quint32 id);
    static bool update(Cutelyst::Context *c, SkaffariError *e, AdminAccount *a, const Cutelyst::ParamsMultiMap &params);
    static bool update(Cutelyst::Context *c, SkaffariError *e, AdminAccount *a, Cutelyst::AuthenticationUser *u, const Cutelyst::ParamsMultiMap &p);
    static bool remove(Cutelyst::Context *c, SkaffariError *e, const AdminAccount &a);
    static void toStash(Cutelyst::Context *c, quint32 adminId);
    static AdminAccount fromStash(Cutelyst::Context *c);

protected:
    QSharedDataPointer<AdminAccountData> d;

private:
    static quint32 setAdminAccount(Cutelyst::Context *c, SkaffariError *error, const QString &user, const QByteArray &pass, qint16 type);
    static bool rollbackAdminAccount(Cutelyst::Context *c, SkaffariError *error, quint32 adminId);

    static bool setAdminSettings(Cutelyst::Context *c, SkaffariError *error, quint32 adminId);
    static bool rollbackAdminSettings(Cutelyst::Context *c, SkaffariError *error, quint32 adminId);

    static bool setAdminDomains(Cutelyst::Context *c, SkaffariError *error, quint32 adminId, const QList<quint32> &domains);
    static bool rollbackAdminDomains(Cutelyst::Context *c, SkaffariError *error, quint32 adminId);
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
