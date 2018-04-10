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

#include "folder.h"
#include <QDebug>
#include <QDataStream>
#include <algorithm>

#include <QSharedData>

class Folder::Data : public QSharedData
{
public:
    Data() : QSharedData() {}

    Data(dbid_t _id, dbid_t _domainId, const QString &_name) :
        QSharedData(),
        name(_name),
        id(_id),
        domainId(_domainId)
    {}

    ~Data() {}

    QString name;
    dbid_t id = 0;
    dbid_t domainId = 0;
};

Folder::Folder() :
    d(new Data)
{

}

Folder::Folder(dbid_t id, dbid_t domainId, const QString &name) :
    d(new Data(id, domainId, name))
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

QDebug operator<<(QDebug dbg, const Folder &folder)
{
    QDebugStateSaver saver(dbg);
    Q_UNUSED(saver);
    dbg.nospace() << "Folder(";
    dbg << "ID: " << folder.getId();
    dbg << ", Domain ID: " << folder.getDomainId();
    dbg << ", Name: " << folder.getName();
    dbg << ')';
    return dbg.maybeSpace();
}

QDataStream &operator<<(QDataStream &stream, const Folder &folder)
{
    stream << folder.getName() << folder.getId() << folder.getDomainId();

    return stream;
}

QDataStream &operator>>(QDataStream &stream, Folder &folder)
{
    stream >> folder.d->name;
    stream >> folder.d->id;
    stream >> folder.d->domainId;

    return stream;
}
