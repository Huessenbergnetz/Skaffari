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
     * \brief Constructs a new %Utils object.
     */
    Utils();

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
     * \brief Returns the name of the current user.
     * \param c The current Context.
     * \return name of the current user.
     */
    static QString getUserName(Cutelyst::Context *c);

    /*!
     * \brief Converts a human readable quota size string into KiB.
     *
     * The string has to be in a format like \a 7,5 \a GiB.
     *
     * \param c     Context object, used for localized number extraction.
     * \param size  The size string to convert.
     * \param ok    Pointer to a boolean value that will be \c true if conversion succeeded.
     * \return      The quota size in KiB.
     */
    static quota_size_t humanToIntSize(Cutelyst::Context *c, const QString &size, bool *ok);

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
};

#endif // UTILS_H
