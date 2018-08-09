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

#include "adminaccount_p.h"
#include "skaffarierror.h"
#include "../utils/utils.h"
#include "../utils/skaffariconfig.h"
#include <Cutelyst/Context>
#include <Cutelyst/Plugins/Utils/Sql>
#include <Cutelyst/Plugins/Authentication/credentialpassword.h>
#include <Cutelyst/Plugins/Authentication/authentication.h>
#include <Cutelyst/Plugins/Session/Session>
#include <Cutelyst/Response>
#include <QSqlQuery>
#include <QSqlError>
#include <QStringList>
#include <QLoggingCategory>
#include <QTimeZone>
#include <QJsonValue>
#include <QDebug>
#include <QMetaEnum>
#include <QLocale>
#include <QDataStream>

Q_LOGGING_CATEGORY(SK_ADMIN, "skaffari.admin")

#define ADMIN_ACCOUNT_STASH_KEY "adminaccount"
#define ADMIN_USER_STASH_KEY "user"

AdminAccount::AdminAccount() :
    d(new AdminAccountData)
{

}

AdminAccount::AdminAccount(dbid_t id, const QString &username, AdminAccountType type, const QList<dbid_t> &domains) :
    d(new AdminAccountData(id, username, type, domains))
{

}

AdminAccount::AdminAccount(dbid_t id, const QString& username, quint8 type, const QList<dbid_t> &domains) :
    d(new AdminAccountData(id, username, type, domains))
{

}

AdminAccount::AdminAccount(const Cutelyst::AuthenticationUser &user) :
    d(new AdminAccountData(user))
{

}

AdminAccount::AdminAccount(dbid_t id, const QString &username, AdminAccountType type, const QList<dbid_t> &domains, const QString &tz, const QString &lang, const QString &tmpl, quint8 maxDisplay, quint8 warnLevel, const QDateTime &created, const QDateTime &updated) :
    d(new AdminAccountData)
{
    d->id = id;
    d->username = username;
    d->type = type;
    d->domains = domains;
    d->tz = tz;
    d->tmpl = tmpl;
    d->lang = lang;
    d->maxDisplay = maxDisplay;
    d->warnLevel = warnLevel;
    d->created = created;
    d->updated = updated;
}

AdminAccount::AdminAccount(const AdminAccount& other) :
    d(other.d)
{

}

AdminAccount::AdminAccount(AdminAccount &&other) noexcept :
    d(std::move(other.d))
{
    other.d = nullptr;
}

AdminAccount& AdminAccount::operator=(const AdminAccount& other)
{
    d = other.d;
    return *this;
}

AdminAccount& AdminAccount::operator=(AdminAccount &&other) noexcept
{
    swap(other);
    return *this;
}

AdminAccount::~AdminAccount()
{

}

void AdminAccount::swap(AdminAccount &other) noexcept
{
    std::swap(d, other.d);
}

dbid_t AdminAccount::id() const
{
    return d->id;
}

QString AdminAccount::username() const
{
    return d->username;
}

QString AdminAccount::nameIdString() const
{
    QString ret;
    ret = d->username + QLatin1String(" (ID: ") + QString::number(d->id) + QLatin1Char(')');
    return ret;
}

AdminAccount::AdminAccountType AdminAccount::type() const
{
    return d->type;
}

quint8 AdminAccount::typeInt() const
{
    return static_cast<quint8>(d->type);
}

QString AdminAccount::typeStr() const
{
    return QString::number(typeInt());
}

QString AdminAccount::typeName(Cutelyst::Context *c) const
{
    return AdminAccount::typeToName(d->type, c);
}

QList<dbid_t> AdminAccount::domains() const
{
    return d->domains;
}

bool AdminAccount::isSuperUser() const
{
    return (d->type == SuperUser);
}

bool AdminAccount::isValid() const
{
    return !d->username.isEmpty() && (d->id > 0);
}

QString AdminAccount::lang() const
{
    return d->lang;
}

QString AdminAccount::tz() const
{
    return d->tz;
}

QDateTime AdminAccount::created() const
{
    return d->created;
}

QDateTime AdminAccount::updated() const
{
    return d->updated;
}

quint8 AdminAccount::maxDisplay() const
{
    return d->maxDisplay;
}

quint8 AdminAccount::warnLevel() const
{
    return d->warnLevel;
}

QString AdminAccount::getTemplate() const
{
    return d->tmpl;
}

QJsonObject AdminAccount::toJson() const
{
    QJsonObject o;

    if (isValid()) {
        o.insert(QStringLiteral("id"), static_cast<qint64>(d->id));
        o.insert(QStringLiteral("username"), d->username);
        o.insert(QStringLiteral("type"), static_cast<int>(d->type));
        o.insert(QStringLiteral("lang"), d->lang);
        o.insert(QStringLiteral("tz"), d->tz);
        o.insert(QStringLiteral("maxDisplay"), static_cast<int>(d->maxDisplay));
        o.insert(QStringLiteral("warnLevel"), static_cast<int>(d->warnLevel));
        o.insert(QStringLiteral("created"), d->created.toString(Qt::ISODate));
        o.insert(QStringLiteral("updated"), d->updated.toString(Qt::ISODate));
    }

    return o;
}

AdminAccount AdminAccount::create(Cutelyst::Context *c, const QVariantHash &params, SkaffariError &error)
{
    AdminAccount aa;

    Q_ASSERT_X(c, "create new adminaccount", "invalid context object");
    Q_ASSERT_X(!params.empty(), "create new adminaccount", "params can not be empty");

    const QString username = params.value(QStringLiteral("username")).toString().trimmed();

    // for logging
    const QString errStr = AdminAccount::getUserName(c) + QLatin1String(" failed to create new admin account ") + username;
    const QByteArray errBa = errStr.toUtf8();
    const char *err = errBa.constData();

    const QVariant typeVar = params.value(QStringLiteral("type"));
    if (!typeVar.canConvert<quint8>()) {
        error.setErrorType(SkaffariError::InputError);
        error.setErrorText(c->translate("AdminAccount", "Invalid administrator type."));
        qCWarning(SK_ADMIN, "%s: invalid administrator type.", err);
        return aa;
    }
    const AdminAccount::AdminAccountType type = AdminAccount::getUserType(typeVar);

    if (type == AdminAccount::Disabled) {
        error.setErrorType(SkaffariError::InputError);
        error.setErrorText(c->translate("AdminAccount", "Invalid administrator type."));
        qCWarning(SK_ADMIN, "%s: invalid administrator type.", err);
        return aa;
    }

    if (type >= AdminAccount::getUserType(c)) {
        error.setErrorType(SkaffariError::AuthorizationError);
        error.setErrorText(c->translate("AdminAccount", "You are not allowed to create users of type %1.").arg(AdminAccount::typeToName(type, c)));
        qCWarning(SK_ADMIN, "%s: not allowed to create users of type %u.", err, type);
        return aa;
    }

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT id FROM adminuser WHERE username = :username"));
    q.bindValue(QStringLiteral(":username"), username);

    if (Q_UNLIKELY(!q.exec())) {
        error.setSqlError(q.lastError(), c->translate("AdminAccount", "Cannot check whether the user name is already assigned."));
        qCCritical(SK_ADMIN, "%s because query to check if username is already in use failed: %s", err, qUtf8Printable(q.lastError().text()));
        return aa;
    }

    if (Q_UNLIKELY(q.next())) {
        error.setErrorType(SkaffariError::InputError);
        error.setErrorText(c->translate("AdminAccount", "This administrator user name is already in use."));
        qCWarning(SK_ADMIN, "%s: username is already in use by ID %u.", err, q.value(0).value<dbid_t>());
        return aa;
    }

    const QByteArray password = Cutelyst::CredentialPassword::createPassword(params.value(QStringLiteral("password")).toString().toUtf8(),
                                                                             SkaffariConfig::admPwAlgorithm(),
                                                                             SkaffariConfig::admPwRounds(),
                                                                             24,
                                                                             27);

    if (Q_UNLIKELY(password.isEmpty())) {
        error.setErrorType(SkaffariError::ApplicationError);
        error.setErrorText(c->translate("AdminAccount", "Password encryption failed. Please check your encryption settings."));
        qCCritical(SK_ADMIN, "%s: password encryption failed.", err);
        return aa;
    }

    const QDateTime currentUtc = QDateTime::currentDateTimeUtc();

    QSqlDatabase db = QSqlDatabase::database(Cutelyst::Sql::databaseNameThread());
    if (Q_UNLIKELY(!db.isOpen())) {
        error.setSqlError(db.lastError());
        qCCritical(SK_ADMIN, "%s: failed to open database connection: %s", err, qUtf8Printable(db.lastError().text()));
        return aa;
    }

    if (Q_UNLIKELY(!db.transaction())) {
        error.setSqlError(db.lastError());
        qCCritical(SK_ADMIN, "%s: failed to start database transaction: %s", err, qUtf8Printable(db.lastError().text()));
        return aa;
    }

    q = QSqlQuery(db);

    if (Q_UNLIKELY(!q.prepare(QStringLiteral("INSERT INTO adminuser (username, password, type, created_at, updated_at) "
                                             "VALUES (:username, :password, :type, :created_at, :updated_at)")))) {
        error.setSqlError(q.lastError());
        qCCritical(SK_ADMIN, "%s: failed to prepare databse query: %s", err, qUtf8Printable(q.lastError().text()));
        return aa;
    }
    q.bindValue(QStringLiteral(":username"), username);
    q.bindValue(QStringLiteral(":password"), password);
    q.bindValue(QStringLiteral(":type"), type);
    q.bindValue(QStringLiteral(":created_at"), currentUtc);
    q.bindValue(QStringLiteral(":updated_at"), currentUtc);

    if (Q_UNLIKELY(!q.exec())) {
        error.setSqlError(q.lastError());
        qCCritical(SK_ADMIN, "%s: failed to execute database query: %s", err, qUtf8Printable(q.lastError().text()));
        db.rollback();
        return aa;
    }

    const dbid_t id = q.lastInsertId().value<dbid_t>();
    if (Q_UNLIKELY(id <= 0)) {
        error.setErrorType(SkaffariError::ApplicationError);
        error.setErrorText(c->translate("AdminAccount", "Faild to insert new administrator data into the database."));
        qCCritical(SK_ADMIN, "%s: failed to insert new administrator data into the database.", err);
        db.rollback();
        return aa;
    }

    if (Q_UNLIKELY(!q.prepare(QStringLiteral("INSERT INTO settings (admin_id, template, maxdisplay, warnlevel, tz, lang) "
                                             "VALUES (:admin_id, :template, :maxdisplay, :warnlevel, :tz, :lang)")))) {
        error.setSqlError(q.lastError());
        qCCritical(SK_ADMIN, "%s: failed to prepare database query: %s", err, qUtf8Printable(q.lastError().text()));
        db.rollback();
        return aa;
    }
    q.bindValue(QStringLiteral(":admin_id"), id);
    q.bindValue(QStringLiteral(":template"), QStringLiteral("default"));
    q.bindValue(QStringLiteral(":maxdisplay"), SkaffariConfig::defMaxdisplay());
    q.bindValue(QStringLiteral(":warnlevel"), SkaffariConfig::defWarnlevel());
    q.bindValue(QStringLiteral(":tz"), SkaffariConfig::defTimezone());
    q.bindValue(QStringLiteral(":lang"), SkaffariConfig::defLanguage());

    if (Q_UNLIKELY(!q.exec())) {
        error.setSqlError(q.lastError());
        qCCritical(SK_ADMIN, "%s: failed to execute database query: %s", err, qUtf8Printable(q.lastError().text()));
        db.rollback();
        return aa;
    }

    QList<dbid_t> domIds;
    if (type < AdminAccount::Administrator) {
        const QStringList assocdoms = params.value(QStringLiteral("assocdomains")).toStringList();
        domIds.reserve(assocdoms.size());
        for (const QString &did : assocdoms) {
            bool ok = false;
            const dbid_t domId = static_cast<dbid_t>(did.toULong(&ok));
            if (ok && domId) {
                domIds << domId;
            }
        }
        if (!domIds.empty()) {
            if (Q_UNLIKELY(!q.prepare(QStringLiteral("INSERT INTO domainadmin (domain_id, admin_id) VALUES (:domain_id, :admin_id)")))) {
                error.setSqlError(q.lastError());
                qCCritical(SK_ADMIN, "%s: failed to prepare database query: %s", err, qUtf8Printable(q.lastError().text()));
                db.rollback();
                return aa;
            }
            auto it = domIds.cbegin();
            auto end = domIds.cend();
            while (it != end) {
                q.bindValue(QStringLiteral(":domain_id"), *it);
                q.bindValue(QStringLiteral(":admin_id"), id);
                if (Q_UNLIKELY(!q.exec())) {
                    error.setSqlError(q.lastError());
                    qCCritical(SK_ADMIN, "%s: failed to execute database query: %s", err, qUtf8Printable(q.lastError().text()));
                    db.rollback();
                    return aa;
                }
                ++it;
            }
        }
    }

    if (Q_UNLIKELY(!db.commit())) {
        error.setSqlError(db.lastError());
        qCCritical(SK_ADMIN, "%s: failed to commit database transaction: %s", err, qUtf8Printable(q.lastError().text()));
        db.rollback();
        return aa;
    }

    aa = AdminAccount(id, username, type, domIds, QString::fromLatin1(SkaffariConfig::defTimezone()), SkaffariConfig::defLanguage(), QStringLiteral("default"), SkaffariConfig::defMaxdisplay(), SkaffariConfig::defWarnlevel(), currentUtc, currentUtc);

    qCInfo(SK_ADMIN, "%s created new admin acccount %s of type %s.", qUtf8Printable(AdminAccount::getUserNameIdString(c)), qUtf8Printable(aa.nameIdString()), AdminAccount::staticMetaObject.enumerator(AdminAccount::staticMetaObject.indexOfEnumerator("AdminAccountType")).valueToKey(type));
    qCDebug(SK_ADMIN) << aa;

    return aa;
}

std::vector<AdminAccount> AdminAccount::list(Cutelyst::Context *c, SkaffariError &error)
{
    std::vector<AdminAccount> list;

    Q_ASSERT_X(c, "list admins", "invalid context object");

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT id, username, type FROM adminuser ORDER BY username ASC"));

    if (Q_UNLIKELY(!q.exec())) {
        error.setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to query list of admins from database."));
        qCCritical(SK_ADMIN, "%s failed to query list of admins from database: %s", qUtf8Printable(AdminAccount::getUserNameIdString(c)), qUtf8Printable(q.lastError().text()));
        return list;
    }

    list.reserve(q.size());

    while (q.next()) {
        list.emplace_back(q.value(0).value<dbid_t>(),
                          q.value(1).toString(),
                          q.value(2).value<quint8>(),
                          QList<dbid_t>());
    }

    return list;
}

AdminAccount AdminAccount::get(Cutelyst::Context *c, SkaffariError &e, dbid_t id)
{
    AdminAccount acc;

    Q_ASSERT_X(c, "get admin", "invalid context object");
    Q_ASSERT_X(id > 0, "get admin", "invalid database id");

    // for logging
    const QByteArray uniBa = AdminAccount::getUserNameIdString(c).toUtf8();
    const char *uniStr = uniBa.constData();

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT a.username, a.type, s.tz, s.lang, s.template, s.maxdisplay, s.warnlevel, a.created_at, a.updated_at FROM adminuser a JOIN settings s ON a.id = s.admin_id WHERE a.id = :id"));
    q.bindValue(QStringLiteral(":id"), id);

    if (Q_UNLIKELY(!q.exec())) {
        e.setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to query administrator account with ID %1 from database.").arg(id));
        qCCritical(SK_ADMIN, "%s failed to query admin account with ID %u from database: %s", uniStr, id, qUtf8Printable(q.lastError().text()));
        return acc;
    }

    if (Q_UNLIKELY(!q.next())) {
        e.setErrorType(SkaffariError::NotFound);
        e.setErrorText(c->translate("AdminAccount", "Can not find administrator account with database ID %1.").arg(id));
        qCWarning(SK_ADMIN, "%s failed to find admin account with database ID %u.", uniStr, id);
        return acc;
    }

    const QString _username = q.value(0).toString();
    QDateTime createdTime = q.value(7).toDateTime();
    createdTime.setTimeSpec(Qt::UTC);
    QDateTime updatedTime = q.value(8).toDateTime();
    updatedTime.setTimeSpec(Qt::UTC);

    QList<dbid_t> doms;
    if (acc.type() < Administrator) {

        QSqlQuery q2 = CPreparedSqlQueryThread(QStringLiteral("SELECT domain_id FROM domainadmin WHERE admin_id = :admin_id"));
        q2.bindValue(QStringLiteral(":admin_id"), id);

        if (Q_UNLIKELY(!q2.exec())) {
            e.setSqlError(q2.lastError(), c->translate("AdminAccount", "Failed to query domain IDs from database this domain manager is responsible for."));
            qCCritical(SK_ADMIN, "%s failed to query domain IDs admin %s (ID: %u) is responsible for: %s", uniStr, qUtf8Printable(_username), id, qUtf8Printable(q2.lastError().text()));
            return acc;
        }

        while (q2.next()) {
            doms.push_back(q2.value(0).value<dbid_t>());
        }
    }

    acc = AdminAccount(id,
                       _username,
                       AdminAccountData::getUserType(q.value(1).value<quint8>()),
                       doms,
                       q.value(2).toString(),
                       q.value(3).toString(),
                       q.value(4).toString(),
                       q.value(5).value<quint8>(),
                       q.value(6).value<quint8>(),
                       createdTime,
                       updatedTime);

    return acc;
}

bool AdminAccount::update(Cutelyst::Context *c, SkaffariError &e, const QVariantHash &params)
{
    bool ret = false;

    Q_ASSERT_X(c, "update adminaccount", "invalid context object");
    Q_ASSERT_X(!params.empty(), "update adminaccount", "empty parameters");

    // for logging
    const QString errStr = AdminAccount::getUserNameIdString(c) + QLatin1String(" failed to update admin account ") + nameIdString();
    const QByteArray errBa = errStr.toUtf8();
    const char *err = errBa.constData();

    const QVariant typeVar = params.value(QStringLiteral("type"));

    if (!typeVar.canConvert<quint8>()) {
        e.setErrorType(SkaffariError::InputError);
        e.setErrorText(c->translate("AdminAccount", "Invalid administrator type."));
        qCCritical(SK_ADMIN, "%s: invalid administrator type.", err);
        return ret;
    }

    const AdminAccountType type = AdminAccountData::getUserType(typeVar.value<quint8>());

    if (type >= AdminAccount::getUserType(c)) {
        e.setErrorType(SkaffariError::AuthorizationError);
        e.setErrorText(c->translate("AdminAccount", "You are not allowed to set the type of this account to %1.").arg(AdminAccount::typeToName(type, c)));
        qCCritical(SK_ADMIN, "%s: not allowed to set the type of the account to %u.", err, type);
        return ret;
    }

    const QString password = params.value(QStringLiteral("password")).toString();

    if ((d->type == SuperUser) && (type != SuperUser)) {

        QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT COUNT(id) FROM adminuser WHERE type = 255"));

        if (Q_UNLIKELY(!(q.exec() && q.next()))) {
            e.setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to query count of administrators to check if this is the last administrator account."));
            qCCritical(SK_ADMIN, "%s: failed to query count of administrators to check if this is the last administrator account: %s", err, qUtf8Printable(q.lastError().text()));
            return ret;
        }

        if (q.value(0).toInt() <= 1) {
            e.setErrorType(SkaffariError::InputError);
            e.setErrorText(c->translate("AdminAccount", "You can not remove the last administrator."));
            qCWarning(SK_ADMIN, "%s: can not remove the last super user account.", err);
            return ret;
        }
    }

    QSqlDatabase db = QSqlDatabase::database(Cutelyst::Sql::databaseNameThread());
    if (Q_UNLIKELY(!db.isOpen())) {
        e.setSqlError(db.lastError(), c->translate("AdminAccount", "Failed to update administrator account in database."));
        qCCritical(SK_ADMIN, "%s: can not establish database connection: %s", err, qUtf8Printable(db.lastError().text()));
        return ret;
    }

    if (Q_UNLIKELY(!db.transaction())) {
        e.setSqlError(db.lastError(), c->translate("AdminAccount", "Failed to update administrator account in database."));
        qCCritical(SK_ADMIN, "%s: can not initiate databse transaction: %s", err, qUtf8Printable(db.lastError().text()));
        return ret;
    }

    QSqlQuery q(db);

    if (!password.isEmpty()) {
        const QByteArray encPw = Cutelyst::CredentialPassword::createPassword(params.value(QStringLiteral("password")).toString().toUtf8(),
                                                                              SkaffariConfig::admPwAlgorithm(),
                                                                              SkaffariConfig::admPwRounds(),
                                                                              24, 27);
        if (Q_UNLIKELY(encPw.isEmpty())) {
            e.setErrorType(SkaffariError::ApplicationError);
            e.setErrorText(c->translate("AdminAccount", "Password encryption failed. Please check your encryption settings."));
            qCCritical(SK_ADMIN, "%s: password encryption failed.", err);
            return ret;
        }

        if (Q_UNLIKELY(!q.prepare(QStringLiteral("UPDATE adminuser SET type = :type, password = :password, updated_at = :updated_at WHERE id = :id")))) {
            e.setSqlError(q.lastError());
            qCCritical(SK_ADMIN, "%s: can not prepare database query: %s", err, qUtf8Printable(q.lastError().text()));
            return ret;
        }

        q.bindValue(QStringLiteral(":password"), encPw);

    } else {
        if (Q_UNLIKELY(!q.prepare(QStringLiteral("UPDATE adminuser SET type = :type, updated_at = :updated_at WHERE id = :id")))) {
            e.setSqlError(q.lastError());
            qCCritical(SK_ADMIN, "%s: can not prepare database query: %s", err, qUtf8Printable(q.lastError().text()));
            return ret;
        }
    }

    const QDateTime currentUtc = QDateTime::currentDateTimeUtc();

    q.bindValue(QStringLiteral(":type"), static_cast<quint8>(type));
    q.bindValue(QStringLiteral(":updated_at"), currentUtc);
    q.bindValue(QStringLiteral(":id"), d->id);

    if (Q_UNLIKELY(!q.exec())) {
        e.setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to update administrator account in database."));
        qCCritical(SK_ADMIN, "%s: failed to update database entry: %s", err, qUtf8Printable(q.lastError().text()));;
        db.rollback();
        return ret;
    }

    if (Q_UNLIKELY(!q.prepare(QStringLiteral("DELETE FROM domainadmin WHERE admin_id = :id")))) {
        e.setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to update domain manager to domain connections in database."));
        qCCritical(SK_ADMIN, "%s: can not prepare database query: %s", err, qUtf8Printable(q.lastError().text()));
        db.rollback();
        return ret;
    }
    q.bindValue(QStringLiteral(":id"), d->id);

    if (Q_UNLIKELY(!q.exec())) {
        e.setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to update domain manager to domain connections in database."));
        qCCritical(SK_ADMIN, "%s: failed to update connections between domain manager and domains in database: %s", err, qUtf8Printable(q.lastError().text()));
        db.rollback();
        return ret;
    }

    QList<dbid_t> domIdList;
    if (type < AdminAccount::Administrator) {
        const QStringList domains = params.value(QStringLiteral("assocdomains")).toStringList();
        if (!domains.empty()) {
            domIdList.reserve(domains.size());
            if (Q_LIKELY(!q.prepare(QStringLiteral("INSERT INTO domainadmin (domain_id, admin_id) VALUES (:domain_id, :admin_id)")))) {
                e.setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to update domain manager to domain connections in database."));
                qCCritical(SK_ADMIN, "%s: can not prepare database query: %s", err, qUtf8Printable(q.lastError().text()));
                db.rollback();
                return ret;
            }
            for (const QString &adom : domains) {
                bool ok = false;
                const dbid_t did = adom.toULong(&ok);
                if (ok && did) {
                    q.bindValue(QStringLiteral(":domain_id"), QVariant::fromValue<dbid_t>(did));
                    q.bindValue(QStringLiteral(":admin_id"), d->id);
                    if (Q_UNLIKELY(!q.exec())) {
                        e.setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to update domain manager to domain connections in database."));
                        qCCritical(SK_ADMIN, "%s: failed to update connections between domain manager and domains in database: %s", err, qUtf8Printable(q.lastError().text()));
                        db.rollback();
                        return ret;
                    }
                    domIdList << did;
                }
            }
        }
    }

    if (Q_UNLIKELY(!db.commit())) {
        e.setSqlError(db.lastError(), c->translate("AdminAccount", "Failed to update administrator account in database."));
        qCCritical(SK_ADMIN, "%s: failed to commit database transaction: %s", err, qUtf8Printable(db.lastError().text()));
        db.rollback();
        return ret;
    }

    d->domains = domIdList;
    d->type = type;
    d->updated = currentUtc;

    ret = true;

    qCInfo(SK_ADMIN, "%s updated admin account %s of type %s.", qUtf8Printable(AdminAccount::getUserNameIdString(c)), qUtf8Printable(nameIdString()), AdminAccount::staticMetaObject.enumerator(AdminAccount::staticMetaObject.indexOfEnumerator("AdminAccountType")).valueToKey(type));
    qCDebug(SK_ADMIN) << *this;

    return ret;
}

bool AdminAccount::updateOwn(Cutelyst::Context *c, SkaffariError &e, const QVariantHash &p)
{
    bool ret = false;

    Q_ASSERT_X(c, "update own account", "invalid context object");
    Q_ASSERT_X(!p.empty(), "update own account", "empty parameters");

    // for logging
    const QString errStr = AdminAccount::getUserNameIdString(c) + QLatin1String(" failed to update own account");
    const QByteArray errBa = errStr.toUtf8();
    const char *err = errBa.constData();

    if (d->id != Cutelyst::Authentication::user(c).id().value<dbid_t>()) {
        e.setErrorType(SkaffariError::AuthorizationError);
        e.setErrorText(c->translate("AdminAccount", "You are not allowed to change this administrator account."));
        qCWarning(SK_ADMIN, "%s: access denied.", err);
        return ret;
    }

    const QString password = p.value(QStringLiteral("password")).toString();
    const QDateTime currentUtc = QDateTime::currentDateTimeUtc();
    const QString tz = p.value(QStringLiteral("tz"), d->tz).toString();
    const QString langCode = p.value(QStringLiteral("lang"), d->lang).toString();
    const QLocale lang(langCode);

    if (lang.language() == QLocale::C) {
        e.setErrorType(SkaffariError::InputError);
        e.setErrorText(c->translate("AdminAccount", "%1 is not a valid locale code.").arg(langCode));
        qCWarning(SK_ADMIN, "%s: invalid locale code %s.", err, qUtf8Printable(langCode));
        return ret;
    }

    QTimeZone timeZone(tz.toLatin1());
    if (!timeZone.isValid()) {
        e.setErrorType(SkaffariError::InputError);
        e.setErrorText(c->translate("AdminAccount", "%1 is not a valid IANA time zone ID.").arg(tz));
        qCWarning(SK_ADMIN, "%s: invalid IANA time zone ID %s.", err, qUtf8Printable(tz));
        return ret;
    }

    QSqlDatabase db = QSqlDatabase::database(Cutelyst::Sql::databaseNameThread());

    if (Q_UNLIKELY(!db.isOpen())) {
        e.setSqlError(db.lastError(), c->translate("AdminAccount", "Failed to update administrator in database."));
        qCCritical(SK_ADMIN, "%s: failed to establish database connection: %s", err, qUtf8Printable(db.lastError().text()));
        return ret;
    }

    if (Q_UNLIKELY(!db.transaction())) {
        e.setSqlError(db.lastError(), c->translate("AdminAccount", "Failed to update administrator in database."));
        qCCritical(SK_ADMIN, "%s: failed to start database transaction: %s", err, qUtf8Printable(db.lastError().text()));
        return ret;
    }

    QSqlQuery q(db);

    if (!password.isEmpty()) {
        const QByteArray encPw = Cutelyst::CredentialPassword::createPassword(password.toUtf8(),
                                                                              SkaffariConfig::admPwAlgorithm(),
                                                                              SkaffariConfig::admPwRounds(),
                                                                              24,
                                                                              27);

        if (Q_UNLIKELY(encPw.isEmpty())) {
            e.setErrorType(SkaffariError::ApplicationError);
            e.setErrorText(c->translate("AdminAccount", "Password encryption failed. Please check your encryption settings."));
            qCCritical(SK_ADMIN, "%s: password encryption failed.", err);
            return ret;
        }

        if (Q_UNLIKELY(!q.prepare(QStringLiteral("UPDATE adminuser SET password = :password, updated_at = :updated_at WHERE id = :id")))) {
             e.setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to update administrator in database."));
             qCCritical(SK_ADMIN, "%s: failed to prepare database query: %s", err, qUtf8Printable(q.lastError().text()));
             return ret;
        }

        q.bindValue(QStringLiteral(":password"), encPw);

    } else {
        if (Q_UNLIKELY(!q.prepare(QStringLiteral("UPDATE adminuser SET updated_at = :updated_at WHERE id = :id")))) {
            e.setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to update administrator in database."));
            qCCritical(SK_ADMIN, "%s: failed to prepare database query: %s", err, qUtf8Printable(q.lastError().text()));
            return ret;
        }
    }

    q.bindValue(QStringLiteral(":updated_at"), currentUtc);
    q.bindValue(QStringLiteral(":id"), d->id);

    if (Q_UNLIKELY(!q.exec())) {
        e.setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to update administrator in database."));
        qCCritical(SK_ADMIN, "%s: update account in database failed: %s", err, qUtf8Printable(q.lastError().text()));
        db.rollback();
        return ret;
    }

    const quint8 maxdisplay = p.value(QStringLiteral("maxdisplay"), d->maxDisplay).value<quint8>();
    const quint8 warnlevel = p.value(QStringLiteral("warnlevel"), d->warnLevel).value<quint8>();

    if (Q_UNLIKELY(!q.prepare(QStringLiteral("UPDATE settings SET maxdisplay = :maxdisplay, warnlevel = :warnlevel, lang = :lang, tz = :tz WHERE admin_id = :admin_id")))) {
        e.setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to update administrator in database."));
        qCCritical(SK_ADMIN, "%s: failed to prepare database query: %s", err, qUtf8Printable(q.lastError().text()));
        db.rollback();
        return ret;
    }

    q.bindValue(QStringLiteral(":maxdisplay"), maxdisplay);
    q.bindValue(QStringLiteral(":warnlevel"), warnlevel);
    q.bindValue(QStringLiteral(":lang"), lang.name());
    q.bindValue(QStringLiteral(":admin_id"), d->id);
    q.bindValue(QStringLiteral(":tz"), tz);

    if (Q_UNLIKELY(!q.exec())) {
        e.setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to update administrator settings in database."));
        qCCritical(SK_ADMIN, "%s: update settings in database failed: %s", err, qUtf8Printable(q.lastError().text()));
        db.rollback();
        return ret;
    }

    if (Q_UNLIKELY(!db.commit())) {
        e.setSqlError(db.lastError(), c->translate("AdminAccount", "Failed to update administrator in database."));
        qCCritical(SK_ADMIN, "%s: failed to commit database transaction: %s", err, qUtf8Printable(db.lastError().text()));
        return ret;
    }

    Cutelyst::Session::setValue(c, QStringLiteral("maxdisplay"), maxdisplay);
    Cutelyst::Session::setValue(c, QStringLiteral("warnlevel"), warnlevel);
    Cutelyst::Session::setValue(c, QStringLiteral("lang"), lang);
    Cutelyst::Session::setValue(c, QStringLiteral("tz"), tz);

    d->maxDisplay = maxdisplay;
    d->warnLevel = warnlevel;
    d->lang = lang.name();
    d->tz = tz;
    d->updated = currentUtc;

    c->setLocale(lang);
    c->stash({
                 {QStringLiteral("userMaxDisplay"), maxdisplay},
                 {QStringLiteral("userWarnLevel"), warnlevel},
                 {QStringLiteral("userTz"), tz},
                 {QStringLiteral("lang"), lang.name()}
             });

    ret = true;

    qCInfo(SK_ADMIN, "%s updated his/her own account.", qUtf8Printable(AdminAccount::getUserNameIdString(c)));
    qCDebug(SK_ADMIN) << *this;

    return ret;
}

bool AdminAccount::remove(Cutelyst::Context *c, SkaffariError &e)
{
    bool ret = false;

    // for logging
    const QString errStr = AdminAccount::getUserNameIdString(c) + QLatin1String(" failed to remove admin acccount ") + nameIdString();
    const QByteArray errBa = errStr.toUtf8();
    const char *err = errBa.constData();

    if (d->type >= AdminAccount::getUserType(c)) {
        e.setErrorType(SkaffariError::AuthorizationError);
        e.setErrorText(c->translate("AdminAccount", "You are not allowed to remove accounts of type %1.").arg(typeName(c)));
        qCWarning(SK_ADMIN, "%s: not allowed to remove accounts of type %u.", err, d->type);
        return ret;
    }

    QSqlQuery q;

    if (isSuperUser()) {

        q = CPreparedSqlQueryThread(QStringLiteral("SELECT COUNT(id) FROM adminuser WHERE type = 255"));

        if (Q_UNLIKELY(!(q.exec() && q.next()))) {
            e.setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to query count of super users to check if this is the last super user account."));
            qCCritical(SK_ADMIN, "%s: query to count current super user accounts failed: %s", err, qUtf8Printable(q.lastError().text()));
            return ret;
        }

        if (q.value(0).toInt() <= 1) {
            e.setErrorType(SkaffariError::InputError);
            e.setErrorText(c->translate("AdminAccount", "You can not remove the last super user."));
            qCWarning(SK_ADMIN, "%s: can not remove last super user account.", err);
            return ret;
        }

    }

    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM adminuser WHERE id = :id"));
    q.bindValue(QStringLiteral(":id"), d->id);

    if (Q_UNLIKELY(!q.exec())) {
        e.setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to delete administrator %1 from database.").arg(d->username));
        qCCritical(SK_ADMIN, "%s: can not delete admin from database: %s", err, qUtf8Printable(q.lastError().text()));
        return ret;
    }

    ret = true;
    qCInfo(SK_ADMIN, "%s removed admin %s of type %s.", qUtf8Printable(AdminAccount::getUserNameIdString(c)), qUtf8Printable(nameIdString()), AdminAccount::staticMetaObject.enumerator(AdminAccount::staticMetaObject.indexOfEnumerator("AdminAccountType")).valueToKey(d->type));
    qCDebug(SK_ADMIN) << *this;

    return ret;
}

void AdminAccount::toStash(Cutelyst::Context *c, dbid_t adminId)
{
    Q_ASSERT_X(c, "admin account to stash", "invalid context object");

    SkaffariError e(c);
    AdminAccount a = AdminAccount::get(c, e, adminId);
    if (Q_LIKELY(a.isValid())) {
        c->stash({
                     {QStringLiteral(ADMIN_ACCOUNT_STASH_KEY), QVariant::fromValue<AdminAccount>(a)},
                     {QStringLiteral("site_title"), a.username()}
                 });
    } else {
        e.toStash(c);
        c->res()->setStatus(404);
        c->detach(c->getAction(QStringLiteral("error")));
    }
}

void AdminAccount::toStash(Cutelyst::Context *c, const AdminAccount &adminAccount)
{
    Q_ASSERT_X(c, "admin account to stash", "invalid context object");

    c->stash({
                 {QStringLiteral(ADMIN_ACCOUNT_STASH_KEY), QVariant::fromValue<AdminAccount>(adminAccount)},
                 {QStringLiteral("site_title"), adminAccount.username()}
             });
}

AdminAccount AdminAccount::fromStash(Cutelyst::Context *c)
{
    AdminAccount a;

    Q_ASSERT_X(c, "admin account from stash", "invalid context object");

    a = c->stash(QStringLiteral(ADMIN_ACCOUNT_STASH_KEY)).value<AdminAccount>();

    return a;
}

AdminAccount AdminAccount::getUser(Cutelyst::Context *c)
{
    return c->stash(QStringLiteral(ADMIN_USER_STASH_KEY)).value<AdminAccount>();
}

AdminAccount::AdminAccountType AdminAccount::getUserType(quint8 type)
{
    return AdminAccountData::getUserType(type);
}

AdminAccount::AdminAccountType AdminAccount::getUserType(const Cutelyst::AuthenticationUser &user)
{
    return AdminAccountData::getUserType(user.value(QStringLiteral("type"), 0).value<quint8>());
}

AdminAccount::AdminAccountType AdminAccount::getUserType(Cutelyst::Context *c)
{
    return c->stash(QStringLiteral(ADMIN_USER_STASH_KEY)).value<AdminAccount>().type();
}

AdminAccount::AdminAccountType AdminAccount::getUserType(const QVariant &user)
{
    return AdminAccountData::getUserType(user.value<quint8>());
}

dbid_t AdminAccount::getUserId(Cutelyst::Context *c)
{
    return c->stash(QStringLiteral(ADMIN_USER_STASH_KEY)).value<AdminAccount>().id();
}

QString AdminAccount::getUserName(Cutelyst::Context *c)
{
    return c->stash(QStringLiteral(ADMIN_USER_STASH_KEY)).value<AdminAccount>().username();
}

QString AdminAccount::getUserNameIdString(Cutelyst::Context *c)
{
    QString ret;
    const AdminAccount a = c->stash(QStringLiteral(ADMIN_USER_STASH_KEY)).value<AdminAccount>();
    ret = a.username() + QLatin1String(" (ID: ") + QString::number(a.id()) + QLatin1Char(')');
    return ret;
}

QString AdminAccount::typeToName(AdminAccount::AdminAccountType type, Cutelyst::Context *c)
{
    QString name;

    Q_ASSERT(c);

    switch (type) {
    case SuperUser:
        name = c->translate("AdminAccount", "Super user");
        break;
    case Administrator:
        name = c->translate("AdminAccount", "Administrator");
        break;
    case DomainMaster:
        name = c->translate("AdminAccount", "Domain manager");
        break;
    default:
        name = c->translate("AdminAccount", "Disabled account");
        break;
    }

    return name;
}

QStringList AdminAccount::allowedTypes(AdminAccount::AdminAccountType userType)
{
    QStringList lst;

    const QMetaEnum me = QMetaEnum::fromType<AdminAccount::AdminAccountType>();

    if (me.isValid() && (me.keyCount() > 0)) {
        const int _userType = static_cast<int>(userType);
        for (int i = 0; i < me.keyCount(); ++i) {
            const auto type = me.value(i);
            if ((type != 0) && ((_userType == 255) || (type < _userType))) {
                lst << QString::number(type); // clazy:exclude=reserve-candidates
            }
        }
    }

    return lst;
}

QStringList AdminAccount::allowedTypes(Cutelyst::Context *c)
{
    return AdminAccount::allowedTypes(AdminAccount::getUserType(c));
}

AdminAccount::AdminAccountType AdminAccount::maxAllowedType(AdminAccount::AdminAccountType userType)
{
    AdminAccount::AdminAccountType max = AdminAccount::Disabled;

    if (userType == AdminAccount::SuperUser) {
        max = AdminAccount::SuperUser;
    } else {
        const QMetaEnum me = QMetaEnum::fromType<AdminAccount::AdminAccountType>();

        if (me.isValid() && (me.keyCount() > 0)) {
            for (int i = 0; i < me.keyCount(); ++i) {
                const auto cur = static_cast<AdminAccount::AdminAccountType>(me.value(i));
                if ((cur < userType) && (cur > max)) {
                    max = cur;
                }
            }
        }
    }

    return max;
}

AdminAccount::AdminAccountType AdminAccount::maxAllowedType(Cutelyst::Context *c)
{
    return AdminAccount::maxAllowedType(AdminAccount::getUserType(c));
}

QDebug operator<<(QDebug dbg, const AdminAccount &account)
{
    const bool oldSetting = dbg.autoInsertSpaces();
    dbg.nospace() << "AdminAccount(";
    dbg << "ID: " << account.id() << ", ";
    dbg << "Username: " << account.username() << ", ";
    dbg << "Type: " << QMetaEnum::fromType<AdminAccount::AdminAccountType>().valueToKey(account.type()) << ", ";
    dbg << "Language: " << account.lang() << ", ";
    dbg << "Timezone: " << account.tz() << ", ";
    dbg << "Warnlevel: " << account.warnLevel() << ", ";
    dbg << "Maxdisplay: " << account.maxDisplay() << ", ";
    dbg << "Created: " << account.created().toString(Qt::ISODate) << ", ";
    dbg << "Updated " << account.updated().toString(Qt::ISODate) << ", ";
    if (!account.domains().empty()) {
        dbg << "Domains: [";
        const QList<dbid_t> doms = account.domains();
        const int domsSize = doms.size();
        const int lastDom = domsSize-1;
        for (int i = 0; i < domsSize; ++i) {
            dbg << doms.at(i);
            if (i != lastDom) {
                dbg << ',';
            }
        }
        dbg << "]";
    } else {
        dbg << "Domains: []";
    }
    dbg << ')';
    dbg.setAutoInsertSpaces(oldSetting);
    return dbg.maybeSpace();
}

QDataStream& operator<<(QDataStream &stream, const AdminAccount &account)
{
    stream << account.domains() << account.username() << account.lang()
           << account.getTemplate() << account.tz() << account.created()
           << account.updated() << account.id() << static_cast<quint8>(account.type())
           << account.maxDisplay() << account.warnLevel();

    return stream;
}

QDataStream& operator>>(QDataStream &stream, AdminAccount &account)
{
    stream >> account.d->domains;
    stream >> account.d->username;
    stream >> account.d->lang;
    stream >> account.d->tmpl;
    stream >> account.d->tz;
    stream >> account.d->created;
    stream >> account.d->updated;
    stream >> account.d->id;
    quint8 _type;
    stream >> _type;
    account.d->type = AdminAccount::getUserType(_type);
    stream >> account.d->maxDisplay;
    stream >> account.d->warnLevel;

    return stream;
}

#include "moc_adminaccount.cpp"
