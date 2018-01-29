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

#ifndef SKVALIDATORDOMAINEXISTS_H
#define SKVALIDATORDOMAINEXISTS_H

#include <Cutelyst/Plugins/Utils/ValidatorRule>

class SkValidatorDomainExists : public Cutelyst::ValidatorRule
{
public:
    /*!
     * \brief Constructs a new %SkValidatorDomainExists with the given parameters.
     * \param field     Name of the input field.
     * \param messages  Optional custom error messages if validation fails.
     */
    SkValidatorDomainExists(const QString &field, const Cutelyst::ValidatorMessages &messages = Cutelyst::ValidatorMessages());

    /*!
     * \brief Destroys the %SkValidatorDomainExists.
     */
    ~SkValidatorDomainExists();

protected:
    /*!
     * \brief Executes the validation and returns the result.
     *
     * If validation succeeded Cutelyst::ValidatorReturnType::value will contain the databas ID of the checked domain.
     */
    Cutelyst::ValidatorReturnType validate(Cutelyst::Context *c, const Cutelyst::ParamsMultiMap &params) const override;

    /*!
     * \brief Returns the generic validation error message if no custom message has been set.
     */
    QString genericValidationError(Cutelyst::Context *c, const QVariant &errorData = QVariant()) const override;

    /*!
     * \brief Return the generic parsing error message if input value could not be converted into unsigned int.
     */
    QString genericParsingError(Cutelyst::Context *c, const QVariant &errorData) const override;
};

#endif // SKVALIDATORDOMAINEXISTS_H
