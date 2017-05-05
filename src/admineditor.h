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

#ifndef ADMINEDITOR_H
#define ADMINEDITOR_H

#include <Cutelyst/Controller>

using namespace Cutelyst;
class SkaffariEngine;

class AdminEditor : public Controller
{
    Q_OBJECT
    C_NAMESPACE("admin")
public:
    explicit AdminEditor(QObject *parent = nullptr);
    ~AdminEditor();

    C_ATTR(index, :Path :Args(0))
    void index(Context *c);

    C_ATTR(base, :Chained("/") :PathPart("admin") :CaptureArgs(1))
    void base(Context *c, const QString &id);

    C_ATTR(edit, :Chained("base") :PathPart("edit") :Args(0))
    void edit(Context *c);

    C_ATTR(remove, :Chained("base") :PathPart("remove") : Args(0))
    void remove(Context *c);

    C_ATTR(create, :Local("create") :Args(0))
    void create(Context *c);

private:
    static bool checkAccess(Context *c);

    static bool accessGranted(Context *c);
};

#endif // ADMINEDITOR_H
