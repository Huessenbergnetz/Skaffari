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

#ifndef SIMPLEADMIN_P_H
#define SIMPLEADMIN_P_H

#include "simpleadmin.h"
#include <QSharedData>

class SimpleAdminData : public QSharedData
{
public:
    SimpleAdminData() {}

    SimpleAdminData(dbid_t _id, const QString &_name) :
        name(_name),
        id(_id)
    {}

    SimpleAdminData(const SimpleAdminData &other) :
        QSharedData(other),
        name(other.name),
        id(other.id)
    {}

    ~SimpleAdminData() {}

    QString name;
    dbid_t id = 0;
};

#endif // SIMPLEADMIN_P_H
