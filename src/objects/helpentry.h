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

#ifndef HELPENTRY_H
#define HELPENTRY_H

#include <QSharedDataPointer>
#include <grantlee5/grantlee/metatype.h>

class HelpEntryData;

/*!
 * \ingroup skaffaricore
 * \brief Represents a single help entry.
 */
class HelpEntry
{
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
    QString getTitle() const;

    /*!
     * \brief Returns the help text.
     */
    QString getText() const;

protected:
    QSharedDataPointer<HelpEntryData> d;
};

Q_DECLARE_METATYPE(HelpEntry)
Q_DECLARE_TYPEINFO(HelpEntry, Q_MOVABLE_TYPE);

GRANTLEE_BEGIN_LOOKUP(HelpEntry)
if (property == QLatin1String("title")) {
    return QVariant(object.getTitle());
} else if (property == QLatin1String("text")) {
    return QVariant(object.getText());
}
return QVariant();
GRANTLEE_END_LOOKUP

typedef QHash<QString, HelpEntry> HelpHash;
Q_DECLARE_METATYPE(HelpHash)

#endif // HELPENTRY_H
