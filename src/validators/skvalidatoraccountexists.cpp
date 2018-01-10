/*
 * Skaffari - a mail account administration web interface based on Cutelyst
 * Copyright (C) 2018 Matthias Fehring <kontakt@buschmann23.de>
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

#include "skvalidatoraccountexists.h"
#include "../common/global.h"
#include <Cutelyst/Plugins/Utils/Sql>
#include <QSqlQuery>
#include <QSqlError>

SkValidatorAccountExists::SkValidatorAccountExists(const QString &field, const QString &label, const QString &customError) :
    Cutelyst::ValidatorRule(field, label, customError)
{

}

SkValidatorAccountExists::~SkValidatorAccountExists()
{

}

QString SkValidatorAccountExists::validate() const
{
    QString result;

    if (!value().isEmpty()) {
        const dbid_t id = SKAFFARI_STRING_TO_DBID(value());
        if (id > 0) {
            QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT username FROM accountuser WHERE domain_id != 0 AND id = :id"));
            q.bindValue(QStringLiteral(":id"), id);

            if (q.exec()) {
                if (!q.next() || q.value(0).toString().isEmpty()) {
                    result = validationError();
                }
            } else {
                result = QStringLiteral("Failed to check for existing account: %1").arg(q.lastError().text());
            }

        } else {
            result = validationError();
        }
    }

    return result;
}

QString SkValidatorAccountExists::genericValidationError() const
{
    QString error;

    if (label().isEmpty()) {
        error = QStringLiteral("The account with ID %1 does not exist.").arg(value());
    } else {
        error = QStringLiteral("The account with ID %1 selected for the field “%2” does not exist.").arg(value(), label());
    }

    return error;
}
