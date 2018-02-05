/*
 * Skaffari - a mail account administration web interface based on Cutelyst
 * Copyright (C) 2017-2018 Matthias Fehring <kontakt@buschmann23.de>
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

#ifndef MYACCOUNT_H
#define MYACCOUNT_H

#include <Cutelyst/Controller>

using namespace Cutelyst;
class SkaffariEngine;

/*!
 * \brief Routes for the myaccount namespace.
 */
class MyAccount : public Controller
{
    Q_OBJECT
    C_NAMESPACE("myaccount")
public:
    explicit MyAccount(QObject *parent = nullptr);
    ~MyAccount();

    C_ATTR(index, :Path :Args(0))
    void index(Context *c);
};

#endif // MYACCOUNT_H
