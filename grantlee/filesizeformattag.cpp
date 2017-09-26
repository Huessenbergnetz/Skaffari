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

#include "filesizeformattag.h"

#include <grantlee/exception.h>
#include <grantlee/parser.h>
#include <grantlee/safestring.h>

#include <Cutelyst/Context>

#include <QLocale>

Grantlee::Node *FileSizeFormatTag::getNode(const QString &tagContent, Grantlee::Parser *p) const
{
    QStringList parts = smartSplit(tagContent);
    parts.removeFirst(); // not intereseted in the name of the tag
    if (parts.empty()) {
        throw Grantlee::Exception(Grantlee::TagSyntaxError, QStringLiteral("sk_fsf requires at least the file size"));
    }

    Grantlee::FilterExpression size(parts.at(0), p);

    Grantlee::FilterExpression precision;
    if (parts.size() > 1) {
        precision = Grantlee::FilterExpression(parts.at(1), p);
    }

    Grantlee::FilterExpression base;
    if (parts.size() > 2) {
        base = Grantlee::FilterExpression(parts.at(2), p);
    }

    Grantlee::FilterExpression multiplier;
    if (parts.size() > 3) {
        multiplier = Grantlee::FilterExpression(parts.at(3), p);
    }


    return new FileSizeFormat(size, base, precision, multiplier, p);
}


FileSizeFormat::FileSizeFormat(const Grantlee::FilterExpression &size, const Grantlee::FilterExpression &base, const Grantlee::FilterExpression &precision, const Grantlee::FilterExpression &multiplier, Grantlee::Parser *parser) :
    Grantlee::Node(parser), m_size(size), m_base(base), m_precision(precision), m_multiplier(multiplier)
{

}


void FileSizeFormat::render(Grantlee::OutputStream *stream, Grantlee::Context *gc) const
{
    const QVariant sizeVar = m_size.resolve(gc);
    const int sizeVarType = sizeVar.userType();
    if (sizeVarType == qMetaTypeId<Grantlee::SafeString>()) {
        *stream << sizeVar.value<Grantlee::SafeString>().get();
        return;
    } else if (sizeVarType == QVariant::String) {
        *stream << sizeVar.toString();
        return;
    }

    // In case cutelyst context is not set as "c"
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

        if (!c) {
            qWarning("%s", "Can not find Cutelyst context.");
            return;
        }
    }

    bool convertNumbers = true;

    qreal size = m_size.resolve(gc).toReal(&convertNumbers);
    if (!convertNumbers) {
        qWarning("%s", "Failed to convert input file size into a floating point number.");
        return;
    }

    int base = m_base.isValid() ? m_base.resolve(gc).toInt(&convertNumbers) : 2;
    if (!convertNumbers) {
        qWarning("%s", "Failed to convert base system for file size into integer value. Using default value 2.");
    }

    int precision = m_precision.isValid() ? m_precision.resolve(gc).toInt(&convertNumbers) : 2;
    if (!convertNumbers) {
        qWarning("%s", "Failed to convert decimal precision for file size into an integer value. Using default value 2.");
        precision = 2;
    }

    qreal multiplier = m_multiplier.isValid() ? m_multiplier.resolve(gc).toReal(&convertNumbers) : 1.0f;
    if (!convertNumbers) {
        qWarning("%s", "Failed to convert multiplier file size into a floating point number. Using default value 1.0.");
        multiplier = 1.0f;
    }

    if (multiplier == 0.0f) {
        qWarning("%s", "It makes no sense to mulitply the file size by zero. Using default value 1.0.");
        multiplier = 1.0f;
    }

    size *= multiplier;

    static const QStringList binaryUnits({
                                             QStringLiteral("B"),
                                             QStringLiteral("KiB"),
                                             QStringLiteral("MiB"),
                                             QStringLiteral("GiB"),
                                             QStringLiteral("TiB"),
                                             QStringLiteral("PiB"),
                                             QStringLiteral("EiB"),
                                             QStringLiteral("ZiB"),
                                             QStringLiteral("YiB")
                                         });

    static const QStringList decimalUnits({
                                              QStringLiteral("B"),
                                              QStringLiteral("KB"),
                                              QStringLiteral("MB"),
                                              QStringLiteral("GB"),
                                              QStringLiteral("TB"),
                                              QStringLiteral("PB"),
                                              QStringLiteral("EB"),
                                              QStringLiteral("ZB"),
                                              QStringLiteral("YB")
                                          });

    bool found = false;
    int count = 0;
    const qreal baseVal = (base == 10) ? 1000.0f : 1024.0f;
    qreal current = 1.0f;
    while (!found && (count < decimalUnits.size())) {
        current *= baseVal;
        if (size < current) {
            found = true;
            break;
        }
        count++;
    }

    qreal devider = current/baseVal;
    size = size/devider;

    const QString sizeStr = c->locale().toString(size, 'f', precision) + QLatin1Char(' ') + ((base == 10) ? decimalUnits.at(count) : binaryUnits.at(count));

    *stream << sizeStr;
}

#include "moc_filesizeformattag.cpp"
