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

#include <grantlee5/grantlee/metatype.h>
#include <QString>
#include <QVariant>
#include <QLocale>
#include <vector>

class QLocale;

namespace Cutelyst {
class Context;
}

/*!
 * \ingroup skaffaricore
 * \brief Contains information about a supported language.
 */
class Language
{
public:
    /*!
     * \brief Constructs an empty %Language object with C locale.
     */
    Language();
    /*!
     * \brief Constructs a %Language object for the given \a locale.
     */
    explicit Language(const QLocale &locale);
    /*!
     * \brief Constructs a copy of \a other.
     */
    Language(const Language &other);
    /*!
     * \brief Move-constructs a %Language instance, making it point at the same object that \a other was pointing to.
     */
    Language(Language &&other) noexcept;
    /*!
     * \brief Assigns \a other to this %Language instance and returns a reference to this %Language.
     */
    Language& operator=(const Language &other);
    /*!
     * \brief Move-assigns \a other to this %Language instance.
     */
    Language& operator=(Language &&other) noexcept;
    /*!
     * Destroys the %Language instance.
     */
    ~Language();

    /*!
     * \brief Swaps this %Language instance with \a other.
     */
    void swap(Language &other) noexcept;

    /*!
     * \brief Returns QLocale::name() from the internal QLocale object.
     */
    QString code() const;

    /*!
     * \brief Returns QLocale::nativeLanguageName() and QLocale::nativeCountryName().
     */
    QString name() const;

    /*!
     * \brief Returns a list of of supported language codes from QLocale::name().
     */
    static QStringList supportedLangsList();

    /*!
     * \brief Returns a vector of supported %Language objects.
     */
    static QVector<Language> supportedLangs(Cutelyst::Context *c);

private:
    QLocale m_locale;
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
