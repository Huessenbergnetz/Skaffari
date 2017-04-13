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
#include "../utils/timeutils.h"
#include <QSqlQuery>
#include <QSqlError>
#include <Cutelyst/Context>
#include <Cutelyst/Plugins/Utils/Sql>
#include <Cutelyst/Plugins/Authentication/credentialpassword.h>
#include <Cutelyst/Plugins/Authentication/authentication.h>
#include <Cutelyst/Plugins/Session/Session>
#include <Cutelyst/Response>
#include <QStringList>

AdminAccount::AdminAccount() :
    d(new AdminAccountData)
{

}

AdminAccount::AdminAccount(quint32 id, const QString& username, qint16 type, const QList<quint32> &domains) :
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


quint32 AdminAccount::getId() const
{
    return d->id;
}


void AdminAccount::setId(quint32 id)
{
    d->id = id;
}


QString AdminAccount::getUsername() const
{
	return d->username;
}



void AdminAccount::setUsername(const QString& nUsername)
{
	d->username = nUsername;
}



qint16 AdminAccount::getType() const
{
	return d->type;
}



void AdminAccount::setType(qint16 nType)
{
    d->type = nType;
}



QList<quint32> AdminAccount::getDomains() const
{
	return d->domains;
}



void AdminAccount::setDomains(const QList<quint32> &nDomains)
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


QString AdminAccount::getLang() const
{
    return d->lang;
}


void AdminAccount::setLang(const QString &lang)
{
    d->lang = lang;
}


QByteArray AdminAccount::getTz() const
{
    return d->tz;
}


void AdminAccount::setTz(const QByteArray &tz)
{
    d->tz = tz;
}


QDateTime AdminAccount::getCreated() const
{
    return d->created;
}


void AdminAccount::setCreated(const QDateTime &created)
{
    d->created = created;
}

QDateTime AdminAccount::getUpdated() const
{
    return d->updated;
}


void AdminAccount::setUpdated(const QDateTime &updated)
{
    d->updated = updated;
}


quint8 AdminAccount::getMaxDisplay() const
{
    return d->maxDisplay;
}

void AdminAccount::setMaxDisplay(quint8 maxDisplay)
{
    d->maxDisplay = maxDisplay;
}

quint8 AdminAccount::getWarnLevel() const
{
    return d->warnLevel;
}

void AdminAccount::setWarnLevel(quint8 warnLevel)
{
    d->warnLevel = warnLevel;
}

QString AdminAccount::getTemplate() const
{
    return d->tmpl;
}


void AdminAccount::setTemplate(const QString &tmpl)
{
    d->tmpl = tmpl;
}

AdminAccount AdminAccount::create(Cutelyst::Context *c, const Cutelyst::ParamsMultiMap &params, SkaffariError *error, QCryptographicHash::Algorithm algorithm, quint32 iterations)
{
    AdminAccount aa;

    Q_ASSERT_X(error, "create new adminaccount", "invalid error object");
    Q_ASSERT_X(c, "create new adminaccount", "invalid context object");
    Q_ASSERT_X(!params.empty(), "create new adminaccount", "params can not be empty");

    const QString username = params.value(QStringLiteral("username")).trimmed();

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT id FROM adminuser WHERE username = :username"));
    q.bindValue(QStringLiteral(":username"), username);

    if (Q_UNLIKELY(!q.exec())) {
        error->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to check if user name is already in use."));
        return aa;
    }

    if (Q_UNLIKELY(q.next())) {
        error->setErrorType(SkaffariError::InputError);
        error->setErrorText(c->translate("AdminAccount", "This admin user name is already in use."));
        return aa;
    }

    const QByteArray password = Cutelyst::CredentialPassword::createPassword(params.value(QStringLiteral("password")).toUtf8(), algorithm, iterations, 24, 27);
    const qint16 type = params.value(QStringLiteral("type")).toShort();

    const quint32 id = AdminAccount::setAdminAccount(c, error, params.value(QStringLiteral("username")), password, type);
    if (id <= 0) {
        return aa;
    }

    if (!AdminAccount::setAdminSettings(c, error, id)) {
        AdminAccount::rollbackAdminAccount(c, error, id);
        return aa;
    }

    QList<quint32> domIds;
    QStringList assocdoms = params.values(QStringLiteral("assocdomains"));
    for (const QString &did : assocdoms) {
        domIds << did.toULong();
    }
    if (!AdminAccount::setAdminDomains(c, error, id, domIds)) {
        AdminAccount::rollbackAdminDomains(c, error, id);
        AdminAccount::rollbackAdminSettings(c, error, id);
        AdminAccount::rollbackAdminAccount(c, error, id);
        return aa;
    }

    const QDateTime currentTimeUtc = QDateTime::currentDateTimeUtc();
    const QDateTime userTime = TimeUtils::toUserTZ(c, currentTimeUtc);

    aa.setId(id);
    aa.setUsername(username);
    aa.setType(type);
    aa.setDomains(domIds);
    aa.setLang(QStringLiteral("en"));
    aa.setTz(QByteArrayLiteral("UTC"));
    aa.setTemplate(QStringLiteral("default"));
    aa.setMaxDisplay(25);
    aa.setWarnLevel(90);
    aa.setCreated(userTime);
    aa.setUpdated(userTime);

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
        return list;
    }

    while (q.next()) {
        list.append(AdminAccount(q.value(0).value<quint32>(),
                                 q.value(1).toString(),
                                 static_cast<AdminAccount::AdminAccountType>(q.value(2).value<qint8>()),
                                 QList<quint32>()));
    }

    return list;
}



AdminAccount AdminAccount::get(Cutelyst::Context *c, SkaffariError *e, quint32 id)
{
    AdminAccount acc;

    Q_ASSERT_X(c, "get admin", "invalid context object");
    Q_ASSERT_X(e, "get admin", "invalid error object");
    Q_ASSERT_X(id > 0, "get admin", "invalid database id");

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT a.username, a.type, s.lang, s.tz, s.template, s.maxdisplay, s.warnlevel, a.created_at, a.updated_at FROM adminuser a JOIN settings s ON a.id = s.admin_id WHERE a.id = :id"));
    q.bindValue(QStringLiteral(":id"), id);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to query admin account with ID %1 from database.").arg(id));
        return acc;
    }

    if (Q_UNLIKELY(!q.next())) {
        e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to query admin account with ID %1 from database.").arg(id));
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
    acc.setCreated(TimeUtils::toUserTZ(c, q.value(7).toDateTime()));
    acc.setUpdated(TimeUtils::toUserTZ(c, q.value(8).toDateTime()));

    if (acc.getType() != AdminAccountType::SuperUser) {

        q = CPreparedSqlQueryThread(QStringLiteral("SELECT domain_id FROM domainadmin WHERE admin_id = :admin_id"));
        q.bindValue(QStringLiteral(":admin_id"), id);

        if (Q_UNLIKELY(!q.exec())) {
            e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to query domain IDs from database this admin is responsible for."));
            return acc;
        }

        QList<quint32> doms;
        while (q.next()) {
            doms.append(q.value(0).value<quint32>());
        }

        acc.setDomains(doms);
    }

    return acc;
}



bool AdminAccount::update(Cutelyst::Context *c, SkaffariError *e, AdminAccount *a, const Cutelyst::ParamsMultiMap &params, QCryptographicHash::Algorithm algorithm, quint32 iterations)
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
        q.exec();

        if (!q.next()) {
            e->setErrorType(SkaffariError::InputError);
            e->setErrorText(c->translate("AdminAccount", "You can not remove the last super user."));
            return ret;
        }

        if (q.value(0).toInt() <= 1) {
            e->setErrorType(SkaffariError::InputError);
            e->setErrorText(c->translate("AdminAccount", "You can not remove the last super user."));
            return ret;
        }
    }

    if (!password.isEmpty()) {
        q = CPreparedSqlQueryThread(QStringLiteral("UPDATE adminuser SET type = :type, password = :password, updated_at = :updated_at WHERE id = :id"));
        q.bindValue(QStringLiteral(":password"), Cutelyst::CredentialPassword::createPassword(params.value(QStringLiteral("password")).toUtf8(), algorithm, iterations, 24, 27));
    } else {
        q = CPreparedSqlQueryThread(QStringLiteral("UPDATE adminuser SET type = :type, updated_at = :updated_at WHERE id = :id"));
    }

    const QDateTime currentUtc = QDateTime::currentDateTimeUtc();

    q.bindValue(QStringLiteral(":type"), type);
    q.bindValue(QStringLiteral(":updated_at"), currentUtc);
    q.bindValue(QStringLiteral(":id"), a->getId());

    if (!q.exec()) {
        e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to update admin account in database."));
        return ret;
    }

    a->setType(type);
    a->setUpdated(TimeUtils::toUserTZ(c, currentUtc));

    QStringList domains;
    if (a->getType() != AdminAccount::SuperUser) {
        domains = params.values(QStringLiteral("assocdomains"));
    }

    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM domainadmin WHERE admin_id = :id"));
    q.bindValue(QStringLiteral(":id"), a->getId());

    if (!q.exec()) {
        e->setSqlError(q.lastError(), c->translate("AdminAccount", "Faild to update admin to domain connections in database."));
        return ret;
    }

    QList<quint32> domIds;
    if (!domains.isEmpty()) {
        for (int i = 0; i < domains.size(); ++i) {
            const quint32 did = domains.at(i).toULong();
            q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO domainadmin (domain_id, admin_id) VALUES (:domain_id, :admin_id)"));
            q.bindValue(QStringLiteral(":domain_id"), QVariant::fromValue<quint32>(did));
            q.bindValue(QStringLiteral(":admin_id"), a->getId());
            if (!q.exec()) {
                e->setSqlError(q.lastError(), c->translate("AdminAccount", "Faild to update admin to domain connections in database."));
                return ret;
            }
            domIds << did;
        }
    }

    a->setDomains(domIds);

    ret = true;

    return ret;
}


bool AdminAccount::update(Cutelyst::Context *c, SkaffariError *e, AdminAccount *a, Cutelyst::AuthenticationUser *u, const Cutelyst::ParamsMultiMap &p, QCryptographicHash::Algorithm algorithm, quint32 iterations)
{
    bool ret = false;

    Q_ASSERT_X(c, "update own account", "invalid context object");
    Q_ASSERT_X(e, "update own account", "invalid error object");
    Q_ASSERT_X(!p.empty(), "update own account", "empty parameters");
    Q_ASSERT_X(u, "update own accountt", "invalid authentication user object");

    const quint32 id = u->id().toULong();
    const QString password = p.value(QStringLiteral("password"));
    const QDateTime currentUtc = QDateTime::currentDateTimeUtc();

    QSqlQuery q;

    if (!password.isEmpty()) {
        q = CPreparedSqlQueryThread(QStringLiteral("UPDATE adminuser SET password = :password, updated_at = :updated_at WHERE id = :id"));
        q.bindValue(QStringLiteral(":password"), Cutelyst::CredentialPassword::createPassword(p.value(QStringLiteral("password")).toUtf8(), algorithm, iterations, 24, 27));
    } else {
        q = CPreparedSqlQueryThread(QStringLiteral("UPDATE adminuser SET updated_at = :updated_at WHERE id = :id"));
    }

    q.bindValue(QStringLiteral(":updated_at"), currentUtc);
    q.bindValue(QStringLiteral(":id"), QVariant::fromValue<quint32>(id));

    if (!q.exec()) {
        e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to update admin in database."));
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

    if (!q.exec()) {
        e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to update admin settings in database."));
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
    a->setUpdated(TimeUtils::toUserTZ(c, currentUtc));

    c->stash({
                 {QStringLiteral("userMaxDisplay"), maxdisplay},
                 {QStringLiteral("userWarnLevel"), warnlevel},
                 {QStringLiteral("lang"), lang}
             });

    ret = true;

    return ret;
}


bool AdminAccount::remove(Cutelyst::Context *c, SkaffariError *e, const AdminAccount &a)
{
    bool ret = false;

    QSqlQuery q;

    if (a.isSuperUser()) {

        q = CPreparedSqlQueryThread(QStringLiteral("SELECT COUNT(id) FROM adminuser WHERE type = 0"));
        q.exec();

        if (!q.next()) {
            e->setErrorType(SkaffariError::InputError);
            e->setErrorText(c->translate("AdminAccount", "You can not remove the last super user."));
            return ret;
        }

        if (q.value(0).toInt() <= 1) {
            e->setErrorType(SkaffariError::InputError);
            e->setErrorText(c->translate("AdminAccount", "You can not remove the last super user."));
            return ret;
        }

    }

    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM domainadmin WHERE admin_id = :admin_id"));
    q.bindValue(QStringLiteral(":admin_id"), a.getId());

    if (!q.exec()) {
        e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to delete admin to domain connections from database for admin %1.").arg(a.getUsername()));
        return ret;
    }

    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM settings WHERE admin_id = :admin_id"));
    q.bindValue(QStringLiteral(":admin_id"), a.getId());

    if (!q.exec()) {
        e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to delete admin settings from database for admin %1.").arg(a.getUsername()));
        return ret;
    }

    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM adminuser WHERE id = :id"));
    q.bindValue(QStringLiteral(":id"), a.getId());

    if (!q.exec()) {
        e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to delete admin %1 from database.").arg(a.getUsername()));
        return ret;
    }

    ret = true;

    return ret;
}


quint32 AdminAccount::setAdminAccount(Cutelyst::Context *c, SkaffariError *error, const QString &user, const QByteArray &pass, qint16 type)
{
    quint32 id = 0;

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO adminuser (username, password, type, created_at, updated_at) "
                                                         "VALUES (:username, :password, :type, :created_at, :updated_at)"));

    const QDateTime currentUtc = QDateTime::currentDateTimeUtc();
    q.bindValue(QStringLiteral(":username"), user);
    q.bindValue(QStringLiteral(":password"), pass);
    q.bindValue(QStringLiteral(":type"), type);
    q.bindValue(QStringLiteral(":created_at"), currentUtc);
    q.bindValue(QStringLiteral(":updated_at"), currentUtc);

    if (!q.exec()) {
        error->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to create admin account in database."));
        return id;
    }

    id = q.lastInsertId().value<quint32>();

    return id;
}


bool AdminAccount::rollbackAdminAccount(Cutelyst::Context *c, SkaffariError *error, quint32 adminId)
{
    bool ret = false;

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM adminuser WHERE id = :id"));
    q.bindValue(QStringLiteral(":id"), adminId);

    if (!q.exec()) {
        error->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to revert admin account changes in database."));
        return ret;
    }

    ret = true;

    return ret;
}


bool AdminAccount::setAdminSettings(Cutelyst::Context *c, SkaffariError *error, quint32 adminId)
{
    bool ret = false;

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO settings (admin_id) VALUES (:id)"));
    q.bindValue(QStringLiteral(":id"), adminId);

    if (!q.exec()) {
        error->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to insert admin settings into database."));
        return ret;
    }

    ret = true;

    return ret;
}


bool AdminAccount::rollbackAdminSettings(Cutelyst::Context *c, SkaffariError *error, quint32 adminId)
{
    bool ret = false;

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM settings WHERE admin_id = :id"));

    q.addBindValue(adminId);

    if (!q.exec()) {
        error->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to revert admin settings in database."));
        return ret;
    }

    ret = true;

    return ret;
}


bool AdminAccount::setAdminDomains(Cutelyst::Context *c, SkaffariError *error, quint32 adminId, const QList<quint32> &domains)
{
    bool ret = false;

    if (Q_LIKELY(!domains.empty())) {

        for (quint32 domId : domains) {

            QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO domainadmin (domain_id, admin_id) VALUES (:domain_id, :admin_id)"));

            q.bindValue(QStringLiteral(":domain_id"), domId);
            q.bindValue(QStringLiteral(":admin_id"), adminId);

            if (!q.exec()) {
                error->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to insert admin to domain connection into database."));
                return ret;
            }

        }

    }

    ret = true;

    return ret;
}


bool AdminAccount::rollbackAdminDomains(Cutelyst::Context *c, SkaffariError *error, quint32 adminId)
{
    bool ret = false;

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM domainadmin WHERE admin_id = :admin_id"));
    q.bindValue(QStringLiteral(":admin_id"), adminId);

    if (!q.exec()) {
        error->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to revert admin to domain connections in database."));
        return ret;
    }

    ret = true;

    return ret;
}



void AdminAccount::toStash(Cutelyst::Context *c, quint32 adminId)
{
    Q_ASSERT_X(c, "admin account to stash", "invalid context object");

    SkaffariError e(c);
    AdminAccount a = AdminAccount::get(c, &e, adminId);
    if (Q_LIKELY(a.isValid())) {
        c->stash({
                     {QStringLiteral("adminaccount"), QVariant::fromValue<AdminAccount>(a)},
                     {QStringLiteral("site_title"), a.getUsername()}
                 });
    } else {
        c->stash({
                     {QStringLiteral("template"), QStringLiteral("404.html")},
                     {QStringLiteral("site_title"), c->translate("AdminAccount", "Not found")},
                     {QStringLiteral("not_found_text"), c->translate("AdminAccount", "Tehe is no admin account with database ID %1.").arg(adminId)}
                 });
        c->res()->setStatus(404);
    }
}


AdminAccount AdminAccount::fromStash(Cutelyst::Context *c)
{
    AdminAccount a;

    Q_ASSERT_X(c, "admin account from stash", "invalid context object");

    a = c->stash(QStringLiteral("adminaccount")).value<AdminAccount>();

    return a;
}
