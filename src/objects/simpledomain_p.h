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

#ifndef SIMPLEDOMAIN_P_H
#define SIMPLEDOMAIN_P_H

#include "simpledomain.h"
#include <QSharedData>
#include <QCollator>

class SimpleDomainNameCollator : public QCollator
{
public:
    SimpleDomainNameCollator(const QLocale &locale) :
        QCollator(locale)
    {}

    bool operator() (const SimpleDomain &left, const SimpleDomain &right) { return (compare(left.name(), right.name()) < 0); }
};

class SimpleDomainData : public QSharedData
{
public:
    SimpleDomainData() {}

    SimpleDomainData(quint32 _id, const QString &_name) :
        id(_id),
        name(_name)
    {}

    SimpleDomainData(const SimpleDomainData &other) :
        QSharedData(other),
        id(other.id),
        name(other.name)
    {}

    ~SimpleDomainData() {}

    quint32 id = 0;
    QString name;
};

#endif // SIMPLEDOMAIN_P_H
