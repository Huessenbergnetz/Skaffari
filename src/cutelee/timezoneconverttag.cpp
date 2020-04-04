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

#include "timezoneconverttag.h"

#include <cutelee/exception.h>
#include <cutelee/parser.h>
#include <cutelee/safestring.h>

#include <Cutelyst/Context>
#include <Cutelyst/Plugins/Session/Session>

#include <QLocale>
#include <QDateTime>
#include <QTimeZone>

Q_DECLARE_METATYPE(QTimeZone)

TimeZoneConvertTag::TimeZoneConvertTag(QObject *parent) : Cutelee::AbstractNodeFactory(parent)
{

}

Cutelee::Node *TimeZoneConvertTag::getNode(const QString &tagContent, Cutelee::Parser *p) const
{
    QStringList parts = smartSplit(tagContent);
    parts.removeFirst();
    if (parts.empty()) {
        throw Cutelee::Exception(Cutelee::TagSyntaxError, QStringLiteral("sk_tzc requires at least the date or time"));
    }

    Cutelee::FilterExpression dateTime(parts.at(0), p);

    Cutelee::FilterExpression format;
    if (parts.size() > 1) {
        format = Cutelee::FilterExpression(parts.at(1), p);
    }

    return new TimeZoneConvert(dateTime, format, p);
}


TimeZoneConvert::TimeZoneConvert(const Cutelee::FilterExpression &dateTime, const Cutelee::FilterExpression &format, Cutelee::Parser *parser) :
    Cutelee::Node(parser), m_dateTime(dateTime), m_format(format)
{

}


void TimeZoneConvert::render(Cutelee::OutputStream *stream, Cutelee::Context *gc) const
{
    const QVariant dtVar = m_dateTime.resolve(gc);
    const auto dtVarType = dtVar.userType();
    if (dtVarType == qMetaTypeId<Cutelee::SafeString>()) {
        *stream << dtVar.value<Cutelee::SafeString>().get();
        return;
    } else if (dtVarType == QVariant::String) {
        *stream << dtVar.toString();
        return;
    } else {
        switch (dtVarType) {
        case QVariant::DateTime:
        case QVariant::Date:
        case QVariant::Time:
            break;
        default:
            qWarning("%s", "sk_tzc can only operate on QDateTime values.");
            return;
        }
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

    QString formatString;
    if (m_format.isValid()) {
        const QVariant formatVar = m_format.resolve(gc);
        const int formatVarType = formatVar.userType();
        if (formatVarType == qMetaTypeId<Cutelee::SafeString>()) {
            formatString = formatVar.value<Cutelee::SafeString>().get();
        } else if (formatVarType == QVariant::String) {
            formatString = formatVar.toString();
        }
    }

    if (dtVarType == QVariant::DateTime) {

        QDateTime dtVal = dtVar.toDateTime();

        const QTimeZone userTz = Cutelyst::Session::value(c, QStringLiteral("timeZone"), QVariant::fromValue<QTimeZone>(QTimeZone::utc())).value<QTimeZone>();

        if (userTz != QTimeZone::utc()) {
            dtVal.setTimeSpec(Qt::UTC);
            dtVal = dtVal.toTimeZone(userTz);
        }

        if (formatString.isEmpty()) {
            *stream << c->locale().toString(dtVal, QLocale::ShortFormat);
        } else {
            *stream << c->locale().toString(dtVal, formatString);
        }

    } else if (dtVarType == QVariant::Date) {

        const QDate dateVal = dtVar.toDate();

        if (formatString.isEmpty()) {
            *stream << c->locale().toString(dateVal, QLocale::ShortFormat);
        } else {
            *stream << c->locale().toString(dateVal, formatString);
        }

    } else if (dtVarType == QVariant::Time) {

        const QTime timeVal = dtVar.toTime();

        if (formatString.isEmpty()) {
            *stream << c->locale().toString(timeVal, QLocale::ShortFormat);
        } else {
            *stream << c->locale().toString(timeVal, formatString);
        }

    }

}

#include "moc_timezoneconverttag.cpp"
