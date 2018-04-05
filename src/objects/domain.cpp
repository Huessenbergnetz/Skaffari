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
#include "adminaccount.h"
#include "../utils/utils.h"
#include "../utils/skaffariconfig.h"
#include <Cutelyst/ParamsMultiMap>
#include <Cutelyst/Response>
#include <Cutelyst/Context>
#include <Cutelyst/Plugins/Utils/validatoremail.h>
#include <Cutelyst/Plugins/Utils/Sql>
#include <Cutelyst/Plugins/Authentication/authentication.h>
#include <Cutelyst/Plugins/Authentication/authenticationuser.h>
#include <Cutelyst/Plugins/Session/Session>
#include <QUrl>
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QTimeZone>
#include <algorithm>
#include <vector>

Q_LOGGING_CATEGORY(SK_DOMAIN, "skaffari.domain")

#define DOMAIN_STASH_KEY "domain"

Domain::Domain() : d(new DomainData)
{

}

Domain::Domain(dbid_t id,
               dbid_t aceId,
               const QString& name,
               const QString& prefix,
               const QString& transport,
               quota_size_t quota,
               quint32 maxAccounts,
               quota_size_t domainQuota,
               quota_size_t domainQuotaUsed,
               bool freeNames,
               bool freeAddress,
               quint32 accounts,
               const QDateTime &created,
               const QDateTime &updated,
               const QDateTime &validUntil,
               const SimpleDomain &parent,
               const std::vector<SimpleDomain> &children,
               const std::vector<SimpleAdmin> &admins,
               const std::vector<Folder> &folders) :
    d(new DomainData(id, aceId, name, prefix, transport, quota, maxAccounts, domainQuota, domainQuotaUsed, freeNames, freeAddress, accounts, created, updated, validUntil, parent, children, admins, folders))
{

}

Domain::Domain(const Domain& other) :
    d(other.d)
{

}

Domain::Domain(Domain &&other) noexcept :
    d(std::move(other.d))
{
    other.d = nullptr;
}

Domain& Domain::operator=(const Domain& other)
{
    d = other.d;
    return *this;
}

Domain& Domain::operator=(Domain &&other) noexcept
{
    swap(other);
    return *this;
}

Domain::~Domain()
{

}

void Domain::swap(Domain &other) noexcept
{
    std::swap(d, other.d);
}

dbid_t Domain::id() const
{
    return d->id;
}

QString Domain::name() const
{
    return d->name;
}

QString Domain::aceName() const
{
    return QString::fromLatin1(QUrl::toAce(d->name));
}

QString Domain::nameIdString() const
{
    QString ret;
    ret = d->name + QLatin1String(" (ID: ") + QString::number(d->id) + QLatin1Char(')');
    return ret;
}

QString Domain::prefix() const
{
    return d->prefix;
}

QString Domain::transport() const
{
    return d->transport;
}

quota_size_t Domain::quota() const
{
    return d->quota;
}

quint32 Domain::maxAccounts() const
{
    return d->maxAccounts;
}

quota_size_t Domain::domainQuota() const
{
    return d->domainQuota;
}

quota_size_t Domain::domainQuotaUsed() const
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

bool Domain::isFreeAddressEnabled() const
{
    return d->freeAddress;
}

std::vector<Folder> Domain::folders() const
{
    return d->folders;
}

quint32 Domain::accounts() const
{
    return d->accounts;
}

std::vector<SimpleAdmin> Domain::admins() const
{
    return d->admins;
}

bool Domain::isValid() const
{
    return ((d->id > 0) && !d->name.isEmpty() && !d->prefix.isEmpty());
}

QDateTime Domain::created() const
{
    return d->created;
}

QDateTime Domain::updated() const
{
    return d->updated;
}

QDateTime Domain::validUntil() const
{
    return d->validUntil;
}

SimpleDomain Domain::parent() const
{
    return d->parent;
}

std::vector<SimpleDomain> Domain::children() const
{
    return d->children;
}

bool Domain::isIdn() const
{
    return (d->ace_id > 0);
}

dbid_t Domain::aceId() const
{
    return d->ace_id;
}

float Domain::domainQuotaUsagePercent() const
{
    if ((d->domainQuota == 0) && (d->domainQuotaUsed == 0)) {
        return 0.0;
    }

    return ((float)d->domainQuotaUsed / (float)d->domainQuota) * 100.0;
}

float Domain::accountUsagePercent() const
{
    if ((d->accounts == 0) || (d->maxAccounts == 0)) {
        return 0.0;
    }

    return ((float)d->accounts / (float)d->maxAccounts) * 100.0;
}

bool Domain::hasAccess(Cutelyst::Context *c) const
{
    bool ret = false;
    Q_ASSERT_X(c, "cheking domain access", "invalid context object");

    const AdminAccount aa = AdminAccount::getUser(c);
    if (aa.isValid()) {
        if (aa.type() >= AdminAccount::Administrator) {
            ret = true;
        } else {

            const dbid_t uid = aa.id();

            if (!d->admins.empty()) {
                for (std::vector<SimpleAdmin>::size_type i = 0; i < d->admins.size(); ++i) {
                    if (d->admins.at(i).id() == uid) {
                        ret = true;
                        break;
                    }
                }
            }
        }
    }

    return ret;
}

SimpleDomain Domain::toSimple() const
{
    return SimpleDomain(d->id, d->name);
}

Domain Domain::create(Cutelyst::Context *c, const QVariantHash &params, SkaffariError *errorData)
{
    Domain dom;

    Q_ASSERT_X(errorData, "create new domain", "invalid errorData object");

    const QString domainNameAce = params.value(QStringLiteral("domainName")).toString().toLower();
    const QString domainName = QUrl::fromAce(domainNameAce.toLatin1());
    const QString prefix = !SkaffariConfig::imapDomainasprefix() ? params.value(QStringLiteral("prefix")).toString().toLower() : domainName;
    const quota_size_t quota = params.value(QStringLiteral("quota")).value<quota_size_t>() / Q_UINT64_C(1024);
    const quota_size_t domainQuota = params.value(QStringLiteral("domainQuota")).value<quota_size_t>() / Q_UINT64_C(1024);
    const quint32 maxAccounts = params.value(QStringLiteral("maxAccounts")).value<quint32>();
    const bool freeNames = params.value(QStringLiteral("freeNames")).toBool();
    const bool freeAddress = params.value(QStringLiteral("freeAddress")).toBool();
    const QStringList folders = params.value(QStringLiteral("folders")).toString().trimmed().split(QLatin1Char(','), QString::SkipEmptyParts);
    const QString transport = params.value(QStringLiteral("transport"), QStringLiteral("cyrus")).toString();
    const dbid_t parentId = params.value(QStringLiteral("parent")).value<dbid_t>();
    const QDateTime defDateTime(QDate(2999, 12, 31), QTime(0, 0), QTimeZone::utc());
    const QDateTime validUntil = params.value(QStringLiteral("validUntil"), defDateTime).toDateTime().toUTC();
    const QDateTime currentTimeUtc = QDateTime::currentDateTimeUtc();

    // for logging
    const QString errStr = AdminAccount::getUserNameIdString(c) + QLatin1String(" failed to create new domain ") + domainName;
    const QByteArray errBa = errStr.toUtf8();
    const char *err = errBa.constData();

    QSqlDatabase db = QSqlDatabase::database(Cutelyst::Sql::databaseNameThread());

    if (Q_UNLIKELY(!db.isOpen())) {
        errorData->setSqlError(db.lastError(), c->translate("Domain", "Failed to insert new domain into database."));
        qCCritical(SK_DOMAIN, "%s: can not establish database connection: %s", err, qUtf8Printable(db.lastError().text()));
        return dom;
    }

    if (Q_UNLIKELY(!db.transaction())) {
        errorData->setSqlError(db.lastError(), c->translate("Domain", "Failed to insert new domain into database."));
        qCCritical(SK_DOMAIN, "%s: can not initiate database transaction: %s", err, qUtf8Printable(db.lastError().text()));
        return dom;
    }

    QSqlQuery q(db);

    if (Q_UNLIKELY(!q.prepare(QStringLiteral("INSERT INTO domain (parent_id, domain_name, prefix, maxaccounts, quota, domainquota, freenames, freeaddress, transport, created_at, updated_at, valid_until, idn_id, ace_id) "
                                             "VALUES (:parent_id, :domain_name, :prefix, :maxaccounts, :quota, :domainquota, :freenames, :freeaddress, :transport, :created_at, :updated_at, :valid_until, 0, 0)")))) {
        errorData->setSqlError(q.lastError(), c->translate("Domain", "Failed to create new domain in database."));
        qCCritical(SK_DOMAIN, "%s: can not prepare database query to insert data for new domain: %s", err, qUtf8Printable(q.lastError().text()));
        return dom;
    }

    q.bindValue(QStringLiteral(":parent_id"), parentId);
    q.bindValue(QStringLiteral(":domain_name"), domainName);
    q.bindValue(QStringLiteral(":prefix"), prefix);
    q.bindValue(QStringLiteral(":maxaccounts"), maxAccounts);
    q.bindValue(QStringLiteral(":quota"), quota);
    q.bindValue(QStringLiteral(":domainquota"), domainQuota);
    q.bindValue(QStringLiteral(":freenames"), freeNames);
    q.bindValue(QStringLiteral(":freeaddress"), freeAddress);
    q.bindValue(QStringLiteral(":transport"), transport);
    q.bindValue(QStringLiteral(":created_at"), currentTimeUtc);
    q.bindValue(QStringLiteral(":updated_at"), currentTimeUtc);
    q.bindValue(QStringLiteral(":valid_until"), validUntil);

    if (Q_UNLIKELY(!q.exec())) {
        errorData->setSqlError(q.lastError(), c->translate("Domain", "Failed to insert new domain into database."));
        qCCritical(SK_DOMAIN, "%s: can not execute database query to insert data for new domain: %s", err, qUtf8Printable(q.lastError().text()));
        db.rollback();
        return dom;
    }

    const dbid_t domainId = q.lastInsertId().value<dbid_t>();

    std::vector<Folder> foldersVect;
    if (!folders.empty()) {
        if (Q_UNLIKELY(!q.prepare(QStringLiteral("INSERT INTO folder (domain_id, name) VALUES (:domain_id, :name)")))) {
            errorData->setSqlError(q.lastError(), c->translate("Domain", "Failed to insert default folders for new domain into database."));
            qCCritical(SK_DOMAIN, "%s: can not prepare database query to insert default folders for new domain: %s", err, qUtf8Printable(q.lastError().text()));
            db.rollback();
            return dom;
        }

        foldersVect.reserve(folders.size());

        for (const QString &folder : folders) {
            const QString _folder = folder.trimmed();
            if (Q_LIKELY(!_folder.isEmpty())) {
                q.bindValue(QStringLiteral(":domain_id"), domainId);
                q.bindValue(QStringLiteral(":name"), folder);
                if (Q_LIKELY(q.exec())) {
                    const dbid_t folderId = q.lastInsertId().value<dbid_t>();
                    foldersVect.emplace_back(folderId, domainId, folder);
                } else {
                    errorData->setSqlError(q.lastError(), c->translate("Domain", "Failed to insert default folders for new domain into database."));
                    qCCritical(SK_DOMAIN, "%s: can not execute database query to insert default folder for new domain: %s", err, qUtf8Printable(q.lastError().text()));
                    db.rollback();
                    return dom;
                }
            }
        }
    }

    dbid_t domainAceId = 0;
    if (domainName != domainNameAce) {
        if (Q_UNLIKELY(!q.prepare(QStringLiteral("INSERT INTO domain (idn_id, domain_name, prefix, maxaccounts, quota, created_at, updated_at, valid_until) "
                                                 "VALUES (:idn_id, :domain_name, :prefix, 0, 0, :created_at, :updated_at, :valid_until)")))) {
            errorData->setSqlError(q.lastError(), c->translate("Domain", "Failed to insert ACE version of new domain into database."));
            qCCritical(SK_DOMAIN, "%s: can not prepare database query to insert ACE version for new domain: %s", err, qUtf8Printable(q.lastError().text()));
            db.rollback();
            return dom;
        }

        q.bindValue(QStringLiteral(":idn_id"), domainId);
        q.bindValue(QStringLiteral(":domain_name"), domainNameAce);
        const QString acePrefix = QLatin1String("xn--") + prefix;
        q.bindValue(QStringLiteral(":prefix"), acePrefix);
        q.bindValue(QStringLiteral(":created_at"), currentTimeUtc);
        q.bindValue(QStringLiteral(":updated_at"), currentTimeUtc);
        q.bindValue(QStringLiteral(":valid_until"), validUntil);

        if (Q_UNLIKELY(!q.exec())) {
            errorData->setSqlError(q.lastError(), c->translate("Domain", "Failed to insert ACE version of new domain into database."));
            qCCritical(SK_DOMAIN, "%s: can not execute database query to insert ACE version for new domain: %s", err, qUtf8Printable(q.lastError().text()));
            db.rollback();
            return dom;
        }

        domainAceId = q.lastInsertId().value<dbid_t>();

        if (Q_UNLIKELY(!q.prepare(QStringLiteral("UPDATE domain SET ace_id = :ace_id WHERE id = :id")))) {
            errorData->setSqlError(q.lastError(), c->translate("Domain", "Failed to update ACE id of new domain in database."));
            qCCritical(SK_DOMAIN, "%s: can not prepare database query to update ACE ID for new domain: %s", err, qUtf8Printable(q.lastError().text()));
            db.rollback();
            return dom;
        }

        q.bindValue(QStringLiteral(":ace_id"), domainAceId);
        q.bindValue(QStringLiteral(":id"), domainId);

        if (Q_UNLIKELY(!q.exec())) {
            errorData->setSqlError(q.lastError(), c->translate("Domain", "Failed to update ACE id of new domain in database."));
            qCCritical(SK_DOMAIN, "%s: can not execute database query to update ACE ID for new domain: %s", err, qUtf8Printable(q.lastError().text()));
            db.rollback();
            return dom;
        }
    }

    const SimpleDomain parent = (parentId > 0) ? SimpleDomain::get(c, errorData, parentId) : SimpleDomain();

    if (Q_UNLIKELY(errorData->type() != SkaffariError::NoError)) {
        qCCritical(SK_DOMAIN, "%s: can not find parent domain with ID %lu for new domain.", err);
        db.rollback();
        return dom;
    }

    static const QHash<QString,QString> roleAccounts({
                                                         {QStringLiteral("abuseAccount"), QStringLiteral("abuse")},
                                                         {QStringLiteral("nocAccount"), QStringLiteral("noc")},
                                                         {QStringLiteral("securityAccount"), QStringLiteral("security")},
                                                         {QStringLiteral("postmasterAccount"), QStringLiteral("postmaster")},
                                                         {QStringLiteral("hostmasterAccount"), QStringLiteral("hostmaster")},
                                                         {QStringLiteral("webmasterAccount"), QStringLiteral("webmaster")}
                                                     });

    QHash<dbid_t,QString> addedRoleAddresses;

    auto it = roleAccounts.constBegin();
    auto end = roleAccounts.constEnd();
    while (it != end) {
        dbid_t roleAccId = params.value(it.key(), 0).value<dbid_t>();
        if (roleAccId > 0) {
            auto roleAcc = Account::get(c, errorData, roleAccId);
            if (Q_UNLIKELY(errorData->type() != SkaffariError::NoError)) {
                qCCritical(SK_DOMAIN, "%s: can not find account with ID %lu to use as %s account for new domain %s.", err, roleAccId, qUtf8Printable(it.value()), qUtf8Printable(domainName));
                db.rollback();
                return dom;
            }

            const QVariantHash roleAccParams({
                                                 {QStringLiteral("newlocalpart"), it.value()},
                                                 {QStringLiteral("newmaildomain"), domainId}
                                             });
            const QString addedRoleAddress = roleAcc.addEmail(c, errorData, roleAccParams);

            if (Q_UNLIKELY(errorData->type() != SkaffariError::NoError)) {
                qCCritical(SK_DOMAIN, "%s: can not create role account email address for new domain.", err);
                db.rollback();
                auto addedRolesIt = addedRoleAddresses.constBegin();
                auto addedRolesEnd = addedRoleAddresses.constEnd();
                while (addedRolesIt != addedRolesEnd) {
                    auto delRoleAcc = (addedRolesIt.key() == roleAcc.id()) ? roleAcc : Account::get(c, errorData, addedRolesIt.key());
                    delRoleAcc.removeEmail(c, errorData, addedRolesIt.value());
                    ++addedRolesIt;
                }
                errorData->setErrorType(SkaffariError::SqlError);
                errorData->setErrorText(c->translate("Domain", "Failed to insert role account email addresses into databaes."));
                return dom;
            }

            addedRoleAddresses.insert(roleAccId, addedRoleAddress);
        }
        ++it;
    }

    if (Q_UNLIKELY(!db.commit())) {
        errorData->setSqlError(db.lastError(), c->translate("Domain", "Failed to insert new domain into database."));
        qCCritical(SK_DOMAIN, "%s: can not commit database transaction to add new domain into database: %s", err, qUtf8Printable(db.lastError().text()));
        db.rollback();
        return dom;
    }

    dom = Domain(domainId, domainAceId, domainName, prefix, transport, quota, maxAccounts, domainQuota, 0, freeNames, freeAddress, 0, currentTimeUtc, currentTimeUtc, validUntil, parent, std::vector<SimpleDomain>(), std::vector<SimpleAdmin>(), foldersVect);

    qCInfo(SK_DOMAIN, "%s created new domain %s.", qUtf8Printable(AdminAccount::getUserNameIdString(c)), qUtf8Printable(dom.nameIdString()));
    qCDebug(SK_DOMAIN) << dom;

    return dom;
}

Domain Domain::get(Cutelyst::Context *c, dbid_t domId, SkaffariError *errorData)
{
    Domain dom;

    Q_ASSERT_X(errorData, "get domain", "invalid errorData object");
    Q_ASSERT_X(c, "get domain", "invalid Cutelyst context");

    // for logging
    const QString errStr = AdminAccount::getUserNameIdString(c) + QLatin1String(" failed to get domain with ID ") + QString::number(domId) + QLatin1String(" from the database");
    const QByteArray errBa = errStr.toUtf8();
    const char *err = errBa.constData();

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT parent_id, ace_id, domain_name, prefix, transport, quota, maxaccounts, domainquota, domainquotaused, freenames, freeaddress, accountcount, created_at, updated_at, valid_until FROM domain WHERE id = :id"));
    q.bindValue(QStringLiteral(":id"), domId);

    if (Q_LIKELY(q.exec() && q.next())) {

        QSqlQuery fq = CPreparedSqlQueryThread(QStringLiteral("SELECT id, name FROM folder WHERE domain_id = :domain_id"));
        fq.bindValue(QStringLiteral(":domain_id"), domId);

        if (Q_LIKELY(fq.exec())) {
            std::vector<Folder> defFolders;
            defFolders.reserve(fq.size());
            while (fq.next()) {
                defFolders.emplace_back(fq.value(0).value<dbid_t>(), domId, fq.value(1).toString());
            }

            QSqlQuery aq = CPreparedSqlQueryThread(QStringLiteral("SELECT a.id, a.username FROM domainadmin da JOIN adminuser a ON a.id = da.admin_id WHERE da.domain_id = :domain_id"));
            aq.bindValue(QStringLiteral(":domain_id"), domId);

            if (Q_LIKELY(aq.exec())) {
                std::vector<SimpleAdmin> admins;
                admins.reserve(aq.size());
                while (aq.next()) {
                    admins.emplace_back(aq.value(0).value<dbid_t>(), aq.value(1).toString());
                }

                const dbid_t parentId = q.value(0).value<dbid_t>();
                SimpleDomain parentDom;
                std::vector<SimpleDomain> children;
                if (parentId > 0) {
                    parentDom = SimpleDomain::get(c, errorData, parentId);
                    if (!parentDom.isValid()) {
                        if (errorData->type() == SkaffariError::NotFound) {
                            errorData->setErrorText(c->translate("Domain", "Can not find parent domain with ID %1.").arg(parentId));
                            qCCritical(SK_DOMAIN, "%s: can not find parent domain with ID %lu.", err, parentId);
                        } else {
                            errorData->setSqlError(errorData->qSqlError(), c->translate("Domain", "Failed to query parent domain from database."));
                            qCCritical(SK_DOMAIN, "%s: failed to query database for parent domain.", err);
                        }
                    }
                } else {
                    QSqlQuery cq = CPreparedSqlQueryThread(QStringLiteral("SELECT id, domain_name FROM domain WHERE parent_id = :id"));
                    cq.bindValue(QStringLiteral(":id"), domId);
                    if (Q_UNLIKELY(!cq.exec())) {
                        errorData->setSqlError(cq.lastError(), c->translate("Domain", "Failed to query child domains from the database."));
                        qCCritical(SK_DOMAIN, "%s: failed to execute database query to get child domains: %s", err, qUtf8Printable(cq.lastError().text()));
                    }
                }

                if (errorData->type() == SkaffariError::NoError) {
                    QDateTime createdTime = q.value(12).toDateTime();
                    createdTime.setTimeSpec(Qt::UTC);
                    QDateTime updatedTime = q.value(13).toDateTime();
                    updatedTime.setTimeSpec(Qt::UTC);
                    QDateTime validUntilTime = q.value(14).toDateTime();
                    validUntilTime.setTimeSpec(Qt::UTC);

                    dom = Domain(domId,
                                 q.value(1).value<dbid_t>(),
                                 q.value(2).toString(),
                                 q.value(3).toString(),
                                 q.value(4).toString(),
                                 q.value(5).value<quota_size_t>(),
                                 q.value(6).value<quint32>(),
                                 q.value(7).value<quota_size_t>(),
                                 q.value(8).value<quota_size_t>(),
                                 q.value(9).toBool(),
                                 q.value(10).toBool(),
                                 q.value(11).value<quint32>(),
                                 createdTime,
                                 updatedTime,
                                 validUntilTime,
                                 parentDom,
                                 children,
                                 admins,
                                 defFolders);
                }

            } else {
                errorData->setSqlError(aq.lastError(), c->translate("Domain", "Failed to query responsible administrators from the database."));
                qCCritical(SK_DOMAIN, "%s: failed to execute database query to get responsible admins: %s", err, qUtf8Printable(aq.lastError().text()));
            }

        } else {

            errorData->setSqlError(fq.lastError(), c->translate("Domain", "Failed to query default folders from the database."));
            qCCritical(SK_DOMAIN, "%s: failed to execute dabase query to get default folders: %s", err, qUtf8Printable(fq.lastError().text()));

        }

    } else {
        if (q.lastError().type() != QSqlError::NoError) {
            errorData->setSqlError(q.lastError());
            qCCritical(SK_DOMAIN, "Failed to query database for domain with ID %u: %s", domId, qUtf8Printable(q.lastError().text()));
        } else {
            errorData->setErrorType(SkaffariError::NotFound);
            errorData->setErrorText(c->translate("Domain", "The domain with ID %1 could not be found in the database.").arg(domId));
            qCWarning(SK_DOMAIN, "Domain ID %u not found in database.", domId);
        }
    }

    return dom;
}

std::vector<Domain> Domain::list(Cutelyst::Context *c, SkaffariError *errorData, const Cutelyst::AuthenticationUser &user, const QString orderBy, const QString sort, quint32 limit)
{
    std::vector<Domain> lst;

    Q_ASSERT_X(errorData, "list domains", "invalid errorData object");
    Q_ASSERT_X(c, "list domains", "invalid Cutelyst context");

    // for logging
    const QString errStr = AdminAccount::getUserNameIdString(c) + QLatin1String(" failed to query domain list from the database");
    const QByteArray errBa = errStr.toUtf8();
    const char *err = errBa.constData();

    QSqlQuery q(QSqlDatabase::database(Cutelyst::Sql::databaseNameThread()));

    QString prepString = QStringLiteral("SELECT dom.id, dom.parent_id, dom.ace_id, dom.domain_name, dom.prefix, dom.transport, dom.quota, dom.maxaccounts, dom.domainquota, dom.domainquotaused, dom.freenames, dom.freeaddress, dom.accountcount, dom.created_at, dom.updated_at, dom.valid_until FROM domain dom");
    const bool isAdmin = AdminAccount::getUserType(user) >= AdminAccount::Administrator;
    if (isAdmin) {
        prepString.append(QStringLiteral(" WHERE dom.idn_id = 0"));
    } else {
        prepString.append(QStringLiteral(" LEFT JOIN domainadmin da ON dom.id = da.domain_id WHERE dom.idn_id = 0 AND da.admin_id = :admin_id"));
    }

    prepString.append(QStringLiteral(" ORDER BY dom.%1 %2").arg(orderBy, sort));

    if (limit > 0) {
        prepString.append(QStringLiteral(" LIMIT %1").arg(limit));
    }
    if (Q_UNLIKELY(!q.prepare(prepString))) {
        errorData->setSqlError(q.lastError(), c->translate("Domain", "Failed to query domain list from database."));
        qCCritical(SK_DOMAIN, "%s: failed to prepare database query: %s", err, qUtf8Printable(q.lastError().text()));
        return lst;
    }
    if (!isAdmin) {
        q.bindValue(QStringLiteral(":admin_id"), user.id());
    }

    if (Q_LIKELY(q.exec())) {
        lst.reserve(static_cast<std::vector<Domain>::size_type>(q.size()));
        while (q.next()) {
            const dbid_t domId = q.value(0).value<dbid_t>();
            const dbid_t parentId = q.value(1).value<dbid_t>();

            SimpleDomain parentDom;
            if (parentId > 0) {
                parentDom = SimpleDomain::get(c, errorData, parentId);
            }

            QDateTime createdTime = q.value(13).toDateTime();
            createdTime.setTimeSpec(Qt::UTC);
            QDateTime updatedTime = q.value(14).toDateTime();
            updatedTime.setTimeSpec(Qt::UTC);
            QDateTime validUntilTime = q.value(15).toDateTime();
            validUntilTime.setTimeSpec(Qt::UTC);

            lst.emplace_back(domId,
                             q.value(2).value<dbid_t>(),
                             q.value(3).toString(),
                             q.value(4).toString(),
                             q.value(5).toString(),
                             q.value(6).value<quota_size_t>(),
                             q.value(7).value<quint32>(),
                             q.value(8).value<quota_size_t>(),
                             q.value(9).value<quota_size_t>(),
                             q.value(10).toBool(),
                             q.value(11).toBool(),
                             q.value(12).value<quint32>(),
                             createdTime,
                             updatedTime,
                             validUntilTime,
                             parentDom,
                             std::vector<SimpleDomain>(),
                             std::vector<SimpleAdmin>(),
                             std::vector<Folder>());
        }
    } else {
        errorData->setSqlError(q.lastError(), c->translate("Domain", "Failed to query domain list from database."));
        qCCritical(SK_DOMAIN, "%s: failed to execute database query: %s", err, qUtf8Printable(q.lastError().text()));
    }

    if ((orderBy == QLatin1String("domain_name")) && lst.size() > 1) {
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

bool Domain::remove(Cutelyst::Context *c, SkaffariError *error, dbid_t newParentId, bool deleteChildren)
{
    bool ret = false;

    Q_ASSERT_X(error, "remove domain", "invalid errorData object");
    Q_ASSERT_X(c, "remove domain", "invalid Cutelyst context");

    // for logging
    const QString errStr = AdminAccount::getUserNameIdString(c) + QLatin1String(" failed to remove domain ") + nameIdString() + QStringLiteral(" from database");
    const QByteArray errBa = errStr.toUtf8();
    const char *err = errBa.constData();

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT id FROM accountuser WHERE domain_id = :domain_id"));
    q.bindValue(QStringLiteral(":domain_id"), d->id);

    if (Q_UNLIKELY(!q.exec())) {
        error->setSqlError(q.lastError(), c->translate("Domain", "Failed to get database IDs of the accounts for this domain."));
        qCCritical(SK_DOMAIN, "%s: failed to execute query to get acount IDs for the domain: %s", err, qUtf8Printable(q.lastError().text()));
        return ret;
    }

    while (q.next() && (error->type() == SkaffariError::NoError)) {
        Account a = Account::get(c, error, q.value(0).value<dbid_t>());
        if (a.isValid()) {
            if (Q_UNLIKELY(!a.remove(c, error))) {
                qCCritical(SK_DOMAIN, "%s: failed to remove account %s.", err, qUtf8Printable(a.nameIdString()));
                return ret;
            }
        }
        d->accounts--;
    }

    if (deleteChildren) {
        auto childrenStart = d->children.cbegin();
        while (childrenStart != d->children.cend()) {
            const SimpleDomain kid = d->children.back();
            Domain child = Domain::get(c, kid.id(), error);
            if (child) {
                if (Q_UNLIKELY(!child.remove(c, error, 0, true))) {
                    qCCritical(SK_DOMAIN, "%s: failed to remove child domain %s.", err, qUtf8Printable(child.nameIdString()));
                    return ret;
                }
            }
            d->children.pop_back();
        }
    }

    QSqlDatabase db = QSqlDatabase::database(Cutelyst::Sql::databaseNameThread());

    if (Q_UNLIKELY(!db.isOpen())) {
        error->setSqlError(db.lastError(), c->translate("Domain", "Failed to remove domain from database."));
        qCCritical(SK_DOMAIN, "%s: can not establish database connection: %s", err, qUtf8Printable(q.lastError().text()));
        return ret;
    }

    if (Q_UNLIKELY(!db.transaction())) {
        error->setSqlError(db.lastError(), c->translate("Domain", "Failed to remove domain from database."));
        qCCritical(SK_DOMAIN, "%s: can not initiate database transaction: %s", err, qUtf8Printable(q.lastError().text()));
        return ret;
    }

    q = QSqlQuery(db);

    if (!deleteChildren) {

        if (Q_UNLIKELY(!q.prepare(QStringLiteral("UPDATE domain SET parent_id = :new_parent_id WHERE parent_id = :old_parent_id")))) {
            error->setSqlError(q.lastError(), c->translate("Domain", "Failed to set new parent domain for child domains."));
            qCCritical(SK_DOMAIN, "%s: can not prepare database query to set new parent domain: %s", err, qUtf8Printable(q.lastError().text()));
            return ret;
        }

        q.bindValue(QStringLiteral(":new_parent_id"), newParentId);
        q.bindValue(QStringLiteral(":old_parent_id"), d->id);

        if (Q_UNLIKELY(!q.exec())) {
            error->setSqlError(q.lastError(), c->translate("Domain", "Failed to set new parent domain for child domains."));
            qCCritical(SK_DOMAIN, "%s: can not execute database query to set new parent domain: %s", err, qUtf8Printable(q.lastError().text()));
            db.rollback();
            return ret;
        }
    }



    if (isIdn()) {
        const QString aceEmailLike = QLatin1String("%@") + aceName();

        if (Q_UNLIKELY(!q.prepare(QStringLiteral("DELETE FROM virtual WHERE alias LIKE :alias")))) {
            error->setSqlError(q.lastError(), c->translate("Domain", "Failed to rmove ACE email addresses from the database."));
            qCCritical(SK_DOMAIN, "%s: can not prepare query to remove ACE email addresses from database: %s", err, qUtf8Printable(q.lastError().text()));
            db.rollback();
            return ret;
        }

        q.bindValue(QStringLiteral(":alias"), aceEmailLike);

        if (Q_UNLIKELY(!q.exec())) {
            error->setSqlError(q.lastError(), c->translate("Domain", "Failed to remove ACE email addresses from the database."));
            qCCritical(SK_DOMAIN, "%s: can not execute query to remove ACE email addresses from database: %s", err, qUtf8Printable(q.lastError().text()));
            db.rollback();
            return ret;
        }

        if (Q_UNLIKELY(!q.prepare(QStringLiteral("DELETE FROM domain WHERE idn_id = :id")))) {
            error->setSqlError(q.lastError(), c->translate("Domain", "Failed to remove ACE domain name from the database."));
            qCCritical(SK_DOMAIN, "%s: can not prepare query to remove ACE domain name from database: %s", err, qUtf8Printable(q.lastError().text()));
            db.rollback();
            return ret;
        }

        q.bindValue(QStringLiteral(":id"), d->id);

        if (Q_UNLIKELY(!q.exec())) {
            error->setSqlError(q.lastError(), c->translate("Domain", "Failed to remove ACE domain name from the database."));
            qCCritical(SK_DOMAIN, "%s: can not execute query to remove ACE domain name from database: %s", err, qUtf8Printable(q.lastError().text()));
            db.rollback();
            return ret;
        }
    }

    const QString emailLike = QLatin1String("%@") + d->name;

    if (Q_UNLIKELY(!q.prepare(QStringLiteral("DELETE FROM virtual WHERE alias LIKE :alias")))) {
        error->setSqlError(q.lastError(), c->translate("Domain", "Failed to remove email addresses from database."));
        qCCritical(SK_DOMAIN, "%s: can not prepare query to remove email addresses from database: %s", err, qUtf8Printable(q.lastError().text()));
        db.rollback();
        return ret;
    }

    q.bindValue(QStringLiteral(":alias"), emailLike);

    if (Q_UNLIKELY(!q.exec())) {
        error->setSqlError(q.lastError(), c->translate("Domain", "Failed to remove email addresses from database."));
        qCCritical(SK_DOMAIN, "%s: can not execute query to remove email addresses from database: %s", err, qUtf8Printable(q.lastError().text()));
        db.rollback();
        return ret;
    }

    if (Q_UNLIKELY(!q.prepare(QStringLiteral("DELETE FROM domain WHERE id = :id")))) {
        error->setSqlError(q.lastError(), c->translate("Domain", "Failed to remove domain from database."));
        qCCritical(SK_DOMAIN, "%s: can not prepare database query: %s", err, qUtf8Printable(q.lastError().text()));
        db.rollback();
        return ret;
    }

    q.bindValue(QStringLiteral(":id"), d->id);

    if (Q_UNLIKELY(!q.exec())) {
        error->setSqlError(q.lastError(), c->translate("Domain", "Failed to remove domain from database."));
        qCCritical(SK_DOMAIN, "%s: can not execute database query: %s", err, qUtf8Printable(q.lastError().text()));
        db.rollback();
        return ret;
    }

    if (Q_UNLIKELY(!db.commit())) {
        error->setSqlError(db.lastError(), c->translate("Domain", "Failed to remove domain from database."));
        qCCritical(SK_DOMAIN, "%s: can not commit database transaction: %s", err, qUtf8Printable(db.lastError().text()));
        db.rollback();
        return ret;
    }

    ret = true;

    qCInfo(SK_DOMAIN, "%s removed domain %s", qUtf8Printable(AdminAccount::getUserNameIdString(c)), qUtf8Printable(nameIdString()));
    qCDebug(SK_DOMAIN) << *this;

    return ret;
}

bool Domain::update(Cutelyst::Context *c, const QVariantHash &p, SkaffariError *e, const Cutelyst::AuthenticationUser &u)
{
    bool ret = false;

    Q_ASSERT_X(c, "update domain", "invalid context object");
    Q_ASSERT_X(!p.empty(), "update domain", "empty parameters");
    Q_ASSERT_X(e, "update domain", "invalid error object");
    Q_ASSERT_X(!u.isNull(), "update domain", "invalid user");

    QSqlQuery q;

    const QStringList folders = p.value(QStringLiteral("folders")).toString().trimmed().split(QLatin1Char(','), QString::SkipEmptyParts);
    const QDateTime currentTimeUtc = QDateTime::currentDateTimeUtc();
    const quota_size_t quota = p.value(QStringLiteral("quota")).value<quota_size_t>() / Q_UINT64_C(1024);


    if (AdminAccount::getUserType(u) >= AdminAccount::Administrator) {

        const quota_size_t domainQuota = p.value(QStringLiteral("domainQuota")).value<quota_size_t>() / Q_UINT64_C(1024);

        const dbid_t parentId = p.value(QStringLiteral("parent"), 0).value<dbid_t>();
        if (parentId == d->id) {
            e->setErrorType(SkaffariError::InputError);
            e->setErrorText(c->translate("Domain", "You can not set a domain as its own parent domain."));
            qCCritical(SK_DOMAIN, "Tried to set domain ID %u as parent for domain ID %u.", parentId, parentId);
            return ret;
        }

        const quint32 maxAccounts = p.value(QStringLiteral("maxAccounts"), d->maxAccounts).value<quint32>();
        const bool freeNames = p.value(QStringLiteral("freeNames"), d->freeNames).toBool();
        const bool freeAddress = p.value(QStringLiteral("freeAddress"), d->freeAddress).toBool();
        const QString transport = p.value(QStringLiteral("transport"), d->transport).toString();
        const QDateTime validUntil = p.value(QStringLiteral("validUntil"), d->validUntil).toDateTime().toUTC();

        q = CPreparedSqlQueryThread(QStringLiteral("UPDATE domain SET maxaccounts = :maxaccounts, quota = :quota, domainquota = :domainquota, freenames = :freenames, freeaddress = :freeaddress, transport = :transport, updated_at = :updated_at, parent_id = :parent_id, valid_until = :valid_until WHERE id = :id"));
        q.bindValue(QStringLiteral(":maxaccounts"), maxAccounts);
        q.bindValue(QStringLiteral(":quota"), quota);
        q.bindValue(QStringLiteral(":domainquota"), domainQuota);
        q.bindValue(QStringLiteral(":freenames"), freeNames);
        q.bindValue(QStringLiteral(":freeaddress"), freeAddress);
        q.bindValue(QStringLiteral(":transport"), transport);
        q.bindValue(QStringLiteral(":updated_at"), currentTimeUtc);
        q.bindValue(QStringLiteral(":parent_id"), parentId);
        q.bindValue(QStringLiteral(":valid_until"), validUntil);
        q.bindValue(QStringLiteral(":id"), d->id);

        if (!q.exec()) {
            e->setSqlError(q.lastError(), c->translate("Domain", "Failed to update domain %1 in database.").arg(d->name));
            qCCritical(SK_DOMAIN, "Failed to update domain %s in database: %s", qUtf8Printable(d->name), qUtf8Printable(q.lastError().text()));
            return ret;
        }

        d->quota = quota;
        d->maxAccounts = maxAccounts;
        d->domainQuota = domainQuota;
        d->freeNames = freeNames;
        d->freeAddress = freeAddress;
        d->transport = transport;
        d->updated = currentTimeUtc;

        if (parentId != d->parent.id()) {
            if (parentId > 0) {
                d->parent = SimpleDomain::get(c, e, parentId);
                if (Q_UNLIKELY(!d->parent)) {
                    qWarning(SK_DOMAIN, "Can not find parent domain with ID %u for domain %s (ID: %u).", parentId, qUtf8Printable(d->name), d->id);
                }
            } else {
                d->parent = SimpleDomain();
            }
        }

        ret = true;

    } else if (u.value(QStringLiteral("domains")).value<QVariantList>().contains(d->id)) {

        q = CPreparedSqlQueryThread(QStringLiteral("UPDATE domain SET quota = :quota, updated_at = :updated_at WHERE id = :id"));
        q.bindValue(QStringLiteral(":quota"), quota);
        q.bindValue(QStringLiteral(":updated_at"), currentTimeUtc);
        q.bindValue(QStringLiteral(":id"), d->id);

        if (!q.exec()) {
            e->setSqlError(q.lastError(), c->translate("Domain", "Failed to update domain %1 in database.").arg(d->name));
            qCCritical(SK_DOMAIN, "Failed to update domain %s in database: %s", qUtf8Printable(d->name), qUtf8Printable(q.lastError().text()));
            return ret;
        }

        d->quota = quota;
        d->updated = currentTimeUtc;

        ret = true;
    } else {
        e->setErrorType(SkaffariError::AuthorizationError);
        e->setErrorText(c->translate("Domain", "You are not authorized to update this domain."));
        qCWarning(SK_DOMAIN, "Access denied: %s tried to update domain %s (ID: %lu)", qUtf8Printable(AdminAccount::getUserNameIdString(c)), qUtf8Printable(d->name), d->id);
        return ret;
    }

    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM folder WHERE domain_id = :domain_id"));
    q.bindValue(QStringLiteral(":domain_id"), d->id);
    if (Q_LIKELY(q.exec())) {
        d->folders.clear();
        if (!folders.empty()) {
            d->folders.reserve(folders.size());
            for (const QString &folder : folders) {
                const QString _folder = folder.trimmed();
                if (Q_LIKELY(!_folder.isEmpty())) {
                    q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO folder (domain_id, name) VALUES (:domain_id, :name)"));
                    q.bindValue(QStringLiteral(":domain_id"), d->id);
                    q.bindValue(QStringLiteral(":name"), folder);
                    if (Q_LIKELY(q.exec())) {
                        d->folders.emplace_back(q.lastInsertId().value<dbid_t>(), d->id, folder);
                    } else {
                        qCCritical(SK_DOMAIN, "Failed to save default folder \"%s\" for domain %s (ID: %lu) to database: %s", qUtf8Printable(folder), qUtf8Printable(d->name), d->id, qUtf8Printable(q.lastError().text()));
                    }
                }
            }
            d->folders.shrink_to_fit();
        }
    } else {
        qCCritical(SK_DOMAIN) << "Failed to delete default folders for domain" << d->name << "from database:" << q.lastError().text();
    }

    if (ret) {
        qCInfo(SK_DOMAIN, "%s updated domain %s (ID: %lu)", qUtf8Printable(AdminAccount::getUserNameIdString(c)), qUtf8Printable(d->name), d->id);
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
                     {QStringLiteral("site_title"), d.name()}
                 });
    } else {
        e.toStash(c);
        c->detach(c->getAction(QStringLiteral("error")));
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

    const AdminAccount user = AdminAccount::getUser(c);
    if (user.isValid()) {
        if (user.type() >= AdminAccount::Administrator) {
            allowed = true;
        } else {
            allowed = user.domains().contains(domainId);
        }
    } else {
        qCWarning(SK_DOMAIN, "Access denied to domain ID %u, invalid AdminAccount object in stash.", domainId);
    }

    if (!allowed) {
        c->res()->setStatus(403);
        qCWarning(SK_DOMAIN, "Access denied: %s tried to access domain with ID %u", qUtf8Printable(user.nameIdString()), domainId);
    }

    return allowed;
}

QString Domain::getCatchAllAccount(Cutelyst::Context *c, SkaffariError *e) const
{
    QString username;

    Q_ASSERT_X(c, "get catch-all account", "invalid context object");
    Q_ASSERT_X(e, "get catch-all account", "invalid error object");

    const QString catchAllAlias = QLatin1Char('@') + QString::fromLatin1(QUrl::toAce(name()));

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT username FROM virtual WHERE alias = :alias"));
    q.bindValue(QStringLiteral(":alias"), catchAllAlias);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Domain", "Failed to get catch-all account for this domain."));
        qCCritical(SK_DOMAIN, "Failed to get catch-all account for domain ID %u: %s", id(), qUtf8Printable(q.lastError().text()));
        return username;
    }

    if (q.next()) {
        username = q.value(0).toString();
    }

    return username;
}

QDebug operator<<(QDebug dbg, const Domain &domain)
{
    QDebugStateSaver saver(dbg);
    Q_UNUSED(saver);
    dbg.nospace() << "Domain(";
    dbg << "ID: " << domain.id();
    dbg << ", Name: " << domain.name();
    dbg << ", Prefix: " << domain.prefix();
    dbg << ", ACE ID: " << domain.aceId();
    if (domain.parent()) {
        dbg << ", Parent: " << domain.parent();
    }
#if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0))
    if (!domain.children().empty()) {
        dbg << ", Children: " << domain.children();
    }
    if (!domain.admins().empty()) {
        dbg << ", Admins: " << domain.admins();
    }
    if (!domain.folders().empty()) {
        dbg << ", Default folders: " << domain.folders();
    }
#else
    if (!domain.children().empty()) {
        dbg << ", Children: std::vector(";
        auto it = domain.folders().cbegin();
        auto end = domain.folders().cend();
        if (it != end) {
            dbg << *it;
            ++it;
        }
        while (it != end) {
            dbg << ", " << *it;
            ++it;
        }
        dbg << ')';
    }
    if (!domain.admins().empty()) {
        dbg << ", Admins: std::vector(";
        auto it = domain.folders().cbegin();
        auto end = domain.folders().cend();
        if (it != end) {
            dbg << *it;
            ++it;
        }
        while (it != end) {
            dbg << ", " << *it;
            ++it;
        }
        dbg << ')';
    }
    if (!domain.folders().empty()) {
        dbg << ", Default folders: std::vector(";
        auto it = domain.folders().cbegin();
        auto end = domain.folders().cend();
        if (it != end) {
            dbg << *it;
            ++it;
        }
        while (it != end) {
            dbg << ", " << *it;
            ++it;
        }
        dbg << ')';
    }
#endif
    dbg << ", Accounts: " << domain.accounts() << '/' << domain.maxAccounts();
    dbg << ", Domain quota: " << domain.domainQuotaUsed() << '/' << domain.domainQuota() << "KiB";
    dbg << ", Default account quota: " << domain.quota() << "KiB";
    dbg << ", Transport: " << domain.transport();
    dbg << ", Free names: " << domain.isFreeNamesEnabled();
    dbg << ", Free addresses: " << domain.isFreeAddressEnabled();
    dbg << ", Created: " << domain.created();
    dbg << ", Updated: " << domain.updated();
    dbg << ", Valid until: " << domain.validUntil();
    dbg << ')';
    return dbg.maybeSpace();
}

QDataStream &operator<<(QDataStream &stream, const Domain &domain)
{
    stream << domain.quota() << domain.domainQuota() << domain.domainQuotaUsed();

    const quint64 childrenCount = static_cast<quint64>(domain.children().size());
    stream << childrenCount;
    if (childrenCount > 0) {
        for (const SimpleDomain &child : domain.children()) {
            stream << child;
        }
    }

    const quint64 adminCount = static_cast<quint64>(domain.admins().size());
    stream << adminCount;
    if (adminCount > 0) {
        for (const SimpleAdmin &admin : domain.admins()) {
            stream << admin;
        }
    }

    const quint64 folderCount = static_cast<quint64>(domain.folders().size());
    stream << folderCount;
    if (folderCount > 0) {
        for (const Folder &folder : domain.folders()) {
            stream << folder;
        }
    }

    stream << domain.parent();
    stream << domain.name() << domain.prefix() << domain.transport();
    stream << domain.created() << domain.updated() << domain.validUntil();
    stream << domain.id() << domain.aceId();
    stream << domain.maxAccounts() << domain.accounts();
    stream << domain.isFreeNamesEnabled() << domain.isFreeAddressEnabled();

    return stream;

    return stream;
}

QDataStream &operator>>(QDataStream &stream, Domain &domain)
{
    stream >> domain.d->quota;
    stream >> domain.d->domainQuota;
    stream >> domain.d->domainQuotaUsed;

    quint64 childrenCount = 0;
    stream >> childrenCount;
    if (childrenCount > 0) {
        domain.d->children.reserve(childrenCount);
        for (quint64 i = 0; i < childrenCount; ++i) {
            SimpleDomain child;
            stream >> child;
            domain.d->children.push_back(child);
        }
    }

    quint64 adminCount = 0;
    stream >> adminCount;
    if (adminCount > 0) {
        domain.d->admins.reserve(adminCount);
        for (quint64 i = 0; i < adminCount; ++i) {
            SimpleAdmin admin;
            stream >> admin;
            domain.d->admins.push_back(admin);
        }
    }

    quint64 folderCount = 0;
    stream >> folderCount;
    if (folderCount > 0) {
        domain.d->folders.reserve(folderCount);
        for (quint64 i = 0; i < folderCount; ++i) {
            Folder folder;
            stream >> folder;
            domain.d->folders.push_back(folder);
        }
    }

    stream >> domain.d->parent;
    stream >> domain.d->name;
    stream >> domain.d->prefix;
    stream >> domain.d->transport;
    stream >> domain.d->created;
    stream >> domain.d->updated;
    stream >> domain.d->validUntil;
    stream >> domain.d->id;
    stream >> domain.d->ace_id;
    stream >> domain.d->maxAccounts;
    stream >> domain.d->accounts;
    stream >> domain.d->freeNames;
    stream >> domain.d->freeAddress;

    return stream;
}
