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

#ifndef SETUP_H
#define SETUP_H

#include <QCoreApplication>
#include "configinput.h"
#include "configfile.h"

/*!
 * \ingroup skaffaricmd
 * \brief Handles the setup for a Skaffari installation.
 */
class Setup : public ConfigInput
{
    Q_DECLARE_TR_FUNCTIONS(Setup)
public:
    /*!
     * \brief Constructs a new Setup object using the given \a confFile.
     * \param confFile  absolute path to a configuration file
     */
    explicit Setup(const QString &confFile, bool quiet = false);

    /*!
     * \brief Executes the setup routines and returns \c 0 on success.
     */
    int exec() const;

private:
    ConfigFile m_confFile;

    static void insertParamsDefault(QVariantHash &params, const QString &key, const QVariant &defVal);
};

#endif // SETUP_H
