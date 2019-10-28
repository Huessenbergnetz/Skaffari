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

#ifndef SKAFFARIGLOBAL_H
#define SKAFFARIGLOBAL_H

#include <QtGlobal>
#include <utility>

/*!
 * \file global.h
 * \brief Global %Skaffari Definitions
 */

/*!
 * \brief 64bit unsigned integer type for mailbox storage quota.
 */
typedef quint64 quota_size_t;

/*!
 * \brief 32bit unsigned integer type for database IDs.
 */
typedef quint32 dbid_t;

/*!
 * \brief Holds information about available and used mailbox storage quota.
 *
 * The first value stores the used quota in KiB, the second value stores the
 * quota limit in KiB.
 */
typedef std::pair<quota_size_t,quota_size_t> quota_pair;

/*!
 * \def SKAFFARI_STRING_TO_DBID(str)
 * \brief Converts a string into an unsigned long integer.
 *
 * The unsigned long integer is the type used to store database IDs.
 */
#ifndef SKAFFARI_STRING_TO_DBID
# define SKAFFARI_STRING_TO_DBID(str) str.toULong()
#endif

#define DEFAULT_FOLDER_TYPES {QStringLiteral("sentFolder"), QStringLiteral("draftsFolder"), QStringLiteral("trashFolder"), QStringLiteral("junkFolder"), QStringLiteral("archiveFolder"), QStringLiteral("otherFolders")}

#endif // SKAFFARIGLOBAL_H
