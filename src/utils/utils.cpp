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
#include <QLocale>

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



QString Utils::humanBinarySize(Cutelyst::Context *c, quint64 sizeInByte)
{
    QString sizeStr;

    if (sizeInByte < 1048576) {
        float kibFloat = (float)sizeInByte/1024.0f;
        sizeStr = c->locale().toString(kibFloat, 'f', 2);
        sizeStr.append(QLatin1String(" KiB"));
    } else if (sizeInByte < 1073741824) {
        float mibFloat = (float)sizeInByte/1048576.0f;
        sizeStr = c->locale().toString(mibFloat, 'f', 2);
        sizeStr.append(QLatin1String(" MiB"));
    } else if (sizeInByte < 1099511627776) {
        float gibFloat = (float)sizeInByte/1073741824.0f;
        sizeStr = c->locale().toString(gibFloat, 'f', 2);
        sizeStr.append(QLatin1String(" GiB"));
    } else {
        float tibFloat = (float)sizeInByte/1099511627776.0f;
        sizeStr = c->locale().toString(tibFloat, 'f', 2);
        sizeStr.append(QLatin1String(" TiB"));
    }

    return sizeStr;
}
