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

    AccountData(dbid_t _id, dbid_t _domainId, const QString &_username, const QString &_prefix, const QString &_domainName, bool _imap, bool _pop, bool _sieve, bool _smptauth, const QStringList &_addresses, const QStringList _forwards, quota_size_t _quota, quota_size_t _usage, const QDateTime &_created, const QDateTime &_updated, const QDateTime &_validUntil, const QDateTime &_pwExpires, bool _keepLocal, bool _catchAll, quint8 _status) :
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
        passwordExpires(_pwExpires),
        keepLocal(_keepLocal),
        catchAll(_catchAll),
        status(_status)
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
        usage(other.usage),
        created(other.created),
        updated(other.updated),
        validUntil(other.validUntil),
        passwordExpires(other.passwordExpires),
        keepLocal(other.keepLocal),
        catchAll(other.catchAll),
        status(other.status)
    {}

    ~AccountData() {}

    dbid_t id = 0;
    dbid_t domainId = 0;
    QString username;
    QString prefix;
    QString domainName;
    bool imap = false;
    bool pop = false;
    bool sieve = false;
    bool smtpauth = false;
    QStringList addresses;
    QStringList forwards;
    quota_size_t quota = 0;
    quota_size_t usage = 0;
    QDateTime created;
    QDateTime updated;
    QDateTime validUntil;
    QDateTime passwordExpires;
    bool keepLocal = false;
    bool catchAll = false;
    quint8 status = 0;
};

#endif // ACCOUNT_P_H
