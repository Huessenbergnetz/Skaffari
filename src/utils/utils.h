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

#ifndef UTILS_H
#define UTILS_H

#include <QDateTime>

namespace Cutelyst {
class Context;
}

class Utils
{
public:
    Utils();

    static QDateTime toUserTZ(Cutelyst::Context *c, const QDateTime &dt);
    static QString humanBinarySize(Cutelyst::Context *c, quint64 sizeInByte);
    static QString getUserName(Cutelyst::Context *c);
};

#endif // UTILS_H
