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

#ifndef SETTINGSEDITOR_H
#define SETTINGSEDITOR_H

#include <Cutelyst/Controller>

using namespace Cutelyst;

/*!
 * \ingroup skaffaricore
 * \brief Routes for the settings namespace.
 */
class SettingsEditor : public Controller
{
    Q_OBJECT
    C_NAMESPACE("settings")
public:
    explicit SettingsEditor(QObject *parent = nullptr);
    ~SettingsEditor();

    C_ATTR(index, :Path :Args(0))
    void index(Context *c);

private:
    static bool checkAccess(Context *c);

    static bool accessGranted(Context *c);
};

#endif // SETTINGSEDITOR_H
