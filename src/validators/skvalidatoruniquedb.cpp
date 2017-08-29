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

#include "skvalidatoruniquedb.h"
#include "../objects/account.h"
#include <Cutelyst/Plugins/Utils/Sql>
#include <QSqlError>
#include <QSqlQuery>
#include <QUrl>

SkValidatorUniqueDb::SkValidatorUniqueDb(const QString &field, const QString &table, const QString &column, ColumnType colType, const QString &label, const QString &customError) :
    Cutelyst::ValidatorRule(field, label, customError), m_table(table), m_column(column), m_columnType(colType)
{

}


SkValidatorUniqueDb::~SkValidatorUniqueDb()
{

}


QString SkValidatorUniqueDb::validate() const
{
    QString result;

    if (m_table.isEmpty() || m_column.isEmpty()) {
        result = validationDataError();
        return result;
    }

    const QString v = (m_columnType == DomainName) ? QString::fromLatin1(QUrl::toAce(value())) : (m_columnType == EmailAddress) ? Account::addressToACE(value()) : value();

    if (!v.isEmpty()) {

        QSqlQuery q(QSqlDatabase::database(Cutelyst::Sql::databaseNameThread()));

        q.prepare(QStringLiteral("SELECT %1 FROM %2 WHERE %1 = :val").arg(m_column, m_table));
        q.bindValue(QStringLiteral(":val"), v);

        if (Q_LIKELY(q.exec())) {
            if (q.next()) {
                result = validationError();
            }
        } else {
            result = q.lastError().text();
        }
    }

    return result;
}


QString SkValidatorUniqueDb::genericValidationError() const
{
    QString error;

    if (label().isEmpty()) {
        error = QStringLiteral("Has to be unique but is already in use somewhere else.");
    } else {
        error = QStringLiteral("The %1 field has to be unique but is already in use somewhere else.");
    }

    return error;
}
