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

#include "skaffariimaperror.h"
#include <QSharedData>
#include <QSslError>

class SkaffariIMAPError::Data : public QSharedData // clazy:exclude=rule-of-three
{
public:
    Data(SkaffariIMAPError::ErrorType _errorType, const QString &_errorText) :
        errorText(_errorText),
        errorType(_errorType)
    {}

    explicit Data(const QSslError &sslError) :
        errorText(sslError.errorString()),
        errorType(SkaffariIMAPError::EncryptionError)
    {}

    explicit Data(const Data &other) :
        QSharedData(other),
        errorText(other.errorText),
        errorType(other.errorType)
    {}

    ~Data() {}

    QString errorText;
    SkaffariIMAPError::ErrorType errorType = SkaffariIMAPError::NoError;
};

SkaffariIMAPError::SkaffariIMAPError(SkaffariIMAPError::ErrorType type, const QString &errorText) :
    d(new Data(type, errorText))
{

}

SkaffariIMAPError::SkaffariIMAPError(const QSslError &sslError) :
    d(new Data(sslError))
{

}

SkaffariIMAPError::SkaffariIMAPError(const SkaffariIMAPError& other) : d(other.d)
{

}

SkaffariIMAPError::SkaffariIMAPError(SkaffariIMAPError &&other) noexcept :
    d(std::move(other.d))
{
    other.d = nullptr;
}

SkaffariIMAPError& SkaffariIMAPError::operator=(const SkaffariIMAPError& other)
{
    d = other.d;
    return *this;
}

SkaffariIMAPError& SkaffariIMAPError::operator=(SkaffariIMAPError &&other) noexcept
{
    swap(other);
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

void SkaffariIMAPError::swap(SkaffariIMAPError &other) noexcept
{
    std::swap(d, other.d);
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
