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

#ifndef STRINGLISTSORTFILTER_H
#define STRINGLISTSORTFILTER_H

#include <grantlee5/grantlee/filter.h>

/*!
 * \ingroup skaffarigrantlee
 * \brief Grantlee template filter to sort a list of strings according to the current locale.
 *
 * This filter can be used as \c sk_stringlistsort in your Grantlee templates. It sorts a list
 * of strings accodring to the current locale.
 */
class StringListSortFilter : public Grantlee::Filter
{
public:
    bool isSafe() const override { return true; }

    QVariant doFilter(const QVariant &input, const QVariant &argument = QVariant(), bool autoescape = false) const override;
};

#endif // STRINGLISTSORTFILTER_H
