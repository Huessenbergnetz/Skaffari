/*
 * Skaffari - a mail account administration web interface based on Cutelyst
 * Copyright (C) 2019 Matthias Fehring <mf@huessenbergnetz.de>
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

#ifndef AUTOCONFIG_H
#define AUTOCONFIG_H

#include <Cutelyst/Controller>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(SK_AUTOCONFIG)

using namespace Cutelyst;

/*!
 * \ingroup skaffaricontrollers
 * \brief Controller for the Microsoft autoconfiguration feature.
 */
class Autoconfig : public Controller
{
    Q_OBJECT
    C_NAMESPACE("mail/config-v1.1.xml")
public:
    explicit Autoconfig(QObject *parent = nullptr);

    C_ATTR(index, :Path)
    void index(Context *c);
};

#endif // AUTOCONFIG_H
