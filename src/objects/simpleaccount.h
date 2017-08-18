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
     * \param id        Database ID.
     * \param username  Account user name.
     */
    SimpleAccount(dbid_t id, const QString &username);

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
     */
    dbid_t id() const;

    /*!
     * \brief Returns the user name.
     *
     * Empty by default.
     */
    QString username() const;

    /*!
     * \brief Sets database ID and user name.
     */
    void setData(dbid_t id, const QString &username);

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
}
return var;
GRANTLEE_END_LOOKUP

#endif // SIMPLEACCOUNT_H
