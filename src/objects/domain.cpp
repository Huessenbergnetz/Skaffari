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

#include "domain_p.h"
#include "skaffarierror.h"
#include "account.h"
#include "../utils/utils.h"
#include <Cutelyst/ParamsMultiMap>
#include <Cutelyst/Response>
#include <QSqlQuery>
#include <QSqlError>
#include <Cutelyst/Plugins/Utils/Sql>
#include <Cutelyst/Context>
#include <Cutelyst/Plugins/Authentication/authentication.h>
#include <Cutelyst/Plugins/Authentication/authenticationuser.h>
#include <Cutelyst/Plugins/Session/Session>
#include <QUrl>

Q_LOGGING_CATEGORY(SK_DOMAIN, "skaffari.domain")

Domain::Domain() : d(new DomainData)
{

}

Domain::Domain(quint32 id,
               const QString& name,
               const QString& prefix,
               const QString& transport,
               quint32 quota,
               quint32 maxAccounts,
               quint32 domainQuota,
               quint32 domainQuotaUsed,
               bool freeNames,
               bool freeAddress,
               const QVector<Folder> &folders,
               quint32 accounts,
               const QDateTime &created,
               const QDateTime &updated) :
    d(new DomainData(id,name, prefix, transport, quota, maxAccounts, domainQuota, domainQuotaUsed, freeNames, freeAddress, folders, accounts, created, updated))
{

}



Domain::Domain(const Domain& other) :
    d(other.d)
{

}


Domain& Domain::operator=(const Domain& other)
{
    d = other.d;
    return *this;
}




Domain::~Domain()
{

}


quint32 Domain::id() const
{
    return d->id;
}

void Domain::setId(quint32 id)
{
    d->id = id;
}

QString Domain::getName() const
{
    return d->name;
}


void Domain::setName(const QString &nName)
{
    d->name = nName;
}



QString Domain::getPrefix() const
{
    return d->prefix;
}


void Domain::setPrefix(const QString &nPrefix)
{
    d->prefix = nPrefix;
}



QString Domain::getTransport() const
{
    return d->transport;
}


void Domain::setTransport(const QString &nTransport)
{
    d->transport = nTransport;
}


quint32 Domain::getQuota() const
{
    return d->quota;
}


void Domain::setQuota(quint32 nQuota)
{
    d->quota = nQuota;
}


QString Domain::getHumanQuota() const
{
    return d->humanQuota;
}


void Domain::setHumanQuota(const QString &nHumanQuota)
{
    d->humanQuota = nHumanQuota;
}


quint32 Domain::getMaxAccounts() const
{
    return d->maxAccounts;
}


void Domain::setMaxAccounts(quint32 nMaxAccounts)
{
    d->maxAccounts = nMaxAccounts;
}


quint32 Domain::getDomainQuota() const
{
    return d->domainQuota;
}


void Domain::setDomainQuota(quint32 nDomainQuota)
{
    d->domainQuota = nDomainQuota;
}


QString Domain::getHumanDomainQuota() const
{
    return d->humanDomainQuota;
}


void Domain::setHumanDomainQuota(const QString &humanDomainQuota)
{
    d->humanDomainQuota = humanDomainQuota;
}


quint32 Domain::getDomainQuotaUsed() const
{
    return d->domainQuotaUsed;
}


void Domain::setDomainQuotaUsed(quint32 nDomainQuotaUsed)
{
    d->domainQuotaUsed = nDomainQuotaUsed;
}


QString Domain::getHumanDomainQuotaUsed() const
{
    return d->humanDomainQuotaUsed;
}


void Domain::setHumanDomainQuotaUsed(const QString &humanDomainQuotaUsed)
{
    d->humanDomainQuotaUsed = humanDomainQuotaUsed;
}


bool Domain::isFreeNamesEnabled() const
{
    return d->freeNames;
}


void Domain::setFreeNamesEnabled(bool nFreeNames)
{
    d->freeNames = nFreeNames;
}



bool Domain::isFreeAddressEnabled() const
{
    return d->freeAddress;
}


void Domain::setFreeAddressEnabled(bool nFreeAddress)
{
    d->freeAddress = nFreeAddress;
}



QVector<Folder> Domain::getFolders() const
{
    return d->folders;
}


void Domain::setFolders(const QVector<Folder> &nFolders)
{
    d->folders = nFolders;
}

quint32 Domain::getAccounts() const
{
    return d->accounts;
}

void Domain::setAccounts(quint32 nAccounts)
{
    d->accounts = nAccounts;
}

QVector<SimpleAdmin> Domain::getAdmins() const
{
    return d->admins;
}

void Domain::setAdmins(const QVector<SimpleAdmin> &adminList)
{
    d->admins = adminList;
}


bool Domain::isValid() const
{
    return (!d->name.isEmpty() && !d->prefix.isEmpty());
}

QDateTime Domain::created() const
{
    return d->created;
}

void Domain::setCreated(const QDateTime &dt)
{
    d->created = dt;
}


QDateTime Domain::updated() const
{
    return d->updated;
}

void Domain::setUpdated(const QDateTime &dt)
{
    d->updated = dt;
}


float Domain::domainQuotaUsagePercent() const
{
    if ((d->domainQuota == 0) && (d->domainQuotaUsed == 0)) {
        return 0.0;
    }

    return ((float)d->domainQuotaUsed / (float)d->domainQuota) * 100.0;
}


bool Domain::hasAccess(Cutelyst::Context *c) const
{
    bool ret = false;
    Q_ASSERT_X(c, "cheking domain access", "invalid context object");

    const int type = c->stash(QStringLiteral("userType")).toInt();

    if (type == 0) {
        ret = true;
    } else {

        const quint32 uid = c->stash(QStringLiteral("userId")).value<quint32>();

        if (!d->admins.empty()) {
            for (int i = 0; i < d->admins.size(); ++i) {
                if (d->admins.at(i).id() == uid) {
                    ret = true;
                    break;
                }
            }
        }
    }

    return ret;
}


Domain Domain::create(Cutelyst::Context *c, const Cutelyst::ParamsMultiMap &params, SkaffariError *errorData, bool domainAsPrefix)
{
    Domain dom;

    Q_ASSERT_X(errorData, "create new domain", "invalid errorData object");

    const QString domainName = params.value(QStringLiteral("domainName")).trimmed().toLower();
    const QString prefix = !domainAsPrefix ? params.value(QStringLiteral("prefix")).trimmed().toLower() : domainName;

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT domain_name FROM domain WHERE prefix = :prefix"));
    q.bindValue(QStringLiteral(":prefix"), prefix);

    if (Q_UNLIKELY(!q.exec())) {
        errorData->setSqlError(q.lastError(), c->translate("Domain", "Failed to check if prefix is alredy in use."));
        return dom;
    }

    if (Q_UNLIKELY(q.next())) {
        errorData->setErrorType(SkaffariError::InputError);
        errorData->setErrorText(c->translate("Domain", "The prefix “%1” is already in use by another domain.").arg(prefix));
        return dom;
    }

    const quint32 maxAccounts = params.value(QStringLiteral("maxAccounts")).toULong();
    const quint32 quota = params.value(QStringLiteral("quota")).toULong();
    const quint32 domainQuota = params.value(QStringLiteral("domainQuota")).toULong();
    const bool freeNames = params.contains(QStringLiteral("freeNames"));
    const bool freeAddress = params.contains(QStringLiteral("freeAddress"));
    const QStringList folders = Domain::trimStringList(params.value(QStringLiteral("folders")).split(QLatin1Char(','), QString::SkipEmptyParts));
    const QString transport = params.value(QStringLiteral("transport"), QStringLiteral("cyrus"));
    const QDateTime currentTimeUtc = QDateTime::currentDateTimeUtc();

    q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO domain (domain_name, prefix, maxaccounts, quota, domainquota, freenames, freeaddress, transport, created_at, updated_at) "
                                                         "VALUES (:domain_name, :prefix, :maxaccounts, :quota, :domainquota, :freenames, :freeaddress, :transport, :created_at, :updated_at)"));
    q.bindValue(QStringLiteral(":domain_name"), QUrl::toAce(domainName));
    q.bindValue(QStringLiteral(":prefix"), prefix);
    q.bindValue(QStringLiteral(":maxaccounts"), maxAccounts);
    q.bindValue(QStringLiteral(":quota"), quota);
    q.bindValue(QStringLiteral(":domainquota"), domainQuota);
    q.bindValue(QStringLiteral(":freenames"), freeNames);
    q.bindValue(QStringLiteral(":freeaddress"), freeAddress);
    q.bindValue(QStringLiteral(":transport"), transport);
    q.bindValue(QStringLiteral(":created_at"), currentTimeUtc);
    q.bindValue(QStringLiteral(":updated_at"), currentTimeUtc);

    if (Q_LIKELY(q.exec())) {
        const quint32 domainId = q.lastInsertId().value<quint32>();
        QVector<Folder> foldersVect;
        for (const QString &folder : folders) {
            q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO folder (domain_id, name) VALUES (:domain_id, :name)"));
            q.bindValue(QStringLiteral(":domain_id"), domainId);
            q.bindValue(QStringLiteral(":name"), folder);
            if (Q_LIKELY(q.exec())) {
                const quint32 folderId = q.lastInsertId().value<quint32>();
                foldersVect.append(Folder(folderId, domainId, folder));
            } else {
                errorData->setSqlError(q.lastError());
                q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM folder WHERE domain_id = :domain_id"));
                q.bindValue(QStringLiteral(":domain_id"), domainId);
                q.exec();
                q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM domain WHERE id = :domain_id"));
                q.bindValue(QStringLiteral(":domain_id"), domainId);
                q.exec();
                return dom;
            }
        }

        dom.setId(domainId);
        dom.setName(domainName);
        dom.setPrefix(prefix);
        dom.setMaxAccounts(maxAccounts);
        dom.setQuota(quota);
        dom.setDomainQuota(domainQuota);
        dom.setFreeNamesEnabled(freeNames);
        dom.setFreeAddressEnabled(freeAddress);
        dom.setFolders(foldersVect);
        dom.setTransport(transport);
        dom.setCreated(Utils::toUserTZ(c, currentTimeUtc));
        dom.setUpdated(Utils::toUserTZ(c, currentTimeUtc));

        dom.setHumanQuota(Utils::humanBinarySize(c, (quint64)dom.getQuota() * 1024));
        dom.setHumanDomainQuota(Utils::humanBinarySize(c, (quint64)dom.getDomainQuota() * 1024));
        dom.setHumanDomainQuotaUsed(Utils::humanBinarySize(c, (quint64)dom.getDomainQuotaUsed() * 1024));
    } else {
        errorData->setSqlError(q.lastError());
    }

    return dom;
}


Domain Domain::get(Cutelyst::Context *c, quint32 domId, SkaffariError *errorData)
{
    Domain dom;

    Q_ASSERT_X(errorData, "get domain", "invalid errorData object");
    Q_ASSERT_X(c, "get domain", "invalid Cutelyst context");

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT dom.prefix, dom.domain_name, dom.transport, dom.quota, dom.maxaccounts, dom.domainquota, dom.domainquotaused, dom.freenames, dom.freeaddress, dom.accountcount, dom.created_at, dom.updated_at FROM domain dom WHERE dom.id = :id"));
    q.bindValue(QStringLiteral(":id"), domId);

    if (Q_LIKELY(q.exec() && q.next())) {

        dom.setId(domId);
        dom.setPrefix(q.value(0).toString());
        dom.setName(QUrl::fromAce(q.value(1).toByteArray()));
        dom.setTransport(q.value(2).toString());
        dom.setQuota(q.value(3).value<quint32>());
        dom.setMaxAccounts(q.value(4).value<quint32>());
        dom.setDomainQuota(q.value(5).value<quint32>());
        dom.setDomainQuotaUsed(q.value(6).value<quint32>());
        dom.setFreeNamesEnabled(q.value(7).toBool());
        dom.setFreeAddressEnabled(q.value(8).toBool());
        dom.setAccounts(q.value(9).value<quint32>());
        dom.setCreated(Utils::toUserTZ(c, q.value(10).toDateTime()));
        dom.setUpdated(Utils::toUserTZ(c, q.value(11).toDateTime()));

        dom.setHumanQuota(Utils::humanBinarySize(c, (quint64)dom.getQuota() * 1024));
        dom.setHumanDomainQuota(Utils::humanBinarySize(c, (quint64)dom.getDomainQuota() * 1024));
        dom.setHumanDomainQuotaUsed(Utils::humanBinarySize(c, (quint64)dom.getDomainQuotaUsed() * 1024));

        q = CPreparedSqlQueryThread(QStringLiteral("SELECT id, name FROM folder WHERE domain_id = :domain_id"));
        q.bindValue(QStringLiteral(":domain_id"), domId);
        if (Q_LIKELY(q.exec())) {
            QVector<Folder> folders;
            while (q.next()) {
                folders.append(Folder(q.value(0).value<quint32>(), domId, q.value(1).toString()));
            }
        } else {
            qCCritical(SK_DOMAIN) << "Failed to query default folders for domain" << dom.getName() << "fom database:" << q.lastError().text();
        }

    } else {
        if (q.lastError().type() != QSqlError::NoError) {
            errorData->setSqlError(q.lastError());
        } else {
            errorData->setErrorType(SkaffariError::ApplicationError);
            errorData->setErrorText(c->translate("Domain", "The domain with ID %1 could not be found in the database.").arg(domId));
        }
    }

    q = CPreparedSqlQueryThread(QStringLiteral("SELECT a.id, a.username FROM domainadmin da JOIN adminuser a ON a.id = da.admin_id WHERE da.domain_id = :domain_id"));
    q.bindValue(QStringLiteral(":domain_id"), dom.id());

    if (Q_LIKELY(q.exec())) {
        QVector<SimpleAdmin> admins;
        if (q.next()) {
            admins.append(SimpleAdmin(q.value(0).value<quint32>(), q.value(1).toString()));
        } else {
            errorData->setErrorType(SkaffariError::InputError);
            errorData->setErrorText(c->translate("Domain", "There is no domain with database ID %1.").arg(domId));
        }
    } else {
        errorData->setSqlError(q.lastError());
    }

    return dom;
}


std::vector<Domain> Domain::list(Cutelyst::Context *c, SkaffariError *errorData, const Cutelyst::AuthenticationUser &user)
{
    std::vector<Domain> lst;

    Q_ASSERT_X(errorData, "list domains", "invalid errorData object");
    Q_ASSERT_X(c, "list domains", "invalid Cutelyst context");

    QSqlQuery q;

    if (user.value(QStringLiteral("type")).value<qint16>() == 0) {
        q = CPreparedSqlQueryThread(QStringLiteral("SELECT dom.id, dom.domain_name, dom.prefix, dom.transport, dom.quota, dom.maxaccounts, dom.domainquota, dom.domainquotaused, dom.freenames, dom.freeaddress, dom.accountcount, dom.created_at, dom.updated_at FROM domain dom"));
    } else {
        q = CPreparedSqlQueryThread(QStringLiteral("SELECT dom.id, dom.domain_name, dom.prefix, dom.transport, dom.quota, dom.maxaccounts, dom.domainquota, dom.domainquotaused, dom.freenames, dom.freeaddress, dom.accountcount, dom.created_at, dom.updated_at FROM domain dom LEFT JOIN domainadmin da ON dom.id = da.domain_id WHERE da.admin_id = :admin_id;"));
        q.bindValue(QStringLiteral(":admin_id"), QVariant::fromValue<quint32>(user.id().toULong()));
    }


    if (Q_LIKELY(q.exec())) {
        lst.reserve(q.size());
        while (q.next()) {
            Domain dom(q.value(0).value<quint32>(),
                       QUrl::fromAce(q.value(1).toByteArray()),
                       q.value(2).toString(),
                       q.value(3).toString(),
                       q.value(4).value<quint32>(),
                       q.value(5).value<quint32>(),
                       q.value(6).value<quint32>(),
                       q.value(7).value<quint32>(),
                       q.value(8).toBool(),
                       q.value(9).toBool(),
                       QVector<Folder>(),
                       q.value(10).value<quint32>(),
                       Utils::toUserTZ(c, q.value(11).toDateTime()),
                       Utils::toUserTZ(c, q.value(12).toDateTime()));
            dom.setHumanQuota(Utils::humanBinarySize(c, (quint64)dom.getQuota() * 1024));
            dom.setHumanDomainQuota(Utils::humanBinarySize(c, (quint64)dom.getDomainQuota() * 1024));
            dom.setHumanDomainQuotaUsed(Utils::humanBinarySize(c, (quint64)dom.getDomainQuotaUsed() * 1024));
            lst.push_back(dom);
        }
    } else {
        errorData->setSqlError(q.lastError());
    }

    return lst;
}


bool Domain::remove(Cutelyst::Context *c, Domain *domain, SkaffariError *error)
{
    bool ret = false;

    Q_ASSERT_X(error, "remove domain", "invalid errorData object");
    Q_ASSERT_X(c, "remove domain", "invalid Cutelyst context");
    Q_ASSERT_X(domain, "remove domain", "invalid domain");

    if (!Account::remove(c, error, domain)) {
        return ret;
    }

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM domainadmin WHERE domain_id = :domain_id"));
    q.bindValue(QStringLiteral(":domain_id"), domain->id());

    if (Q_UNLIKELY(!q.exec())) {
        error->setSqlError(q.lastError(), c->translate("Domain", "Failed to remove domain to admin connections from database."));
        return ret;
    }

    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM folder WHERE domain_id = :domain_id"));
    q.bindValue(QStringLiteral(":domain_id"), domain->id());

    if (Q_UNLIKELY(!q.exec())) {
        error->setSqlError(q.lastError(), c->translate("Domain", "Failed to remove domain default folders from database."));
        return ret;
    }

    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM domain WHERE id = :id"));
    q.bindValue(QStringLiteral(":id"), domain->id());

    if (!q.exec()) {
        error->setSqlError(q.lastError(), c->translate("Domain", "Failed to remove domain %1 from database.").arg(domain->getName()));
        return ret;
    }

    ret = true;

    return ret;
}



bool Domain::update(Cutelyst::Context *c, const Cutelyst::ParamsMultiMap &p, SkaffariError *e, Domain *d, const Cutelyst::AuthenticationUser &u)
{
    bool ret = false;

    Q_ASSERT_X(c, "update domain", "invalid context object");
    Q_ASSERT_X(!p.empty(), "update domain", "empty parameters");
    Q_ASSERT_X(e, "update domain", "invalid error object");
    Q_ASSERT_X(d && d->isValid(), "update domain", "invalid domain object");
    Q_ASSERT_X(!u.isNull(), "update domain", "invalid user");

    QSqlQuery q;

    const quint32 quota = p.value(QStringLiteral("quota"), QString::number(d->getQuota())).toULong();
    const QStringList folders = Domain::trimStringList(p.value(QStringLiteral("folders")).split(QLatin1Char(','), QString::SkipEmptyParts));
    const QDateTime currentTimeUtc = QDateTime::currentDateTimeUtc();

    if (u.value(QStringLiteral("type")).value<qint16>() == 0) {

        const quint32 maxAccounts = p.value(QStringLiteral("maxAccounts"), QString::number(d->getMaxAccounts())).toULong();
        const quint32 domainQuota = p.value(QStringLiteral("domainQuota"), QString::number(d->getDomainQuota())).toULong();
        const bool freeNames = p.contains(QStringLiteral("freeNames"));
        const bool freeAddress = p.contains(QStringLiteral("freeAddress"));
        const QString transport = p.value(QStringLiteral("transport"), d->getTransport());

        q = CPreparedSqlQueryThread(QStringLiteral("UPDATE domain SET maxaccounts = :maxaccounts, quota = :quota, domainquota = :domainquota, freenames = :freenames, freeaddress = :freeaddress, transport = :transport, updated_at = :updated_at WHERE id = :id"));
        q.bindValue(QStringLiteral(":maxaccounts"), maxAccounts);
        q.bindValue(QStringLiteral(":quota"), quota);
        q.bindValue(QStringLiteral(":domainquota"), domainQuota);
        q.bindValue(QStringLiteral(":freenames"), freeNames);
        q.bindValue(QStringLiteral(":freeaddress"), freeAddress);
        q.bindValue(QStringLiteral(":transport"), transport);
        q.bindValue(QStringLiteral(":updated_at"), currentTimeUtc);
        q.bindValue(QStringLiteral(":id"), d->id());

        if (!q.exec()) {
            e->setSqlError(q.lastError(), c->translate("Domain", "Failed to update domain %1 in database.").arg(d->getName()));
            return ret;
        }

        d->setQuota(quota);
        d->setMaxAccounts(maxAccounts);
        d->setDomainQuota(domainQuota);
        d->setFreeNamesEnabled(freeNames);
        d->setFreeAddressEnabled(freeAddress);
        d->setTransport(transport);
        d->setUpdated(Utils::toUserTZ(c, currentTimeUtc));

        ret = true;

    } else if (u.value(QStringLiteral("domains")).value<QVariantList>().contains(d->id())) {

        q = CPreparedSqlQueryThread(QStringLiteral("UPDATE domain SET quota = :quota, updated_at = :updated_at WHERE id = :id"));
        q.bindValue(QStringLiteral(":quota"), quota);
        q.bindValue(QStringLiteral(":updated_at"), currentTimeUtc);
        q.bindValue(QStringLiteral(":id"), d->id());

        if (!q.exec()) {
            e->setSqlError(q.lastError(), c->translate("Domain", "Failed to update domain %1 in database.").arg(d->getName()));
            return ret;
        }

        d->setQuota(quota);
        d->setUpdated(Utils::toUserTZ(c, currentTimeUtc));

        ret = true;
    } else {
        e->setErrorType(SkaffariError::AutorizationError);
        e->setErrorText(c->translate("Domain", "You are not authorized to update this domain."));
        return ret;
    }

    if (folders.empty()) {
        q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM folder WHERE domain_id = :domain_id"));
        q.bindValue(QStringLiteral(":domain_id"), d->id());
        if (Q_UNLIKELY(!q.exec())) {
            qCCritical(SK_DOMAIN) << "Failed to delete folder from domain" << d->getName() << "from database:" << q.lastError().text();
        }
        d->setFolders(QVector<Folder>());
    } else {
        QStringList newFolders;
        QStringList currentFolderNames;
        QList<quint32> deletedFolders;
        const QVector<Folder> currentFolders = d->getFolders();
        for (const Folder &folder : currentFolders) {
            currentFolderNames << folder.getName();
            if (!folders.contains(folder.getName())) {
                deletedFolders << folder.getId();
            }
        }

        for (const QString &folder : folders) {
            if (!currentFolderNames.contains(folder)) {
                newFolders << folder;
            }
        }

        if (!deletedFolders.empty()) {
            const QList<quint32> deletedFoldersConst = deletedFolders;
            for (quint32 fid : deletedFoldersConst) {
                q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM folder WHERE id = :id"));
                q.bindValue(QStringLiteral(":id"), fid);
                if (Q_UNLIKELY(!q.exec())) {
                    qCCritical(SK_DOMAIN) << "Failed to delete default folders for domain" << d->getName() << "from database:" << q.lastError().text();
                }
            }
        }

        if (!newFolders.empty()) {
            const QStringList newFoldersConst = newFolders;
            for (const QString &folder : newFoldersConst) {
                q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO folder (domain_id, name) VALUES (:domain_id, :name)"));
                q.bindValue(QStringLiteral(":domain_id"), d->id());
                q.bindValue(QStringLiteral(":name"), folder);
                if (Q_UNLIKELY(!q.exec())) {
                    qCCritical(SK_DOMAIN) << "Failed to add new default folder for domain" << d->getName() << "to database:" << q.lastError().text();
                }
            }
        }

        QVector<Folder> foldersVect;
        q = CPreparedSqlQueryThread(QStringLiteral("SELECT id, name FROM folder WHERE domain_id = :domain_id"));
        q.bindValue(QStringLiteral(":domain_id"), d->id());
        if (Q_UNLIKELY(!q.exec())) {
            qCCritical(SK_DOMAIN) << "Failed to query default folders for domain" << d->getName() << "from database:" << q.lastError().text();
        } else {
            while (q.next()) {
                foldersVect.append(Folder(q.value(0).value<quint32>(), d->id(), q.value(1).toString()));
            }
        }
        d->setFolders(foldersVect);
    }

    return ret;
}


void Domain::toStash(Cutelyst::Context *c, quint32 domainId)
{
    Q_ASSERT_X(c, "domain to stash", "invalid context object");

    SkaffariError e(c);
    Domain d = Domain::get(c, domainId, &e);
    if (Q_LIKELY(d.isValid())) {
        c->stash({
                     {QStringLiteral("domain"), QVariant::fromValue<Domain>(d)},
                     {QStringLiteral("site_title"), d.getName()}
                 });
    } else {
        c->stash({
                     {QStringLiteral("template"), QStringLiteral("404.html")},
                     {QStringLiteral("site_title"), c->translate("Domain", "Not found")},
                     {QStringLiteral("not_found_text"), c->translate("Domain", "There is no domain with database ID %1.").arg(domainId)}
                 });
        c->res()->setStatus(404);
    }
}


Domain Domain::fromStash(Cutelyst::Context *c)
{
    Domain d;

    d = c->stash(QStringLiteral("domain")).value<Domain>();

    return d;
}


bool Domain::checkAccess(Cutelyst::Context *c, quint32 domainId)
{
    bool allowed = false;

    Cutelyst::AuthenticationUser user = Cutelyst::Authentication::user(c);

    if (user.value(QStringLiteral("type")).value<qint16>() == 0) {
        allowed = true;
    } else if (domainId > 0) {
        allowed = (user.value(QStringLiteral("domains")).value<QVariantList>().contains(domainId));
    }

    if (!allowed) {
        c->stash({
                     {QStringLiteral("template"), QStringLiteral("403.html")},
                     {QStringLiteral("site_title"), c->translate("Domain", "Access denied")}
                 });
        c->res()->setStatus(403);
    }

    return allowed;
}


bool Domain::accessGranted(Cutelyst::Context *c)
{
    return ((c->res()->status() != 404) && (c->res()->status() != 403));
}


QStringList Domain::trimStringList(const QStringList &list)
{
    QStringList tlist;

    if (!list.empty()) {

        for (const QString &str : list) {
            tlist << str.trimmed();
        }

    }

    return tlist;
}
