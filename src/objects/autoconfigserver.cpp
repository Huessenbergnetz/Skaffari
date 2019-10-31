/*
 * Skaffari - a mail account administration web interface based on Cutelyst
 * Copyright (C) 2019 Matthias Fehring <mf@huessenbergnetz.de>
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

#include "autoconfigserver.h"
#include "skaffarierror.h"
#include "../utils/skaffariconfig.h"
#include <Cutelyst/Context>
#include <Cutelyst/Plugins/Utils/Sql>
#include <Cutelyst/Plugins/Memcached/Memcached>
#include <QSharedData>
#include <QDebug>
#include <QDataStream>
#include <QSqlError>
#include <QSqlQuery>

#define SK_AC_MEMC_AUTOCONFIG_GLOBAL "autoconfig_global"
#define SK_AC_MEMC_AUTOCONFIG_PREFIX "autoconfig_"
#define SK_AC_MEMC_AUTOCONFIG_STORAGE_DURATION 7200

class AutoconfigServer::Data : public QSharedData
{
public:
    Data() : QSharedData() {}

    Data(dbid_t _id, dbid_t _domainId, Type _type, const QString &_hostname, quint16 _port, SocketType _socketType, Authentication _authentication, qint8 _sorting) :
        QSharedData(),
        hostname(_hostname),
        id(_id),
        domainId(_domainId),
        port(_port),
        type(_type),
        socketType(_socketType),
        authentication(_authentication),
        sorting(_sorting)
    {}

    ~Data() {}

    QString hostname;
    dbid_t id = 0;
    dbid_t domainId = 0;
    quint16 port = 0;
    AutoconfigServer::Type type = AutoconfigServer::Imap;
    AutoconfigServer::SocketType socketType = AutoconfigServer::Plain;
    AutoconfigServer::Authentication authentication = AutoconfigServer::Cleartext;
    qint8 sorting = 0;
};

AutoconfigServer::AutoconfigServer() : d(new Data)
{

}

AutoconfigServer::AutoconfigServer(dbid_t id, dbid_t domainId, Type type, const QString &hostname, quint16 port, SocketType socketType, Authentication authentication, qint8 sorting) :
    d(new Data(id, domainId, type, hostname, port, socketType, authentication, sorting))
{

}

AutoconfigServer::AutoconfigServer(const AutoconfigServer &other) :
    d(other.d)
{

}

AutoconfigServer::AutoconfigServer(AutoconfigServer &&other) noexcept :
    d(std::move(other.d))
{
    other.d = nullptr;
}

AutoconfigServer& AutoconfigServer::operator=(const AutoconfigServer &other)
{
    d = other.d;
    return *this;
}

AutoconfigServer& AutoconfigServer::operator=(AutoconfigServer &&other) noexcept
{
    swap(other);
    return *this;
}

AutoconfigServer::~AutoconfigServer()
{

}

void AutoconfigServer::swap(AutoconfigServer &other)
{
    std::swap(d, other.d);
}

dbid_t AutoconfigServer::id() const
{
    return d->id;
}

dbid_t AutoconfigServer::domainId() const
{
    return d->domainId;
}

AutoconfigServer::Type AutoconfigServer::type() const
{
    return d->type;
}

QString AutoconfigServer::hostname() const
{
    return d->hostname;
}

quint16 AutoconfigServer::port() const
{
    return d->port;
}

AutoconfigServer::SocketType AutoconfigServer::socketType() const
{
    return d->socketType;
}

AutoconfigServer::Authentication AutoconfigServer::authentication() const
{
    return d->authentication;
}

qint8 AutoconfigServer::sorting() const
{
    return d->sorting;
}

bool AutoconfigServer::isValid() const
{
    return d->id > 0;
}

bool AutoconfigServer::operator==(const AutoconfigServer &other)
{
    return d->id == other.d->id && d->domainId == other.d->domainId && d->type == other.d->type && d->hostname == other.d->hostname && d->port == other.d->port && d->socketType == other.d->socketType && d->authentication == other.d->authentication;
}

bool AutoconfigServer::operator!=(const AutoconfigServer &other)
{
    return d->id != other.d->id || d->domainId != other.d->domainId || d->type != other.d->type || d->hostname != other.d->hostname || d->port != other.d->port || d->socketType != other.d->socketType || d->authentication != other.d->authentication;
}

AutoconfigServer AutoconfigServer::get(Cutelyst::Context *c, dbid_t domainId, dbid_t serverId, SkaffariError &e)
{
    AutoconfigServer s;

    Q_ASSERT_X(c, "get autoconfig server", "invalid context object");
    Q_ASSERT_X(serverId, "get autoconfig server", "invalid server datbase id");

    QSqlQuery q;

    if (domainId) {
        q = CPreparedSqlQueryThread(QStringLiteral("SELECT type, hostname, port, sockettype, authentication, sorting FROM autoconfig WHERE id = :id AND domain_id = :domain_id"));
        q.bindValue(QStringLiteral(":id"), serverId);
        q.bindValue(QStringLiteral(":domain_id"), domainId);
    } else {
        q = CPreparedSqlQueryThread(QStringLiteral("SELECT type, hostname, port, sockettype, authentication, sorting FROM autoconfig_global WHERE id = :id"));
        q.bindValue(QStringLiteral(":id"), serverId);
    }

    if (Q_UNLIKELY(!q.exec())) {
        const QString errorText = domainId
                ? c->translate("AutoconfigServer", "Failed to get autoconfig server with ID %1 from the database.").arg(serverId)
                : c->translate("AutoconfigServer", "Failed to get global autoconfig server with ID %1 from the database.").arg(serverId);
        e.setSqlError(q.lastError(), errorText);
        return s;
    }

    if (Q_UNLIKELY(!q.next())) {
        e.setErrorType(SkaffariError::NotFound);
        if (domainId) {
            e.setErrorText(c->translate("AutoconfigServer", "Can not find autoconfig server with ID %1 in the database.").arg(serverId));
        } else {
            e.setErrorText(c->translate("AutoconfigServer", "Can not find global autoconfig server with ID %1 in the database.").arg(serverId));
        }
        return s;
    }

    s = AutoconfigServer(serverId,
                         domainId,
                         static_cast<AutoconfigServer::Type>(q.value(0).value<qint8>()),
                         q.value(1).toString(),
                         q.value(2).value<quint16>(),
                         static_cast<AutoconfigServer::SocketType>(q.value(3).value<qint8>()),
                         static_cast<AutoconfigServer::Authentication>(q.value(4).value<qint8>()),
                         q.value(5).value<qint8>());

    return s;
}

std::vector<AutoconfigServer> AutoconfigServer::list(Cutelyst::Context *c, dbid_t domainId, SkaffariError &e)
{
    std::vector<AutoconfigServer> lst;

    Q_ASSERT_X(c, "list autoconfig server by domain id", "invalid context object");

    if (SkaffariConfig::useMemcached()) {
        Cutelyst::Memcached::MemcachedReturnType memrt = Cutelyst::Memcached::NotFound;
        const QString memKey = domainId ? QLatin1String(SK_AC_MEMC_AUTOCONFIG_PREFIX) + QString::number(domainId) : QStringLiteral(SK_AC_MEMC_AUTOCONFIG_GLOBAL);
        lst = Cutelyst::Memcached::get<std::vector<AutoconfigServer>>(memKey, nullptr, &memrt);
        if (memrt == Cutelyst::Memcached::Success) {
            return lst;
        }
    }

    QSqlQuery q;

    if (domainId) {
        q = CPreparedSqlQueryThread(QStringLiteral("SELECT id, domain_id, type, hostname, port, sockettype, authentication, sorting FROM autoconfig WHERE domain_id = :domain_id ORDER BY sorting ASC"));
        q.bindValue(QStringLiteral(":domain_id"), domainId);
    } else {
        q = CPreparedSqlQueryThread(QStringLiteral("SELECT id, 0 as domain_id, type, hostname, port, sockettype, authentication, sorting FROM autoconfig_global ORDER BY sorting ASC"));
    }

    if (Q_UNLIKELY(!q.exec())) {
        const QString errorText = domainId
                ? c->translate("AutoconfigServer", "Failed to query the list of autoconfig servers for domain ID %1 from the database.").arg(domainId)
                : c->translate("AutoconfigServer", "Failed to query the list of global autoconfig servers from the database.");
        e.setSqlError(q.lastError(), errorText);
        return lst;
    }

    if (q.size() > -1) {
        lst.reserve(static_cast<std::vector<AutoconfigServer>::size_type>(q.size()));
    }

    while (q.next()) {
        lst.emplace_back(q.value(0).value<dbid_t>(),
                         q.value(1).value<dbid_t>(),
                         static_cast<AutoconfigServer::Type>(q.value(2).value<qint8>()),
                         q.value(3).toString(),
                         q.value(4).value<quint16>(),
                         static_cast<AutoconfigServer::SocketType>(q.value(5).value<qint8>()),
                         static_cast<AutoconfigServer::Authentication>(q.value(6).value<qint8>()),
                         q.value(7).value<qint8>());
    }

    if (SkaffariConfig::useMemcached()) {
        const QString memKey = domainId ? QLatin1String(SK_AC_MEMC_AUTOCONFIG_PREFIX) + QString::number(domainId) : QStringLiteral(SK_AC_MEMC_AUTOCONFIG_GLOBAL);
        Cutelyst::Memcached::set<std::vector<AutoconfigServer>>(memKey, lst, SK_AC_MEMC_AUTOCONFIG_STORAGE_DURATION);
    }

    return lst;
}

AutoconfigServer AutoconfigServer::create(Cutelyst::Context *c, dbid_t domainId, const QVariantHash &p, SkaffariError &e)
{
    AutoconfigServer s;

    Q_ASSERT_X(c, "create autoconfig server", "invalid context object");

    const qint8 type = p.value(QStringLiteral("type")).value<qint8>();
    const QString hostname = p.value(QStringLiteral("hostname")).toString();
    const quint16 port = p.value(QStringLiteral("port")).value<quint16>();
    const qint8 socketType = p.value(QStringLiteral("socketType")).value<qint8>();
    const qint8 authentication = p.value(QStringLiteral("authentication")).value<qint8>();
    const qint8 sorting = p.value(QStringLiteral("sorting"), 0).value<qint8>();

    QSqlQuery q;

    if (domainId) {
        q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO autoconfig (domain_id, type, hostname, port, sockettype, authentication, sorting) "
                                                   "VALUES (:domain_id, :type, :hostname, :port, :sockettype, :authentication, :sorting)"));
        q.bindValue(QStringLiteral(":domain_id"), domainId);
    } else {
        q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO autoconfig_global (type, hostname, port, sockettype, authentication, sorting) "
                                                   "VALUES (:type, :hostname, :port, :sockettype, :authentication, :sorting)"));
    }

    q.bindValue(QStringLiteral(":type"), type);
    q.bindValue(QStringLiteral(":hostname"), hostname);
    q.bindValue(QStringLiteral(":port"), port);
    q.bindValue(QStringLiteral(":sockettype"), socketType);
    q.bindValue(QStringLiteral(":authentication"), authentication);
    q.bindValue(QStringLiteral(":sorting"), sorting);

    if (Q_UNLIKELY(!q.exec())) {
        const QString errorText = domainId
                ? c->translate("AutoconfigServer", "Failed to insert new autoconfig server for domain ID %i into the database.").arg(domainId)
                : c->translate("AutoconfigServer", "Failed to insert new global autoconfig server into the database.");
        e.setSqlError(q.lastError(), errorText);
        return s;
    }

    const dbid_t id = q.lastInsertId().value<dbid_t>();

    s = AutoconfigServer(id,
                         domainId,
                         static_cast<AutoconfigServer::Type>(type),
                         hostname,
                         port,
                         static_cast<AutoconfigServer::SocketType>(socketType),
                         static_cast<AutoconfigServer::Authentication>(authentication),
                         sorting);

    if (SkaffariConfig::useMemcached()) {
        const QString memKey = domainId ? QLatin1String(SK_AC_MEMC_AUTOCONFIG_PREFIX) + QString::number(domainId) : QStringLiteral(SK_AC_MEMC_AUTOCONFIG_GLOBAL);
        Cutelyst::Memcached::remove(memKey);
    }

    return s;
}

bool AutoconfigServer::remove(Cutelyst::Context *c, SkaffariError &e)
{
    Q_ASSERT_X(c, "remove autoconfig server", "invalid context object");

    QSqlQuery q;
    if (d->domainId) {
        q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM autoconfig WHERE id = :id AND domain_id = :domain_id"));
        q.bindValue(QStringLiteral(":id"), d->id);
        q.bindValue(QStringLiteral(":domain_id"), d->domainId);
    } else {
        q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM autoconfig_global WHERE id = :id"));
        q.bindValue(QStringLiteral(":id"), d->id);
    }

    if (Q_UNLIKELY(!q.exec())) {
        const QString errorText = d->domainId
                ? c->translate("AutoconfigServer", "Failed to remove autoconfig server with ID %1 from the database.").arg(d->id)
                : c->translate("AutoconfigServer", "Failed to remove global autoconfig server with ID %1 from the database.").arg(d->id);
        e.setSqlError(q.lastError(), errorText);
        return false;
    }

    if (SkaffariConfig::useMemcached()) {
        const QString memKey = d->domainId ? QLatin1String(SK_AC_MEMC_AUTOCONFIG_PREFIX) + QString::number(d->domainId) : QStringLiteral(SK_AC_MEMC_AUTOCONFIG_GLOBAL);
        Cutelyst::Memcached::remove(memKey);
    }

    return true;
}

bool AutoconfigServer::update(Cutelyst::Context *c, const QVariantHash &p, SkaffariError &e)
{
    Q_ASSERT_X(c, "update autoconfig server", "invalid context object");

    const qint8 typeInt = p.value(QStringLiteral("type")).value<qint8>();
    const Type _type = static_cast<Type>(typeInt);
    const QString _hostname = p.value(QStringLiteral("hostname")).toString();
    const quint16 _port = p.value(QStringLiteral("port")).value<quint16>();
    const qint8 socketTypeInt = p.value(QStringLiteral("socketType")).value<qint8>();
    const SocketType _socketType = static_cast<SocketType>(socketTypeInt);
    const qint8 authenticationInt = p.value(QStringLiteral("authentication")).value<qint8>();
    const Authentication _authentication = static_cast<Authentication>(authenticationInt);
    const qint8 _sorting = p.value(QStringLiteral("sorting"), 0).value<qint8>();

    if (type() == _type && hostname() == _hostname && port() == _port && socketType() == _socketType && authentication() == _authentication && sorting() == _sorting) {
        // nothing changed
        return true;
    }

    QSqlQuery q;
    if (d->domainId) {
        q = CPreparedSqlQueryThread(QStringLiteral("UPDATE autoconfig SET type = :type, hostname = :hostname, port = :port, sockettype = :sockettype, authentication = :authentication, sorting = :sorting WHERE id = :id"));
    } else{
        q = CPreparedSqlQueryThread(QStringLiteral("UPDATE autoconfig_global SET type = :type, hostname = :hostname, port = :port, sockettype = :sockettype, authentication = :authentication, sorting = :sorting WHERE id = :id"));
    }
    q.bindValue(QStringLiteral(":type"), typeInt);
    q.bindValue(QStringLiteral(":hostname"), _hostname);
    q.bindValue(QStringLiteral(":port"), _port);
    q.bindValue(QStringLiteral(":sockettype"), socketTypeInt);
    q.bindValue(QStringLiteral(":authentication"), authenticationInt);
    q.bindValue(QStringLiteral(":sorting"), _sorting);
    q.bindValue(QStringLiteral(":id"), d->id);

    if (Q_UNLIKELY(!q.exec())) {
        const QString errorText = d->domainId
                ? c->translate("AutoconfigServer", "Failed to update autoconfig server with ID %1 in the database.").arg(d->id)
                : c->translate("AutoconfigServer", "Failed to update global autoconfig server with ID %1 in the database.").arg(d->id);
        e.setSqlError(q.lastError(), errorText);
        return false;
    }

    if (SkaffariConfig::useMemcached()) {
        const QString memKey = d->domainId ? QLatin1String(SK_AC_MEMC_AUTOCONFIG_PREFIX) + QString::number(d->domainId) : QStringLiteral(SK_AC_MEMC_AUTOCONFIG_GLOBAL);
        Cutelyst::Memcached::remove(memKey);
    }

    return true;
}

QDebug operator<<(QDebug dbg, const AutoconfigServer &server)
{
    QDebugStateSaver saver(dbg);

    dbg.nospace() << "AutoconfigServer(";
    dbg << "ID: " << server.id();
    dbg << ", Domain ID: " << server.domainId();
    dbg << ", Type: " << server.type();
    dbg << ", Hostname: " << server.hostname();
    dbg << ", Port: " << server.port();
    dbg << ", SocketType: " << server.socketType();
    dbg << ", Authentication: " << server.authentication();
    dbg << ", Sorting: " << server.sorting();
    dbg << ')';
    return dbg.maybeSpace();
}

QDataStream &operator<<(QDataStream &stream, const AutoconfigServer &server)
{
    stream << server.id()
           << server.domainId()
           << static_cast<qint8>(server.type())
           << server.hostname()
           << server.port()
           << static_cast<qint8>(server.socketType())
           << static_cast<qint8>(server.authentication())
           << server.sorting();
    return stream;
}

QDataStream &operator>>(QDataStream &stream, AutoconfigServer &server)
{
    qint8 type = 0;
    qint8 socketType = 0;
    qint8 authentication = 0;

    stream >> server.d->id;
    stream >> server.d->domainId;
    stream >> type;
    stream >> server.d->hostname;
    stream >> server.d->port;
    stream >> socketType;
    stream >> authentication;
    stream >> server.d->sorting;

    server.d->type = static_cast<AutoconfigServer::Type>(type);
    server.d->socketType = static_cast<AutoconfigServer::SocketType>(socketType);
    server.d->authentication = static_cast<AutoconfigServer::Authentication>(authentication);

    return stream;
}

QDataStream &operator<<(QDataStream &stream, const std::vector<AutoconfigServer> &servers)
{
    const quint64 serverCount = servers.size();
    stream << serverCount;
    if (serverCount > 0) {
        for (const AutoconfigServer &server : servers) {
            stream << server;
        }
    }

    return stream;
}

QDataStream &operator>>(QDataStream &stream, std::vector<AutoconfigServer> &servers)
{
    quint64 serverCount = 0;
    stream >> serverCount;
    if (serverCount > 0) {
        servers.reserve(serverCount);
        for (quint64 i = 0; i < serverCount; ++i) {
            AutoconfigServer server;
            stream >> server;
            servers.push_back(server);
        }
    }

    return stream;
}
