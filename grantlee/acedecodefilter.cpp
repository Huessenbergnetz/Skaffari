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

#include "acedecodefilter.h"
#include <QUrl>
#include <grantlee5/grantlee/util.h>

QVariant AceDecodeFilter::doFilter(const QVariant &input, const QVariant &argument, bool autoescape) const
{
    QVariant ret;

    Q_UNUSED(argument)
    Q_UNUSED(autoescape)

    ret.setValue<Grantlee::SafeString>(Grantlee::SafeString(QUrl::fromAce(Grantlee::getSafeString(input).get().toLatin1()), true));

    return ret;
}
