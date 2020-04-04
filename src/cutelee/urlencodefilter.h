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

#ifndef URLENCODEFILTER_H
#define URLENCODEFILTER_H

#include <cutelee/filter.h>

/*!
 * \ingroup skaffaricuteleefilters
 * \brief Cutelee template filter to encode strings to percent encoding.
 *
 * This filter can be used as \c sk_urlencode in your Cutelee templates. It encodes
 * a string to percent encoding like used in URLs. Additionally to normal percent encoding
 * it will also encode the '.' (dot).
 *
 * \par Example
 * \code
 * {{ email_address|sk_urlencode }}
 * \endcode
 */
class UrlEncodeFilter : public Cutelee::Filter // clazy:exclude=copyable-polymorphic
{
public:
    /*!
     * \brief Returns \c true because this filter is safe.
     */
    bool isSafe() const override { return true; }

    /*!
     * \brief Performs this filter on \a input.
     *
     * \a argument and \a autoescape are not used.
     */
    QVariant doFilter(const QVariant &input, const QVariant &argument = QVariant(), bool autoescape = false) const override;
};

#endif // URLENCODEFILTER_H
