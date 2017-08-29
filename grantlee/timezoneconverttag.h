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

#ifndef TIMEZONECONVERTTAG_H
#define TIMEZONECONVERTTAG_H

#include <grantlee/filter.h>
#include <grantlee/safestring.h>
#include <grantlee/util.h>
#include <grantlee/node.h>

class TimeZoneConvertTag : public Grantlee::AbstractNodeFactory
{
    Q_OBJECT
public:
    Grantlee::Node *getNode(const QString &tagContent, Grantlee::Parser *p) const override;
};


class TimeZoneConvert : public Grantlee::Node
{
    Q_OBJECT
public:
    explicit TimeZoneConvert(const Grantlee::FilterExpression &dateTime, const Grantlee::FilterExpression &format, Grantlee::Parser *parser = nullptr);

    void render(Grantlee::OutputStream *stream, Grantlee::Context *gc) const override;

private:
    mutable QString m_cutelystContext = QStringLiteral("c");
    Grantlee::FilterExpression m_dateTime;
    Grantlee::FilterExpression m_format;
};

#endif // TIMEZONECONVERTTAG_H
