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

#ifndef FOLDER_H
#define FOLDER_H

#include <QSharedDataPointer>
#include <grantlee5/grantlee/metatype.h>
#include "../../common/global.h"

class FolderData;

class Folder
{
public:
    Folder();
    Folder(dbid_t id, dbid_t domainId, const QString &name);
    Folder(const Folder &other);
    Folder& operator=(const Folder &other);
    ~Folder();

    dbid_t getId() const;
    dbid_t getDomainId() const;
    QString getName() const;

    void setId(dbid_t id);
    void setDomainId(dbid_t domainId);
    void setName(const QString &name);

protected:
    QSharedDataPointer<FolderData> d;
};

Q_DECLARE_METATYPE(Folder)
Q_DECLARE_TYPEINFO(Folder, Q_MOVABLE_TYPE);

GRANTLEE_BEGIN_LOOKUP(Folder)
QVariant var;
if (property == QLatin1String("id")) {
    var.setValue(object.getId());
} else if (property == QLatin1String("domainId")) {
    var.setValue(object.getDomainId());
} else if (property == QLatin1String("name")) {
    var.setValue(object.getName());
}
return var;
GRANTLEE_END_LOOKUP

#endif // FOLDER_H
