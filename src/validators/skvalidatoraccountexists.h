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

#ifndef SKVALIDATORACCOUNTEXISTS_H
#define SKVALIDATORACCOUNTEXISTS_H

#include <Cutelyst/Plugins/Utils/ValidatorRule>

/*!
 * \ingroup skaffaricore
 * \brief Cutelyst input validator to check if an account exists by the database ID.
 *
 * This validator fails if the database ID got from the input field does not exist
 * in the \a accountuser table.
 */
class SkValidatorAccountExists : public Cutelyst::ValidatorRule
{
public:
    /*!
     * \brief Constructs a new %SkValidatorAccountExits with the given parameters.
     * \param field         Name of the input field.
     * \param label         Optional field label for the generic error messages.
     * \param customError   Optional custom error message if validation fails.
     */
    SkValidatorAccountExists(const QString &field, const QString &label = QString(), const QString &customError = QString());

    /*!
     * \brief Destroys the %SkValidatorAccountExists.
     */
    ~SkValidatorAccountExists();

    /*!
     * \brief Executes the validation and returns an empty string on success.
     *
     * If validation fails, the returned string will not be empty but contains
     * the validation error messsage.
     */
    QString validate() const override;

protected:
    /*!
     * \brief Returns the generic validation error message if no custom message has been set.
     */
    QString genericValidationError() const override;
};

#endif // SKVALIDATORACCOUNTEXISTS_H