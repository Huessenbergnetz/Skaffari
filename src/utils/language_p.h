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

#ifndef LANGUAGE_P_H
#define LANGUAGE_P_H

#include "language.h"
#include <QSharedData>
#include <QLocale>
#include <QCollator>

class LanguageNameCollator : public QCollator
{
public:
    explicit LanguageNameCollator(const QLocale &locale) :
        QCollator(locale)
    {}

    bool operator() (const Language &left, const Language &right) { return (compare(left.name(), right.name())); }
};

class LanguageData : public QSharedData
{
public:
    LanguageData() {}

    explicit LanguageData(const QLocale &_locale) :
        locale(_locale)
    {}

    LanguageData(const LanguageData &other) :
        QSharedData(other),
        locale(other.locale)
    {}

    ~LanguageData() {}

    QLocale locale;
};

#endif // LANGUAGE_P_H
