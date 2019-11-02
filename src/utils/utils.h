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

#ifndef UTILS_H
#define UTILS_H

#include "../../common/global.h"
#include <QDateTime>
#include <Cutelyst/ParamsMultiMap>
#include <Cutelyst/Response>

namespace Cutelyst {
class Context;
}

class QJsonObject;
class SkaffariError;

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
     * \brief Returns \c true if \a params contains the \a field with with a specific value.
     *
     * Values that are treated as boolean \c true are the following strings:
     * \li 1
     * \li on
     * \li true
     */
    static bool checkCheckbox(const Cutelyst::ParamsMultiMap &params, const QString &field);

    /*!
     * \brief Sets the Response of the Context \a c to the status 405 and adds an error message.
     *
     * The HTTP status code of the Respsonse will be set to 405 (Method Not Allowed) and the JSON
     * object to respond will contain a key named \a error_msg that will contain a human readable
     * error message string. This will also set the Repsonse Header \a Allow to \c POST.
     *
     * \param c     Pointer to the current context.
     */
    static bool ajaxPostOnly(Cutelyst::Context *c, bool isAjax);

    /*!
     * \brief Converts a string \a str into a database ID of type dbid_t.
     * \param str       Input string.
     * \param ok        If not a \c nullptr, failure is reported by setting it to \c false and success by setting it to \c true.
     * \param errorMsg  If not an empty string, will put a SkaffariError object with the message and of type SkaffariError::InputError to the stash and will detach context to the error function.
     * \param c         Pointer to the current context.
     * \return The string converted into database ID type dbid_t. Returns \c 0 if the conversion fails.
     */
    static dbid_t strToDbid(const QString &str, bool *ok = nullptr, const QString &errorMsg = QString(), Cutelyst::Context *c = nullptr);

    /*!
     * \brief Sets error data depending on request type.
     *
     * If Cutelyst::Request::xhr() returns \c true, the \a errorMsg will be put into
     * the \a json object with the key “error_msg“. Otherwise it will be put into the
     * context stash with the key “error_msg“.
     *
     * \param c         Pointer to the current context.
     * \param json      JSON object used if request is xhr.
     * \param errorMsg  The error message to set.
     * \param status    The response status code to set.
     */
    static void setError(Cutelyst::Context *c, QJsonObject &json, const QString &errorMsg, Cutelyst::Response::HttpStatus status);

    /*!
     * \brief Sets \a error data depending on request type.
     *
     * If Cutelyst::Request::xhr() returns \c true, the SkaffariError::errorText() of \a error
     * will be put into the \a json object with the key “error_msg“. Otherwise it will be put into the
     * context stash with the key “error_msg“.
     *
     * Additionally the response status will be set to the return value of SkaffariError::status().
     *
     * \param c     Pointer to the current context.
     * \param json  JSON object used if request is xhr.
     * \param error The error object to the get text and status from.
     */
    static void setError(Cutelyst::Context *c, QJsonObject &json, const SkaffariError &error);

private:
    // prevent construction
    Utils();
    ~Utils();
};

#endif // UTILS_H
