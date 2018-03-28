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

#include "simpleadmin.h"

SimpleAdmin::SimpleAdmin()
{

}

SimpleAdmin::SimpleAdmin(dbid_t id, const QString &name) :
    m_id(id), m_name(name)
{

}

SimpleAdmin::SimpleAdmin(const SimpleAdmin &other) :
    m_id(other.m_id), m_name(other.m_name)
{

}

SimpleAdmin::SimpleAdmin(SimpleAdmin &&other) noexcept :
    m_id(std::move(other.m_id)), m_name(std::move(other.m_name))
{

}

SimpleAdmin& SimpleAdmin::operator=(const SimpleAdmin &other)
{
    m_id = other.m_id;
    m_name = other.m_name;
    return *this;
}

SimpleAdmin& SimpleAdmin::operator=(SimpleAdmin &&other) noexcept
{
    swap(other);
    return *this;
}

SimpleAdmin::~SimpleAdmin()
{

}

void SimpleAdmin::swap(SimpleAdmin &other) noexcept
{
    std::swap(m_id, other.m_id);
    std::swap(m_name, other.m_name);
}

dbid_t SimpleAdmin::id() const
{
    return m_id;
}

QString SimpleAdmin::name() const
{
    return m_name;
}
