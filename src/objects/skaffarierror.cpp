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

#include "skaffarierror_p.h"

QString SkaffariErrorData::databaseErrorText() const
{
    QString es;
    switch(qSqlError.type()) {
        case QSqlError::NoError: es = c->translate("SkaffariErrorData", "Database, no error occured.");
        case QSqlError::ConnectionError: es = c->translate("SkaffariErrorData", "Database connection error:");
        case QSqlError::StatementError: es = c->translate("SkaffariErrorData", "SQL statement syntax error:");
        case QSqlError::TransactionError: es = c->translate("SkaffariErrorData", "Database transaction failed error:");
        case QSqlError::UnknownError: es = c->translate("SkaffariErrorData", "Unkown database error:");
        default: es = c->translate("SkaffariErrorData", "Unknown database error:");
    }
    return es;
}

SkaffariError::SkaffariError(Cutelyst::Context *c) :
    d(new SkaffariErrorData(c))
{
    Q_ASSERT_X(d->c, "creating new SkaffariError", "invalid context");
}


SkaffariError::SkaffariError(Cutelyst::Context *c, SkaffariError::ErrorType type, const QString errorText, const QVariant errorData) :
    d(new SkaffariErrorData(c, type, errorText, errorData))
{
    Q_ASSERT_X(d->c, "creating new SkaffariError", "invalid context");
}


SkaffariError::SkaffariError(Cutelyst::Context *c, const QSqlError& sqlError, const QString errorText) :
    d(new SkaffariErrorData(c, sqlError, errorText))
{
    Q_ASSERT_X(d->c, "creating new SkaffariError", "invalid context");
}



SkaffariError::SkaffariError(Cutelyst::Context *c, const SkaffariIMAPError& imapError, const QString errorText) :
    d(new SkaffariErrorData(c, imapError, errorText))
{
    Q_ASSERT_X(d->c, "creating new SkaffariError", "invalid context");
}


SkaffariError::SkaffariError(const SkaffariError& other) :
    d(other.d)
{

}



SkaffariError::~SkaffariError()
{

}




SkaffariError::ErrorType SkaffariError::type() const
{
	return d->errorType;
}



QSqlError SkaffariError::qSqlError() const
{
	return d->qSqlError;
}



QString SkaffariError::errorText() const
{
	QString text;

	switch(type()) {
    case NoError:
        text = d->c->translate("SkaffariError", "No error occured, everything is fine.");
		break;
    case ImapError:
        text = d->c->translate("SkaffariError", "IMAP error:");
		break;
    case SqlError:
        text = d->databaseErrorText();
		break;
    case ConfigError:
        text = d->c->translate("SkaffariError", "Configuration error:");
		break;
    case AuthenticationError:
        text = d->c->translate("SkaffariError", "Authentication error:");
		break;
    case InputError:
        text = d->c->translate("SkaffariError", "Invalid input:");
		break;
    case NotFound:
        text = d->c->translate("SkaffariError", "Not found:");
        break;
    default:
        text = d->c->translate("SkaffariError", "Unknown error:");
        break;
	}

    text.append(QChar(QChar::Space));

	text.append(d->errorText);

	return text;
}



QVariant SkaffariError::errorData() const
{
	return d->errorData;
}



SkaffariIMAPError SkaffariError::imapError() const
{
	return d->imapError;
}



SkaffariError& SkaffariError::operator=(const SkaffariError& other)
{
    d = other.d;
	return *this;
}



SkaffariError& SkaffariError::operator=(const QSqlError& sqlError)
{
    d->errorType = SqlError;
    d->imapError.clear();
    d->qSqlError = sqlError;
    d->errorText = sqlError.text();
    d->errorData.clear();
    return *this;
}




bool SkaffariError::operator==(const SkaffariError& other) const
{
	return ((d->errorType == other.d->errorType) && (d->qSqlError == other.d->qSqlError));
}



bool SkaffariError::operator!=(const SkaffariError& other) const
{
	return ((d->errorType != other.d->errorType) || (d->qSqlError != other.d->qSqlError));
}



void SkaffariError::setErrorType(ErrorType nType)
{
    d->errorType = nType;
}


void SkaffariError::setErrorText(const QString &nText)
{
    d->errorText = nText;
}

void SkaffariError::setSqlError(const QSqlError &error, const QString &text)
{
    d->errorType = SqlError;
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
}


void SkaffariError::setImapError(const SkaffariIMAPError &error, const QString &text)
{
    d->errorType = ImapError;
    d->qSqlError = QSqlError();
    d->imapError = error;
    if (text.isEmpty()) {
        d->errorText = error.errorText();
    } else {
        d->errorText = text;
        d->errorText.append(QChar(QChar::Space));
        d->errorText.append(error.errorText());
    }
    d->errorData.clear();
}
