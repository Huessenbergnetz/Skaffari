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

Q_LOGGING_CATEGORY(SK_ADMIN, "skaffari.admin")

#define ADMIN_ACCOUNT_STASH_KEY "adminaccount"

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

void AdminAccount::setType(AdminAccount::AdminAccountType nType)
{
    d->type = nType;
}

void AdminAccount::setType(quint8 nType)
{
    d->type = AdminAccount::getUserType(nType);
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

void AdminAccount::setLang(const QString &lang)
{
    d->lang = lang;
}

QString AdminAccount::tz() const
{
    return d->tz;
}

void AdminAccount::setTz(const QString &tz)
{
    d->tz = tz;
}

void AdminAccount::setTz(const QByteArray &tz)
{
    d->tz = QString::fromLatin1(tz);
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

AdminAccount AdminAccount::create(Cutelyst::Context *c, const QVariantHash &params, SkaffariError *error)
{
    AdminAccount aa;

    Q_ASSERT_X(error, "create new adminaccount", "invalid error object");
    Q_ASSERT_X(c, "create new adminaccount", "invalid context object");
    Q_ASSERT_X(!params.empty(), "create new adminaccount", "params can not be empty");

    const QString username = params.value(QStringLiteral("username")).toString().trimmed();

    // for logging
    const QString errStr = AdminAccount::getUserName(c) + QLatin1String(" failed to create new admin account ") + username;
    const QByteArray errBa = errStr.toUtf8();
    const char *err = errBa.constData();

    const QVariant typeVar = params.value(QStringLiteral("type"));
    if (!typeVar.canConvert<quint8>()) {
        error->setErrorType(SkaffariError::InputError);
        error->setErrorText(c->translate("AdminAccount", "Invalid administrator type."));
        qCWarning(SK_ADMIN, "%s: invalid administrator type.", err);
        return aa;
    }
    const AdminAccount::AdminAccountType type = AdminAccount::getUserType(typeVar);

    if (type == AdminAccount::Disabled) {
        error->setErrorType(SkaffariError::InputError);
        error->setErrorText(c->translate("AdminAccount", "Invalid administrator type."));
        qCWarning(SK_ADMIN, "%s: invalid administrator type.", err);
        return aa;
    }

    if (type >= AdminAccount::getUserType(c)) {
        error->setErrorType(SkaffariError::AuthorizationError);
        error->setErrorText(c->translate("AdminAccount", "You are not allowed to create users of type %1.").arg(AdminAccount::typeToName(type, c)));
        qCWarning(SK_ADMIN, "%s: not allowed to create users of type %u.", type);
        return aa;
    }

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT id FROM adminuser WHERE username = :username"));
    q.bindValue(QStringLiteral(":username"), username);

    if (Q_UNLIKELY(!q.exec())) {
        error->setSqlError(q.lastError(), c->translate("AdminAccount", "Cannot check whether the user name is already assigned."));
        qCCritical(SK_ADMIN, "%s because query to check if username is already in use failed: %s", err, qUtf8Printable(q.lastError().text()));
        return aa;
    }

    if (Q_UNLIKELY(q.next())) {
        error->setErrorType(SkaffariError::InputError);
        error->setErrorText(c->translate("AdminAccount", "This administrator user name is already in use."));
        qCWarning(SK_ADMIN, "%s: username is already in use by ID %lu.", err, q.value(0).value<dbid_t>());
        return aa;
    }

    const QByteArray password = Cutelyst::CredentialPassword::createPassword(params.value(QStringLiteral("password")).toString().toUtf8(),
                                                                             SkaffariConfig::admPwAlgorithm(),
                                                                             SkaffariConfig::admPwRounds(),
                                                                             24,
                                                                             27);

    if (Q_UNLIKELY(password.isEmpty())) {
        error->setErrorType(SkaffariError::ApplicationError);
        error->setErrorText(c->translate("AdminAccount", "Password encryption failed. Please check your encryption settings."));
        qCCritical(SK_ADMIN, "%s: password encryption failed.");
        return aa;
    }

    const QDateTime currentUtc = QDateTime::currentDateTimeUtc();
    const dbid_t id = AdminAccount::setAdminAccount(c, error, username, password, type);
    if (id <= 0) {
        return aa;
    }

    if (!AdminAccount::setAdminSettings(c, error, id)) {
        AdminAccount::rollbackAdminAccount(c, error, id);
        return aa;
    }

    QList<dbid_t> domIds;
    if (type < AdminAccount::Administrator) {
        const QStringList assocdoms = params.value(QStringLiteral("assocdomains")).toStringList();
        for (const QString &did : assocdoms) {
            domIds << did.toULong();
        }
        if (!AdminAccount::setAdminDomains(c, error, id, domIds)) {
            AdminAccount::rollbackAdminDomains(c, error, id);
            AdminAccount::rollbackAdminSettings(c, error, id);
            AdminAccount::rollbackAdminAccount(c, error, id);
            return aa;
        }
    }

    aa.setId(id);
    aa.setUsername(username);
    aa.setType(type);
    aa.setDomains(domIds);
    aa.setLang(SkaffariConfig::defLanguage());
    aa.setTz(SkaffariConfig::defTimezone());
    aa.setTemplate(QStringLiteral("default"));
    aa.setMaxDisplay(25);
    aa.setWarnLevel(90);
    aa.setCreated(currentUtc);
    aa.setUpdated(currentUtc);

    qCInfo(SK_ADMIN, "%s created new admin acccount %s of type %s.", qUtf8Printable(AdminAccount::getUserNameIdString(c)), qUtf8Printable(aa.nameIdString()), AdminAccount::staticMetaObject.enumerator(AdminAccount::staticMetaObject.indexOfEnumerator("AdminAccountType")).valueToKey(type));
    qCDebug(SK_ADMIN) << aa;

    return aa;
}

std::vector<AdminAccount> AdminAccount::list(Cutelyst::Context *c, SkaffariError *error)
{
    std::vector<AdminAccount> list;

    Q_ASSERT_X(c, "list admins", "invalid context object");
    Q_ASSERT_X(error, "list admins", "invalid error object");

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT id, username, type FROM adminuser ORDER BY username ASC"));

    if (Q_UNLIKELY(!q.exec())) {
        error->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to query list of admins from database."));
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

AdminAccount AdminAccount::get(Cutelyst::Context *c, SkaffariError *e, dbid_t id)
{
    AdminAccount acc;

    Q_ASSERT_X(c, "get admin", "invalid context object");
    Q_ASSERT_X(e, "get admin", "invalid error object");
    Q_ASSERT_X(id > 0, "get admin", "invalid database id");

    // for logging
    const QByteArray uniBa = AdminAccount::getUserNameIdString(c).toUtf8();
    const char *uniStr = uniBa.constData();

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT a.username, a.type, s.lang, s.tz, s.template, s.maxdisplay, s.warnlevel, a.created_at, a.updated_at FROM adminuser a JOIN settings s ON a.id = s.admin_id WHERE a.id = :id"));
    q.bindValue(QStringLiteral(":id"), id);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to query administrator account with ID %1 from database.").arg(id));
        qCCritical(SK_ADMIN, "%s failed to query admin account with ID %lu from database: %s", uniStr, id, qUtf8Printable(q.lastError().text()));
        return acc;
    }

    if (Q_UNLIKELY(!q.next())) {
        e->setErrorType(SkaffariError::NotFound);
        e->setErrorText(c->translate("AdminAccount", "Can not find administrator account with database ID %1.").arg(id));
        qCWarning(SK_ADMIN, "%s failed to find admin account with database ID %lu.", uniStr, id);
        return acc;
    }

    acc.setId(id);
    acc.setUsername(q.value(0).toString());
    acc.setType(q.value(1).value<quint8>());
    acc.setLang(q.value(2).toString());
    acc.setTz(q.value(3).toString());
    acc.setTemplate(q.value(4).toString());
    acc.setMaxDisplay(q.value(5).value<quint8>());
    acc.setWarnLevel(q.value(6).value<quint8>());
    QDateTime createdTime = q.value(7).toDateTime();
    createdTime.setTimeSpec(Qt::UTC);
    acc.setCreated(createdTime);
    QDateTime updatedTime = q.value(8).toDateTime();
    updatedTime.setTimeSpec(Qt::UTC);
    acc.setUpdated(updatedTime);

    if (acc.type() < Administrator) {

        q = CPreparedSqlQueryThread(QStringLiteral("SELECT domain_id FROM domainadmin WHERE admin_id = :admin_id"));
        q.bindValue(QStringLiteral(":admin_id"), id);

        if (Q_UNLIKELY(!q.exec())) {
            e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to query domain IDs from database this domain manager is responsible for."));
            qCCritical(SK_ADMIN, "%s failed to query domain IDs admin %s is responsible for: %s", uniStr, qUtf8Printable(acc.nameIdString()), qUtf8Printable(q.lastError().text()));
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

bool AdminAccount::update(Cutelyst::Context *c, SkaffariError *e, const QVariantHash &params)
{
    bool ret = false;

    Q_ASSERT_X(c, "update adminaccount", "invalid context object");
    Q_ASSERT_X(e, "update adminaccount", "invalid error object");
    Q_ASSERT_X(!params.empty(), "update adminaccount", "empty parameters");

    // for logging
    const QString errStr = AdminAccount::getUserNameIdString(c) + QLatin1String(" failed to update admin account ") + nameIdString();
    const QByteArray errBa = errStr.toUtf8();
    const char *err = errBa.constData();

    const QVariant typeVar = params.value(QStringLiteral("type"));

    if (!typeVar.canConvert<quint8>()) {
        e->setErrorType(SkaffariError::InputError);
        e->setErrorText(c->translate("AdminAccount", "Invalid administrator type."));
        qCCritical(SK_ADMIN, "%s: invalid administrator type.", err);
        return ret;
    }

    const AdminAccountType type = AdminAccountData::getUserType(typeVar.value<quint8>());

    if (type >= AdminAccount::getUserType(c)) {
        e->setErrorType(SkaffariError::AuthorizationError);
        e->setErrorText(c->translate("AdminAccount", "You are not allowed to set the type of this account to %1.").arg(AdminAccount::typeToName(type, c)));
        qCCritical(SK_ADMIN, "%s: not allowed to set the type of the account to %u.", type);
        return ret;
    }

    const QString password = params.value(QStringLiteral("password")).toString();

    QSqlQuery q;

    if ((d->type == SuperUser) && (type != SuperUser)) {

        q = CPreparedSqlQueryThread(QStringLiteral("SELECT COUNT(id) FROM adminuser WHERE type = 255"));

        if (Q_UNLIKELY(!(q.exec() && q.next()))) {
            e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to query count of administrators to check if this is the last administrator account."));
            qCCritical(SK_ADMIN, "%s: failed to query count of administrators to check if this is the last administrator account: %s", err, qUtf8Printable(q.lastError().text()));
            return ret;
        }

        if (q.value(0).toInt() <= 1) {
            e->setErrorType(SkaffariError::InputError);
            e->setErrorText(c->translate("AdminAccount", "You can not remove the last administrator."));
            qCWarning(SK_ADMIN, "%s: can not remove the last super user account.", err);
            return ret;
        }
    }

    if (!password.isEmpty()) {
        const QByteArray encPw = Cutelyst::CredentialPassword::createPassword(params.value(QStringLiteral("password")).toString().toUtf8(),
                                                                              SkaffariConfig::admPwAlgorithm(),
                                                                              SkaffariConfig::admPwRounds(),
                                                                              24, 27);
        if (Q_UNLIKELY(encPw.isEmpty())) {
            e->setErrorType(SkaffariError::ApplicationError);
            e->setErrorText(c->translate("AdminAccount", "Password encryption failed. Please check your encryption settings."));
            qCCritical(SK_ADMIN, "%s: password encryption failed.", err);
            return ret;
        }

        q = CPreparedSqlQueryThread(QStringLiteral("UPDATE adminuser SET type = :type, password = :password, updated_at = :updated_at WHERE id = :id"));
        q.bindValue(QStringLiteral(":password"), encPw);

    } else {
        q = CPreparedSqlQueryThread(QStringLiteral("UPDATE adminuser SET type = :type, updated_at = :updated_at WHERE id = :id"));
    }

    const QDateTime currentUtc = QDateTime::currentDateTimeUtc();

    q.bindValue(QStringLiteral(":type"), static_cast<quint8>(type));
    q.bindValue(QStringLiteral(":updated_at"), currentUtc);
    q.bindValue(QStringLiteral(":id"), d->id);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to update administrator account in database."));
        qCCritical(SK_ADMIN, "%s: failed to update database entry: %s", err, qUtf8Printable(q.lastError().text()));;
        return ret;
    }

    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM domainadmin WHERE admin_id = :id"));
    q.bindValue(QStringLiteral(":id"), d->id);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to update domain manager to domain connections in database."));
        qCCritical(SK_ADMIN, "%s: failed to update connections between domain manager and domains in database: %s", err, qUtf8Printable(q.lastError().text()));
        return ret;
    }

    d->domains.clear();

    if (type < AdminAccount::Administrator) {
        const QStringList domains = params.value(QStringLiteral("assocdomains")).toStringList();

        for (const QString &adom : domains) {
            const dbid_t did = adom.toULong();
            q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO domainadmin (domain_id, admin_id) VALUES (:domain_id, :admin_id)"));
            q.bindValue(QStringLiteral(":domain_id"), QVariant::fromValue<dbid_t>(did));
            q.bindValue(QStringLiteral(":admin_id"), d->id);
            if (Q_UNLIKELY(!q.exec())) {
                e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to update domain manager to domain connections in database."));
                qCCritical(SK_ADMIN, "%s: failed to update connections between domain manager and domains in database: %s", err, qUtf8Printable(q.lastError().text()));
                return ret;
            }
            d->domains << did;
        }
    }

    d->type = type;
    d->updated = currentUtc;

    ret = true;

    qCInfo(SK_ADMIN, "%s updated admin account %s of type %s.", qUtf8Printable(AdminAccount::getUserNameIdString(c)), qUtf8Printable(nameIdString()), AdminAccount::staticMetaObject.enumerator(AdminAccount::staticMetaObject.indexOfEnumerator("AdminAccountType")).valueToKey(type));
    qCDebug(SK_ADMIN) << *this;

    return ret;
}

bool AdminAccount::updateOwn(Cutelyst::Context *c, SkaffariError *e, const QVariantHash &p)
{
    bool ret = false;

    Q_ASSERT_X(c, "update own account", "invalid context object");
    Q_ASSERT_X(e, "update own account", "invalid error object");
    Q_ASSERT_X(!p.empty(), "update own account", "empty parameters");

    // for logging
    const QString errStr = AdminAccount::getUserNameIdString(c) + QLatin1String(" failed to update own account");
    const QByteArray errBa = errStr.toUtf8();
    const char *err = errBa.constData();

    if (d->id != Cutelyst::Authentication::user(c).id().value<dbid_t>()) {
        e->setErrorType(SkaffariError::AuthorizationError);
        e->setErrorText(c->translate("AdminAccount", "You are not allowed to change this administrator account."));
        qCWarning(SK_ADMIN, "%s: access denied.", err);
        return ret;
    }

    const QString password = p.value(QStringLiteral("password")).toString();
    const QDateTime currentUtc = QDateTime::currentDateTimeUtc();
    const QString tz = p.value(QStringLiteral("tz"), d->tz).toString();
    const QString langCode = p.value(QStringLiteral("lang"), d->lang).toString();
    const QLocale lang(langCode);

    if (lang.language() == QLocale::C) {
        e->setErrorType(SkaffariError::InputError);
        e->setErrorText(c->translate("AdminAccount", "%1 is not a valid locale code.").arg(langCode));
        qCWarning(SK_ADMIN, "%s: invalid locale code %s.", err, qUtf8Printable(langCode));
        return ret;
    }

    QTimeZone timeZone(tz.toLatin1());
    if (!timeZone.isValid()) {
        e->setErrorType(SkaffariError::InputError);
        e->setErrorText(c->translate("AdminAccount", "%1 is not a valid IANA time zone ID.").arg(tz));
        qCWarning(SK_ADMIN, "%s: invalid IANA time zone ID %s.", err, tz.constData());
        return ret;
    }

    QSqlQuery q;

    if (!password.isEmpty()) {
        const QByteArray encPw = Cutelyst::CredentialPassword::createPassword(password.toUtf8(),
                                                                              SkaffariConfig::admPwAlgorithm(),
                                                                              SkaffariConfig::admPwRounds(),
                                                                              24,
                                                                              27);

        if (Q_UNLIKELY(encPw.isEmpty())) {
            e->setErrorType(SkaffariError::ApplicationError);
            e->setErrorText(c->translate("AdminAccount", "Password encryption failed. Please check your encryption settings."));
            qCCritical(SK_ADMIN, "%s: password encryption failed.");
            return ret;
        }

        q = CPreparedSqlQueryThread(QStringLiteral("UPDATE adminuser SET password = :password, updated_at = :updated_at WHERE id = :id"));
        q.bindValue(QStringLiteral(":password"), encPw);
    } else {
        q = CPreparedSqlQueryThread(QStringLiteral("UPDATE adminuser SET updated_at = :updated_at WHERE id = :id"));
    }

    q.bindValue(QStringLiteral(":updated_at"), currentUtc);
    q.bindValue(QStringLiteral(":id"), d->id);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to update administrator in database."));
        qCCritical(SK_ADMIN, "%s: update account in database failed: %s", err, qUtf8Printable(q.lastError().text()));
        return ret;
    }

    const quint8 maxdisplay = p.value(QStringLiteral("maxdisplay"), d->maxDisplay).value<quint8>();
    const quint8 warnlevel = p.value(QStringLiteral("warnlevel"), d->warnLevel).value<quint8>();

    q = CPreparedSqlQueryThread(QStringLiteral("UPDATE settings SET maxdisplay = :maxdisplay, warnlevel = :warnlevel, lang = :lang, tz = :tz WHERE admin_id = :admin_id"));
    q.bindValue(QStringLiteral(":maxdisplay"), maxdisplay);
    q.bindValue(QStringLiteral(":warnlevel"), warnlevel);
    q.bindValue(QStringLiteral(":lang"), lang.name());
    q.bindValue(QStringLiteral(":admin_id"), d->id);
    q.bindValue(QStringLiteral(":tz"), tz);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to update administrator settings in database."));
        qCCritical(SK_ADMIN, "%s: update settings in database failed: %s", err, qUtf8Printable(q.lastError().text()));
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

bool AdminAccount::remove(Cutelyst::Context *c, SkaffariError *e)
{
    bool ret = false;

    // for logging
    const QString errStr = AdminAccount::getUserNameIdString(c) + QLatin1String(" failed to remove admin acccount ") + nameIdString();
    const QByteArray errBa = errStr.toUtf8();
    const char *err = errBa.constData();

    if (d->type <= AdminAccount::getUserType(c)) {
        e->setErrorType(SkaffariError::AuthorizationError);
        e->setErrorText(c->translate("AdminAccount", "You are not allowed to remove accounts of type %1.").arg(typeName(c)));
        qCWarning(SK_ADMIN, "%s: not allowed to remove accounts of type %u.", err, d->type);
        return ret;
    }

    QSqlQuery q;

    if (isSuperUser()) {

        q = CPreparedSqlQueryThread(QStringLiteral("SELECT COUNT(id) FROM adminuser WHERE type = 255"));

        if (Q_UNLIKELY(!(q.exec() && q.next()))) {
            e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to query count of super users to check if this is the last super user account."));
            qCCritical(SK_ADMIN, "%s: query to count current super user accounts failed: %s", err, qUtf8Printable(q.lastError().text()));
            return ret;
        }

        if (q.value(0).toInt() <= 1) {
            e->setErrorType(SkaffariError::InputError);
            e->setErrorText(c->translate("AdminAccount", "You can not remove the last super user."));
            qCWarning(SK_ADMIN, "%s: can not remove last super user account.", err);
            return ret;
        }

    }

    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM settings WHERE admin_id = :admin_id"));
    q.bindValue(QStringLiteral(":admin_id"), d->id);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to delete administrator settings from database for administrator %1.").arg(d->username));
        qCCritical(SK_ADMIN, "%s: can not delete settings from database: %s", err, qUtf8Printable(q.lastError().text()));
        return ret;
    }

    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM domainadmin WHERE admin_id = :admin_id"));
    q.bindValue(QStringLiteral(":admin_id"), d->id);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to delete connections between domain manager %1 and associated domains from the database.").arg(d->username));
        qCCritical(SK_ADMIN, "%s: can not delete admin to domain connections from database: %s", err, qUtf8Printable(q.lastError().text()));
        return ret;
    }

    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM adminuser WHERE id = :id"));
    q.bindValue(QStringLiteral(":id"), d->id);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("AdminAccount", "Failed to delete administrator %1 from database.").arg(d->username));
        qCCritical(SK_ADMIN, "%s: can not delete admin from database: %s", err, qUtf8Printable(q.lastError().text()));
        return ret;
    }

    ret = true;
    qCInfo(SK_ADMIN, "%s removed admin %s of type %s.", qUtf8Printable(AdminAccount::getUserNameIdString(c)), qUtf8Printable(nameIdString()), AdminAccount::staticMetaObject.enumerator(AdminAccount::staticMetaObject.indexOfEnumerator("AdminAccountType")).valueToKey(d->type));
    qCDebug(SK_ADMIN) << *this;

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
    return c->stash(QStringLiteral("user")).value<AdminAccount>();
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
    return c->stash(QStringLiteral("user")).value<AdminAccount>().type();
}

AdminAccount::AdminAccountType AdminAccount::getUserType(const QVariant &type)
{
    return AdminAccountData::getUserType(type.value<quint8>());
}

dbid_t AdminAccount::getUserId(Cutelyst::Context *c)
{
    return c->stash(QStringLiteral("user")).value<AdminAccount>().id();
}

QString AdminAccount::getUserName(Cutelyst::Context *c)
{
    return c->stash(QStringLiteral("user")).value<AdminAccount>().username();
}

QString AdminAccount::getUserNameIdString(Cutelyst::Context *c)
{
    QString ret;
    const AdminAccount a = c->stash(QStringLiteral("user")).value<AdminAccount>();
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

QStringList AdminAccount::allowedTypes(Cutelyst::Context *c)
{
    QStringList lst;

    const QMetaEnum me = QMetaEnum::fromType<AdminAccount::AdminAccountType>();

    if (me.isValid() && (me.keyCount() > 0)) {
        const int userType = static_cast<int>(AdminAccount::getUserType(c));
        for (int i = 0; i < me.keyCount(); ++i) {
            const auto type = me.value(i);
            if ((type != 0) && ((userType == 255) || (type < userType))) {
                lst << QString::number(type);
            }
        }
    }

    return lst;
}

AdminAccount::AdminAccountType AdminAccount::maxAllowedType(Cutelyst::Context *c)
{
    AdminAccount::AdminAccountType max = AdminAccount::Disabled;

    const auto userType = AdminAccount::getUserType(c);

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

#include "moc_adminaccount.cpp"
