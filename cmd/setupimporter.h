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

#ifndef SETUPIMPORTER_H
#define SETUPIMPORTER_H

#include <QCoreApplication>
#include "configinput.h"
#include "configfile.h"
#include "configchecker.h"

class SetupImporter : public ConfigInput
{
    Q_DECLARE_TR_FUNCTIONS(SetupImporter)
public:
    /*!
     * \brief Constructs a new SetupImport object using the given parameters.
     * \param confFile      absolute path to the configuration file to create
     * \param importFile    absolute path to the file to import the configuration from
     * \param quiet         set to \c true tu suppress output
     */
    explicit SetupImporter(const QString &confFile, const QString &importFile, bool quiet = false);

    /*!
     * \brief Executes the import setup routines and returns \c 0 on success.
     */
    int exec() const;

private:
    ConfigFile m_confFile;
    ConfigChecker m_importFile;

    void insertParamsDefault(QVariantHash &params, const QString &key, const QVariant &defVal) const;

};

#endif // SETUPIMPORTER_H
