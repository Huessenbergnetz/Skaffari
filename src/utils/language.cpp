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
#include "skaffariconfig.h"
#include <Cutelyst/Context>
#include <Cutelyst/Plugins/Utils/LangSelect>
#include <QLocale>
#include <algorithm>

Language::Language() :
    d(new LanguageData)
{

}

Language::Language(const QLocale &locale) :
    d(new LanguageData(locale))
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
    return d->locale.name();
}

QString Language::name() const
{
    return d->locale.nativeLanguageName() + QLatin1String(" (") + d->locale.nativeCountryName() + QLatin1Char(')');
}

QStringList Language::supportedLangsList()
{
    QStringList list;
    const QVector<QLocale> locales = Cutelyst::LangSelect::getSupportedLocales();
    list.reserve(locales.size());
    for (const QLocale &l : locales) {
        list.push_back(l.name());
    }
    return list;
}

QVector<Language> Language::supportedLangs(Cutelyst::Context *c)
{
    QVector<Language> langs;
    const QVector<QLocale> locales = Cutelyst::LangSelect::getSupportedLocales();
    langs.reserve(locales.size());
    for (const QLocale &l : locales) {
        langs.push_back(Language(l));
    }
    if (langs.size() > 1) {
        LanguageNameCollator lnc(c->locale());
        std::sort(langs.begin(), langs.end(), lnc);
    }
    return langs;
}
