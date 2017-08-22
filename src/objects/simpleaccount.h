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

#ifndef SIMPLEACCOUNT_H
#define SIMPLEACCOUNT_H

#include <QString>
#include <QSharedDataPointer>
#include <grantlee5/grantlee/metatype.h>
#include <QVariant>
#include <QJsonArray>
#include <QJsonObject>
#include "../../common/global.h"

namespace Cutelyst {
class Context;
}

class SkaffariError;
class SimpleAccountData;

/*!
 * \brief Contains basic account data (only database ID and user name).
 */
class SimpleAccount
{
public:
    /*!
     * \brief Constructs an invalid, empty SimpleAccount.
     */
    SimpleAccount();

    /*!
     * \brief Constructs a new SimpleAccount with the given parameters.
     * \param id            Database ID.
     * \param username      Account user name.
     * \param domainname    Name of the domain the account belongs to.
     */
    SimpleAccount(dbid_t id, const QString &username, const QString &domainname);

    /*!
     * \brief Constructs a copy of \a other.
     */
    SimpleAccount(const SimpleAccount &other);

    /*!
     * \brief Assigns \a other to this simple account and returns a reference to this simple account.
     */
    SimpleAccount& operator=(const SimpleAccount &other);

    /*!
     * \brief Destroys the simple account.
     */
    ~SimpleAccount();

    /*!
     * \brief Returns the database ID.
     *
     * \c 0 by default.
     *
     * \par Grantlee property name
     * id
     */
    dbid_t id() const;

    /*!
     * \brief Returns the user name.
     *
     * Empty by default.
     *
     * \par Grantlee property name
     * username
     */
    QString username() const;

    /*!
     * \brief Returns the name of the domain the accounts belongs to.
     *
     * Empty by default.
     *
     * \par Grantlee property name
     * domainname
     */
    QString domainname() const;

    /*!
     * \brief Sets database ID, user name and domain name.
     */
    void setData(dbid_t id, const QString &username, const QString &domainname);

    /*!
     * \brief Returns \c true if this account is valid.
     *
     * An account is valid if the \a id is greater than \c 0 anf if the \a name is not empty.
     *
     * \sa operator bool()
     */
    bool isValid() const;

    /*!
     * \brief Returns \c true if this account is valid.
     *
     * An account is valid if the \a id is greater than \c 0 anf if the \a name is not empty.
     *
     * \sa isValid()
     */
    explicit operator bool() const
    {
        return isValid();
    }

    /*!
     * \brief Converts the SimpleAccount object into a QJsonObject.
     *
     * \par JSON object content
     * Key        | Value
     * ---------- | ----------------------------------------------
     * id         | The database ID of the account.
     * username   | The account user name.
     * domainname | The name of the domain the account belongs to.
     */
    QJsonObject toJson() const;

    /*!
     * \brief Returns a list of accounts.
     * \param c         Pointer to the current context, used for localization.
     * \param e         Pointer to an object taking occuring errors.
     * \param userType  The type of the admin user to determine domain access.
     * \param adminId   The database ID of the admin user to determine domain access.
     * \param domainId  The database ID of the domain to request accounts for. If 0 and permission is granted, all accounts will be returned.
     * \return          List of simple account objects.
     */
    static std::vector<SimpleAccount> list(Cutelyst::Context *c, SkaffariError *e, qint16 userType, dbid_t adminId, dbid_t domainId = 0, const QString searchString = QString());

    /*!
     * \brief Returns a JSON array of accounts.
     *
     * \sa toJson()
     *
     * \param c         Pointer to the current context, used for localization.
     * \param e         Pointer to an object taking occuring errors.
     * \param userType  The type of the admin user to determine domain access.
     * \param adminId   The database ID of the admin user to determine domain access.
     * \param domainId  The database ID of the domain to request accounts for. If 0 and permission is granted, all accounts will be returned.
     * \return          JSON array containing objects with account ID, user name and domain name.
     */
    static QJsonArray listJson(Cutelyst::Context *c, SkaffariError *e, qint16 userType, dbid_t adminId, dbid_t domainId = 0, const QString searchString = QString());

private:
    QSharedDataPointer<SimpleAccountData> d;
};

Q_DECLARE_METATYPE(SimpleAccount)
Q_DECLARE_TYPEINFO(SimpleAccount, Q_MOVABLE_TYPE);

GRANTLEE_BEGIN_LOOKUP(SimpleAccount)
QVariant var;
if (property == QLatin1String("id")) {
    var.setValue(object.id());
} else if (property == QLatin1String("username")) {
    var.setValue(object.username());
} else if (property == QLatin1String("domainname"))
return var;
GRANTLEE_END_LOOKUP

#endif // SIMPLEACCOUNT_H
