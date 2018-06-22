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

#ifndef UTILS_H
#define UTILS_H

#include <QDateTime>
#include <Cutelyst/ParamsMultiMap>
#include "../../common/global.h"

namespace Cutelyst {
class Context;
}

/*!
 * \ingroup skaffaricore
 * \brief Small helper methods.
 */
class Utils
{
public:
    /*!
     * \brief Converts the \a dt to the time zone of the current user in the context \a c.
     * \param c     The current Context to get the user from.
     * \param dt    The date and time to convert.
     * \return date and time converted into current user's time zone.
     */
    static QDateTime toUserTZ(Cutelyst::Context *c, const QDateTime &dt);

    /*!
     * \brief Returns a human readable formatted data size string.
     *
     * On Qt >= 5.10 and if the \a sizeInByte fits into qint64, this uses
     * QLocale::formattedDataSize(), otherwise it uses a similar but Skaffari
     * specific implementation.
     *
     * \param c             The current Context, used for localization.
     * \param sizeInByte    The data size in bytes.
     * \return human readable formatted data size string.
     */
    static QString humanBinarySize(Cutelyst::Context *c, quota_size_t sizeInByte);

    /*!
     * \brief Returns \c true if the current request is made via AJAX.
     *
     * The request will be considered as AJAX request if the \a Accept HTTP header
     * contains \c application/json.
     *
     * \param c Pointer to the current context.
     * \return  \c true if the request is made via AJAX.
     */
    static bool isAjax(Cutelyst::Context *c);

    /*!
     * \brief Returns \c true if \a params contains the \a field with with a specific value.
     *
     * Values that are treated as boolean \c true are the following strings:
     * \li 1
     * \li on
     * \li true
     */
    static bool checkCheckbox(const Cutelyst::ParamsMultiMap &params, const QString &field);

private:
    // prevent construction
    Utils();
    ~Utils();
};

#endif // UTILS_H
