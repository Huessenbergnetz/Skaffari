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
#include <Cutelyst/Context>
#include <QSqlError>
#include <QSqlQuery>
#include <QUrl>

SkValidatorUniqueDb::SkValidatorUniqueDb(const QString &field, const QString &table, const QString &column, ColumnType colType, const Cutelyst::ValidatorMessages &messags) :
    Cutelyst::ValidatorRule(field, messags, QString()), m_table(table), m_column(column), m_columnType(colType)
{

}


SkValidatorUniqueDb::~SkValidatorUniqueDb()
{

}


Cutelyst::ValidatorReturnType SkValidatorUniqueDb::validate(Cutelyst::Context *c, const Cutelyst::ParamsMultiMap &params) const
{
    Cutelyst::ValidatorReturnType result;

    if (m_table.isEmpty() || m_column.isEmpty()) {
        result.errorMessage = validationDataError(c);
        return result;
    }

    const QString v = (m_columnType == DomainName) ? QString::fromLatin1(QUrl::toAce(value(params))) : (m_columnType == EmailAddress) ? Account::addressToACE(value(params)) : value(params);

    if (!v.isEmpty()) {

        QSqlQuery q(QSqlDatabase::database(Cutelyst::Sql::databaseNameThread()));

        q.prepare(QStringLiteral("SELECT %1 FROM %2 WHERE %1 = :val").arg(m_column, m_table));
        q.bindValue(QStringLiteral(":val"), v);

        if (Q_LIKELY(q.exec())) {
            if (q.next()) {
                result.errorMessage = validationError(c);
            } else {
                result.value.setValue<QString>(v);
            }
        } else {
            result.errorMessage = q.lastError().text();
        }
    }

    return result;
}


QString SkValidatorUniqueDb::genericValidationError(Cutelyst::Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("SkValidatorUniqueDb", "Has to be unique but is already in use somewhere else.");
    } else {
        error = c->translate("SkValidatorUniqueDb", "The “%1” field has to be unique but the content is already in use somewhere else.");
    }

    return error;
}
