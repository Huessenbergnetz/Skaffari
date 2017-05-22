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

#ifndef HELPENTRY_P_H
#define HELPENTRY_P_H

#include "helpentry.h"
#include <QSharedData>

class HelpEntryData : public QSharedData
{
public:
    HelpEntryData() {}

    HelpEntryData(const QString &_title, const QString &_text) :
        title(_title),
        text(_text)
    {}

    HelpEntryData(const HelpEntryData &other) :
        QSharedData(other),
        title(other.title),
        text(other.text)
    {}

    ~HelpEntryData() {}

    QString title;
    QString text;
};

#endif // HELPENTRY_P_H
