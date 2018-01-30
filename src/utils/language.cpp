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
#include <Cutelyst/Plugins/Session/Session>
#include <QLocale>
#include <map>
#include <utility>

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

void Language::setLang(Cutelyst::Context *c)
{
    QString lang = Cutelyst::Session::value(c, QStringLiteral("lang")).toString();
    if (Q_UNLIKELY(lang.isEmpty())) {
        const QStringList acceptedLangs = c->req()->header(QStringLiteral("Accept-Language")).split(QLatin1Char(','), QString::SkipEmptyParts);
        if (Q_LIKELY(!acceptedLangs.empty())) {
            std::map<float,QString> langMap;
            for (const QString &al : acceptedLangs) {
                const int scidx = al.indexOf(QLatin1Char(';'));
                float priority = 1.0f;
                QString langPart;
                bool ok = true;
                if (scidx > -1) {
                    langPart = al.left(scidx);
                    const QStringRef ref = al.midRef(scidx +1);
                    priority = ref.mid(ref.indexOf(QLatin1Char('=')) + 1).toFloat(&ok);
                } else {
                    langPart = al;
                }
                if (ok && !langPart.isEmpty()) {
                    auto search = langMap.find(priority);
                    if (search == langMap.cend()) {
                        langMap.insert({priority, langPart});
                    }
                }
            }
            if (!langMap.empty()) {
                auto i = langMap.crbegin();
                while (i != langMap.crend()) {
                    if (Language::supportedLangsList().contains(i->second)) {
                        lang = i->second;
                        break;
                    }
                    ++i;
                }
            }
        }
        if (lang.isEmpty()) {
            lang = SkaffariConfig::defLanguage();
        }
    }

    c->setLocale(QLocale(lang));
    c->setStash(QStringLiteral("lang"), lang);
}
