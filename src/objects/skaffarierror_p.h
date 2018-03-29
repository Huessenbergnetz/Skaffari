/*
 * Skaffari - a mail account administration web interface based on Cutelyst
 * Copyright (C) 2017-2018 Matthias Fehring <kontakt@buschmann23.de>
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

#ifndef SKAFFARIERROR_P_H
#define SKAFFARIERROR_P_H

#include "skaffarierror.h"
#include <QSharedData>
#include <Cutelyst/Context>

class SkaffariErrorData : public QSharedData
{
public:
    explicit SkaffariErrorData(Cutelyst::Context *_c) :
        QSharedData(),
        c(_c)
    {}

    SkaffariErrorData(Cutelyst::Context *_c, SkaffariError::ErrorType _type , const QString _errorText, const QVariant _errorData) :
        QSharedData(),
        c(_c),
        errorText(_errorText),
        errorData(_errorData),
        errorType(_type)
    {
        switch (errorType) {
        case SkaffariError::AuthenticationError:
            status = Cutelyst::Response::Unauthorized;
            break;
        case SkaffariError::AuthorizationError:
            status = Cutelyst::Response::Forbidden;
            break;
        case SkaffariError::NotFound:
            status = Cutelyst::Response::NotFound;
            break;
        case SkaffariError::InputError:
            status = Cutelyst::Response::BadRequest;
            break;
        default:
            status = Cutelyst::Response::InternalServerError;
            break;
        }
    }

    SkaffariErrorData(Cutelyst::Context *_c, const QSqlError &_sqlError, const QString &_errorText) :
        QSharedData(),
        c(_c),
        qSqlError(_sqlError),
        errorType(SkaffariError::SqlError),
        status(Cutelyst::Response::InternalServerError)
    {
        if (_errorText.isEmpty()) {
            errorText = qSqlError.text();
        } else {
            errorText = _errorText;
            errorText.append(QChar(QChar::Space));
            errorText.append(qSqlError.text());
        }
    }

    SkaffariErrorData(Cutelyst::Context *_c, const SkaffariIMAPError& _imapError, const QString _errorText) :
        QSharedData(),
        c(_c),
        imapError(_imapError),
        errorType(SkaffariError::ImapError),
        status(Cutelyst::Response::InternalServerError)
    {
        if (_errorText.isEmpty()) {
            errorText = imapError.errorText();
        } else {
            errorText = _errorText;
            errorText.append(QChar(QChar::Space));
            errorText.append(imapError.errorText());
        }
    }

    ~SkaffariErrorData() {}

    QString databaseErrorText() const;

    Cutelyst::Context *c = nullptr;
    QSqlError qSqlError;
    QString errorText;
    QVariant errorData;
    SkaffariIMAPError imapError;
    SkaffariError::ErrorType errorType = SkaffariError::NoError;
    Cutelyst::Response::HttpStatus status = Cutelyst::Response::OK;
};

#endif // SKAFFARIERROR_P_H
