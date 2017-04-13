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

#include "simpledomain_p.h"
#include "skaffarierror.h"
#include <Cutelyst/Context>
#include <Cutelyst/Plugins/Utils/Sql>
#include <QSqlQuery>
#include <QSqlError>

SimpleDomain::SimpleDomain() : d(new SimpleDomainData)
{

}


SimpleDomain::SimpleDomain(quint32 id, const QString &name) :
    d(new SimpleDomainData(id, name))
{

}


SimpleDomain::SimpleDomain(const SimpleDomain &other) :
    d(other.d)
{

}


SimpleDomain& SimpleDomain::operator=(const SimpleDomain &other)
{
    d = other.d;
    return *this;
}

SimpleDomain::~SimpleDomain()
{

}


quint32 SimpleDomain::id() const
{
    return d->id;
}


QString SimpleDomain::name() const
{
    return d->name;
}


std::vector<SimpleDomain> SimpleDomain::list(Cutelyst::Context *c, SkaffariError *e, quint16 userType, quint32 adminId)
{
    std::vector<SimpleDomain> lst;

    Q_ASSERT_X(c, "list simple domains", "invalid context object");
    Q_ASSERT_X(e, "list simple domains", "invalid error object");

    QSqlQuery q;

    if (userType == 0) {
        q = CPreparedSqlQueryThread(QStringLiteral("SELECT id, domain_name FROM domain ORDER BY domain_name ASC"));
    } else {
        q = CPreparedSqlQueryThread(QStringLiteral("SELECT dom.id, dom.domain_name FROM domain dom LEFT JOIN domainadmin da ON dom.id = da.domain_id WHERE da.admin_id = :admin_id ORDER BY dom.domain_name ASC"));
        q.bindValue(QStringLiteral(":admin_id"), adminId);
    }

    if (!q.exec()) {
        e->setSqlError(q.lastError(), c->translate("SimpleDomain", "Failed to query the list of domains from the database."));
        return lst;
    }

    while (q.next()) {
        lst.push_back(SimpleDomain(q.value(0).value<quint32>(), q.value(1).toString()));
    }

    return lst;
}
