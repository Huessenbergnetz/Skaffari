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

#ifndef SIMPLEDOMAIN_H
#define SIMPLEDOMAIN_H

#include "../../common/global.h"
#include <QObject>
#include <QString>
#include <QVariant>
#include <QJsonArray>

namespace Cutelyst {
class Context;
}

class SkaffariError;

/*!
 * \ingroup skaffaricore
 * \brief Contains basic domain data (only database ID and domain name).
 */
class SimpleDomain
{
    Q_GADGET
    Q_PROPERTY(dbid_t id READ id CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
public:
    /*!
     * \brief Constructs an invalid, empty SimpleDomain.
     */
    SimpleDomain() = default;

    /*!
     * \brief Constructs a new SimpleDomain with given database \a id and domain \a name.
     */
    SimpleDomain(dbid_t id, const QString &name);

    /*!
     * \brief Constructs a copy of \a other.
     */
    SimpleDomain(const SimpleDomain &other);

    /*!
     * \brief Move-constructs a %SimpleDomain instance, making it point at the same object that \a other was pointing to.
     */
    SimpleDomain(SimpleDomain &&other) noexcept;

    /*!
     * \brief Assigns \a other to this simple domain and returns a reference to this simple domain.
     */
    SimpleDomain& operator=(const SimpleDomain &other);

    /*!
     * \brief Move-assigns \a other to this %SimpleDomain instance.
     */
    SimpleDomain& operator=(SimpleDomain &&other) noexcept;

    /*!
     * \brief Destroys the simple domain.
     */
    ~SimpleDomain() = default;

    /*!
     * \brief Swaps this %SimpleDomain instance with \a other.
     */
    void swap(SimpleDomain &other) noexcept;

    /*!
     * \brief Returns the databas ID.
     *
     * \c 0 by default.
     */
    dbid_t id() const;

    /*!
     * \brief Returns the domain name.
     */
    QString name() const;

    /*!
     * \brief Returns a string that contains the username and the database ID.
     *
     * This will mostly be used in log messages and returns a string like
     * <code>"example.com (ID: 123)"</code>.
     */
    QString nameIdString() const;

    /*!
     * \brief Returns \c true if this domain is valid.
     *
     * A domain is valid if the \a id is greater than \c 0 and if the \a name is not empty.
     *
     * \sa operator bool()
     */
    bool isValid() const;

    /*!
     * \brief Returns \c true if this domain is valid.
     *
     * A domain is valid if the \a id is greater than \c 0 and if the \a name is not empty.
     *
     * \sa isValid()
     */
    explicit operator bool() const
    {
        return isValid();
    }

    /*!
     * \brief Returns a list of domains for the admin defined by \a adminId.
     * \param c             Pointer to the current context, used for localization.
     * \param e             Object taking occurring errors.
     * \param userType      The type of the admin user to determine domain access.
     * \param adminId       The database ID of the admin user to determine domain access.
     * \param orphansOnly   Only return domains that do not have a parent domain.
     */
    static std::vector<SimpleDomain> list(Cutelyst::Context *c, SkaffariError &e, quint8 userType, dbid_t adminId, bool orphansOnly = false);

    /*!
     * \brief Returns a list of domains for the currently logged in administrator.
     * \param c             Pointer to the current context, used for localization and getting the current admin.
     * \param e             Object taking occuring errors.
     * \param orphansOnly   Only return domains that do not have a parent domain.
     */
    static std::vector<SimpleDomain> list(Cutelyst::Context *c, SkaffariError &e, bool orphansOnly = false);

    /*!
     * \brief Returns a JSON array of domains for the admin defined by \a adminId.
     * \param c         Pointer to the current context, used for localization.
     * \param e         Object taking occurring errors.
     * \param userType  The type of the admin user to determine domain access.
     * \param adminId   The database ID of the admin user to determine domain access.
     */
    static QJsonArray listJson(Cutelyst::Context *c , SkaffariError &e, quint8 userType, dbid_t adminId, bool orphansOnly = false);

    /*!
     * \brief Returns a single simple domain object identified by its database \a id.
     * \param c     Pointer to the current context, used for localization.
     * \param e     Object taking occurring errors.
     * \param id    The database ID of the domain to retrieve.
     */
    static SimpleDomain get(Cutelyst::Context *c, SkaffariError &e, dbid_t id);

private:
    QString m_name;
    dbid_t m_id = 0;

    friend QDataStream &operator>>(QDataStream &stream, SimpleDomain &domain);
    friend QDataStream &operator<<(QDataStream &stream, const SimpleDomain &domain);
};

Q_DECLARE_METATYPE(SimpleDomain)
Q_DECLARE_TYPEINFO(SimpleDomain, Q_MOVABLE_TYPE);

/*!
 * \relates SimpleDomain
 * \brief Writes the \a domain to the \a debug stream and returns the stream.
 */
QDebug operator<<(QDebug debug, const SimpleDomain &domain);

/*!
 * \relates SimpleDomain
 * \brief Writes the given \a domain to the given \a stream.
 */
QDataStream &operator<<(QDataStream &stream, const SimpleDomain &domain);

/*!
 * \relates SimpleDomain
 * \brief Reads a %SimpleDomain from the given \a stream and stores it in the given \a domain.
 */
QDataStream &operator>>(QDataStream &stream, SimpleDomain &domain);

#endif // SIMPLEDOMAIN_H
