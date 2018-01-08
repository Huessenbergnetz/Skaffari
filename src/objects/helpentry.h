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
    HelpEntry();
    HelpEntry(const QString &title, const QString &text);
    HelpEntry(const HelpEntry &other);
    HelpEntry& operator=(const HelpEntry &other);
    ~HelpEntry();

    QString getTitle() const;
    QString getText() const;

protected:
    QSharedDataPointer<HelpEntryData> d;
};

Q_DECLARE_METATYPE(HelpEntry)
Q_DECLARE_TYPEINFO(HelpEntry, Q_MOVABLE_TYPE);

GRANTLEE_BEGIN_LOOKUP(HelpEntry)
QVariant var;
if (property == QLatin1String("title")) {
    var.setValue(object.getTitle());
} else if (property == QLatin1String("text")) {
    var.setValue(object.getText());
}
return var;
GRANTLEE_END_LOOKUP

typedef QHash<QString, HelpEntry> HelpHash;
Q_DECLARE_METATYPE(HelpHash)

#endif // HELPENTRY_H
