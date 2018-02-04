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

#include "skvalidatordomainexists.h"
#include "../common/global.h"
#include <Cutelyst/Plugins/Utils/Sql>
#include <Cutelyst/Context>
#include <Cutelyst/Plugins/Authentication/authentication.h>
#include <Cutelyst/Plugins/Authentication/authenticationuser.h>
#include <QSqlQuery>
#include <QSqlError>

SkValidatorDomainExists::SkValidatorDomainExists(const QString &field, const Cutelyst::ValidatorMessages &messages) :
    ValidatorRule(field, messages, QString())
{
}

SkValidatorDomainExists::~SkValidatorDomainExists()
{
}

Cutelyst::ValidatorReturnType SkValidatorDomainExists::validate(Cutelyst::Context *c, const Cutelyst::ParamsMultiMap &params) const
{
    Cutelyst::ValidatorReturnType result;

    const QString v = value(params);

    if (!v.isEmpty()) {
        bool ok = false;
        const dbid_t id = v.toUInt(&ok);
        if (ok) {
            if (id > 0) {
                QSqlQuery q;
                if (c->stash(QStringLiteral("userType")).value<qint16>() == 0) {
                    q = CPreparedSqlQueryThread(QStringLiteral("SELECT domain_name FROM domain WHERE id = :id"));
                } else {
                    const dbid_t userId = c->stash(QStringLiteral("userId")).value<dbid_t>();
                    q = CPreparedSqlQueryThread(QStringLiteral("SELECT dom.domain_name FROM domain dom LEFT JOIN domainadmin da ON dom.id = da.domain_id WHERE da.admin_id = :admin_id AND dom.id = :id"));
                    q.bindValue(QStringLiteral(":admin_id"), userId);
                }
                q.bindValue(QStringLiteral(":id"), id);

                if (q.exec()) {
                    if (!q.next() || q.value(0).toString().isEmpty()) {
                        result.errorMessage = validationError(c, id);
                    } else {
                        result.value.setValue<dbid_t>(id);
                    }
                } else {
                    result.errorMessage = c->translate("SkValidatorDomainExists", "Failed to check for existing domain: %1").arg(q.lastError().text());
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

QString SkValidatorDomainExists::genericValidationError(Cutelyst::Context *c, const QVariant &errorData) const
{
    QString error;
    dbid_t id = errorData.value<dbid_t>();
    const QString _label = label(c);
    const qint16 userType = c->stash(QStringLiteral("userType")).value<qint16>();
    if (_label.isEmpty()) {
        if (userType == 0) {
            error = c->translate("SkValidatorDomainExists", "The domain with ID %1 does not exist.").arg(id);
        } else {
            error = c->translate("SkValidatorDomainExists", "The domain with ID %1 either does not exist or you do not have access rights for it.").arg(id);
        }
    } else {
        if (userType == 0) {
            error = c->translate("SkValidatorDomainExists", "The domain with ID %1 selected for the “%2” field does not exist.").arg(QString::number(id), _label);
        } else {
            error = c->translate("SkValidatorDomainExists", "The domain with ID %1 selected for the “%2“ field either does not exist or you do not have access rights for it.").arg(QString::number(id), _label);
        }
    }
    return error;
}

QString SkValidatorDomainExists::genericParsingError(Cutelyst::Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("SkValidatorDomainExists", "Failed to convert the input data into a database ID.");
    } else {
        error = c->translate("SkValidatorDomainExists", "Failed to convert the input data of the “%1” field into a database ID.").arg(_label);
    }
    return error;
}
