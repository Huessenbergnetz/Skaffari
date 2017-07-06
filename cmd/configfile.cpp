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

#include "configfile.h"
#include <QDir>

ConfigFile::ConfigFile(const QString &confFile, bool createIfNotExists, bool checkIfWritable, bool quiet) :
    ConsoleOutput(quiet), m_confFile(confFile), m_createIfNotExists(createIfNotExists), m_checkIfWritable(checkIfWritable)
{

}


int ConfigFile::checkConfigFile() const
{
    printStatus(tr("Checking configuration file"));

    if (m_confFile.exists()) {
        if (!m_confFile.isReadable()) {
            printFailed();
            return fileError(tr("Configuration file exists at %1 but is not readable.").arg(m_confFile.absoluteFilePath()));
        }

        if (m_checkIfWritable && !m_confFile.isWritable()) {
            printFailed();
            return fileError(tr("Configuration file exists at %1 but is not writable.").arg(m_confFile.absoluteFilePath()));
        }
        printDone(tr("Found"));
        printMessage(tr("Using existing configuration file at %1.").arg(m_confFile.absoluteFilePath()));

    } else if (m_createIfNotExists) {

        QDir confDir = m_confFile.absoluteDir();
        QFileInfo confDirInfo(confDir.absolutePath());
        if (!confDir.exists() && !confDir.mkpath(confDir.absolutePath())) {
            printFailed();
            return fileError(tr("Failed to create configuation directory at %1.").arg(confDir.absolutePath()));
        } else if (confDir.exists() && !confDirInfo.isWritable()) {
            printFailed();
            return fileError(tr("Can not write to configuration directory at %1.").arg(confDir.absolutePath()));
        }

        printDone(tr("Created"));
        printMessage(tr("Creating configuration file at %1.").arg(m_confFile.absoluteFilePath()));
    } else {
        printFailed();
        return fileError(tr("Can not find configuration file at %1.").arg(m_confFile.absoluteFilePath()));
    }

    return 0;
}


QString ConfigFile::configFileName() const
{
    return m_confFile.absoluteFilePath();
}
