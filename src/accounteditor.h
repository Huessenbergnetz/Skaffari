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

    C_ATTR(email, :Chained("base") :PathPart("email") :Args(1))
    void email(Context *c, const QString &address);

    C_ATTR(remove_email, :Chained("base") :PathPart("remove_email") :Args(1))
    void remove_email(Context *c, const QString &address);

    C_ATTR(new_email, :Chained("base") :PathPart("new_email") :Args(0))
    void new_email(Context *c);

    C_ATTR(forwards, :Chained("base") :PathPart("forwards") :Args(0))
    void forwards(Context *c);

    C_ATTR(check, :Chained("base") :PathPart("check") :Args(0))
    void check(Context *c);
};

#endif //ACCOUNTEDITOR_H

