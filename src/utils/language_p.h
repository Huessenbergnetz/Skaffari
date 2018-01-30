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
#include <QStringList>

class LanguageData : public QSharedData
{
public:
    LanguageData() {}

    explicit LanguageData(const QString &_code) :
        code(_code)
    {
        QLocale l(code);
        name = l.nativeLanguageName();
        if (code.size() > 2) {
            name.append(QLatin1String(" ("));
            name.append(l.nativeCountryName());
            name.append(QLatin1Char(')'));
        }
    }

    LanguageData(const LanguageData &other) :
        QSharedData(other),
        code(other.code),
        name(other.name)
    {}

    ~LanguageData() {}

    QString code;
    QString name;
};

#endif // LANGUAGE_P_H
