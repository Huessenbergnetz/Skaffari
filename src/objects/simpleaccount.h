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

#include "../../common/global.h"
#include "adminaccount.h"
#include <grantlee5/grantlee/metatype.h>
#include <QSharedDataPointer>
#include <QString>
#include <QVariant>
#include <QJsonArray>
#include <QJsonObject>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(SK_SIMPLEACCOUNT)

namespace Cutelyst {
class Context;
}

class SkaffariError;

/*!
 * \ingroup skaffaricore
 * \brief Contains basic account data (only database ID, user name and domain name).
 */
class SimpleAccount
{
    class Data;
    QSharedDataPointer<Data> d;
public:
    /*!
     * \brief Constructs an invalid, empty %SimpleAccount.
     */
    SimpleAccount();

    /*!
     * \brief Constructs a new %SimpleAccount with the given parameters.
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
     * \brief Move-constructs a %SimpleAccount instance, making it point at the same object that \a other was pointing to.
     */
    SimpleAccount(SimpleAccount &&other) noexcept;

    /*!
     * \brief Assigns \a other to this simple account and returns a reference to this simple account.
     */
    SimpleAccount& operator=(const SimpleAccount &other);

    /*!
     * \brief Move-assigns \a other to this %SimpleAccount instance.
     */
    SimpleAccount& operator=(SimpleAccount &&other) noexcept;

    /*!
     * \brief Destroys the simple account.
     */
    ~SimpleAccount();

    /*!
     * \brief Swaps this %SimpleAccount instance with \a other.
     */
    void swap(SimpleAccount &other) noexcept;

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
     * \param e         Object taking occurring errors.
     * \param adminType The type of the admin user to determine domain access.
     * \param adminId   The database ID of the admin user to determine domain access.
     * \param domainId  The database ID of the domain to request accounts for. If 0 and permission is granted, all accounts will be returned.
     * \return          List of simple account objects.
     */
    static std::vector<SimpleAccount> list(Cutelyst::Context *c, SkaffariError &e, AdminAccount::AdminAccountType adminType, dbid_t adminId, dbid_t domainId = 0, const QString searchString = QString());

    /*!
     * \brief Returns a JSON array of accounts.
     *
     * \sa toJson()
     *
     * \param c         Pointer to the current context, used for localization.
     * \param e         Object taking occurring errors.
     * \param adminType The type of the admin user to determine domain access.
     * \param adminId   The database ID of the admin user to determine domain access.
     * \param domainId  The database ID of the domain to request accounts for. If 0 and permission is granted, all accounts will be returned.
     * \return          JSON array containing objects with account ID, user name and domain name.
     */
    static QJsonArray listJson(Cutelyst::Context *c, SkaffariError &e, AdminAccount::AdminAccountType adminType, dbid_t adminId, dbid_t domainId = 0, const QString searchString = QString());

    /*!
     * \brief Returns the account with the specified database \a id.
     * \param c         Poiner to the current context, used for localization.
     * \param e         Object taking occurring errors.
     * \param id        The database ID of the accout to get.
     * \return          Single account.
     */
    static SimpleAccount get(Cutelyst::Context *c, SkaffariError &e, dbid_t id);
};

Q_DECLARE_METATYPE(SimpleAccount)
Q_DECLARE_TYPEINFO(SimpleAccount, Q_MOVABLE_TYPE);

/*!
 * \relates SimpleAccount
 * \brief Writes the \a account to the \a dbg stream and returns the stream.
 */
QDebug operator<<(QDebug dbg, const SimpleAccount &account);

GRANTLEE_BEGIN_LOOKUP(SimpleAccount)
if (property == QLatin1String("id")) {
    return QVariant(object.id());
} else if (property == QLatin1String("username")) {
    return QVariant(object.username());
} else if (property == QLatin1String("domainname")) {
    return QVariant(object.domainname());
}
return QVariant();
GRANTLEE_END_LOOKUP

#endif // SIMPLEACCOUNT_H
