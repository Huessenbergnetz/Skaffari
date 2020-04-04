/*
 * Skaffari - a mail account administration web interface based on Cutelyst
 * Copyright (C) 2018 Matthias Fehring <mf@huessenbergnetz.de>
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

#include "admintypetag.h"
#include "../src/objects/adminaccount.h"
#include <cutelee/exception.h>
#include <cutelee/parser.h>
#include <Cutelyst/Context>

AdminTypeTag::AdminTypeTag(QObject *parent) : Cutelee::AbstractNodeFactory(parent)
{

}

Cutelee::Node *AdminTypeTag::getNode(const QString &tagContent, Cutelee::Parser *p) const
{
    QStringList parts = smartSplit(tagContent);
    parts.removeFirst(); // not interested in the name of the tag
    if (parts.empty()) {
        throw Cutelee::Exception(Cutelee::TagSyntaxError, QStringLiteral("sk_admintypename requires at least the AdminAccount object"));
    }

    Cutelee::FilterExpression admin(parts.at(0), p);

    return new AdminType(admin, p);
}

AdminType::AdminType(const Cutelee::FilterExpression &admin, Cutelee::Parser *parser) :
    Cutelee::Node(parser), m_admin(admin)
{

}

void AdminType::render(Cutelee::OutputStream *stream, Cutelee::Context *gc) const
{
    const QVariant adminVar = m_admin.resolve(gc);
    AdminAccount::AdminAccountType type;
    if (adminVar.canConvert<AdminAccount>()) {
        const AdminAccount a = adminVar.value<AdminAccount>();

        if (!a.isValid()) {
            qWarning("%s", "Invalid AdminAcount object.");
            return;
        }

        type = a.type();
    } else if (adminVar.canConvert<AdminAccount::AdminAccountType>()) {
        type = adminVar.value<AdminAccount::AdminAccountType>();
    } else if (adminVar.canConvert<quint8>()) {
        type = static_cast<AdminAccount::AdminAccountType>(adminVar.value<quint8>());
    } else if (adminVar.canConvert<Cutelee::SafeString>()) {
        type = static_cast<AdminAccount::AdminAccountType>(adminVar.value<Cutelee::SafeString>().get().toUShort());
    } else {
        qWarning("%s", "Failed to convert input into AdminAccount object.");
        return;
    }



    // In case cutelyst context is not set as "c"
    auto c = gc->lookup(m_cutelystContext).value<Cutelyst::Context *>();
    if (!c) {
        const QVariantHash hash = gc->stackHash(0);
        auto it = hash.constBegin();
        while (it != hash.constEnd()) {
            if (it.value().userType() == qMetaTypeId<Cutelyst::Context *>()) {
                c = it.value().value<Cutelyst::Context *>();
                if (c) {
                    m_cutelystContext = it.key();
                    break;
                }
            }
            ++it;
        }

        if (!c) {
            qWarning("%s", "Can not find Cutelyst context.");
            return;
        }
    }

    *stream << AdminAccount::typeToName(type, c);
}
