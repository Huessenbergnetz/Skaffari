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

#include "skvalidatorfilesize.h"
#include <QRegularExpression>

SkValidatorFilesize::SkValidatorFilesize(const QString &field, const QString &label, const QString &customError) :
    Cutelyst::ValidatorRule(field, label, customError)
{

}

SkValidatorFilesize::~SkValidatorFilesize()
{

}

QString SkValidatorFilesize::validate() const
{
    QString result;

    if (!value().isEmpty() && !value().contains(QRegularExpression(QStringLiteral("^\\d+[,.٫]?\\d*\\s*[KMGT]?i?B?"), QRegularExpression::CaseInsensitiveOption))) {

        result = validationError();

    }

    return result;
}

QString SkValidatorFilesize::genericValidationError() const
{
    QString error;

    if (label().isEmpty()) {
        error = QStringLiteral("Can not be parsed as file size. Has to be something like 2G or 2048 KiB.");
    } else {
        error = QStringLiteral("The “%1” field can not be parsed as file size. Has to be something like 2G or 2048 KiB.").arg(label());
    }

    return error;
}
