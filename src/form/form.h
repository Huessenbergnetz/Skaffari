/*
 * Skaffari - a mail account administration web interface based on Cutelyst
 * Copyright (C) 2017-2019 Matthias Fehring <mf@huessenbergnetz.de>
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

#ifndef FORM_H
#define FORM_H

#include "fieldset.h"
#include <initializer_list>
#include <QHash>

class Form
{
public:
    Form(std::initializer_list<Fieldset> fieldsets);
    ~Form();

private:
    QHash<QString,Fieldset> m_fieldsets;
};

#endif // FORM_H
