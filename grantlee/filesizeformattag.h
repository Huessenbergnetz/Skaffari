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

#ifndef FILESIZEFORMATTAG_H
#define FILESIZEFORMATTAG_H

#include <grantlee/filter.h>
#include <grantlee/safestring.h>
#include <grantlee/util.h>
#include <grantlee/node.h>

/*!
 * \internal
 * \brief Grantlee node factory for the FileSizeFormat tag.
 */
class FileSizeFormatTag : public Grantlee::AbstractNodeFactory
{
    Q_OBJECT
public:
    /*!
     * \brief Constructs a new %FileSizeFormatTag object with the given \a parent.
     */
    explicit FileSizeFormatTag(QObject *parent = nullptr);

    /*!
     * \brief Returns the FileSizeFormat node.
     */
    Grantlee::Node *getNode(const QString &tagContent, Grantlee::Parser *p) const override;
};


/*!
 * \ingroup skaffarigrantlee
 * \brief Grantlee template tag to convert a numeric value into a human readable file size.
 *
 * Converts for example 1024 into 1 KiB. It takes the current user's locale into account. The
 * user's locale will be taken from the Cutelyst stash.
 *
 * The tag can be used as \c sk_fsf in your grantlee templates. It accepts up to 4 parameters.
 * The first and required parameter has to be the size value, either as a variable or as a fixed number.
 * The second parameter is the decimal precision for the converted value, the default is 2.
 * The third parameter is the base to be used to convert the number value. It can be either 2 (binary)
 * or 10 (decimal). The default is 2 and any other value than 2 or 10 will be interpreted as 2. Using
 * the binary base will for example give Mebibytes (MiB) instead of Megabytes (MB).
 * The fourth parameter is an optional multiplier every input value will be multiplied by before it
 * will be converted in a file size string.
 *
 * \par Examples
 * \code
 * // will be converted into 2.04 KiB
 * {% sk_fsf 2048 %}
 *
 * // will be converted into 2 KB
 * {% sk_fsf 2048 0 10 %}
 *
 * // will be converted into 2 MiB
 * {% sk_fsf 2048 2 2 1024 %}
 * \endcode
 */
class FileSizeFormat : public Grantlee::Node // clazy:exclude=ctor-missing-parent-argument
{
    Q_OBJECT
public:
    /*!
     * \brief Constructs a new %FileSizeFormat node with the given parameters.
     * \param size          the file size that should be formatted
     * \param base          the base to use, either 2 (binary) or 10 (decimal)
     * \param decimal       the decimal precision for the converted value
     * \param multiplier    value the size should be multiplied with
     * \param parser        pointer to the parser
     */
    explicit FileSizeFormat(const Grantlee::FilterExpression &size, const Grantlee::FilterExpression &base, const Grantlee::FilterExpression &decimal, const Grantlee::FilterExpression &multiplier, Grantlee::Parser *parser = nullptr);

    /*!
     * \brief Performs the formatting and renders the result into the \a stream.
     */
    void render(Grantlee::OutputStream *stream, Grantlee::Context *gc) const override;

private:
    mutable QString m_cutelystContext = QStringLiteral("c");
    Grantlee::FilterExpression m_size;
    Grantlee::FilterExpression m_base;
    Grantlee::FilterExpression m_precision;
    Grantlee::FilterExpression m_multiplier;

};

#endif // FILESIZEFORMATTAG_H
