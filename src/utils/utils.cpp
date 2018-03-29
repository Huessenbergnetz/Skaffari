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

#include "utils.h"
#include <Cutelyst/Context>
#include <Cutelyst/Request>
#include <Cutelyst/Plugins/Session/Session>
#include <QTimeZone>
#include <QLocale>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <cmath>
#include <limits>

Utils::Utils()
{

}

Utils::~Utils()
{

}

QDateTime Utils::toUserTZ(Cutelyst::Context *c, const QDateTime &dt)
{
    QDateTime retVal;

    const QTimeZone userTz(Cutelyst::Session::value(c, QStringLiteral("tz"), QStringLiteral("UTC")).toByteArray());

    if (userTz != QTimeZone::utc()) {
        QDateTime _dt = dt;
        _dt.setTimeSpec(Qt::UTC);
        retVal = _dt.toTimeZone(userTz);
    } else {
        retVal = dt;
    }

    return retVal;
}

QString Utils::humanBinarySize(Cutelyst::Context *c, quota_size_t sizeInByte)
{
    QString sizeStr;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))

    if (sizeInByte < static_cast<quota_size_t>(std::numeric_limits<qint64>::max())) {
        sizeStr = c->locale().formattedDataSize(static_cast<qint64>(sizeInByte));
    } else {

#endif

        if (sizeInByte < Q_UINT64_C(1048576)) {
            const double kibFloat = static_cast<double>(sizeInByte)/1024.0;
            sizeStr = c->locale().toString(kibFloat, 'f', 2);
            sizeStr.append(QLatin1String(" KiB"));
        } else if (sizeInByte < Q_UINT64_C(1073741824)) {
            const double mibFloat = static_cast<double>(sizeInByte)/1048576.0;
            sizeStr = c->locale().toString(mibFloat, 'f', 2);
            sizeStr.append(QLatin1String(" MiB"));
        } else if (sizeInByte < Q_UINT64_C(1099511627776)) {
            const double gibFloat = static_cast<double>(sizeInByte)/1073741824.0;
            sizeStr = c->locale().toString(gibFloat, 'f', 2);
            sizeStr.append(QLatin1String(" GiB"));
        } else if (sizeInByte < Q_UINT64_C(1125899906842624)) {
            const double tibFloat = static_cast<double>(sizeInByte)/1099511627776.0;
            sizeStr = c->locale().toString(tibFloat, 'f', 2);
            sizeStr.append(QLatin1String(" TiB"));
        } else if (sizeInByte < Q_UINT64_C(1152921504606846976)) {
            const double pibFloat = static_cast<double>(sizeInByte)/1125899906842624.0;
            sizeStr = c->locale().toString(pibFloat, 'f', 2);
            sizeStr.append(QLatin1String(" PiB"));
        } else {
            const double eibFloat = static_cast<double>(sizeInByte)/1152921504606846976.0;
            sizeStr = c->locale().toString(eibFloat, 'f', 2);
            sizeStr.append(QLatin1String(" EiB"));
        }

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    }
#endif

    return sizeStr;
}

quota_size_t Utils::humanToIntSize(Cutelyst::Context *c, const QString &size, bool *ok)
{
    quota_size_t ret = 0;

    Q_ASSERT_X(ok, "convert human quota string to KiB", "invalid pointer to a boolean succeed value (ok)");

    QRegularExpression regex(QStringLiteral("(\\d+[,.٫]?\\d*)\\s*([KMGT]?i?B?)"), QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = regex.match(size);

    if (!match.hasMatch()) {
        *ok = false;
        return ret;
    }

    bool _ok = true;

    float _size = c->locale().toFloat(match.captured(1), &_ok);

    if (!_ok) {
        *ok = false;
        return ret;
    }

    const QString mult = match.captured(2);

    if (mult.startsWith(QLatin1Char('G'), Qt::CaseInsensitive)) {
        _size = _size * 1073741824.0f;
    } else if (mult.startsWith(QLatin1Char('M'), Qt::CaseInsensitive)) {
        _size = _size * 1048576.0f;
    } else if (mult.startsWith(QLatin1Char('T'), Qt::CaseInsensitive)) {
        _size = _size * 1099511627776.0f;
    } else {
        _size = _size * 1024.0f;
    }

    _size = _size / 1024.0f;

//    // we have to check if the size fits into the quint32 even after rounding
//    // as rounding can also round up, we will give an extra margin of 1 to the
//    // maximum value of unsigned 32bit integer
//    if (_size > 4294967294.0f) {
//        *ok = false;
//        return ret;
//    }

    qlonglong _ret = std::llround(_size);

    ret = static_cast<quota_size_t>(_ret);

    return ret;
}

bool Utils::isAjax(Cutelyst::Context *c)
{
    return c->req()->header(QStringLiteral("Accept")).contains(QLatin1String("application/json"), Qt::CaseInsensitive);
}

bool Utils::checkCheckbox(const Cutelyst::ParamsMultiMap &params, const QString &field)
{
    bool ret = false;

    if (params.contains(field)) {
        static QStringList allowedVals({QStringLiteral("1"), QStringLiteral("true"), QStringLiteral("on")});
        ret = allowedVals.contains(params.value(field), Qt::CaseInsensitive);
    }

    return ret;
}
