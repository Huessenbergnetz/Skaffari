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

#include "simpleadmin_p.h"

SimpleAdmin::SimpleAdmin() : d(new SimpleAdminData)
{

}


SimpleAdmin::SimpleAdmin(quint32 id, const QString &name) :
    d(new SimpleAdminData(id, name))
{

}


SimpleAdmin::SimpleAdmin(const SimpleAdmin &other) :
    d(other.d)
{

}


SimpleAdmin& SimpleAdmin::operator=(const SimpleAdmin &other)
{
    d = other.d;
    return *this;
}


SimpleAdmin::~SimpleAdmin()
{

}


quint32 SimpleAdmin::id() const
{
    return d->id;
}


QString SimpleAdmin::name() const
{
    return d->name;
}
