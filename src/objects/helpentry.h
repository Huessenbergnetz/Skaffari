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

#ifndef HELPENTRY_H
#define HELPENTRY_H

#include <QObject>
#include <QSharedDataPointer>

/*!
 * \ingroup skaffariobjects
 * \brief Represents a single help entry.
 */
class HelpEntry
{
    Q_GADGET
    Q_PROPERTY(QString title READ title CONSTANT)
    Q_PROPERTY(QString text READ text CONSTANT)
public:
    /*!
     * \brief Constructs an invalid, empty %HelpEntry object.
     */
    HelpEntry();

    /*!
     * \brief Constructs a new %HelpEntry with the given parameters.
     * \param title Title of the help entry.
     * \param text  Text of the help entry.
     */
    HelpEntry(const QString &title, const QString &text);

    /*!
     * \brief Contstructs a copy of \a other.
     */
    HelpEntry(const HelpEntry &other);

    /*!
     * \brief Move-constructs a %HelpEntry instance, making it point to the same object that \a other was pointing to.
     */
    HelpEntry(HelpEntry &&other) noexcept;

    /*!
     * \brief Assings \a other to this %HelpEntry instance and returns a reference to this %HelpEntry.
     */
    HelpEntry& operator=(const HelpEntry &other);

    /*!
     * \brief Move-assigns \a other to this %HelpEntry instance.
     */
    HelpEntry& operator=(HelpEntry &&other) noexcept;

    /*!
     * \brief Destroys this %HelpEntry instance.
     */
    ~HelpEntry();

    /*!
     * \brief Swapts this %HelpEntry instance with \a other.
     */
    void swap(HelpEntry &other) noexcept;

    /*!
     * \brief Returns the title of the help entry.
     */
    QString title() const;

    /*!
     * \brief Returns the help text.
     */
    QString text() const;

protected:
    class Data;
    QSharedDataPointer<Data> d;
};

Q_DECLARE_METATYPE(HelpEntry)
Q_DECLARE_TYPEINFO(HelpEntry, Q_MOVABLE_TYPE);

typedef QHash<QString, HelpEntry> HelpHash;
Q_DECLARE_METATYPE(HelpHash)

#endif // HELPENTRY_H
