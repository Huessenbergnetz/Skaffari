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

#include "language.h"
#include "../common/config.h"
#include "skaffariconfig.h"
#include <Cutelyst/Context>
#include <Cutelyst/Plugins/Utils/LangSelect>
#include <QCollator>
#include <algorithm>


class LanguageNameCollator : public QCollator
{
public:
    explicit LanguageNameCollator(const QLocale &locale) :
        QCollator(locale)
    {}

    bool operator() (const Language &left, const Language &right) { return (compare(left.name(), right.name())); }
};

Language::Language()
{

}

Language::Language(const QLocale &locale) :
    m_locale(locale)
{

}

Language::Language(const Language &other) :
    m_locale(other.m_locale)
{

}

Language::Language(Language &&other) noexcept :
    m_locale(std::move(other.m_locale))
{

}

Language& Language::operator=(const Language &other)
{
    m_locale = other.m_locale;
    return *this;
}

Language& Language::operator=(Language &&other) noexcept
{
    swap(other);
    return *this;
}

Language::~Language()
{

}

void Language::swap(Language &other) noexcept
{
    std::swap(m_locale, other.m_locale);
}

QString Language::code() const
{
    return m_locale.name();
}

QString Language::name() const
{
    return m_locale.nativeLanguageName() + QLatin1String(" (") + m_locale.nativeCountryName() + QLatin1Char(')');
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
