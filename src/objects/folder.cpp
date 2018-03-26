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

#include "folder_p.h"
#include <algorithm>

Folder::Folder() :
    d(new FolderData)
{

}

Folder::Folder(dbid_t id, dbid_t domainId, const QString &name) :
    d(new FolderData(id, domainId, name))
{

}

Folder::Folder(const Folder &other) :
    d(other.d)
{

}

Folder::Folder(Folder &&other) noexcept :
    d(std::move(other.d))
{
    other.d = nullptr;
}

Folder& Folder::operator=(const Folder &other)
{
    d = other.d;
    return *this;
}

Folder& Folder::operator=(Folder &&other) noexcept
{
    swap(other);
    return *this;
}

Folder::~Folder()
{

}

void Folder::swap(Folder &other) noexcept
{
    std::swap(d, other.d);
}

dbid_t Folder::getId() const
{
    return d->id;
}

dbid_t Folder::getDomainId() const
{
    return d->domainId;
}

QString Folder::getName() const
{
    return d->name;
}
