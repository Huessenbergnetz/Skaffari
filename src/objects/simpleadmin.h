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

#ifndef SIMPLEADMIN_H
#define SIMPLEADMIN_H

#include <QString>
#include <QSharedDataPointer>
#include <grantlee5/grantlee/metatype.h>
#include <QVariant>
#include "../../common/global.h"

class SimpleAdminData;

class SimpleAdmin
{
public:
    SimpleAdmin();
    SimpleAdmin(dbid_t id, const QString &name);
    SimpleAdmin(const SimpleAdmin &other);
    SimpleAdmin& operator=(const SimpleAdmin &other);
    ~SimpleAdmin();

    dbid_t id() const;
    QString name() const;

private:
    QSharedDataPointer<SimpleAdminData> d;
};

Q_DECLARE_METATYPE(SimpleAdmin)
Q_DECLARE_TYPEINFO(SimpleAdmin, Q_MOVABLE_TYPE);

GRANTLEE_BEGIN_LOOKUP(SimpleAdmin)
QVariant var;
if (property == QLatin1String("id")) {
    var.setValue(object.id());
} else if (property == QLatin1String("name")) {
    var.setValue(object.name());
}
return var;
GRANTLEE_END_LOOKUP

#endif // SIMPLEADMIN_H
