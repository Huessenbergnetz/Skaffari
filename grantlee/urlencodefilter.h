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

#ifndef URLENCODEFILTER_H
#define URLENCODEFILTER_H

#include <grantlee5/grantlee/filter.h>

/*!
 * \brief Grantlee template filter to encode strings to percent encoding.
 *
 * This filter can be used as \c sk_urlencode in your Grantlee templates. It encodes
 * a string to percent encoding like used in URLs. Additionally to normal percent encoding
 * it will also encode the '.' (dot).
 *
 * \par Example
 * \code
 * {{ email_address|sk_urlencode }}
 * \endcode
 */
class UrlEncodeFilter : public Grantlee::Filter
{
public:
    bool isSafe() const override { return true; }

    QVariant doFilter(const QVariant &input, const QVariant &argument, bool autoescape) const override;
};

#endif // URLENCODEFILTER_H
