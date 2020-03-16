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

#ifndef SKVALIDATORUNIQUEDB_H
#define SKVALIDATORUNIQUEDB_H

#include <Cutelyst/Plugins/Utils/ValidatorRule>

/*!
 * \ingroup skaffarivalidator
 * \brief Cutelyst input validator to check for unique table entries.
 */
class SkValidatorUniqueDb : public Cutelyst::ValidatorRule
{
public:
    enum ColumnType {
        General         = 0,
        DomainName      = 1,
        EmailAddress    = 2,
        UserName        = 3
    };

    SkValidatorUniqueDb(const QString &field, const QString &table, const QString &column, ColumnType colType = General, const Cutelyst::ValidatorMessages &messags = Cutelyst::ValidatorMessages());

    ~SkValidatorUniqueDb();

protected:
    Cutelyst::ValidatorReturnType validate(Cutelyst::Context *c, const Cutelyst::ParamsMultiMap &params) const override;
    QString genericValidationError(Cutelyst::Context *c, const QVariant &errorData = QVariant()) const override;

private:
    QString m_table;
    QString m_column;
    ColumnType m_columnType;
};

#endif // SKVALIDATORUNIQUEDB_H
