/*
 * Skaffari - a mail account administration web interface based on Cutelyst
 * Copyright (C) 2020 Matthias Fehring <mf@huessenbergnetz.de>
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

#ifndef STRINGFORMATFILTER_H
#define STRINGFORMATFILTER_H

#include <cutelee/filter.h>

/*!
 * \ingroup skaffaricuteleefilters
 * \brief Cutelee template filter to format strings printf style.
 *
 * Formats the variable according to the argument, a string formatting specifier.
 * This specifier uses the printf-style String Formatting syntax, with the exception
 * that the leading “%” is dropped.
 */
class StringformatFilter : public Cutelee::Filter
{
public:
    bool isSafe() const override { return true; }

    QVariant doFilter(const QVariant &input, const QVariant &argument = QVariant(), bool autoescape = false) const override;

private:
    enum LengthModifier {
        hh, h, None, l, ll, j, z, t, L
    };

    enum FormatPosition {
        Start, Flags, MinFieldWidth, Precision, LengthMod, ConvSpec
    };

    LengthModifier getLengthModifier(const QChar &c, LengthModifier current) const;

    template< typename T >
    T convertUNumber(const QVariant &input, bool *ok) const;
};

#endif // STRINGFORMATFILTER_H
