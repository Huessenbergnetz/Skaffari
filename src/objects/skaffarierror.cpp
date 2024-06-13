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

#include "skaffarierror.h"
#include <Cutelyst/Context>
#include <QDebugStateSaver>
#include <QSharedData>

#define STASH_KEY "_sk_error"

class SkaffariError::Data : public QSharedData
{
public:
    explicit Data(Cutelyst::Context *_c) :
        QSharedData(),
        c(_c)
    {}

    Data(Cutelyst::Context *_c, SkaffariError::Type _type , const QString &_errorText, const QVariant &_errorData) :
        QSharedData(),
        c(_c),
        errorText(_errorText),
        errorData(_errorData),
        errorType(_type)
    {
        switch (errorType) {
        case SkaffariError::Authentication:
            status = Cutelyst::Response::Unauthorized;
            break;
        case SkaffariError::Authorization:
            status = Cutelyst::Response::Forbidden;
            break;
        case SkaffariError::NotFound:
            status = Cutelyst::Response::NotFound;
            break;
        case SkaffariError::Input:
            status = Cutelyst::Response::BadRequest;
            break;
        default:
            status = Cutelyst::Response::InternalServerError;
            break;
        }
    }

    Data(Cutelyst::Context *_c, const QSqlError &_sqlError, const QString &_errorText) :
        QSharedData(),
        c(_c),
        qSqlError(_sqlError),
        errorType(SkaffariError::Sql),
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

    Data(Cutelyst::Context *_c, const ImapError& _imapError, const QString &_errorText) :
        QSharedData(),
        c(_c),
        imapError(_imapError),
        errorType(SkaffariError::Imap),
        status(Cutelyst::Response::InternalServerError)
    {
        if (_errorText.isEmpty()) {
            errorText = imapError.text();
        } else {
            errorText = _errorText;
            errorText.append(QChar(QChar::Space));
            errorText.append(imapError.text());
        }
    }

    ~Data() {}

    QString databaseErrorText() const;

    Cutelyst::Context *c = nullptr;
    QSqlError qSqlError;
    QString errorText;
    QVariant errorData;
    ImapError imapError;
    SkaffariError::Type errorType = SkaffariError::NoError;
    Cutelyst::Response::HttpStatus status = Cutelyst::Response::OK;
};

QString SkaffariError::Data::databaseErrorText() const
{
    QString es;
    if (c) {
        switch(qSqlError.type()) {
            case QSqlError::NoError: es = c->translate("SkaffariErrorData", "Database, no error occurred."); break;
            case QSqlError::ConnectionError: es = c->translate("SkaffariErrorData", "Database connection error:"); break;
            case QSqlError::StatementError: es = c->translate("SkaffariErrorData", "SQL statement syntax error:"); break;
            case QSqlError::TransactionError: es = c->translate("SkaffariErrorData", "Database transaction failed error:"); break;
            default: es = c->translate("SkaffariErrorData", "Unknown database error:");
        }
    }
    return es;
}

SkaffariError::SkaffariError(Cutelyst::Context *c) :
    d(new Data(c))
{

}

SkaffariError::SkaffariError(Cutelyst::Context *c, Type type, const QString &errorText, const QVariant &errorData) :
    d(new Data(c, type, errorText, errorData))
{

}

SkaffariError::SkaffariError(Cutelyst::Context *c, const QSqlError& sqlError, const QString &errorText) :
    d(new Data(c, sqlError, errorText))
{

}

SkaffariError::SkaffariError(Cutelyst::Context *c, const ImapError& imapError, const QString &errorText) :
    d(new Data(c, imapError, errorText))
{

}

SkaffariError::SkaffariError(const SkaffariError& other) :
    d(other.d)
{

}

SkaffariError::SkaffariError(SkaffariError &&other) noexcept :
    d(std::move(other.d))
{
    other.d = nullptr;
}

SkaffariError::~SkaffariError()
{

}

void SkaffariError::swap(SkaffariError &other) noexcept
{
    std::swap(d, other.d);
}

SkaffariError::Type SkaffariError::type() const
{
    return d->errorType;
}

void SkaffariError::setErrorType(Type nType)
{
    d->errorType = nType;
    switch (nType) {
    case SkaffariError::Authentication:
        d->status = Cutelyst::Response::Unauthorized;
        break;
    case SkaffariError::Authorization:
        d->status = Cutelyst::Response::Forbidden;
        break;
    case SkaffariError::NotFound:
        d->status = Cutelyst::Response::NotFound;
        break;
    case SkaffariError::Input:
        d->status = Cutelyst::Response::BadRequest;
        break;
    default:
        d->status = Cutelyst::Response::InternalServerError;
        break;
    }
}

Cutelyst::Response::HttpStatus SkaffariError::status() const
{
    return d->status;
}

void SkaffariError::setStatus(Cutelyst::Response::HttpStatus status)
{
    d->status = status;
}

QSqlError SkaffariError::qSqlError() const
{
    return d->qSqlError;
}

QString SkaffariError::errorText() const
{
    QString text;

    if (d->c) {
        switch(type()) {
        case NoError:
            text = d->c->translate("SkaffariError", "No error occurred, everything is fine.");
            break;
        case Imap:
            text = d->c->translate("SkaffariError", "IMAP error:");
            break;
        case Sql:
            text = d->databaseErrorText();
            break;
        case Config:
            text = d->c->translate("SkaffariError", "Configuration error:");
            break;
        case Authentication:
            text = d->c->translate("SkaffariError", "Authentication error:");
            break;
        case Authorization:
            text = d->c->translate("SkaffariError", "Authorization error:");
            break;
        case Input:
            text = d->c->translate("SkaffariError", "Invalid input:");
            break;
        case NotFound:
            text = d->c->translate("SkaffariError", "Not found:");
            break;
        default:
            text = d->c->translate("SkaffariError", "Unknown error:");
            break;
        }
    }

    text.append(QChar(QChar::Space));

    text.append(d->errorText);

    return text;
}

void SkaffariError::setErrorText(const QString &nText)
{
    d->errorText = nText;
}

QVariant SkaffariError::errorData() const
{
    return d->errorData;
}

ImapError SkaffariError::imapError() const
{
    return d->imapError;
}

SkaffariError& SkaffariError::operator=(const SkaffariError& other)
{
    d = other.d;
    return *this;
}

SkaffariError& SkaffariError::operator=(SkaffariError &&other) noexcept
{
    swap(other);
    return *this;
}

SkaffariError& SkaffariError::operator=(const QSqlError& sqlError)
{
    d->errorType = Sql;
    d->imapError.clear();
    d->qSqlError = sqlError;
    d->errorText = sqlError.text();
    d->errorData.clear();
    d->status = Cutelyst::Response::InternalServerError;
    return *this;
}

bool SkaffariError::operator==(const SkaffariError& other) const
{
    return ((d->errorType == other.d->errorType) && (d->qSqlError == other.d->qSqlError) && (d->status == other.d->status));
}

bool SkaffariError::operator!=(const SkaffariError& other) const
{
    return ((d->errorType != other.d->errorType) || (d->qSqlError != other.d->qSqlError) || (d->status != other.d->status));
}

void SkaffariError::setSqlError(const QSqlError &error, const QString &text)
{
    d->errorType = Sql;
    d->qSqlError = error;
    if (text.isEmpty()) {
        d->errorText = error.text();
    } else {
        d->errorText = text;
        d->errorText.append(QChar(QChar::Space));
        d->errorText.append(error.text());
    }
    d->errorData.clear();
    d->imapError.clear();
    d->status = Cutelyst::Response::InternalServerError;
}

void SkaffariError::setImapError(const ImapError &error, const QString &text)
{
    d->errorType = Imap;
    d->qSqlError = QSqlError();
    d->imapError = error;
    if (text.isEmpty()) {
        d->errorText = error.text();
    } else {
        d->errorText = text;
        d->errorText.append(QChar(QChar::Space));
        d->errorText.append(error.text());
    }
    d->errorData.clear();
    d->status = Cutelyst::Response::InternalServerError;
}

QString SkaffariError::typeTitle(Cutelyst::Context *c) const
{
    QString ret;

    Cutelyst::Context *ctx = c;
    if (!ctx) {
        ctx = d->c;
    }

    if (ctx) {
        switch (d->errorType) {
        case Imap:
            ret = ctx->translate("SkaffariError", "IMAP error");
            break;
        case DbLayout:
        case Sql:
            ret = ctx->translate("SkaffariError", "Database error");
            break;
        case Config:
            ret = ctx->translate("SkaffariError", "Configuration error");
            break;
        case EmptyDatabase:
            ret = ctx->translate("SkaffariError", "Empty database");
            break;
        case Authorization:
        case Authentication:
            ret = ctx->translate("SkaffariError", "Access denied");
            break;
        case Input:
            ret = ctx->translate("SkaffariError", "Invalid input data");
            break;
        case Application:
            ret = ctx->translate("SkaffariError", "Internal server error");
            break;
        case NotFound:
            ret = ctx->translate("SkaffariError", "Not found");
            break;
        default:
            ret = ctx->translate("SkaffariError", "Unknown Error");
            break;
        }
    }

    return ret;
}

void SkaffariError::toStash(Cutelyst::Context *c, bool detach) const
{
    Q_ASSERT(c);
    c->setStash(QStringLiteral(STASH_KEY), QVariant::fromValue<SkaffariError>(*this));
    if (detach) {
        c->detach(c->getAction(QStringLiteral("error")));
    }
}

SkaffariError SkaffariError::fromStash(Cutelyst::Context *c)
{
    return c->stash(QStringLiteral(STASH_KEY)).value<SkaffariError>();
}

void SkaffariError::toStash(Cutelyst::Context *c, Type type, const QString &errorText, bool detach)
{
    SkaffariError e(c, type, errorText);
    e.toStash(c, detach);
}

QDebug operator<<(QDebug dbg, const SkaffariError &error)
{
    QDebugStateSaver saver(dbg);
    Q_UNUSED(saver)
    dbg.nospace() << "SkaffariError(";
    dbg << "Type: " << error.type();
    dbg << ", Text: " << error.errorText();
    dbg << ')';
    return dbg.maybeSpace();
}

#include "moc_skaffarierror.cpp"
