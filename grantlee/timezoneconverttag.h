/*
 * Skaffari - a mail account administration web interface based on Cutelyst
 * Copyright (C) 2017-2018 Matthias Fehring <mf@huessenbergnetz.de>
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

#ifndef TIMEZONECONVERTTAG_H
#define TIMEZONECONVERTTAG_H

#include <grantlee/filter.h>
#include <grantlee/safestring.h>
#include <grantlee/util.h>
#include <grantlee/node.h>

/*!
 * \internal
 * \brief Grantlee node factory for the TimeZoneConvert tag.
 */
class TimeZoneConvertTag : public Grantlee::AbstractNodeFactory
{
    Q_OBJECT
public:
    /*!
     * \brief Returns the TimeZoneConvert node.
     */
    Grantlee::Node *getNode(const QString &tagContent, Grantlee::Parser *p) const override;
};

/*!
 * \ingroup skaffarigrantlee
 * \brief Grantlee template tag to convert a datetime into a specific time zone and format string output.
 *
 * This will take the current user's locale and time zone into account. Time zone and locale will be taken
 * from the current Cutelyst stash.
 *
 * This tak can be used as \c sk_tzc in your Grantlee templates. It accepts up to 2 parameters.
 * The first and required parameter has to be either a QDateTime or a string that can be converted into
 * a QDateTime.
 * The second parameter can be a format string to convert the QDateTime into a human readable string.
 * If the second parameter is omitted, the default short format for the current locale will be used.
 *
 * \par Examples
 * \code
 * // will be converted to "25.10.17 18:44" if the current locale is de and the current timezone is Europe/Berlin
 * {% sk_tzc "2017-10-25T16:44:20Z" %}
 *
 * // will be converted to "25. Oktober 2017" if the current locale is de and the current timezone is Europe/Berlin
 * {% sk_tzc "2017-10-25T16:44:20Z" "d. MMMM yyyy" %}
 * \endcode
 */
class TimeZoneConvert : public Grantlee::Node
{
    Q_OBJECT
public:
    /*!
     * \brief Constructs a new %TimeZoneConvert node with the given parameters.
     * \param dateTime  that datetime that should be converted
     * \param format    the output format
     * \param parser    pointer to the parser
     */
    explicit TimeZoneConvert(const Grantlee::FilterExpression &dateTime, const Grantlee::FilterExpression &format, Grantlee::Parser *parser = nullptr);

    /*!
     * \brief Performs the formatting and renders the result into the \a stream.
     */
    void render(Grantlee::OutputStream *stream, Grantlee::Context *gc) const override;

private:
    mutable QString m_cutelystContext = QStringLiteral("c");
    Grantlee::FilterExpression m_dateTime;
    Grantlee::FilterExpression m_format;
};

#endif // TIMEZONECONVERTTAG_H
