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

#ifndef LOGOUT_H
#define LOGOUT_H

#include <Cutelyst/Controller>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(SK_LOGIN)

using namespace Cutelyst;

class Logout : public Controller
{
    Q_OBJECT
public:
    explicit Logout(QObject *parent = 0);
    ~Logout();

    C_ATTR(index, :Path :Args(0))
    void index(Context *c);
};

#endif //LOGOUT_H

