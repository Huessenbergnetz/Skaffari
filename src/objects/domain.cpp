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
#include "../utils/skaffariconfig.h"
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
#include <algorithm>
#include <vector>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

Q_LOGGING_CATEGORY(SK_DOMAIN, "skaffari.domain")

#define DOMAIN_STASH_KEY "domain"

Domain::Domain() : d(new DomainData)
{

}

Domain::Domain(dbid_t id,
               const QString& name,
               const QString& prefix,
               const QString& transport,
               quota_size_t quota,
               quint32 maxAccounts,
               quota_size_t domainQuota,
               quota_size_t domainQuotaUsed,
               bool freeNames,
               bool freeAddress,
               const QVector<Folder> &folders,
               quint32 accounts,
               const QDateTime &created,
               const QDateTime &updated) :
    d(new DomainData(id, name, prefix, transport, quota, maxAccounts, domainQuota, domainQuotaUsed, freeNames, freeAddress, folders, accounts, created, updated))
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


dbid_t Domain::id() const
{
    return d->id;
}

void Domain::setId(dbid_t id)
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


quota_size_t Domain::getQuota() const
{
    return d->quota;
}


void Domain::setQuota(quota_size_t nQuota)
{
    d->quota = nQuota;
}


quint32 Domain::getMaxAccounts() const
{
    return d->maxAccounts;
}


void Domain::setMaxAccounts(quint32 nMaxAccounts)
{
    d->maxAccounts = nMaxAccounts;
}


quota_size_t Domain::getDomainQuota() const
{
    return d->domainQuota;
}


void Domain::setDomainQuota(quota_size_t nDomainQuota)
{
    d->domainQuota = nDomainQuota;
}


quota_size_t Domain::getDomainQuotaUsed() const
{
    return d->domainQuotaUsed;
}


void Domain::setDomainQuotaUsed(quota_size_t nDomainQuotaUsed)
{
    d->domainQuotaUsed = nDomainQuotaUsed;
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
    return (!d->name.isEmpty() && !d->prefix.isEmpty() && (d->id > 0));
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

SimpleDomain Domain::parent() const
{
    return d->parent;
}

void Domain::setParent(const SimpleDomain &parent)
{
    d->parent = parent;
}

QVector<SimpleDomain> Domain::children() const
{
    return d->children;
}

void Domain::setChildren(const QVector<SimpleDomain> &children)
{
    d->children = children;
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

        const dbid_t uid = c->stash(QStringLiteral("userId")).value<dbid_t>();

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


Domain Domain::create(Cutelyst::Context *c, const Cutelyst::ParamsMultiMap &params, SkaffariError *errorData)
{
    Domain dom;

    Q_ASSERT_X(errorData, "create new domain", "invalid errorData object");

    const QString domainName = params.value(QStringLiteral("domainName")).trimmed().toLower();
    const QString prefix = !SkaffariConfig::imapDomainasprefix() ? params.value(QStringLiteral("prefix")).trimmed().toLower() : domainName;

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT domain_name FROM domain WHERE prefix = :prefix"));
    q.bindValue(QStringLiteral(":prefix"), prefix);

    if (Q_UNLIKELY(!q.exec())) {
        errorData->setSqlError(q.lastError(), c->translate("Domain", "Failed to check if prefix is alredy in use."));
        qCCritical(SK_DOMAIN, "Failed to check if prefix %s for new domain %s is already in use: %s", qUtf8Printable(prefix), qUtf8Printable(domainName), qUtf8Printable(q.lastError().text()));
        return dom;
    }

    if (Q_UNLIKELY(q.next())) {
        errorData->setErrorType(SkaffariError::InputError);
        errorData->setErrorText(c->translate("Domain", "The prefix “%1” is already in use by another domain.").arg(prefix));
        qCWarning(SK_DOMAIN, "Failed to create domain %s: prefix %s is already in use by %s", qUtf8Printable(domainName), qUtf8Printable(prefix), qUtf8Printable(q.value(0).toString()));
        return dom;
    }

    q = CPreparedSqlQueryThread(QStringLiteral("SELECT id FROM domain WHERE domain_name = :domain_name"));
    q.bindValue(QStringLiteral(":domain_name"), QUrl::toAce(domainName));

    if (Q_UNLIKELY(!q.exec())) {
        errorData->setSqlError(q.lastError(), c->translate("Domain", "Failed to check if domain name is already in use."));
        qCCritical(SK_DOMAIN, "Failed to check if new domain %s already exists in database: %s", qUtf8Printable(domainName), qUtf8Printable(q.lastError().text()));
        return dom;
    }

    if (Q_UNLIKELY(q.next())) {
        errorData->setErrorType(SkaffariError::InputError);
        errorData->setErrorText(c->translate("Domain", "The domain name “%1” is already in use.").arg(domainName));
        qCWarning(SK_DOMAIN, "Failed to create domain %s: name is already in use by domain ID %u", domainName.toUtf8().constData(), q.value(0).value<dbid_t>());
    }

    quota_size_t quota = 0;
    bool quotaOk = true;
    if (params.contains(QStringLiteral("humanQuota"))) {
        quota = Utils::humanToIntSize(c, params.value(QStringLiteral("humanQuota")), &quotaOk);
        if (!quotaOk) {
            errorData->setErrorType(SkaffariError::InputError);
            errorData->setErrorText(c->translate("Domain", "Failed to convert human readable quota size string into valid integer value."));
            return dom;
        }
    } else {
        quota = params.value(QStringLiteral("quota")).toULongLong(&quotaOk);
        if (!quotaOk) {
            errorData->setErrorType(SkaffariError::InputError);
            errorData->setErrorText(c->translate("Domain", "Failed to parse quota string into integer value."));
            return dom;
        }
    }

    quota_size_t domainQuota = 0;
    bool domainQuotaOk = true;
    if (params.contains(QStringLiteral("humanDomainQuota"))) {
        domainQuota = Utils::humanToIntSize(c, params.value(QStringLiteral("humanDomainQuota")), &domainQuotaOk);
        if (!domainQuotaOk) {
            errorData->setErrorType(SkaffariError::InputError);
            errorData->setErrorText(c->translate("Domain", "Failed to convert human readable quota size string into valid integer value."));
            return dom;
        }
    } else {
        domainQuota = params.value(QStringLiteral("domainQuota")).toULongLong(&domainQuotaOk);
        if (!domainQuotaOk) {
            errorData->setErrorType(SkaffariError::InputError);
            errorData->setErrorText(c->translate("Domain", "Failed to parse quota string into integer value."));
            return dom;
        }
    }

    const quint32 maxAccounts = params.value(QStringLiteral("maxAccounts")).toULong();
    const bool freeNames = params.contains(QStringLiteral("freeNames"));
    const bool freeAddress = params.contains(QStringLiteral("freeAddress"));
    const QStringList folders = Domain::trimStringList(params.value(QStringLiteral("folders")).split(QLatin1Char(','), QString::SkipEmptyParts));
    const QString transport = params.value(QStringLiteral("transport"), QStringLiteral("cyrus"));
    const dbid_t parentId = params.value(QStringLiteral("parent"), QStringLiteral("0")).toULong();
    const QDateTime currentTimeUtc = QDateTime::currentDateTimeUtc();

    q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO domain (parent_id, domain_name, prefix, maxaccounts, quota, domainquota, freenames, freeaddress, transport, created_at, updated_at) "
                                                         "VALUES (:parent_id, :domain_name, :prefix, :maxaccounts, :quota, :domainquota, :freenames, :freeaddress, :transport, :created_at, :updated_at)"));
    q.bindValue(QStringLiteral(":parent_id"), parentId);
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
        const dbid_t domainId = q.lastInsertId().value<dbid_t>();
        QVector<Folder> foldersVect;
        for (const QString &folder : folders) {
            q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO folder (domain_id, name) VALUES (:domain_id, :name)"));
            q.bindValue(QStringLiteral(":domain_id"), domainId);
            q.bindValue(QStringLiteral(":name"), folder);
            if (Q_LIKELY(q.exec())) {
                const dbid_t folderId = q.lastInsertId().value<dbid_t>();
                foldersVect.push_back(Folder(folderId, domainId, folder));
            } else {
                errorData->setSqlError(q.lastError());
                qCCritical(SK_DOMAIN, "Failed to create folder %s for new domain %s in database: %s", qUtf8Printable(folder), qUtf8Printable(domainName), qUtf8Printable(q.lastError().text()));
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

        if (parentId > 0) {
            dom.setParent(SimpleDomain::get(c, errorData, parentId));
            if (Q_UNLIKELY(!dom.parent())) {
                qCWarning(SK_DOMAIN, "Can not find parent domain with ID %u for new domain %s (ID %u).", parentId, qUtf8Printable(domainName), domainId);
            }
        }

    } else {
        errorData->setSqlError(q.lastError());
        qCCritical(SK_DOMAIN, "Failed to insert new domain %s into database: %s", qUtf8Printable(domainName), qUtf8Printable(q.lastError().text()));
    }

    if (dom.isValid()) {
        qCInfo(SK_DOMAIN, "%s created domain %s (ID: %u)", qUtf8Printable(c->stash(QStringLiteral("userName")).toString()), qUtf8Printable(domainName), dom.id());
    }

    return dom;
}


Domain Domain::get(Cutelyst::Context *c, dbid_t domId, SkaffariError *errorData)
{
    Domain dom;

    Q_ASSERT_X(errorData, "get domain", "invalid errorData object");
    Q_ASSERT_X(c, "get domain", "invalid Cutelyst context");

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT dom.prefix, dom.domain_name, dom.transport, dom.quota, dom.maxaccounts, dom.domainquota, dom.domainquotaused, dom.freenames, dom.freeaddress, dom.accountcount, dom.created_at, dom.updated_at, dom.parent_id FROM domain dom WHERE dom.id = :id"));
    q.bindValue(QStringLiteral(":id"), domId);

    dbid_t parentId = 0;

    if (Q_LIKELY(q.exec() && q.next())) {

        dom.setId(domId);
        dom.setPrefix(q.value(0).toString());
        dom.setName(QUrl::fromAce(q.value(1).toByteArray()));
        dom.setTransport(q.value(2).toString());
        dom.setQuota(q.value(3).value<quota_size_t>());
        dom.setMaxAccounts(q.value(4).value<quint32>());
        dom.setDomainQuota(q.value(5).value<quota_size_t>());
        dom.setDomainQuotaUsed(q.value(6).value<quota_size_t>());
        dom.setFreeNamesEnabled(q.value(7).toBool());
        dom.setFreeAddressEnabled(q.value(8).toBool());
        dom.setAccounts(q.value(9).value<quint32>());
        dom.setCreated(Utils::toUserTZ(c, q.value(10).toDateTime()));
        dom.setUpdated(Utils::toUserTZ(c, q.value(11).toDateTime()));

        parentId = q.value(12).value<dbid_t>();

        q = CPreparedSqlQueryThread(QStringLiteral("SELECT id, name FROM folder WHERE domain_id = :domain_id"));
        q.bindValue(QStringLiteral(":domain_id"), domId);
        if (Q_LIKELY(q.exec())) {
            QVector<Folder> folders;
            while (q.next()) {
                folders.push_back(Folder(q.value(0).value<dbid_t>(), domId, q.value(1).toString()));
            }
            dom.setFolders(folders);
        } else {
            qCCritical(SK_DOMAIN) << "Failed to query default folders for domain" << dom.getName() << "fom database:" << q.lastError().text();
        }

    } else {
        if (q.lastError().type() != QSqlError::NoError) {
            errorData->setSqlError(q.lastError());
            qCCritical(SK_DOMAIN, "Failed to query database for domain with ID %u: %s", domId, qUtf8Printable(q.lastError().text()));
        } else {
            errorData->setErrorType(SkaffariError::ApplicationError);
            errorData->setErrorText(c->translate("Domain", "The domain with ID %1 could not be found in the database.").arg(domId));
            qCWarning(SK_DOMAIN, "Domain ID %u not found in database.", domId);
        }
    }

    if (dom.isValid()) {
        if (parentId > 0) {
            dom.setParent(SimpleDomain::get(c, errorData, parentId));
            if (!dom.parent()) {
                qWarning(SK_DOMAIN, "Can not find parent domain with ID %u for domain %s (ID: %u).", parentId, qUtf8Printable(dom.getName()), dom.id());
            }
        } else {
            q = CPreparedSqlQueryThread(QStringLiteral("SELECT id, domain_name FROM domain WHERE parent_id = :id"));
            q.bindValue(QStringLiteral(":id"), domId);

            if (Q_LIKELY(q.exec())) {
                QVector<SimpleDomain> thekids;
                thekids.reserve(q.size());
                while(q.next()) {
                    thekids.push_back(SimpleDomain(q.value(0).value<dbid_t>(), QUrl::fromAce(q.value(1).toByteArray())));
                }
                dom.setChildren(thekids);
            } else {
                qCCritical(SK_DOMAIN, "Failed to query child domains for domain %s (ID %u): %s", qUtf8Printable(dom.getName()), dom.id(), qUtf8Printable(q.lastError().text()));
            }
        }

        q = CPreparedSqlQueryThread(QStringLiteral("SELECT a.id, a.username FROM domainadmin da JOIN adminuser a ON a.id = da.admin_id WHERE da.domain_id = :domain_id"));
        q.bindValue(QStringLiteral(":domain_id"), dom.id());

        if (Q_LIKELY(q.exec())) {
            QVector<SimpleAdmin> admins;
            admins.reserve(q.size());
            while (q.next()) {
                admins.push_back(SimpleAdmin(q.value(0).value<dbid_t>(), q.value(1).toString()));
            }
            dom.setAdmins(admins);
        } else {
            qCCritical(SK_DOMAIN, "Failed to query domain administrators for domain %s from database: %s", qUtf8Printable(dom.getName()), qUtf8Printable(q.lastError().text()));
            errorData->setSqlError(q.lastError());
        }
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
        q = CPreparedSqlQueryThread(QStringLiteral("SELECT dom.id, dom.domain_name, dom.prefix, dom.transport, dom.quota, dom.maxaccounts, dom.domainquota, dom.domainquotaused, dom.freenames, dom.freeaddress, dom.accountcount, dom.created_at, dom.updated_at, dom.parent_id FROM domain dom ORDER BY dom.domain_name ASC"));
    } else {
        q = CPreparedSqlQueryThread(QStringLiteral("SELECT dom.id, dom.domain_name, dom.prefix, dom.transport, dom.quota, dom.maxaccounts, dom.domainquota, dom.domainquotaused, dom.freenames, dom.freeaddress, dom.accountcount, dom.created_at, dom.updated_at, dom.parent_id FROM domain dom LEFT JOIN domainadmin da ON dom.id = da.domain_id WHERE da.admin_id = :admin_id ORDER BY dom.domain_name ASC"));
        q.bindValue(QStringLiteral(":admin_id"), QVariant::fromValue<dbid_t>(user.id().toULong()));
    }

    if (Q_LIKELY(q.exec())) {
        lst.reserve(q.size());
        while (q.next()) {
            Domain dom(q.value(0).value<dbid_t>(),
                       QUrl::fromAce(q.value(1).toByteArray()),
                       q.value(2).toString(),
                       q.value(3).toString(),
                       q.value(4).value<quota_size_t>(),
                       q.value(5).value<quint32>(),
                       q.value(6).value<quota_size_t>(),
                       q.value(7).value<quota_size_t>(),
                       q.value(8).toBool(),
                       q.value(9).toBool(),
                       QVector<Folder>(),
                       q.value(10).value<quint32>(),
                       Utils::toUserTZ(c, q.value(11).toDateTime()),
                       Utils::toUserTZ(c, q.value(12).toDateTime()));

            const dbid_t parentId = q.value(13).value<dbid_t>();
            if (parentId > 0) {
                dom.setParent(SimpleDomain::get(c, errorData, parentId));
                if (Q_UNLIKELY(!dom.parent())) {
                    qWarning(SK_DOMAIN, "Can not find parent domain with ID %u for domain %s (ID: %u).", parentId, qUtf8Printable(dom.getName()), dom.id());
                }
            } else {
                QSqlQuery q2 = CPreparedSqlQueryThread(QStringLiteral("SELECT id, domain_name FROM domain WHERE parent_id = :id"));
                q2.bindValue(QStringLiteral(":id"), dom.id());

                if (Q_LIKELY(q2.exec())) {
                    QVector<SimpleDomain> thekids;
                    thekids.reserve(q2.size());
                    while(q2.next()) {
                        thekids.push_back(SimpleDomain(q2.value(0).value<dbid_t>(), QUrl::fromAce(q2.value(1).toByteArray())));
                    }
                    dom.setChildren(thekids);
                } else {
                    qCCritical(SK_DOMAIN, "Failed to query child domains for domain %s (ID %u): %s", qUtf8Printable(dom.getName()), dom.id(), qUtf8Printable(q2.lastError().text()));
                }
            }

            lst.push_back(dom);
        }
    } else {
        errorData->setSqlError(q.lastError());
        qCCritical(SK_DOMAIN) << "Failed to query domain list from database:" << q.lastError().text();
    }

    if (lst.size() > 1) {
        DomainNameCollator dnc(c->locale());
        std::sort(lst.begin(), lst.end(), dnc);
    }

    return lst;
}


bool Domain::isAvailable(const QString &domainName)
{
    bool available = false;

    Q_ASSERT_X(!domainName.isEmpty(), "check if domain is available", "empty domain name");

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT id FROM domain WHERE domain_name = :domain_name"));
    q.bindValue(QStringLiteral(":domain_name"), QUrl::toAce(domainName));

    if (Q_UNLIKELY(!q.exec())) {
        qCCritical(SK_DOMAIN, "Failed to check availability of domain name %s: %s", qUtf8Printable(domainName), qUtf8Printable(q.lastError().text()));
    } else {
        available = q.next();
    }

    return available;
}


bool Domain::remove(Cutelyst::Context *c, Domain *domain, SkaffariError *error, dbid_t newParentId, bool deleteChildren)
{
    bool ret = false;

    Q_ASSERT_X(error, "remove domain", "invalid errorData object");
    Q_ASSERT_X(c, "remove domain", "invalid Cutelyst context");
    Q_ASSERT_X(domain, "remove domain", "invalid domain");

    if (!Account::remove(c, error, domain)) {
        qCCritical(SK_DOMAIN, "Failed to remove domain %s", domain->getName().toUtf8().constData());
        return ret;
    }

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM domainadmin WHERE domain_id = :domain_id"));
    q.bindValue(QStringLiteral(":domain_id"), domain->id());

    if (Q_UNLIKELY(!q.exec())) {
        error->setSqlError(q.lastError(), c->translate("Domain", "Failed to remove domain to admin connections from database."));
        qCCritical(SK_DOMAIN, "Failed to remove domain to admin connections for domain %s from database: %s. Abort removing domain.", qUtf8Printable(domain->getName()), qUtf8Printable(q.lastError().text()));
        return ret;
    }

    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM folder WHERE domain_id = :domain_id"));
    q.bindValue(QStringLiteral(":domain_id"), domain->id());

    if (Q_UNLIKELY(!q.exec())) {
        error->setSqlError(q.lastError(), c->translate("Domain", "Failed to remove domain default folders from database."));
        qCCritical(SK_DOMAIN, "Failed to remove default folders for domain %s from database: %s. Abort removing domain.", domain->getName().toUtf8().constData(), q.lastError().text().toUtf8().constData());
        return ret;
    }

    if (deleteChildren) {
        if (!domain->children().empty()) {
            const QVector<SimpleDomain> thekids = domain->children();
            for (const SimpleDomain &kid : thekids) {
                Domain child = Domain::get(c, kid.id(), error);
                if (child) {
                    if (Q_UNLIKELY(!Domain::remove(c, &child, error, 0, true))) {
                        qCCritical(SK_DOMAIN, "Failed to remove child domains of domain %s (ID %u).", qUtf8Printable(domain->getName()), domain->id());
                        return ret;
                    }
                }
            }
        }
    } else {
        q = CPreparedSqlQueryThread(QStringLiteral("UPDATE domain SET parent_id = :new_parent_id WHERE parent_id = :old_parent_id"));
        q.bindValue(QStringLiteral(":new_parent_id"), newParentId);
        q.bindValue(QStringLiteral(":old_parent_id"), domain->id());

        if (Q_UNLIKELY(!q.exec())) {
            error->setSqlError(q.lastError(), c->translate("Domain", "Failed to set new parent domain for child domains."));
            qCCritical(SK_DOMAIN, "Failed to set new parent domain ID %u while removing domain %s (ID: %u): %s. Abort removing domain.", newParentId, qUtf8Printable(domain->getName()), domain->id(), qUtf8Printable(q.lastError().text()));
            return ret;
        }
    }

    if (Q_UNLIKELY(!q.exec(QStringLiteral("DELETE FROM virtual WHERE alias LIKE '%@%1'").arg(QString::fromLatin1(QUrl::toAce(domain->getName())))))) {
        error->setSqlError(q.lastError(), c->translate("Domain", "Failed to remove email addresses from database."));
        qCCritical(SK_DOMAIN, "Failed to remove email addresses for domain %s (ID: %u) from database: %s", qUtf8Printable(domain->getName()), domain->id(), qUtf8Printable(q.lastError().text()));
        return ret;
    }


    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM domain WHERE id = :id"));
    q.bindValue(QStringLiteral(":id"), domain->id());

    if (Q_UNLIKELY(!q.exec())) {
        error->setSqlError(q.lastError(), c->translate("Domain", "Failed to remove domain %1 from database.").arg(domain->getName()));
        qCCritical(SK_DOMAIN, "Failed to remove domain %s from database: %s", domain->getName().toUtf8().constData(), q.lastError().text().toUtf8().constData());
        return ret;
    }

    ret = true;

    qCInfo(SK_DOMAIN, "%s removed domain %s (ID: %u)", c->stash(QStringLiteral("userName")).toString().toUtf8().constData(), domain->getName().toUtf8().constData(), domain->id());

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

    const QStringList folders = Domain::trimStringList(p.value(QStringLiteral("folders")).split(QLatin1Char(','), QString::SkipEmptyParts));
    const QDateTime currentTimeUtc = QDateTime::currentDateTimeUtc();


    quota_size_t quota = 0;
    bool quotaOk = true;
    if (p.contains(QStringLiteral("humanQuota"))) {
        quota = Utils::humanToIntSize(c, p.value(QStringLiteral("humanQuota")), &quotaOk);
        if (!quotaOk) {
            e->setErrorType(SkaffariError::InputError);
            e->setErrorText(c->translate("Domain", "Failed to convert human readable quota size string into valid integer value."));
            qCCritical(SK_DOMAIN, "Failed to convert human readable quota size \"%s\" into valid integer value.", qUtf8Printable(p.value(QStringLiteral("humanQuota"))));
            return ret;
        }
    } else {
        quota = p.value(QStringLiteral("quota"), QString::number(d->getQuota())).toULongLong(&quotaOk);
        if (!quotaOk) {
            e->setErrorType(SkaffariError::InputError);
            e->setErrorText(c->translate("Domain", "Failed to parse quota string into integer value."));
            qCCritical(SK_DOMAIN, "Failed to parse quota string \"%s\" into integer value.", qUtf8Printable(p.value(QStringLiteral("quota"))));
            return ret;
        }
    }

    if (u.value(QStringLiteral("type")).value<qint16>() == 0) {

        quota_size_t domainQuota = 0;
        bool domainQuotaOk = true;
        if (p.contains(QStringLiteral("humanDomainQuota"))) {
            domainQuota = Utils::humanToIntSize(c, p.value(QStringLiteral("humanDomainQuota")), &domainQuotaOk);
            if (!domainQuotaOk) {
                e->setErrorType(SkaffariError::InputError);
                e->setErrorText(c->translate("Domain", "Failed to convert human readable quota size string into valid integer value."));
                qCCritical(SK_DOMAIN, "Failed to convert human readable quota size string \"%s\" into valid integer value.", qUtf8Printable(p.value(QStringLiteral("humanDomainQuota"))));
                return ret;
            }
        } else {
            domainQuota = p.value(QStringLiteral("domainQuota"), QString::number(d->getDomainQuota())).toULongLong(&domainQuotaOk);
            if (!domainQuotaOk) {
                e->setErrorType(SkaffariError::InputError);
                e->setErrorText(c->translate("Domain", "Failed to parse quota string into integer value."));
                qCCritical(SK_DOMAIN, "Failed to parse quota string \"%s\" into integer value.", qUtf8Printable(p.value(QStringLiteral("domainQuota"))));
                return ret;
            }
        }

        const dbid_t parentId = p.value(QStringLiteral("parent"), QStringLiteral("0")).toULong();
        if (parentId == d->id()) {
            e->setErrorType(SkaffariError::InputError);
            e->setErrorText(c->translate("Domain", "You can not set a domain as its own parent domain."));
            qCCritical(SK_DOMAIN, "Tried to set domain ID %u as parent for domain ID %u.", parentId, parentId);
            return ret;
        }

        const quint32 maxAccounts = p.value(QStringLiteral("maxAccounts"), QString::number(d->getMaxAccounts())).toULong();
        const bool freeNames = p.contains(QStringLiteral("freeNames"));
        const bool freeAddress = p.contains(QStringLiteral("freeAddress"));
        const QString transport = p.value(QStringLiteral("transport"), d->getTransport());

        q = CPreparedSqlQueryThread(QStringLiteral("UPDATE domain SET maxaccounts = :maxaccounts, quota = :quota, domainquota = :domainquota, freenames = :freenames, freeaddress = :freeaddress, transport = :transport, updated_at = :updated_at, parent_id = :parent_id WHERE id = :id"));
        q.bindValue(QStringLiteral(":maxaccounts"), maxAccounts);
        q.bindValue(QStringLiteral(":quota"), quota);
        q.bindValue(QStringLiteral(":domainquota"), domainQuota);
        q.bindValue(QStringLiteral(":freenames"), freeNames);
        q.bindValue(QStringLiteral(":freeaddress"), freeAddress);
        q.bindValue(QStringLiteral(":transport"), transport);
        q.bindValue(QStringLiteral(":updated_at"), currentTimeUtc);
        q.bindValue(QStringLiteral(":parent_id"), parentId);
        q.bindValue(QStringLiteral(":id"), d->id());

        if (!q.exec()) {
            e->setSqlError(q.lastError(), c->translate("Domain", "Failed to update domain %1 in database.").arg(d->getName()));
            qCCritical(SK_DOMAIN, "Failed to update domain %s in database: %s", d->getName().toUtf8().constData(), q.lastError().text().toUtf8().constData());
            return ret;
        }

        d->setQuota(quota);
        d->setMaxAccounts(maxAccounts);
        d->setDomainQuota(domainQuota);
        d->setFreeNamesEnabled(freeNames);
        d->setFreeAddressEnabled(freeAddress);
        d->setTransport(transport);
        d->setUpdated(Utils::toUserTZ(c, currentTimeUtc));

        if (parentId != d->parent().id()) {
            if (parentId > 0) {
                d->setParent(SimpleDomain::get(c, e, parentId));
                if (Q_UNLIKELY(!d->parent())) {
                    qWarning(SK_DOMAIN, "Can not find parent domain with ID %u for domain %s (ID: %u).", parentId, qUtf8Printable(d->getName()), d->id());
                }
            } else {
                d->setParent(SimpleDomain());
            }
        }

        ret = true;

    } else if (u.value(QStringLiteral("domains")).value<QVariantList>().contains(d->id())) {

        q = CPreparedSqlQueryThread(QStringLiteral("UPDATE domain SET quota = :quota, updated_at = :updated_at WHERE id = :id"));
        q.bindValue(QStringLiteral(":quota"), quota);
        q.bindValue(QStringLiteral(":updated_at"), currentTimeUtc);
        q.bindValue(QStringLiteral(":id"), d->id());

        if (!q.exec()) {
            e->setSqlError(q.lastError(), c->translate("Domain", "Failed to update domain %1 in database.").arg(d->getName()));
            qCCritical(SK_DOMAIN, "Failed to update domain %s in database: %s", d->getName().toUtf8().constData(), q.lastError().text().toUtf8().constData());
            return ret;
        }

        d->setQuota(quota);
        d->setUpdated(Utils::toUserTZ(c, currentTimeUtc));

        ret = true;
    } else {
        e->setErrorType(SkaffariError::AutorizationError);
        e->setErrorText(c->translate("Domain", "You are not authorized to update this domain."));
        qCWarning(SK_DOMAIN, "Access denied: %s tried to update domain %s (ID: %u)", c->stash(QStringLiteral("userName")).toString().toUtf8().constData(), d->getName().toUtf8().constData(), d->id());
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
        QList<dbid_t> deletedFolders;
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
            const QList<dbid_t> deletedFoldersConst = deletedFolders;
            for (dbid_t fid : deletedFoldersConst) {
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
                foldersVect.push_back(Folder(q.value(0).value<dbid_t>(), d->id(), q.value(1).toString()));
            }
        }
        d->setFolders(foldersVect);
    }

    if (ret) {
        qCInfo(SK_DOMAIN) << c->stash(QStringLiteral("userName")).toString() << "updated domain" << d->getName();
    }

    return ret;
}


void Domain::toStash(Cutelyst::Context *c, dbid_t domainId)
{
    Q_ASSERT_X(c, "domain to stash", "invalid context object");

    SkaffariError e(c);
    Domain d = Domain::get(c, domainId, &e);
    if (Q_LIKELY(d.isValid())) {
        c->stash({
                     {QStringLiteral(DOMAIN_STASH_KEY), QVariant::fromValue<Domain>(d)},
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

    d = c->stash(QStringLiteral(DOMAIN_STASH_KEY)).value<Domain>();

    return d;
}


bool Domain::checkAccess(Cutelyst::Context *c, dbid_t domainId)
{
    bool allowed = false;

    const Cutelyst::AuthenticationUser user = Cutelyst::Authentication::user(c);

    if (user.value(QStringLiteral("type")).value<qint16>() == 0) {
        // this is a super administrator, access granted for all domains
        allowed = true;
    } else if (domainId > 0) {
        // this is a domain administrator, access granted only for connected domains
        allowed = (user.value(QStringLiteral("domains")).value<QVariantList>().contains(domainId));
    }

    if (!allowed) {
        if (c->req()->header(QStringLiteral("Accept")).contains(QLatin1String("application/json"), Qt::CaseInsensitive)) {
            QJsonObject json({
                                 {QStringLiteral("error_msg"), c->translate("Domain", "You are not authorized to access this resource or to perform this action.")}
                             });
            c->res()->setJsonBody(QJsonDocument(json));
        } else {
            c->stash({
                         {QStringLiteral("template"), QStringLiteral("403.html")},
                         {QStringLiteral("site_title"), c->translate("Domain", "Access denied")}
                     });
        }
        c->res()->setStatus(403);
        qCWarning(SK_DOMAIN, "Access denied: %s tried to access domain with ID: %u)", c->stash(QStringLiteral("userName")).toString().toUtf8().constData(), domainId);
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


QString Domain::getCatchAllAccount(Cutelyst::Context *c, SkaffariError *e) const
{
    QString username;

    Q_ASSERT_X(c, "get catch all account", "invalid context object");
    Q_ASSERT_X(e, "get catch all account", "invalid error object");

    const QString catchAllAlias = QLatin1Char('@') + QString::fromLatin1(QUrl::toAce(getName()));

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT username FROM virtual WHERE alias = :alias"));
    q.bindValue(QStringLiteral(":alias"), catchAllAlias);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Domain", "Failed to get catch all account for this domain."));
        qCCritical(SK_DOMAIN, "Failed to get catch all account for domain ID %u: %s", id(), qUtf8Printable(q.lastError().text()));
        return username;
    }

    if (q.next()) {
        username = q.value(0).toString();
    }

    return username;
}
