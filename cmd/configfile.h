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

#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include <QFileInfo>
#include <QCoreApplication>
#include "consoleoutput.h"

/*!
 * \brief Stores information about about a Skaffari configuration file.
 */
class ConfigFile : public ConsoleOutput
{
    Q_DECLARE_TR_FUNCTIONS(ConfigFile)
public:
    /*!
     * \brief Constructs a new ConfigFile object.
     * \param confFile          Absolute path to the configuration file.
     * \param createIfNotExists Set to \c true if the file should be created if it does not exist.
     * \param checkIfWritable   Set to \c true if the file should be checked for writability.
     * \param quiet             Set to \c true if no output should be send to stdout.
     */
    ConfigFile(const QString &confFile, bool createIfNotExists = true, bool checkIfWritable = true, bool quiet = false);

    /*!
     * \brief Starts the checks on the configuration file.
     * \return Returns \c 0 on success.
     */
    int checkConfigFile() const;

    /*!
     * \brief Returns the absolute path to the configuration file.
     */
    QString configFileName() const;

private:
    QFileInfo m_confFile;
    bool m_createIfNotExists = true;
    bool m_checkIfWritable = true;
};

#endif // CONFIGFILE_H
