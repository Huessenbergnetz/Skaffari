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

#ifndef SKAFFARIERROR_H
#define SKAFFARIERROR_H

#include <QSharedDataPointer>
#include <QString>
#include <QVariant>
#include <QtSql/QSqlError>

#include "../imap/skaffariimaperror.h"

namespace Cutelyst {
class Context;
}

class SkaffariErrorData;

/*!
 * \brief The SkaffariError class provides Skaffari error information.
 *
 * A SkaffariError objet can provide information about different errors that can happen
 * when using Skaffari.
 */
class SkaffariError
{
public:
	/*!
	 * \brief Different types of errors that can happen in Skaffari.
	 */
	enum ErrorType {
		NoError,				/**< No Error occured */
		ImapError,				/**< An IMAP error occured */
		SqlError,				/**< An SQL error occured */
		ConfigError,			/**< An configuration error occured, or configuration could not be written */
		EmptyDatabase,			/**< The database is completely empty */
		DbLayoutError,			/**< The database layout is not complete */
		AuthenticationError,	/**< Authentication failed */
		InputError,				/**< Input data is corrupted */
        ApplicationError,       /**< An internal error occured */
        AutorizationError,      /**< Authorization for an operation failed */
        NotFound,               /**< The requested resource could not be found */
		UnknownError			/**< Unknown error */
	};

	/*!
	 * \brief Constructs a SkaffariError of type \link SkaffariError::NoError NoError \endlink.
	 */
    explicit SkaffariError(Cutelyst::Context *c);

	/*!
	 * \brief Constructs a SkaffariError with the given parameters.
	 *
	 * \param type \link SkaffariError::ErrorType ErrorType \endlink describing this type of error.
	 * \param errorText Optional QString human readable error description.
	 * \param errorData Optional container for additional error data for further processing.
	 */
    SkaffariError(Cutelyst::Context *c, ErrorType type , const QString errorText = QString(), const QVariant errorData = QVariant());

	/*!
	 * \brief Constructs a SkaffariError object using the sqlError.
	 *
	 * This constructor takes a QSqlError to construct a SkaffariError of type \link SkaffariError::SqlError SqlError \endlink.
	 * The used QSqlError can be returned by qSqlError(). errorText() takes also the text strings of the QSqlError into account.
	 *
	 * You can optionally specify a custom error text that is prepended to the output of errorText().
	 *
	 * \param sqlError A QSqlError
	 * \param errorText Optional custom error text for better explanation.
	 */
    SkaffariError(Cutelyst::Context *c, const QSqlError &sqlError, const QString errorText = QString());

	/*!
     * \brief Constructs a SkaffariError object using the \a imapError.
	 *
	 * This constructor takes a SkaffariImapError to construct a SkaffariError of type \link SkaffariError::ImapError ImapError \endlink.
	 * The used SkaffariIMAPError can be returned by imapError(). errorText() takes also the text strings of the SkaffariIMAPError into account.
	 *
	 * You can optionally specify a custom error text that is prepended to the output of errorText().
	 *
	 * \param imapError A SkaffariIMAPError
	 * \param errorText Optional custom error text for better explanation.
	 */
    SkaffariError(Cutelyst::Context *c, const SkaffariIMAPError &imapError, const QString errorText = QString());

	/*!
	 * \brief Constructs a copy of the error \e other, passed as the argument to this constructor.
	 */
	SkaffariError(const SkaffariError &other);

	/*!
	 * \brief Destroys the SkaffariError.
	 */
    ~SkaffariError();



	/*!
	 * \brief Returns the type of this error.
	 */
	ErrorType type() const;


	/*!
	 * \brief Returns a human readable error text.
	 *
	 * If SkaffariError was constructed using a QSqlError or a SkaffariIMAPError,
	 * this also returns the error texts of them.
	 *
	 * \return QString of human readable error text.
	 */
	QString errorText() const;

	/*!
	 * \brief Returns error data.
	 *
	 * Functions creating SkaffariError have the possibility to add a QVariant that contains
	 * additional data about this error.
	 *
	 * \return QVariant containing additional error data
	 */
	QVariant errorData() const;

	/*!
	 * \brief Returns the QSqlError.
	 *
	 * If a QSqlError was used to construct this SkaffariError, this returns
	 * the used QSqlError.
	 *
	 * \return The QSqlError used for constructing this SkaffariError.
	 */
	QSqlError qSqlError() const;

	/*!
	 * \brief Returns the SkaffariIMAPError.
	 *
	 * If a SkaffariIMAPError was used to construct this SkaffariError, this returns
	 * the used SkaffariIMAPError.
	 *
	 * \return The SkaffariIMAPError used for constructing this SkaffariError.
	 */
	SkaffariIMAPError imapError() const;


    void setErrorType(ErrorType nType);
    void setErrorText(const QString &nText);
    void setSqlError(const QSqlError &error, const QString &text = QString());
    void setImapError(const SkaffariIMAPError &error, const QString &text = QString());



	/*!
	 * \brief Assigns the value of the error \e other to this error.
	 */
    SkaffariError& operator=(const SkaffariError &other);

    /*!
	 * \brief Constructs a SkaffariError object using the sqlError.
	 */
	SkaffariError& operator=(const QSqlError &sqlError);

	/*!
	 * \brief Compares this SkaffariError with \e other and returns \c true if they are equal; otherwise returns \c false.
	 */
	bool operator==(const SkaffariError &other);

	/*!
	 * \brief Compares this SkaffariError with \e other and returns \c true if they are not equal; otherwise returns \c false.
	 */
	bool operator!=(const SkaffariError &other);

private:
    QSharedDataPointer<SkaffariErrorData> d;
};

#endif // SKAFFARIERROR_H
