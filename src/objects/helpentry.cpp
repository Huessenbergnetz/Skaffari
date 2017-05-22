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

#include "helpentry_p.h"

HelpEntry::HelpEntry() :
    d(new HelpEntryData)
{

}

HelpEntry::HelpEntry(const QString &title, const QString &text) :
    d(new HelpEntryData(title, text))
{

}

HelpEntry::HelpEntry(const HelpEntry &other) :
    d(other.d)
{

}

HelpEntry& HelpEntry::operator =(const HelpEntry &other)
{
    d = other.d;
    return *this;
}

HelpEntry::~HelpEntry()
{

}

QString HelpEntry::getTitle() const
{
    return d->title;
}

QString HelpEntry::getText() const
{
    return d->text;
}
