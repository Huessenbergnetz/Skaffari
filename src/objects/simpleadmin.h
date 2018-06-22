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

#ifndef SIMPLEADMIN_H
#define SIMPLEADMIN_H

#include "../../common/global.h"
#include <grantlee5/grantlee/metatype.h>
#include <QString>
#include <QVariant>

/*!
 * \ingroup skaffaricore
 * \brief Provides basic information about an administrator.
 */
class SimpleAdmin
{
public:
    /*!
     * \brief Constructs an invalid, empty %SimpleAdmin.
     */
    SimpleAdmin() = default;
    /*!
     * \brief Constructs a new %SimpleAdmin with the given parameters.
     * \param id    database ID of the account
     * \param name  user name of the account
     */
    SimpleAdmin(dbid_t id, const QString &name);
    /*!
     * \brief Constructs a copy of \a other.
     */
    SimpleAdmin(const SimpleAdmin &other);
    /*!
     * \brief Move-constructs a %SimpleAdmin instance, making it point at the same object that \a other was pointing to.
     */
    SimpleAdmin(SimpleAdmin &&other) noexcept;
    /*!
     * \brief Assigns \a other to this simple account and returns a reference to this instance.
     */
    SimpleAdmin& operator=(const SimpleAdmin &other);
    /*!
     * \brief Move-assigns \a other to this %SimpleAdmin instance.
     */
    SimpleAdmin& operator=(SimpleAdmin &&other) noexcept;
    /*!
     * \brief Destroys this %SimpleAdmin instance.
     */
    ~SimpleAdmin() = default;

    /*!
     * \brief Swapts this %SimpleAdmin instance with \a other.
     */
    void swap(SimpleAdmin &other) noexcept;

    /*!
     * \brief Returns the database ID.
     */
    dbid_t id() const;

    /*!
     * \brief Returns the user name.
     */
    QString name() const;

    /*!
     * \brief Returns \c true if this is a valid admin account.
     *
     * Validity is simply determined by the fact that id() is greater than \c 0
     * and that name() is not empty.
     */
    bool isValid() const;

private:
    dbid_t m_id = 0;
    QString m_name;

    friend QDataStream &operator>>(QDataStream &stream, SimpleAdmin &admin);
    friend QDataStream &operator<<(QDataStream &stream, const SimpleAdmin &admin);
};

Q_DECLARE_METATYPE(SimpleAdmin)
Q_DECLARE_TYPEINFO(SimpleAdmin, Q_MOVABLE_TYPE);

/*!
 * \relates SimpleAdmin
 * \brief Writes the \a admin to the \a dbg stream and returns the stream.
 */
QDebug operator<<(QDebug dbg, const SimpleAdmin &admin);

/*!
 * \relates SimpleAdmin
 * \brief Writes the given \a admin to the given \a stream.
 */
QDataStream &operator<<(QDataStream &stream, const SimpleAdmin &admin);

/*!
 * \relates SimpleAdmin
 * \brief Reads an %SimpleAdmin from the given \a stream and stores it in the given \a admin.
 */
QDataStream &operator>>(QDataStream &stream, SimpleAdmin &admin);

GRANTLEE_BEGIN_LOOKUP(SimpleAdmin)
if (property == QLatin1String("id")) {
    return QVariant(object.id());
} else if (property == QLatin1String("name")) {
    return QVariant(object.name());
}
return QVariant();
GRANTLEE_END_LOOKUP

#endif // SIMPLEADMIN_H
