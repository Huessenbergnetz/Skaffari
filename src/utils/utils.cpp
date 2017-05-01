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

#include "utils.h"
#include <Cutelyst/Context>
#include <Cutelyst/Plugins/Session/Session>
#include <QTimeZone>


Utils::Utils()
{

}



QDateTime Utils::toUserTZ(Cutelyst::Context *c, const QDateTime &dt)
{
    QDateTime retVal;

    const QTimeZone userTz(Cutelyst::Session::value(c, QStringLiteral("tz"), QStringLiteral("UTC")).toByteArray());

    if (userTz != QTimeZone::utc()) {
        QDateTime _dt = dt;
        _dt.setTimeSpec(Qt::UTC);
        retVal = _dt.toTimeZone(userTz);
    } else {
        retVal = dt;
    }

    return retVal;
}
