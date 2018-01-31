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

#include "adminaccount_p.h"
#include "skaffarierror.h"
#include "../utils/utils.h"
#include "../utils/skaffariconfig.h"
#include <QSqlQuery>
#include <QSqlError>
#include <Cutelyst/Context>
#include <Cutelyst/Plugins/Utils/Sql>
#include <Cutelyst/Plugins/Authentication/credentialpassword.h>
#include <Cutelyst/Plugins/Authentication/authentication.h>
#include <Cutelyst/Plugins/Session/Session>
#include <Cutelyst/Response>
#include <QStringList>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(SK_ADMIN, "skaffari.admin")

#define ADMIN_ACCOUNT_STASH_KEY "adminaccount"

AdminAccount::AdminAccount() :
    d(new AdminAccountData)
{

}

AdminAccount::AdminAccount(dbid_t id, const QString& username, qint16 type, const QList<dbid_t> &domains) :
    d(new AdminAccountData(id, username, type, domains))
{

}

AdminAccount::AdminAccount(const AdminAccount& other) :
    d(other.d)
{

}

AdminAccount& AdminAccount::operator=(const AdminAccount& other)
{
    d = other.d;
    return *this;
}

AdminAccount::~AdminAccount()
{

}

dbid_t AdminAccount::id() const
{
    return d->id;
}


void AdminAccount::setId(dbid_t id)
{
    d->id = id;
}

QString AdminAccount::username() const
{
    return d->username;
}

void AdminAccount::setUsername(const QString& nUsername)
{
    d->username = nUsername;
}

qint16 AdminAccount::type() const
{
    return d->type;
}

void AdminAccount::setType(qint16 nType)
{
    d->type = nType;
}

QList<dbid_t> AdminAccount::domains() const
{
    return d->domains;
}

void AdminAccount::setDomains(const QList<dbid_t> &nDomains)
{
    d->domains = nDomains;
}

bool AdminAccount::isSuperUser() const
{
    return (d->type == 0);
}

bool AdminAccount::isValid() const
{
    return !d->username.isEmpty() && (d->id > 0);
}

QString AdminAccount::lang() const
{
    return d->lang;
}

void AdminAccount::setLang(const QString &lang)
{
    d->lang = lang;
}

QByteArray AdminAccount::tz() const
{
    return d->tz;
}

void AdminAccount::setTz(const QByteArray &tz)
{
    d->tz = tz;
}

QDateTime AdminAccount::created() const
{
    return d->created;
}

void AdminAccount::setCreated(const QDateTime &created)
{
    d->created = created;
}

QDateTime AdminAccount::updated() const
{
    return d->updated;
}

void AdminAccount::setUpdated(const QDateTime &updated)
{
    d->updated = updated;
}

quint8 AdminAccount::maxDisplay() const
{
    return d->maxDisplay;
}

void AdminAccount::setMaxDisplay(quint8 maxDisplay)
{
    d->maxDisplay = maxDisplay;
}

quint8 AdminAccount::warnLevel() const
{
    return d->warnLevel;
}

void AdminAccount::setWarnLevel(quint8 warnLevel)
{
    if (warnLevel > 100) {
        d->warnLevel = 100;
    } else {
        d->warnLevel = warnLevel;
    }
}

QString AdminAccount::getTemplate() const
{
    return d->tmpl;
}

void AdminAccount::setTemplate(const QString &tmpl)
{
    d->tmpl = tmpl;
}

AdminAccount AdminAccount::create(Cutelyst::Context *c, const Cutelyst::ParamsMultiMap &params, SkaffariError *error)
{
    AdminAccount aa;

    Q_ASSERT_X(error, "create new adminaccount", "invalid error object");
    Q_ASSERT_X(c, "create new adminaccount", "invalid context object");
    Q_ASSERT_X(!params.empty(), "create new adminaccount", "params can not be empty");

    const QString username = params.value(QStringLiteral("username")).trimmed();

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT id FROM adminuser WHERE username = :username"));
    q.bindValue(QStringLiteral(":username"), username);

    if (Q_UNLIKELY(!q.exec())) {
        error->setSqlError(q.lastError(), c->translate("AdminAccount", "Cannot check whether the user name is already assigned."));
        qCCritical(SK_ADMIN) << "Failed to check if user name is already in use while creating new admin account." << q.lastError().text();
        return aa;
    }

    if (Q_UNLIKELY(q.next())) {
        error->setErrorType(SkaffariError::InputError);
        error->setErrorText(c->translate("AdminAccount", "This administrator user name is already in use."));
        return aa;
    }

    const QByteArray password = Cutelyst::CredentialPassword::createPassword(params.value(QStringLiteral("password")).toUtf8(), SkaffariConfig::admPwAlgorithm(), SkaffariConfig::admPwRounds(), 24, 27);

    if (Q_UNLIKELY(password.isEmpty())) {
        error->setErrorType(SkaffariError::ApplicationError);
        error->setErrorText(c->translate("AdminAccount", "Password encryption failed. Please check your encryption settings."));
        qCCritical(SK_ADMIN) << "Password encryption failed using Cutelyst::CredentialPassword";
        return aa;
    }

    const qint16 type = params.value(QStringLiteral("type")).toShort();

    const QDateTime currentUtc = QDateTime::currentDateTimeUtc();
    const dbid_t id = AdminAccount::setAdminAccount(c, error, params.value(QStringLiteral("username")), password, type);
    if (id <= 0) {
        return aa;
    }

    if (!AdminAccount::setAdminSettings(c, error, id)) {
        AdminAccount::rollbackAdminAccount(c, error, id);
        return aa;
    }

    QList<dbid_t> domIds;
    const QStringList assocdoms = params.values(QStringLiteral("assocdomains"));
    for (const QString &did : assocdoms) {
        domIds << did.toULong();
    }
    if (!AdminAccount::setAdminDomains(c, error, id, domIds)) {
        AdminAccount::rollbackAdminDomains(c, error, id);
        AdminAccount::rollbackAdminSettings(c, error, id);
        AdminAccount::rollbackAdminAccount(c, error, id);
        return aa;
    }

    aa.setId(id);
    aa.setUsername(username);
    aa.setType(type);
    aa.setDomains(domIds);
    aa.setLang(QStringLiteral("en"));
    aa.setTz(QByteArrayLiteral("UTC"));
    aa.setTemplate(QStringLiteral("default"));
    aa.setMaxDisplay(25);
    aa.setWarnLevel(90);
    aa.setCreated(currentUtc);
    aa.setUpdated(currentUtc);

    if (type == 0) {
        qCInfo(SK_ADMIN, "%s created a new administrator account: %s", qUtf8Printable(Utils::getUserName(c)), qUtf8Printable(username));
    } else {
        qCInfo(SK_ADMIN, "%s created a new domain manager account: %s", qUtf8Printable(Utils::getUserName(c)), qUtf8Printable(username));
    }

    return aa;
}

QVector<AdminAccount> AdminAccount::list(Cutelyst::Context *c, SkaffariError *error)
{
    QVector<AdminAccount> list;

    Q_ASSERT_X(c, "list admins", "invalid context object");
    Q_ASSERT_X(error, "list admins", "invalid error object");

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT id, username, type FROM adminuser ORDER BY username ASC"));

    if (Q_UNLIKELY(!q.exec())) {
        error->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to query list of admins from database."));
        qCCritical(SK_ADMIN) << "Failed to query list of admins from database." << q.lastError().text();
        return list;
    }

    while (q.next()) {
        list.push_back(AdminAccount(q.value(0).value<dbid_t>(),
                                    q.value(1).toString(),
                                    static_cast<AdminAccount::AdminAccountType>(q.value(2).value<qint8>()),
                                    QList<dbid_t>()));
    }

    return list;
}

AdminAccount AdminAccount::get(Cutelyst::Context *c, SkaffariError *e, dbid_t id)
{
    AdminAccount acc;

    Q_ASSERT_X(c, "get admin", "invalid context object");
    Q_ASSERT_X(e, "get admin", "invalid error object");
    Q_ASSERT_X(id > 0, "get admin", "invalid database id");

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT a.username, a.type, s.lang, s.tz, s.template, s.maxdisplay, s.warnlevel, a.created_at, a.updated_at FROM adminuser a JOIN settings s ON a.id = s.admin_id WHERE a.id = :id"));
    q.bindValue(QStringLiteral(":id"), id);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to query administrator account with ID %1 from database.").arg(id));
        qCCritical(SK_ADMIN, "Failed to query admin account with ID %u from database: %s", id, qUtf8Printable(q.lastError().text()));
        return acc;
    }

    if (Q_UNLIKELY(!q.next())) {
        e->setErrorType(SkaffariError::InputError);
        e->setErrorText(c->translate("AdminAccount", "Can not find administrator account with database ID %1.").arg(id));
        qCWarning(SK_ADMIN) << "Failed to find admin account with database ID" << id;
        return acc;
    }

    acc.setId(id);
    acc.setUsername(q.value(0).toString());
    acc.setType(q.value(1).value<qint16>());
    acc.setLang(q.value(2).toString());
    acc.setTz(q.value(3).toByteArray());
    acc.setTemplate(q.value(4).toString());
    acc.setMaxDisplay(q.value(5).value<quint8>());
    acc.setWarnLevel(q.value(6).value<quint8>());
    QDateTime createdTime = q.value(7).toDateTime();
    createdTime.setTimeSpec(Qt::UTC);
    acc.setCreated(createdTime);
    QDateTime updatedTime = q.value(8).toDateTime();
    updatedTime.setTimeSpec(Qt::UTC);
    acc.setUpdated(updatedTime);

    if (acc.type() != AdminAccountType::SuperUser) {

        q = CPreparedSqlQueryThread(QStringLiteral("SELECT domain_id FROM domainadmin WHERE admin_id = :admin_id"));
        q.bindValue(QStringLiteral(":admin_id"), id);

        if (Q_UNLIKELY(!q.exec())) {
            e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to query domain IDs from database this domain manager is responsible for."));
            qCCritical(SK_ADMIN, "Failed to query domain IDs from database admin %s is responsible for: %s", qUtf8Printable(acc.username()), qUtf8Printable(q.lastError().text()));
            return acc;
        }

        QList<dbid_t> doms;
        while (q.next()) {
            doms.push_back(q.value(0).value<dbid_t>());
        }

        acc.setDomains(doms);
    }

    return acc;
}

bool AdminAccount::update(Cutelyst::Context *c, SkaffariError *e, AdminAccount *a, const Cutelyst::ParamsMultiMap &params)
{
    bool ret = false;

    Q_ASSERT_X(c, "update adminaccount", "invalid context object");
    Q_ASSERT_X(e, "update adminaccount", "invalid error object");
    Q_ASSERT_X(a, "update adminaccount", "invalid account object");
    Q_ASSERT_X(a->isValid(), "update adminaccount", "invalid account object");
    Q_ASSERT_X(!params.empty(), "update adminaccount", "empty parameters");

    const QString password = params.value(QStringLiteral("password"));
    const qint16 type = params.value(QStringLiteral("type")).toShort();

    QSqlQuery q;

    if (a->isSuperUser() && (type != 0)) {

        q = CPreparedSqlQueryThread(QStringLiteral("SELECT COUNT(id) FROM adminuser WHERE type = 0"));

        if (Q_UNLIKELY(!q.exec())) {
            e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to query count of administrators to check if this is the last administrator account."));
            qCCritical(SK_ADMIN) << "Failed to query count of administrators to check if this is the last administrator account." << q.lastError().text();
            return ret;
        }

        if (Q_UNLIKELY(!q.next())) {
            e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to query count of administrators to check if this is the last administrator account."));
            qCCritical(SK_ADMIN) << "Failed to query count of administrators to check if this is the last administrator account." << q.lastError().text();
            return ret;
        }

        if (q.value(0).toInt() <= 1) {
            e->setErrorType(SkaffariError::InputError);
            e->setErrorText(c->translate("AdminAccount", "You can not remove the last administrator."));
            qCWarning(SK_ADMIN, "%s tried to remove the last administrator account.", qUtf8Printable(Utils::getUserName(c)));
            return ret;
        }
    }

    if (!password.isEmpty()) {
        const QByteArray encPw = Cutelyst::CredentialPassword::createPassword(params.value(QStringLiteral("password")).toUtf8(),
                                                                              SkaffariConfig::admPwAlgorithm(),
                                                                              SkaffariConfig::admPwRounds(),
                                                                              24, 27);
        if (Q_UNLIKELY(encPw.isEmpty())) {
            e->setErrorType(SkaffariError::ApplicationError);
            e->setErrorText(c->translate("AdminAccount", "Password encryption failed. Please check your encryption settings."));
            qCCritical(SK_ADMIN) << "Password encryption failed using Cutelyst::CredentialPassword";
            return ret;
        }

        q = CPreparedSqlQueryThread(QStringLiteral("UPDATE adminuser SET type = :type, password = :password, updated_at = :updated_at WHERE id = :id"));
        q.bindValue(QStringLiteral(":password"), encPw);

    } else {
        q = CPreparedSqlQueryThread(QStringLiteral("UPDATE adminuser SET type = :type, updated_at = :updated_at WHERE id = :id"));
    }

    const QDateTime currentUtc = QDateTime::currentDateTimeUtc();

    q.bindValue(QStringLiteral(":type"), type);
    q.bindValue(QStringLiteral(":updated_at"), currentUtc);
    q.bindValue(QStringLiteral(":id"), a->id());

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to update administrator account in database."));
        qCCritical(SK_ADMIN, "Failed to update administrator account %s in database: %s", qUtf8Printable(a->username()), qUtf8Printable(q.lastError().text()));
        return ret;
    }

    a->setType(type);
    a->setUpdated(currentUtc);

    QStringList domains;
    if (a->type() != AdminAccount::SuperUser) {
        domains = params.values(QStringLiteral("assocdomains"));
    }

    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM domainadmin WHERE admin_id = :id"));
    q.bindValue(QStringLiteral(":id"), a->id());

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to update domain manager to domain connections in database."));
        qCCritical(SK_ADMIN, "Failed to update connections between domain manager %s and domains in database: %s", qUtf8Printable(Utils::getUserName(c)), qUtf8Printable(q.lastError().text()));
        return ret;
    }

    QList<dbid_t> domIds;
    if (!domains.isEmpty()) {
        for (int i = 0; i < domains.size(); ++i) {
            const dbid_t did = domains.at(i).toULong();
            q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO domainadmin (domain_id, admin_id) VALUES (:domain_id, :admin_id)"));
            q.bindValue(QStringLiteral(":domain_id"), QVariant::fromValue<dbid_t>(did));
            q.bindValue(QStringLiteral(":admin_id"), a->id());
            if (Q_UNLIKELY(!q.exec())) {
                e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to update domain manager to domain connections in database."));
                qCCritical(SK_ADMIN, "Failed to update connections between domain manager %s and domains in database: %s", qUtf8Printable(Utils::getUserName(c)), qUtf8Printable(q.lastError().text()));
                return ret;
            }
            domIds << did;
        }
    }

    a->setDomains(domIds);

    ret = true;

    if (a->type() == 0) {
        qCInfo(SK_ADMIN, "%s updated administrator user %s.", qUtf8Printable(Utils::getUserName(c)), qUtf8Printable(a->username()));
    } else {
        qCInfo(SK_ADMIN, "%s updated domain manager %s.", qUtf8Printable(Utils::getUserName(c)), qUtf8Printable(a->username()));
    }


    return ret;
}

bool AdminAccount::update(Cutelyst::Context *c, SkaffariError *e, AdminAccount *a, Cutelyst::AuthenticationUser *u, const Cutelyst::ParamsMultiMap &p)
{
    bool ret = false;

    Q_ASSERT_X(c, "update own account", "invalid context object");
    Q_ASSERT_X(e, "update own account", "invalid error object");
    Q_ASSERT_X(!p.empty(), "update own account", "empty parameters");
    Q_ASSERT_X(u, "update own account", "invalid authentication user object");

    const dbid_t id = u->id().toULong();
    const QString password = p.value(QStringLiteral("password"));
    const QDateTime currentUtc = QDateTime::currentDateTimeUtc();

    QSqlQuery q;

    if (!password.isEmpty()) {
        const QByteArray encPw = Cutelyst::CredentialPassword::createPassword(p.value(QStringLiteral("password")).toUtf8(),
                                                                              SkaffariConfig::admPwAlgorithm(),
                                                                              SkaffariConfig::admPwRounds(),
                                                                              24,
                                                                              27);

        if (Q_UNLIKELY(encPw.isEmpty())) {
            e->setErrorType(SkaffariError::ApplicationError);
            e->setErrorText(c->translate("AdminAccount", "Password encryption failed. Please check your encryption settings."));
            qCCritical(SK_ADMIN) << "Password encryption failed using Cutelyst::CredentialPassword";
            return ret;
        }

        q = CPreparedSqlQueryThread(QStringLiteral("UPDATE adminuser SET password = :password, updated_at = :updated_at WHERE id = :id"));
        q.bindValue(QStringLiteral(":password"), encPw);
    } else {
        q = CPreparedSqlQueryThread(QStringLiteral("UPDATE adminuser SET updated_at = :updated_at WHERE id = :id"));
    }

    q.bindValue(QStringLiteral(":updated_at"), currentUtc);
    q.bindValue(QStringLiteral(":id"), id);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to update administrator in database."));
        qCCritical(SK_ADMIN, "Failed to update administrator %s in database: %s", qUtf8Printable(a->username()), qUtf8Printable(q.lastError().text()));
        return ret;
    }

    const quint8 maxdisplay = p.value(QStringLiteral("maxdisplay"), u->value(QStringLiteral("maxdisplay")).toString()).toUShort();
    const quint8 warnlevel = p.value(QStringLiteral("warnlevel"), u->value(QStringLiteral("warnlevel")).toString()).toUShort();
    const QString lang = p.value(QStringLiteral("lang"), u->value(QStringLiteral("lang")).toString());
    const QByteArray tz = p.value(QStringLiteral("tz"), u->value(QStringLiteral("tz")).toString()).toUtf8();

    q = CPreparedSqlQueryThread(QStringLiteral("UPDATE settings SET maxdisplay = :maxdisplay, warnlevel = :warnlevel, lang = :lang, tz = :tz WHERE admin_id = :admin_id"));
    q.bindValue(QStringLiteral(":maxdisplay"), maxdisplay);
    q.bindValue(QStringLiteral(":warnlevel"), warnlevel);
    q.bindValue(QStringLiteral(":lang"), lang);
    q.bindValue(QStringLiteral(":admin_id"), id);
    q.bindValue(QStringLiteral(":tz"), tz);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to update administrator settings in database."));
        qCCritical(SK_ADMIN, "Failed to update settings for administrator %s in database: %s", qUtf8Printable(a->username()), qUtf8Printable(q.lastError().text()));
        return ret;
    }

    u->insert(QStringLiteral("maxdisplay"), maxdisplay);
    u->insert(QStringLiteral("warnlevel"), warnlevel);
    u->insert(QStringLiteral("lang"), lang);
    u->insert(QStringLiteral("tz"), tz);

    Cutelyst::Session::setValue(c, QStringLiteral("maxdisplay"), maxdisplay);
    Cutelyst::Session::setValue(c, QStringLiteral("warnlevel"), warnlevel);
    Cutelyst::Session::setValue(c, QStringLiteral("lang"), lang);
    Cutelyst::Session::setValue(c, QStringLiteral("tz"), tz);

    a->setMaxDisplay(maxdisplay);
    a->setWarnLevel(warnlevel);
    a->setLang(lang);
    a->setTz(tz);
    a->setUpdated(currentUtc);

    c->stash({
                 {QStringLiteral("userMaxDisplay"), maxdisplay},
                 {QStringLiteral("userWarnLevel"), warnlevel},
                 {QStringLiteral("lang"), lang}
             });

    ret = true;

    if (a->type() == 0) {
        qCInfo(SK_ADMIN, "%s updated administrator user %s.", qUtf8Printable(Utils::getUserName(c)), qUtf8Printable(a->username()));
    } else {
        qCInfo(SK_ADMIN, "%s updated domain manager %s.", qUtf8Printable(Utils::getUserName(c)), qUtf8Printable(a->username()));
    }

    return ret;
}

bool AdminAccount::remove(Cutelyst::Context *c, SkaffariError *e, const AdminAccount &a)
{
    bool ret = false;

    QSqlQuery q;

    if (a.isSuperUser()) {

        q = CPreparedSqlQueryThread(QStringLiteral("SELECT COUNT(id) FROM adminuser WHERE type = 0"));

        if (Q_UNLIKELY(!q.exec())) {
            e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to query count of administrators to check if this is the last administrator account."));
            qCCritical(SK_ADMIN) << "Failed to query count of administrators to check if this is the last administrator account." << q.lastError().text();
            return ret;
        }

        if (Q_UNLIKELY(!q.next())) {
            e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to query count of administrators to check if this is the last administrator account."));
            qCCritical(SK_ADMIN) << "Failed to query count of administrators to check if this is the last administrator account." << q.lastError().text();
            return ret;
        }

        if (q.value(0).toInt() <= 1) {
            e->setErrorType(SkaffariError::InputError);
            e->setErrorText(c->translate("AdminAccount", "You can not remove the last administrator."));
            return ret;
        }

    }

    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM domainadmin WHERE admin_id = :admin_id"));
    q.bindValue(QStringLiteral(":admin_id"), a.id());

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to delete connections between domain manager %1 and associated domains from the database.").arg(a.username()));
        qCCritical(SK_ADMIN, "Failed to delete admin to domain connections for admin %s from the database: %s", qUtf8Printable(a.username()), qUtf8Printable(q.lastError().text()));
        return ret;
    }

    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM settings WHERE admin_id = :admin_id"));
    q.bindValue(QStringLiteral(":admin_id"), a.id());

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to delete administrator settings from database for administrator %1.").arg(a.username()));
        qCCritical(SK_ADMIN, "Failed to delete settings for admin %s from database: %s", qUtf8Printable(a.username()), qUtf8Printable(q.lastError().text()));
        return ret;
    }

    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM adminuser WHERE id = :id"));
    q.bindValue(QStringLiteral(":id"), a.id());

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to delete administrator %1 from database.").arg(a.username()));
        qCCritical(SK_ADMIN, "Failed to delete admin %s from database: %s", qUtf8Printable(a.username()), qUtf8Printable(q.lastError().text()));
        return ret;
    }

    ret = true;

    if (a.type() == 0) {
        qCInfo(SK_ADMIN, "%s removed administrator %s.", qUtf8Printable(Utils::getUserName(c)), qUtf8Printable(a.username()));
    } else {
        qCInfo(SK_ADMIN, "%s removed domain manager %s.", qUtf8Printable(Utils::getUserName(c)), qUtf8Printable(a.username()));
    }

    return ret;
}

dbid_t AdminAccount::setAdminAccount(Cutelyst::Context *c, SkaffariError *error, const QString &user, const QByteArray &pass, qint16 type)
{
    dbid_t id = 0;

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO adminuser (username, password, type, created_at, updated_at) "
                                                         "VALUES (:username, :password, :type, :created_at, :updated_at)"));

    const QDateTime currentUtc = QDateTime::currentDateTimeUtc();
    q.bindValue(QStringLiteral(":username"), user);
    q.bindValue(QStringLiteral(":password"), pass);
    q.bindValue(QStringLiteral(":type"), type);
    q.bindValue(QStringLiteral(":created_at"), currentUtc);
    q.bindValue(QStringLiteral(":updated_at"), currentUtc);

    if (Q_UNLIKELY(!q.exec())) {
        error->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to create administrator account in database."));
        qCCritical(SK_ADMIN) << "Failed to insert new admin account into database." << q.lastError().text();
        return id;
    }

    id = q.lastInsertId().value<dbid_t>();

    return id;
}

bool AdminAccount::rollbackAdminAccount(Cutelyst::Context *c, SkaffariError *error, dbid_t adminId)
{
    bool ret = false;

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM adminuser WHERE id = :id"));
    q.bindValue(QStringLiteral(":id"), adminId);

    if (Q_UNLIKELY(!q.exec())) {
        error->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to revert administrator account changes in database."));
        qCCritical(SK_ADMIN) << "Failed to revert admin account changes in database." << q.lastError().text();
        return ret;
    }

    ret = true;

    return ret;
}

bool AdminAccount::setAdminSettings(Cutelyst::Context *c, SkaffariError *error, dbid_t adminId)
{
    bool ret = false;

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO settings (admin_id) VALUES (:id)"));
    q.bindValue(QStringLiteral(":id"), adminId);

    if (Q_UNLIKELY(!q.exec())) {
        error->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to insert administrator settings into database."));
        qCCritical(SK_ADMIN) << "Failed to insert settings for new administrator into database." << q.lastError().text();
        return ret;
    }

    ret = true;

    return ret;
}

bool AdminAccount::rollbackAdminSettings(Cutelyst::Context *c, SkaffariError *error, dbid_t adminId)
{
    bool ret = false;

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM settings WHERE admin_id = :id"));

    q.addBindValue(adminId);

    if (Q_UNLIKELY(!q.exec())) {
        error->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to revert administrator settings in database."));
        qCCritical(SK_ADMIN) << "Failed to revert admin settings in database." << q.lastError().text();
        return ret;
    }

    ret = true;

    return ret;
}

bool AdminAccount::setAdminDomains(Cutelyst::Context *c, SkaffariError *error, dbid_t adminId, const QList<dbid_t> &domains)
{
    bool ret = false;

    if (Q_LIKELY(!domains.empty())) {

        for (dbid_t domId : domains) {

            QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO domainadmin (domain_id, admin_id) VALUES (:domain_id, :admin_id)"));

            q.bindValue(QStringLiteral(":domain_id"), domId);
            q.bindValue(QStringLiteral(":admin_id"), adminId);

            if (Q_UNLIKELY(!q.exec())) {
                error->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to insert domain manager to domain connection into database."));
                qCCritical(SK_ADMIN) << "Failed to insert omain manager to domain connection into database." << q.lastError().text();
                return ret;
            }

        }

    }

    ret = true;

    return ret;
}

bool AdminAccount::rollbackAdminDomains(Cutelyst::Context *c, SkaffariError *error, dbid_t adminId)
{
    bool ret = false;

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM domainadmin WHERE admin_id = :admin_id"));
    q.bindValue(QStringLiteral(":admin_id"), adminId);

    if (Q_UNLIKELY(!q.exec())) {
        error->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to revert domain manager to domain connections in database."));
        qCCritical(SK_ADMIN) << "Failed to revert domain manager to domain connections in database." << q.lastError().text();
        return ret;
    }

    ret = true;

    return ret;
}

void AdminAccount::toStash(Cutelyst::Context *c, dbid_t adminId)
{
    Q_ASSERT_X(c, "admin account to stash", "invalid context object");

    SkaffariError e(c);
    AdminAccount a = AdminAccount::get(c, &e, adminId);
    if (Q_LIKELY(a.isValid())) {
        c->stash({
                     {QStringLiteral(ADMIN_ACCOUNT_STASH_KEY), QVariant::fromValue<AdminAccount>(a)},
                     {QStringLiteral("site_title"), a.username()}
                 });
    } else {
        c->stash({
                     {QStringLiteral("template"), QStringLiteral("404.html")},
                     {QStringLiteral("site_title"), c->translate("AdminAccount", "Not found")},
                     {QStringLiteral("not_found_text"), c->translate("AdminAccount", "There is no administrator account with database ID %1.").arg(adminId)}
                 });
        c->res()->setStatus(404);
    }
}

AdminAccount AdminAccount::fromStash(Cutelyst::Context *c)
{
    AdminAccount a;

    Q_ASSERT_X(c, "admin account from stash", "invalid context object");

    a = c->stash(QStringLiteral(ADMIN_ACCOUNT_STASH_KEY)).value<AdminAccount>();

    return a;
}

#include "moc_adminaccount.cpp"
