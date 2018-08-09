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

#ifndef ACEDECODEFILTER_H
#define ACEDECODEFILTER_H

#include <grantlee5/grantlee/filter.h>

/*!
 * \ingroup skaffarigrantlee
 * \brief Grantlee template filter to decode ACE encoded strings.
 *
 * This filter can be used as \c sk_acedecode in your Grantlee templates. It decodes
 * a strings that has before been encoded using ASCII Compatible Encoding (ACE).
 *
 * <H3>Example</H3>
 * \code{.html}
 * <!-- will be converted into "hüssenbergnetz.de" if the variable contains "xn--hssenbergnetz-wob.de" -->
 * <p>{{ domain|sk_acedecode }}</p>
 * \endcode
 */
class AceDecodeFilter : public Grantlee::Filter // clazy:exclude=copyable-polymorphic
{
public:
    /*!
     * \brief Returns \c true because this filter is safe.
     */
    bool isSafe() const override { return true; }

    /*!
     * \brief Performs this filter on \a input.
     *
     * \a argument and  \a autoescape are not used.
     */
    QVariant doFilter(const QVariant &input, const QVariant &argument = QVariant(), bool autoescape = false) const override;
};

#endif // ACEDECODEFILTER_H
