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

#include "skvalidatoraccountexists.h"
#include "../common/global.h"
#include <Cutelyst/Plugins/Utils/Sql>
#include <Cutelyst/Context>
#include <QSqlQuery>
#include <QSqlError>

SkValidatorAccountExists::SkValidatorAccountExists(const QString &field, const Cutelyst::ValidatorMessages &messages) :
    Cutelyst::ValidatorRule(field, messages, QString())
{

}

SkValidatorAccountExists::~SkValidatorAccountExists()
{

}

Cutelyst::ValidatorReturnType SkValidatorAccountExists::validate(Cutelyst::Context *c, const Cutelyst::ParamsMultiMap &params) const
{
    Cutelyst::ValidatorReturnType result;

    const QString v = value(params);

    if (!v.isEmpty()) {
        bool ok = false;
        const dbid_t id = v.toUInt(&ok);;
        if (ok) {
            if (id > 0) {
                QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT username FROM accountuser WHERE domain_id != 0 AND id = :id"));
                q.bindValue(QStringLiteral(":id"), id);

                if (q.exec()) {
                    if (!q.next() || q.value(0).toString().isEmpty()) {
                        result.errorMessage = validationError(c, id);
                    } else {
                        result.value.setValue<dbid_t>(id);
                    }
                } else {
                    result.errorMessage = c->translate("SkValidatorAccountExists", "Failed to check for existing account: %1").arg(q.lastError().text());
                }

            } else {
                result.value.setValue<dbid_t>(id);
            }
        } else {
            result.errorMessage = parsingError(c);
        }
    }

    return result;
}

QString SkValidatorAccountExists::genericValidationError(Cutelyst::Context *c, const QVariant &errorData) const
{
    QString error;
    dbid_t id = errorData.value<dbid_t>();
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("SkValidatorAccountExists", "The account with ID %1 does not exist.").arg(id);
    } else {
        error = c->translate("SkValidatorAccountExists", "The account with ID %1 selected for the field “%2” does not exist.").arg(QString::number(id), _label);
    }

    return error;
}

QString SkValidatorAccountExists::genericParsingError(Cutelyst::Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("SkValidatorAccountExists", "Failed to convert the input data into a database ID.");
    } else {
        error = c->translate("SkValidatorAccountExists", "Failed to convert the input data of the “%1” field into a database ID.");
    }
    return error;
}
