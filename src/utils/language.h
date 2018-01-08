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

#ifndef LANGUAGE_H
#define LANGUAGE_H

#include <QString>
#include <QSharedDataPointer>
#include <grantlee5/grantlee/metatype.h>
#include <QVariant>
#include <QVector>

namespace Cutelyst {
class Context;
}

class LanguageData;

/*!
 * \ingroup skaffaricore
 * \brief Contains information about a supported language.
 */
class Language
{
public:
    Language();
    explicit Language(const QString &langCode);
    Language(const Language &other);
    Language& operator=(const Language &other);
    ~Language();

    QString code() const;
    QString name() const;

    static QStringList supportedLangsList();
    static QVector<Language> supportedLangs();
    static void setLang(Cutelyst::Context *c);

private:
    QSharedDataPointer<LanguageData> d;
};

Q_DECLARE_METATYPE(Language)
Q_DECLARE_TYPEINFO(Language, Q_MOVABLE_TYPE);

GRANTLEE_BEGIN_LOOKUP(Language)
QVariant var;
if (property == QLatin1String("code")) {
    var.setValue(object.code());
} else if (property == QLatin1String("name")) {
    var.setValue(object.name());
}
return var;
GRANTLEE_END_LOOKUP

#endif // LANGUAGE_H
