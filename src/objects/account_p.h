/*
 * Skaffari - a mail account administration web interface based on Cutelyst
 * Copyright (C) 2017-2018 Matthias Fehring <mf@huessenbergnetz.de>
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
    AccountData() : QSharedData() {}

    AccountData(dbid_t _id, dbid_t _domainId, const QString &_username, bool _imap, bool _pop, bool _sieve, bool _smptauth, const QStringList &_addresses, const QStringList &_forwards, quota_size_t _quota, quota_size_t _usage, const QDateTime &_created, const QDateTime &_updated, const QDateTime &_validUntil, const QDateTime &_pwExpires, bool _keepLocal, bool _catchAll, quint8 _status) :
        QSharedData(),
        quota(_quota),
        usage(_usage),
        addresses(_addresses),
        forwards(_forwards),
        username(_username),
        created(_created),
        updated(_updated),
        validUntil(_validUntil),
        passwordExpires(_pwExpires),
        id(_id),
        domainId(_domainId),
        status(_status),
        imap(_imap),
        pop(_pop),
        sieve(_sieve),
        smtpauth(_smptauth),
        keepLocal(_keepLocal),
        catchAll(_catchAll)
    {}

    ~AccountData() {}

    QString nameIdString() const;
    bool canAddAddress(Cutelyst::Context *c, SkaffariError &e, const Domain &targetDomain, const QString &address) const;

    quota_size_t quota = 0;
    quota_size_t usage = 0;
    QStringList addresses;
    QStringList forwards;
    QString username;
    QDateTime created;
    QDateTime updated;
    QDateTime validUntil;
    QDateTime passwordExpires;
    dbid_t id = 0;
    dbid_t domainId = 0;
    quint8 status = 0;
    bool imap = false;
    bool pop = false;
    bool sieve = false;
    bool smtpauth = false;
    bool keepLocal = false;
    bool catchAll = false;
};

#endif // ACCOUNT_P_H
