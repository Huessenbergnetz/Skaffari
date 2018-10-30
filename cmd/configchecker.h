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

    /*!
     * \brief Returns \c true if the \a value is an unsigned number within the range of 1-65535
     * \param val       The value to check.
     */
    bool checkPort(const QVariant &value) const;

    /*!
     * \brief Returns \c true if the \a value is an unsigned char within the range of \a min and \a max.
     * \param[in] value         The value to check.
     * \param[in] min           Minimum input range.
     * \param[in] max           Maximum input range.
     * \param[out] quint8Val    Will take the checked value if it is valid.
     */
    bool checkQuint8(const QVariant &value, quint8 min = 0, quint8 max = 255, quint8 *quint8Val = nullptr) const;

    /*!
     * \brief Returns \c true if the \a value is an unsigned long int within the range of \a min and \a max.
     * \param[in] value         The value to check.
     * \param[in] min           Minimum input range.
     * \param[in] max           Maximum input range.
     * \param[out] quint8Val    Will take the checked value if it is valid.
     */
    bool checkQuint32(const QVariant &value, quint32 min = 0, quint32 max = 4294967295, quint32 *quint32Val = nullptr) const;

    /*!
     * \brief Returns \c true if the \a value is a valid host address.
     * \param value The value to check.
     */
    bool checkHostAddress(const QVariant &value) const;
};

#endif // CONFIGCHECKER_H
