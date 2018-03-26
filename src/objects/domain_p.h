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
#include <QCollator>

class DomainNameCollator : public QCollator
{
public:
    explicit DomainNameCollator(const QLocale &locale) :
        QCollator(locale)
    {}

    bool operator() (const Domain &left, const Domain &right) { return (compare(left.name(), right.name()) < 0); }
};

class DomainData : public QSharedData
{
public:
    DomainData() {}

    DomainData(dbid_t _id, const QString &_name, const QString &_prefix, const QString &_transport, quota_size_t _quota, quint32 _maxAccounts, quota_size_t _domainQuota, quota_size_t _domainQuotaUsed, bool _freeNames, bool _freeAddress, const QVector<Folder> &_folders, quint32 _accounts, const QDateTime &_created, const QDateTime &_updated) :
        quota(_quota),
        domainQuota(_domainQuota),
        domainQuotaUsed(_domainQuotaUsed),
        folders(_folders),
        name(_name),
        prefix(_prefix),
        transport(_transport),
        created(_created),
        updated(_updated),
        id(_id),
        maxAccounts(_maxAccounts),
        accounts(_accounts),
        freeNames(_freeNames),
        freeAddress(_freeAddress)
    {}

//    DomainData(const DomainData &other) :
//        QSharedData(other),
//        quota(other.quota),
//        domainQuota(other.domainQuota),
//        domainQuotaUsed(other.domainQuotaUsed),
//        children(other.children),
//        admins(other.admins),
//        folders(other.folders),
//        parent(other.parent),
//        name(other.name),
//        prefix(other.prefix),
//        transport(other.transport),
//        created(other.created),
//        updated(other.updated),
//        validUntil(other.validUntil),
//        id(other.id),
//        ace_id(other.ace_id),
//        maxAccounts(other.maxAccounts),
//        accounts(other.accounts),
//        freeNames(other.freeNames),
//        freeAddress(other.freeAddress)
//    {}

    ~DomainData() {}

    quota_size_t quota = 0;
    quota_size_t domainQuota = 0;
    quota_size_t domainQuotaUsed = 0;
    QVector<SimpleDomain> children;
    QVector<SimpleAdmin> admins;
    QVector<Folder> folders;
    SimpleDomain parent;
    QString name;
    QString prefix;
    QString transport;
    QDateTime created;
    QDateTime updated;
    QDateTime validUntil;
    dbid_t id = 0;
    dbid_t ace_id = 0;
    quint32 maxAccounts = 0;
    quint32 accounts = 0;
    bool freeNames = false;
    bool freeAddress = false;

};

#endif // DOMAIN_P_H
