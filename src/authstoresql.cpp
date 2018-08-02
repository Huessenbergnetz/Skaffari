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

#include "authstoresql.h"
#include "objects/adminaccount.h"

#include <Cutelyst/Plugins/Utils/Sql>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariantList>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(SK_AUTHSTORE, "skaffari.authstore")

AuthStoreSql::AuthStoreSql(QObject *parent) : AuthenticationStore(parent)
{

}

AuthenticationUser AuthStoreSql::findUser(Context *c, const ParamsMultiMap &userinfo)
{
    AuthenticationUser user;

    Q_UNUSED(c);

    const QString username = userinfo.value(QStringLiteral("username"));

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT au.id, au.username, au.password, au.type, au.created_at, au.updated_at, au.valid_until, au.pwd_expire, se.template, se.maxdisplay, se.warnlevel, se.lang, se.tz FROM adminuser au JOIN settings se ON au.id = se.admin_id WHERE au.username = :username AND au.type > 0"));
    q.bindValue(QStringLiteral(":username"), username);

    if (Q_LIKELY(q.exec())) {
        if (Q_LIKELY(q.next())) {
            user.setId(q.value(0));
            user.insert(QStringLiteral("username"),     q.value(1));
            user.insert(QStringLiteral("password"),     q.value(2));
            user.insert(QStringLiteral("type"),         q.value(3));
            user.insert(QStringLiteral("created_at"),   q.value(4));
            user.insert(QStringLiteral("updated_at"),   q.value(5));
            user.insert(QStringLiteral("valid_until"),  q.value(6));
            user.insert(QStringLiteral("pwd_expire"),   q.value(7));
            user.insert(QStringLiteral("style"),        q.value(8));
            user.insert(QStringLiteral("maxdisplay"),   q.value(9));
            user.insert(QStringLiteral("warnlevel"),    q.value(10));
            user.insert(QStringLiteral("lang"),         q.value(11));
            user.insert(QStringLiteral("tz"),           q.value(12));
        } else {
            qCWarning(SK_AUTHSTORE, "Can not find user \"%s\" in the database.", qUtf8Printable(username));
        }
    } else {
        qCCritical(SK_AUTHSTORE, "Failed to execute database query to get user \"%s\" from the database: %s", qUtf8Printable(username), qUtf8Printable(q.lastError().text()));
    }

    if (!user.isNull() && (user.value(QStringLiteral("type")).value<quint8>() < static_cast<quint8>(AdminAccount::Administrator))) {
        q = CPreparedSqlQueryThread(QStringLiteral("SELECT domain_id FROM domainadmin WHERE admin_id = :admin_id"));
        q.bindValue(QStringLiteral(":admin_id"), user.id());

        if (Q_LIKELY(q.exec())) {
            QVariantList domIds;
            while (q.next()) {
                domIds << q.value(0);
            }
            user.insert(QStringLiteral("domains"), domIds);
        } else {
            qCCritical(SK_AUTHSTORE, "Failed to execute database query to get associated domain IDs for user \"%s\" from the database: %s", qUtf8Printable(username), qUtf8Printable(q.lastError().text()));
        }
    }

    return user;
}

#include "moc_authstoresql.cpp"
