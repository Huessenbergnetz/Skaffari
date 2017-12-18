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

#ifndef ACCOUNTEDITOR_H
#define ACCOUNTEDITOR_H

#include <Cutelyst/Controller>

using namespace Cutelyst;
class SkaffariEngine;
class AccountEditor : public Controller
{
    Q_OBJECT
    C_NAMESPACE("account")
public:
    explicit AccountEditor(QObject *parent = nullptr);
    ~AccountEditor();

    C_ATTR(base, :Chained("/") :PathPart("account") :CaptureArgs(2))
    void base(Context *c, const QString &domainId, const QString &accountId);
    
    C_ATTR(edit, :Chained("base") :PathPart("edit") :Args(0))
    void edit(Context *c);
    
    C_ATTR(remove, :Chained("base") :PathPart("remove") :Args(0))
    void remove(Context *c);

    C_ATTR(addresses, :Chained("base") :PathPart("addresses") :Args(0))
    void addresses(Context *c);

    C_ATTR(edit_address, :Chained("base") :PathPart("edit_address") :Args(1))
    void edit_address(Context *c, const QString &address);

    C_ATTR(remove_address, :Chained("base") :PathPart("remove_address") :Args(1))
    void remove_address(Context *c, const QString &address);

    C_ATTR(add_address, :Chained("base") :PathPart("add_address") :Args(0))
    void add_address(Context *c);

    C_ATTR(forwards, :Chained("base") :PathPart("forwards") :Args(0))
    void forwards(Context *c);

    C_ATTR(remove_forward, :Chained("base") :PathPart("remove_forward") :Args(1))
    void remove_forward(Context *c, const QString &forward);

    C_ATTR(add_forward, :Chained("base") :PathPart("add_forward") :Args(0))
    void add_forward(Context *c);

    C_ATTR(edit_forward, :Chained("base") :PathPart("edit_forward") :Args(1))
    void edit_forward(Context *c, const QString &oldForward);

    C_ATTR(keep_local, :Chained("base") :PathPart("keep_local") :Args(0))
    void keep_local(Context *c);

    C_ATTR(check, :Chained("base") :PathPart("check") :Args(0))
    void check(Context *c);

    C_ATTR(list, :Local("list") :Args(0))
    void list(Context *c);
};

#endif //ACCOUNTEDITOR_H

