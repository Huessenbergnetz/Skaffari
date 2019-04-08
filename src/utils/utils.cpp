/*
 * Skaffari - a mail account administration web interface based on Cutelyst
 * Copyright (C) 2017-2019 Matthias Fehring <mf@huessenbergnetz.de>
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
#include <Cutelyst/Response>
#include <Cutelyst/Plugins/Session/Session>
#include <QTimeZone>
#include <QLocale>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QJsonObject>
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
            sizeStr = c->locale().toString(static_cast<double>(sizeInByte)/1024.0, 'f', 2);
            sizeStr.append(QLatin1String(" KiB"));
        } else if (sizeInByte < Q_UINT64_C(1073741824)) {
            sizeStr = c->locale().toString(static_cast<double>(sizeInByte)/1048576.0, 'f', 2);
            sizeStr.append(QLatin1String(" MiB"));
        } else if (sizeInByte < Q_UINT64_C(1099511627776)) {
            sizeStr = c->locale().toString(static_cast<double>(sizeInByte)/1073741824.0, 'f', 2);
            sizeStr.append(QLatin1String(" GiB"));
        } else if (sizeInByte < Q_UINT64_C(1125899906842624)) {
            sizeStr = c->locale().toString(static_cast<double>(sizeInByte)/1099511627776.0, 'f', 2);
            sizeStr.append(QLatin1String(" TiB"));
        } else if (sizeInByte < Q_UINT64_C(1152921504606846976)) {
            sizeStr = c->locale().toString(static_cast<double>(sizeInByte)/1125899906842624.0, 'f', 2);
            sizeStr.append(QLatin1String(" PiB"));
        } else {
            sizeStr = c->locale().toString(static_cast<double>(sizeInByte)/1152921504606846976.0, 'f', 2);
            sizeStr.append(QLatin1String(" EiB"));
        }

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    }
#endif

    return sizeStr;
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

bool Utils::ajaxPostOnly(Cutelyst::Context *c, bool isAjax)
{
    if (isAjax && !c->req()->isPost()) {
        c->response()->setStatus(Cutelyst::Response::MethodNotAllowed);
        c->response()->setHeader(QStringLiteral("Allow"), QStringLiteral("POST"));
        c->response()->setJsonObjectBody({{QStringLiteral("error_msg"), c->translate("Skaffari::Utils", "For AJAX requests, this route is only available via POST requests.")}});
        return true;
    } else {
        return false;
    }
}

dbid_t Utils::strToDbid(const QString &str, bool *ok)
{
    dbid_t val = 0;
    bool _ok = true;
    const qlonglong tmpval = str.toLongLong(&_ok);
    if (_ok) {
        if (Q_UNLIKELY(tmpval < static_cast<qlonglong>(std::numeric_limits<dbid_t>::min()) || tmpval > static_cast<qlonglong>(std::numeric_limits<dbid_t>::max()))) {
            _ok = false;
        } else {
            val = static_cast<dbid_t>(tmpval);
        }
    }
    if (ok) {
        *ok = _ok;
    }
    return val;
}
