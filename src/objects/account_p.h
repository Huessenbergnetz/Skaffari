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

#ifndef ACCOUNT_P_H
#define ACCOUNT_P_H

#include "account.h"
#include <QSharedData>

class AccountData: public QSharedData
{
public:
    AccountData() {}

    AccountData(quint32 _id, quint32 _domainId, const QString &_username, const QString &_prefix, const QString &_domainName, bool _imap, bool _pop, bool _sieve, bool _smptauth, const QStringList &_addresses, const QStringList _forwards, qint32 _quota, qint32 _usage, const QDateTime &_created, const QDateTime &_updated, const QDateTime &_validUntil, bool _keepLocal) :
        id(_id),
        domainId(_domainId),
        username(_username),
        prefix(_prefix),
        domainName(_domainName),
        imap(_imap),
        pop(_pop),
        sieve(_sieve),
        smtpauth(_smptauth),
        addresses(_addresses),
        forwards(_forwards),
        quota(_quota),
        usage(_usage),
        created(_created),
        updated(_updated),
        validUntil(_validUntil),
        keepLocal(_keepLocal)
    {}

    AccountData(const AccountData &other) :
        QSharedData(other),
        id(other.id),
        domainId(other.domainId),
        username(other.username),
        prefix(other.prefix),
        domainName(other.domainName),
        imap(other.imap),
        pop(other.pop),
        sieve(other.sieve),
        smtpauth(other.smtpauth),
        addresses(other.addresses),
        forwards(other.forwards),
        quota(other.quota),
        humanQuota(other.humanQuota),
        usage(other.usage),
        humanUsage(other.humanUsage),
        created(other.created),
        updated(other.updated),
        validUntil(other.validUntil),
        keepLocal(other.keepLocal)
    {}

    ~AccountData() {}

    quint32 id = 0;
    quint32 domainId = 0;
    QString username;
    QString prefix;
    QString domainName;
    bool imap = false;
    bool pop = false;
    bool sieve = false;
    bool smtpauth = false;
    QStringList addresses;
    QStringList forwards;
    qint32 quota = 0;
    QString humanQuota;
    qint32 usage = 0;
    QString humanUsage;
    QDateTime created;
    QDateTime updated;
    QDateTime validUntil;
    bool keepLocal = false;
};

#endif // ACCOUNT_P_H
