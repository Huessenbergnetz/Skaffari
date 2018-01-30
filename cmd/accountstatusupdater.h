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

#ifndef ACCOUNTSTATUSUPDATER_H
#define ACCOUNTSTATUSUPDATER_H

#include <QFileInfo>
#include <QCoreApplication>
#include "configfile.h"

/*!
 * \ingroup skaffaricmd
 * \brief Updates the status column of the accountuser table.
 *
 * Checks every row in the accountuser table for expired account (valid_until) and expired password
 * (pwd_expire). It updates the status column accordingly. The status column contains a integer value
 * that is used for storing different flags. Bit 0 will be flagged if the account has expired, bit 1
 * will be flagged if the password has been expired.
 */
class AccountStatusUpdater : public ConfigFile
{
    Q_DECLARE_TR_FUNCTIONS(AccountStatusUpdater)
public:
    /*!
     * \brief Constructs a new AccountStatusUpdater object.
     * \param confFile  Absolute path to the configuration file that contains database access data.
     * \param quiet     If \c true, no output will be print to stdout.
     */
    explicit AccountStatusUpdater(const QString &confFile, bool quiet = false);

    /*!
     * \brief Starts the execution of the status checks.
     * \return Returns \c 0 on success.
     */
    int exec() const;
};

#endif // ACCOUNTSTATUSUPDATER_H
