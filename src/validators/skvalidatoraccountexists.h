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

#ifndef SKVALIDATORACCOUNTEXISTS_H
#define SKVALIDATORACCOUNTEXISTS_H

#include <Cutelyst/Plugins/Utils/ValidatorRule>

/*!
 * \ingroup skaffarivalidator
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
     * \param customError   Optional custom error message if validation fails.
     */
    explicit SkValidatorAccountExists(const QString &field, const Cutelyst::ValidatorMessages &messages = Cutelyst::ValidatorMessages());

    /*!
     * \brief Destroys the %SkValidatorAccountExists.
     */
    ~SkValidatorAccountExists();

protected:
    /*!
     * \brief Executes the validation and returns the result.
     *
     * If validation fails, the returned string will not be empty but contains
     * the validation error messsage.
     */
    Cutelyst::ValidatorReturnType validate(Cutelyst::Context *c, const Cutelyst::ParamsMultiMap &params) const override;

    /*!
     * \brief Returns the generic validation error message if no custom message has been set.
     */
    QString genericValidationError(Cutelyst::Context *c, const QVariant &errorData = QVariant()) const override;

    /*!
     * \brief Return the generic parsing error message if input value could not be converted into unsigned int.
     */
    QString genericParsingError(Cutelyst::Context *c, const QVariant &errorData = QVariant()) const override;

};

#endif // SKVALIDATORACCOUNTEXISTS_H
