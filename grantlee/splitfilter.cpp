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

#include "splitfilter.h"
#include <grantlee/util.h>

QVariant SplitFilter::doFilter(const QVariant &input, const QVariant &argument, bool autoescape) const
{
    QVariant ret;

    Q_UNUSED(autoescape)

    const QString str = Grantlee::getSafeString(input);

    if (!str.isEmpty()) {
        const QString sep = Grantlee::getSafeString(argument);
        const QStringList list = str.split(sep.isEmpty() ? QStringLiteral(",") : sep);
        ret.setValue<QStringList>(list);
    }

    return ret;
}
