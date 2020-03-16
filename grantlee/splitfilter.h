/*
 * Skaffari - a mail account administration web interface based on Cutelyst
 * Copyright (C) 2019 Matthias Fehring <mf@huessenbergnetz.de>
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

#ifndef SPLITFILTER_H
#define SPLITFILTER_H

#include <grantlee5/grantlee/filter.h>

/*!
 * \ingroup skaffarigrantleefilters
 * \brief Grantlee template filter to split a string into QStringList.
 *
 * This filter can be use as \c sk_split in your grantlee templates. It splits
 * as string into a QStringList. By default it uses a comma (,) as separator
 * but you use the filter argument to select a different separator.
 *
 * \par example
 * \code
 * {% for i in "10,20,30,40,50"|sk_split %}
 * <p>{{ i }}</p>
 * {% endfor %}
 *
 * {% for i in "a;b;c;d;e"|sk_split:";" %}
 * <p>{{ i }}</p>
 * {% endif %}
 * \endcode
 */
class SplitFilter : public Grantlee::Filter // clazy:exclude=copyable-polymorphic
{
public:
    bool isSafe() const override { return true; }

    QVariant doFilter(const QVariant &input, const QVariant &argument = QVariant(), bool autoescape = false) const override;
};

#endif // SPLITFILTER_H
