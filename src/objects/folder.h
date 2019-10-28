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

#ifndef FOLDER_H
#define FOLDER_H

#include <QSharedDataPointer>
#include <grantlee5/grantlee/metatype.h>
#include "../../common/global.h"
#include "../imap/skaffariimap.h"

/*!
 * \ingroup skaffaricore
 * \brief Contains information about a single default folder.
 */
class Folder
{
public:
    /*!
     * \brief Constructs an invalid, empty %Folder object.
     */
    Folder();

    /*!
     * \brief Constructs a new %Folder with the given parameters.
     * \param id        Database ID.
     * \param domainId  Database ID of the domain the folder belongs to.
     * \param name      Name of the folder.
     */
    Folder(dbid_t id, dbid_t domainId, const QString &name, SkaffariIMAP::SpecialUse specialUse);

    /*!
     * \brief Constructs a copy of \a other.
     */
    Folder(const Folder &other);

    /*!
     * \brief Move-constructs a %Folder instance, making it point at the same object that \a other was pointing to.
     */
    Folder(Folder &&other) noexcept;

    /*!
     * \brief Assigns \a other to ths %Folder and returns a reference to this %Folder.
     */
    Folder& operator=(const Folder &other);

    /*!
     * \brief Move-assigns \a other to this %Folder.
     */
    Folder& operator=(Folder &&other) noexcept;

    /*!
     * \brief Destroys the %Folder object.
     */
    ~Folder();

    /*!
     * \brief Swaps this %Folder instance with \a other.
     */
    void swap(Folder &other) noexcept;

    /*!
     * \brief Returns the database ID of the folder.
     */
    dbid_t getId() const;

    /*!
     * \brief Returns the database ID of the domain the folder belongs to.
     */
    dbid_t getDomainId() const;

    /*!
     * \brief Returns the name of the folder.
     */
    QString getName() const;

    /*!
     * \brief Returns the special use flag of the folder.
     */
    SkaffariIMAP::SpecialUse getSpecialUse() const;

    bool operator==(const Folder &other) const;

    bool operator!=(const Folder &other) const;

private:
    class Data;
    QSharedDataPointer<Data> d;

    friend QDataStream &operator<<(QDataStream &stream, const Folder &folder);
    friend QDataStream &operator>>(QDataStream &stream, Folder &folder);
};

Q_DECLARE_METATYPE(Folder)
Q_DECLARE_TYPEINFO(Folder, Q_MOVABLE_TYPE);

/*!
 * \relates Folder
 * \brief Writes the \a folder to the \a dbg stream and returns the stream.
 */
QDebug operator<<(QDebug dbg, const Folder &folder);

/*!
 * \relates Folder
 * \brief Writes the given \a folder to the given \a stream.
 */
QDataStream &operator<<(QDataStream &stream, const Folder &folder);

/*!
 * \relates Folder
 * \brief Reads a %Folder from the given \a stream and stores it in the given \a folder.
 */
QDataStream &operator>>(QDataStream &stream, Folder &folder);

GRANTLEE_BEGIN_LOOKUP(Folder)
if (property == QLatin1String("id")) {
    return QVariant(object.getId());
} else if (property == QLatin1String("domainId")) {
    return QVariant(object.getDomainId());
} else if (property == QLatin1String("name")) {
    return QVariant(object.getName());
}
return QVariant();
GRANTLEE_END_LOOKUP

#endif // FOLDER_H
