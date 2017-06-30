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

#include "authstoresql.h"

#include <Cutelyst/Plugins/Utils/Sql>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariantList>
#include "../common/global.h"

AuthStoreSql::AuthStoreSql(QObject *parent) : AuthenticationStore(parent)
{

}

AuthenticationUser AuthStoreSql::findUser(Context *c, const ParamsMultiMap &userinfo)
{
    AuthenticationUser user;

    Q_UNUSED(c);

    const QString username = userinfo[QStringLiteral("username")];

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT au.id, au.username, au.password, au.type, se.template, se.maxdisplay, se.warnlevel, se.lang, se.tz FROM adminuser au JOIN settings se ON au.id = se.admin_id WHERE au.username = :username"));
    q.bindValue(QStringLiteral(":username"), username);

    if (q.exec() && q.next()) {
        user.setId(q.value(0).toString());
        user.insert(QStringLiteral("username"), q.value(1).toString());
        user.insert(QStringLiteral("password"), q.value(2).toString());
        user.insert(QStringLiteral("type"), q.value(3).value<qint16>());
        user.insert(QStringLiteral("style"), q.value(4).toString());
        user.insert(QStringLiteral("maxdisplay"), q.value(5).value<quint8>());
        user.insert(QStringLiteral("warnlevel"), q.value(6).value<quint8>());
        user.insert(QStringLiteral("lang"), q.value(7).toString());
        user.insert(QStringLiteral("tz"), q.value(8).toByteArray());
    }

    q = CPreparedSqlQueryThread(QStringLiteral("SELECT domain_id FROM domainadmin WHERE admin_id = :admin_id"));
    q.bindValue(QStringLiteral(":admin_id"), QVariant::fromValue<dbid_t>(user.id().toULong()));

    q.exec();

    QVariantList domIds;
    while (q.next()) {
        domIds << q.value(0);
    }

    user.insert(QStringLiteral("domains"), domIds);

    return user;
}
