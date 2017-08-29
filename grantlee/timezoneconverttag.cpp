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

#include "timezoneconverttag.h"

#include <grantlee/exception.h>
#include <grantlee/parser.h>
#include <grantlee/safestring.h>

#include <Cutelyst/Context>
#include <Cutelyst/Plugins/Session/Session>

#include <QLocale>
#include <QDateTime>
#include <QTimeZone>

Grantlee::Node *TimeZoneConvertTag::getNode(const QString &tagContent, Grantlee::Parser *p) const
{
    QStringList parts = smartSplit(tagContent);
    parts.removeFirst();
    if (parts.empty()) {
        throw Grantlee::Exception(Grantlee::TagSyntaxError, QStringLiteral("sk_tzc requires at least the date or time"));
    }

    Grantlee::FilterExpression dateTime(parts.at(0), p);

    Grantlee::FilterExpression format;
    if (parts.size() > 1) {
        format = Grantlee::FilterExpression(parts.at(1), p);
    }

    return new TimeZoneConvert(dateTime, format, p);
}


TimeZoneConvert::TimeZoneConvert(const Grantlee::FilterExpression &dateTime, const Grantlee::FilterExpression &format, Grantlee::Parser *parser) :
    Grantlee::Node(parser), m_dateTime(dateTime), m_format(format)
{

}


void TimeZoneConvert::render(Grantlee::OutputStream *stream, Grantlee::Context *gc) const
{
    const QVariant dtVar = m_dateTime.resolve(gc);
    if (dtVar.userType() != QVariant::DateTime) {
        qWarning("%s", "sk_tzc can only operate on QDateTime values.");
        return;
    }

    auto c = gc->lookup(m_cutelystContext).value<Cutelyst::Context *>();
    if (!c) {
        const QVariantHash hash = gc->stackHash(0);
        auto it = hash.constBegin();
        while (it != hash.constEnd()) {
            if (it.value().userType() == qMetaTypeId<Cutelyst::Context *>()) {
                c = it.value().value<Cutelyst::Context *>();
                if (c) {
                    m_cutelystContext = it.key();
                    break;
                }
            }
            ++it;
        }
    }

    if (!c) {
        qWarning("%s", "Can not find Cutelyst context.");
        return;
    }

    QDateTime dtVal = dtVar.toDateTime();
    QDateTime retVal;

    QTimeZone userTz(Cutelyst::Session::value(c, QStringLiteral("tz"), QStringLiteral("UTZ")).toByteArray());
    if (Q_UNLIKELY(!userTz.isValid())) {
        userTz = QTimeZone::utc();
    }

    if (userTz != QTimeZone::utc()) {
        dtVal.setTimeSpec(Qt::UTC);
        retVal = dtVal.toTimeZone(userTz);
    } else {
        retVal = dtVal;
    }

    QString formatString;
    if (m_format.isValid()) {
        const QVariant formatVar = m_format.resolve(gc);
        const int formatVarType = formatVar.userType();
        if (formatVarType == qMetaTypeId<Grantlee::SafeString>()) {
            formatString = formatVar.value<Grantlee::SafeString>().get();
        } else if (formatVarType == QVariant::String) {
            formatString = formatVar.toString();
        }
    }

    if (formatString.isEmpty()) {
        *stream << c->locale().toString(retVal, QLocale::ShortFormat);
    } else {
        *stream << c->locale().toString(retVal, formatString);
    }
}
