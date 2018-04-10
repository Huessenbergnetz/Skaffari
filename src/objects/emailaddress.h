/*
 * Skaffari - a mail account administration web interface based on Cutelyst
 * Copyright (C) 2018 Matthias Fehring <kontakt@buschmann23.de>
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

#ifndef SKAFFARIEMAILADDRESS_H
#define SKAFFARIEMAILADDRESS_H

#include "../../common/global.h"
#include <QString>
#include <QSharedDataPointer>
#include <grantlee5/grantlee/metatype.h>
#include <QVariant>

namespace Cutelyst {
class Context;
}

class SkaffariError;
class EmailAddressData;

/*!
 * \ingroup skaffaricore
 * \brief Contains information about a single email address from the virtual table.
 */
class EmailAddress
{
public:
    /*!
     * \brief Constructs an invalid, empty %EmailAdress object.
     * \sa isValid()
     */
    EmailAddress();

    /*!
     * \brief Constructs a new %EmailAddress object.
     * \param id        Database ID
     * \param aceId     Database ID of an optional ACE representation of the address.
     * \param name      The email address itself.
     */
    EmailAddress(dbid_t id, dbid_t aceId, const QString &name);

    /*!
     * \brief Constructs a copy of \a other.
     */
    EmailAddress(const EmailAddress &other);

    /*!
     * \brief Move-constructs an %EmailAddress instance, making it point at the same object that \a other was pointing to.
     */
    EmailAddress(EmailAddress &&other) noexcept;

    /*!
     * \brief Assigns \a other to this email address and returns a reference to this address.
     */
    EmailAddress& operator=(const EmailAddress &other);

    /*!
     * \brief Move-assigns \a other to this %EmailAddress instance.
     */
    EmailAddress& operator=(EmailAddress &&other) noexcept;

    /*!
     * \brief Destroys the %EmailAddress object.
     */
    ~EmailAddress();

    /*!
     * \brief Swaps this %EmailAddress instance with \a other.
     */
    void swap(EmailAddress &other) noexcept;

    /*!
     * \brief The database ID.
     */
    dbid_t id() const;

    /*!
     * \brief Database ID of the ACE version of this email address.
     */
    dbid_t aceId() const;

    /*!
     * \brief The email address.
     */
    QString name() const;

    /*!
     * \brief Returns the local part of the email address.
     */
    QString localPart() const;

    /*!
     * \brief Returns the domain part of the email address.
     */
    QString domainPart() const;

    /*!
     * \brief Returns \c true if this email address uses an internationalized domain name (IDN).
     */
    bool isIdn() const;

    /*!
     * \brief Returns \c true if this email address object is valid.
     *
     * An email address object is valid if the id() is greate than \c 0 and if the
     * name() is not empty.
     */
    bool isValid() const;

    /*!
     * \brief \c true if this is a valid email address object.
     * \sa isValid()
     */
    explicit operator bool() const
    {
        return isValid();
    }

    /*!
     * \brief Returns a list of email addresses from the virtual table associacted with the \a username.
     * \param c         Pointer to the current context, used for localization.
     * \param e         Object taking error inforamtion.
     * \param username  The username to lookup the email addresses for.
     * \return          List of email address objects.
     */
    static std::vector<EmailAddress> list(Cutelyst::Context *c, SkaffariError &e, const QString &username);

    /*!
     * \brief Returns the email address identified by its database \a id.
     * \param c     Pointer to the current context, used for localization.
     * \param e     Object taking error information.
     * \param id    The database ID of the email address to query.
     * \return      The email address object.
     */
    static EmailAddress get(Cutelyst::Context *c, SkaffariError &e, dbid_t id);

    /*!
     * \brief Returns the email address identified by the \a alias (address).
     * \param c     Pointer to the current context, used for localization.
     * \param e     Object taking error information.
     * \param alias The alias/addres of the email address to query.
     * \return      The email address object.
     */
    static EmailAddress get(Cutelyst::Context *c, SkaffariError &e, const QString &alias);

private:
    QSharedDataPointer<EmailAddressData> d;
};

Q_DECLARE_METATYPE(EmailAddress)
Q_DECLARE_TYPEINFO(EmailAddress, Q_MOVABLE_TYPE);

/*!
 * \relates EmailAddress
 * \brief Writes the \a address to the \a dbg stream and returns the stream.
 */
QDebug operator<<(QDebug dbg, const EmailAddress &address);

GRANTLEE_BEGIN_LOOKUP(EmailAddress)
if (property == QLatin1String("id")) {
   return QVariant(object.id());
} else if (property == QLatin1String("name")) {
   return QVariant(object.name());
} else if (property == QLatin1String("isIdn")) {
   return QVariant(object.isIdn());
}
return QVariant();
GRANTLEE_END_LOOKUP

#endif // SKAFFARIEMAILADDRESS_H
