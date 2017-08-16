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

#include "language_p.h"
#include "../common/config.h"

Language::Language() :
    d(new LanguageData)
{

}


Language::Language(const QString &langCode) :
    d(new LanguageData(langCode))
{

}


Language::Language(const Language &other) :
    d(other.d)
{

}


Language& Language::operator=(const Language &other)
{
    d = other.d;
    return *this;
}


Language::~Language()
{

}



QString Language::code() const
{
    return d->code;
}



QString Language::name() const
{
    return d->name;
}


QStringList Language::supportedLangsList()
{
    static QStringList list = SKAFFARI_SUPPORTED_LANGS;

    return list;
}



QVector<Language> Language::supportedLangs()
{
    QVector<Language> langs;

    const QStringList list = Language::supportedLangsList();
    for (const QString &lang : list) {
        langs.push_back(Language(lang));
    }

    return langs;
}
