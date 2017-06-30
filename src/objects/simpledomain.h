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

#ifndef SIMPLEDOMAIN_H
#define SIMPLEDOMAIN_H

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
class SimpleDomainData;

/*!
 * \brief Contains basic domain data.
 */
class SimpleDomain
{
public:
    /*!
     * \brief Constructs an invalid, empty SimpleDomain.
     */
    SimpleDomain();

    /*!
     * \brief Constructs a new SimpleDomain with given database \a id and domain \a name.
     */
    SimpleDomain(dbid_t id, const QString &name);

    /*!
     * \brief Constructs a copy of \a other.
     */
    SimpleDomain(const SimpleDomain &other);

    /*!
     * \brief Assigns \a other to this simple domain and returns a reference to this simple domain.
     */
    SimpleDomain& operator=(const SimpleDomain &other);

    /*!
     * \brief Destroys the simple domain.
     */
    ~SimpleDomain();

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
     * \brief Returns a list of domains for the admin defined by \a adminId.
     * \param c         Pointer to the current context, used for localization.
     * \param e         Pointer to an object taking occuring errors.
     * \param userType  The type of the admin user to determine domain access.
     * \param adminId   The database ID of the admin user to determine domain access.
     * \return          List of simple domain objects.
     */
    static std::vector<SimpleDomain> list(Cutelyst::Context *c, SkaffariError *e, quint16 userType, dbid_t adminId);

    /*!
     * \brief Returns a JSON array of domains for the admin defined by \a adminId.
     * \param c         Pointer to the current context, used for localization.
     * \param e         Pointer to an object taking occuring errors.
     * \param userType  The type of the admin user to determine domain access.
     * \param adminId   The database ID of the admin user to determine domain access.
     * \return          JSON array containing objects with domain ID and domain name.
     */
    static QJsonArray listJson(Cutelyst::Context *c , SkaffariError *e, quint16 userType, dbid_t adminId);

private:
    QSharedDataPointer<SimpleDomainData> d;
};

Q_DECLARE_METATYPE(SimpleDomain)
Q_DECLARE_TYPEINFO(SimpleDomain, Q_MOVABLE_TYPE);

GRANTLEE_BEGIN_LOOKUP(SimpleDomain)
QVariant var;
if (property == QLatin1String("id")) {
    var.setValue(object.id());
} else if (property == QLatin1String("name")) {
    var.setValue(object.name());
}
return var;
GRANTLEE_END_LOOKUP

#endif // SIMPLEDOMAIN_H
