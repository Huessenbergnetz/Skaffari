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

#ifndef DOMAIN_P_H
#define DOMAIN_P_H

#include "domain.h"
#include <QSharedData>

class DomainData : public QSharedData
{
public:
    DomainData() {}

    DomainData(quint32 _id, const QString &_name, const QString &_prefix, const QString &_transport, quint32 _quota, quint32 _maxAccounts, quint32 _domainQuota, quint32 _domainQuotaUsed, bool _freeNames, bool _freeAddress, const QVector<Folder> &_folders, quint32 _accounts, const QDateTime &_created, const QDateTime &_updated) :
        id(_id),
        name(_name),
        prefix(_prefix),
        transport(_transport),
        quota(_quota),
        maxAccounts(_maxAccounts),
        domainQuota(_domainQuota),
        domainQuotaUsed(_domainQuotaUsed),
        freeNames(_freeNames),
        freeAddress(_freeAddress),
        folders(_folders),
        accounts(_accounts),
        created(_created),
        updated(_updated)
    {}

    DomainData(const DomainData &other) :
        QSharedData(other),
        id(other.id),
        name(other.name),
        prefix(other.prefix),
        transport(other.transport),
        quota(other.quota),
        humanQuota(other.humanQuota),
        maxAccounts(other.maxAccounts),
        domainQuota(other.domainQuota),
        humanDomainQuota(other.humanDomainQuota),
        domainQuotaUsed(other.domainQuotaUsed),
        humanDomainQuotaUsed(other.humanDomainQuotaUsed),
        freeNames(other.freeNames),
        freeAddress(other.freeAddress),
        folders(other.folders),
        accounts(other.accounts),
        created(other.created),
        updated(other.updated)
    {}

    ~DomainData() {}

    quint32 id = 0;
    QString name;
    QString prefix;
    QString transport;
    quint32 quota = 0;
    QString humanQuota;
    quint32 maxAccounts = 0;
    quint32 domainQuota = 0;
    QString humanDomainQuota;
    quint32 domainQuotaUsed = 0;
    QString humanDomainQuotaUsed;
    bool freeNames = false;
    bool freeAddress = false;
    QVector<Folder> folders;
    quint32 accounts = 0;
    QDateTime created;
    QDateTime updated;
    QVector<SimpleAdmin> admins;
};

#endif // DOMAIN_P_H
