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

#ifndef DOMAINEDITOR_H
#define DOMAINEDITOR_H

#include <Cutelyst/Controller>

using namespace Cutelyst;
class SkaffariEngine;
class DomainEditor : public Controller
{
    Q_OBJECT
    C_NAMESPACE("domain")
public:
    explicit DomainEditor(QObject *parent = 0);
    ~DomainEditor();

    C_ATTR(index, :Path :Args(0))
    void index(Context *c);

    C_ATTR(base, :Chained("/") :PathPart("domain") :CaptureArgs(1))
    void base(Context *c, const QString &id);

    C_ATTR(edit, :Chained("base") :PathPart("edit") :Args(0))
    void edit(Context *c);

    C_ATTR(accounts, :Chained("base") :PathPart("accounts") :Args(0))
    void accounts(Context *c);

    C_ATTR(remove, :Chained("base") :PathPart("remove") :Args(0))
    void remove(Context *c);

    C_ATTR(add_account, :Chained("base") :PathPart("add_account") :Args(0))
    void add_account(Context *c);

    C_ATTR(create, :Local("create") :Args(0))
    void create(Context *c);

private:
    static QStringList trimFolderStrings(const QStringList &folders);
};

#endif //DOMAINEDITOR_H

