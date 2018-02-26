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

#include "skaffariimaperror_p.h"

SkaffariIMAPError::SkaffariIMAPError(SkaffariIMAPError::ErrorType type, const QString errorText) :
    d(new SkaffariIMAPErrorData(type, errorText))
{

}

SkaffariIMAPError::SkaffariIMAPError(const QSslError &sslError) :
    d(new SkaffariIMAPErrorData(sslError))
{

}

SkaffariIMAPError::SkaffariIMAPError(const SkaffariIMAPError& other) : d(other.d)
{

}

SkaffariIMAPError& SkaffariIMAPError::operator=(const SkaffariIMAPError& other)
{
    d = other.d;
    return *this;
}

bool SkaffariIMAPError::operator==(const SkaffariIMAPError& other) const
{
    return d->errorType == other.d->errorType;
}

bool SkaffariIMAPError::operator!=(const SkaffariIMAPError& other) const
{
    return d->errorType != other.d->errorType;
}

SkaffariIMAPError::~SkaffariIMAPError()
{

}

SkaffariIMAPError::ErrorType SkaffariIMAPError::type() const
{
    return d->errorType;
}

QString SkaffariIMAPError::errorText() const
{
    return d->errorText;
}

QString SkaffariIMAPError::text() const
{
    return d->errorText;
}

void SkaffariIMAPError::clear()
{
    d->errorType = NoError;
    d->errorText.clear();
}
