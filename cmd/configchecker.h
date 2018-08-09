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
#ifndef CONFIGCHECKER_H
#define CONFIGCHECKER_H

#include <QCoreApplication>
#include "consoleoutput.h"
#include "configfile.h"

/*!
 * \ingroup skaffaricmd
 * \brief Handles checks of skaffari configuration files.
 */
class ConfigChecker : public ConsoleOutput
{
    Q_DECLARE_TR_FUNCTIONS(ConfigChecker)
public:
    /*!
     * \brief Constructs a new %ConfigChecker object with the given parameters.
     * \param confFile  Absolute path to the config file to check.
     * \param quiet     Set to \c false to suppress any output.
     */
    explicit ConfigChecker(const QString &confFile, bool quiet = false);

    /*!
     * \brief Starts the checks and returns \c 0 on success, otherwise an error code.
     */
    int exec() const;

    /*!
     * \brief Returns the absolute path to the configuration file.
     */
    QString absolutePathToConfigFile() const;

private:
    ConfigFile m_confFile; /**< Stores the absolute path to the configuration file */

    /*!
     * \brief Prints an error message and returns an configuration error code.
     * \param message   The message to print.
     * \param key       The affected configuration key.
     */
    int configErrorWithKey(const QString &message, const QString &key) const;
};

#endif // CONFIGCHECKER_H
