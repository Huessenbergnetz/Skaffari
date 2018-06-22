/*
 * Skaffari - a mail account administration web interface based on Cutelyst
 * Copyright (C) 2018 Matthias Fehring <mf@huessenbergnetz.de>
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

#ifndef ADMINTYPETAG_H
#define ADMINTYPETAG_H

#include <grantlee/node.h>
#include <grantlee/util.h>

namespace Grantlee {
class Parser;
}

/*!
 * \internal
 * \brief Grantlee node factory for the AdminType tag.
 */
class AdminTypeTag : public Grantlee::AbstractNodeFactory
{
    Q_OBJECT
public:
    /*!
     * \brief Returns the AdminType node.
     */
    Grantlee::Node *getNode(const QString &tagContent, Grantlee::Parser *p) const override;
};

class AdminType : public Grantlee::Node
{
    Q_OBJECT
public:
    /*!
     * \brief Constructs a new %AdminType node with the given parameters.
     * \param admin     The AdminAccount object.
     * \param parser    Pointer to the parser.
     */
    explicit AdminType(const Grantlee::FilterExpression &admin, Grantlee::Parser *parser = nullptr);

    /*!
     * \brief Performs the conversion and renders the result into the \a stream.
     */
    void render(Grantlee::OutputStream *stream, Grantlee::Context *gc) const override;

private:
    mutable QString m_cutelystContext = QStringLiteral("c");
    Grantlee::FilterExpression m_admin;
};

#endif // ADMINTYPETAG_H
