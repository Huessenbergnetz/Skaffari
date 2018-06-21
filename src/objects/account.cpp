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

#include "account_p.h"
#include "skaffarierror.h"
#include "emailaddress.h"
#include "adminaccount.h"
#include "../utils/utils.h"
#include "../imap/skaffariimap.h"
#include "../../common/password.h"
#include "../utils/skaffariconfig.h"
#include <Cutelyst/Context>
#include <Cutelyst/Plugins/Utils/Sql>
#include <Cutelyst/Response>
#include <Cutelyst/Plugins/Session/Session>
#include <Cutelyst/Plugins/Memcached/Memcached>
#include <Cutelyst/Plugins/Utils/validatoremail.h>
#include <QTimeZone>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QRegularExpression>
#include <QUrl>
#include <QStringList>
#include <QCollator>
#include <QJsonArray>
#include <QJsonValue>
#include <QLocale>

Q_LOGGING_CATEGORY(SK_ACCOUNT, "skaffari.account")

#define ACCOUNT_STASH_KEY "account"
#define PAM_ACCT_EXPIRED 1
#define PAM_NEW_AUTHTOK_REQD 2

#define MEMC_QUOTA_EXP 900
#define MEMC_QUOTA_KEY QLatin1String("sk_quotausage_")

Account::Account() :
    d(new AccountData)
{

}

Account::Account(dbid_t id, dbid_t domainId, const QString& username, bool imap, bool pop, bool sieve, bool smtpauth, const QStringList &addresses, const QStringList &forwards, quota_size_t quota, quota_size_t usage, const QDateTime &created, const QDateTime &updated, const QDateTime &validUntil, const QDateTime &pwdExpiration, bool keepLocal, bool catchAll, quint8 status) :
    d(new AccountData(id, domainId, username, imap, pop, sieve, smtpauth, addresses, forwards, quota, usage, created, updated, validUntil, pwdExpiration, keepLocal, catchAll, status))
{

}

Account::Account(const Account &other) :
    d(other.d)
{

}

Account::Account(Account &&other) noexcept :
    d(std::move(other.d))
{
    other.d = nullptr;
}

Account& Account::operator=(const Account &other)
{
    d = other.d;
    return *this;
}

Account& Account::operator=(Account &&other) noexcept
{
    swap(other);
    return *this;
}

Account::~Account()
{

}

void Account::swap(Account &other) noexcept
{
    std::swap(d, other.d);
}

dbid_t Account::id() const
{
    return d->id;
}

dbid_t Account::domainId() const
{
    return d->domainId;
}

QString Account::username() const
{
    return d->username;
}

QString Account::nameIdString() const
{
    QString ret;

    ret = d->username + QLatin1String(" (ID: ") + QString::number(d->id) + QLatin1Char(')');

    return ret;
}

bool Account::isImapEnabled() const
{
    return d->imap;
}

bool Account::isPopEnabled() const
{
    return d->pop;
}

bool Account::isSieveEnabled() const
{
    return d->sieve;
}

bool Account::isSmtpauthEnabled() const
{
    return d->smtpauth;
}

QStringList Account::addresses() const
{
    return d->addresses;
}

QStringList Account::forwards() const
{
    return d->forwards;
}

quota_size_t Account::quota() const
{
    return d->quota;
}

quota_size_t Account::usage() const
{
    return d->usage;
}

float Account::usagePercent() const
{
    if ((quota() == 0) && (usage() == 0)) {
        return 0;
    }
    return ((float)usage() / (float)quota()) * (float)100;
}

bool Account::isValid() const
{
    return ((d->id > 0) && (d->domainId > 0));
}

QDateTime Account::created() const
{
    return d->created;
}

QDateTime Account::updated() const
{
    return d->updated;
}

QDateTime Account::validUntil() const
{
    return d->validUntil;
}

bool Account::keepLocal() const
{
    return d->keepLocal;
}

bool Account::catchAll() const
{
    return d->catchAll;
}

QDateTime Account::passwordExpires() const
{
    return d->passwordExpires;
}

bool Account::passwordExpired() const
{
    return (d->passwordExpires < QDateTime::currentDateTimeUtc());
}

bool Account::expired() const
{
    return (d->validUntil < QDateTime::currentDateTimeUtc());
}

quint8 Account::status() const
{
    return d->status;
}

QJsonObject Account::toJson() const
{
    QJsonObject ao;

    ao.insert(QStringLiteral("id"), static_cast<qint64>(d->id));
    ao.insert(QStringLiteral("domainId"), static_cast<qint64>(d->domainId));
    ao.insert(QStringLiteral("username"), d->username);
    ao.insert(QStringLiteral("imap"), d->imap);
    ao.insert(QStringLiteral("pop"), d->pop);
    ao.insert(QStringLiteral("sieve"), d->sieve);
    ao.insert(QStringLiteral("smtpauth"), d->smtpauth);
    ao.insert(QStringLiteral("addresses"), QJsonArray::fromStringList(d->addresses));
    ao.insert(QStringLiteral("forwards"), QJsonArray::fromStringList(d->forwards));
    ao.insert(QStringLiteral("quota"), static_cast<qint64>(d->quota));
    ao.insert(QStringLiteral("usage"), static_cast<qint64>(d->usage));
    ao.insert(QStringLiteral("created"), d->created.toString(Qt::ISODate));
    ao.insert(QStringLiteral("updated"), d->updated.toString(Qt::ISODate));
    ao.insert(QStringLiteral("validUntil"), d->validUntil.toString(Qt::ISODate));
    ao.insert(QStringLiteral("passwordExpires"), d->passwordExpires.toString(Qt::ISODate));
    ao.insert(QStringLiteral("passwordExpired"), passwordExpired());
    ao.insert(QStringLiteral("keepLocal"), d->keepLocal);
    ao.insert(QStringLiteral("catchAll"), d->catchAll);
    ao.insert(QStringLiteral("expired"), expired());
    ao.insert(QStringLiteral("status"), d->status);

    return ao;
}

/*!
 * \internal
 * \brief Returns \c true if the \a alias already exists in the database, otherwise \c false.
 *
 * \q sqlError will contain information about occured errors whil performing the query.
 */
bool aliasExists(const QString &alias, QSqlError &sqlError)
{
    bool ret = false;

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT username FROM virtual WHERE alias = :alias"));
    q.bindValue(QStringLiteral(":alias"), alias);

    if (Q_LIKELY(q.exec())) {
        ret = q.next();
    } else {
        sqlError = q.lastError();
        qCCritical(SK_ACCOUNT, "The database query to check if the email address %s already exists failed: %s", qUtf8Printable(alias), qUtf8Printable(sqlError.text()));
    }

    return ret;
}

/*!
 * \internal
 * \brief Inserts a new column into the virtual table and returns the the new ID.
 *
 * If insertion fails, the returned ID will be \c 0.
 */
dbid_t insertVirtual(dbid_t idn_id, dbid_t ace_id, const QString &alias, const QString &dest, const QString &username, int status, QSqlError &error)
{
    dbid_t ret = 0;

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO virtual (idn_id, ace_id, alias, dest, username, status) "
                                                         "VALUES (:idn_id, :ace_id, :alias, :dest, :username, :status)"));
    q.bindValue(QStringLiteral(":idn_id"), idn_id);
    q.bindValue(QStringLiteral(":ace_id"), ace_id);
    q.bindValue(QStringLiteral(":alias"), alias);
    q.bindValue(QStringLiteral(":dest"), dest);
    q.bindValue(QStringLiteral(":username"), username);
    q.bindValue(QStringLiteral(":status"), status);

    if (Q_LIKELY(q.exec())) {
        ret = q.lastInsertId().value<dbid_t>();
    } else {
        error = q.lastError();
    }

    return ret;
}

/*!
 * \internal
 * \brief Removes all columns from the virtual table that are asscociated to \a username.
 */
QSqlError removeVirtual(const QString &username)
{
    QSqlError ret;

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM virtual WHERE username = :username"));
    q.bindValue(QStringLiteral(":username"), username);
    if (Q_UNLIKELY(!q.exec())) {
        ret = q.lastError();
        qCCritical(SK_ACCOUNT, "Failed to remove all entries for username \"%s\" from the virtual table: %s", qUtf8Printable(username), qUtf8Printable(ret.text()));
    }

    return ret;
}

/*!
 * \internal
 * \brief Removes the column identified by \a id from the virtual table.
 */
QSqlError removeVirtualByID(dbid_t id)
{
    QSqlError ret;

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM virtual WHERE id = :id"));
    q.bindValue(QStringLiteral(":id"), id);
    if (Q_UNLIKELY(!q.exec())) {
        ret = q.lastError();
        qCCritical(SK_ACCOUNT, "Failed to remove column identified by ID %u from virtual table: %s", id, qUtf8Printable(ret.text()));
    }

    return ret;
}

/*!
 * \internal
 * \brief Remove all columns from the alias table that are associated to \a username.
 */
QSqlError removeAlias(const QString &username)
{
    QSqlError ret;

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM alias WHERE username = :username"));
    q.bindValue(QStringLiteral(":username"), username);
    if (Q_UNLIKELY(!q.exec())) {
        ret = q.lastError();
        qCCritical(SK_ACCOUNT, "Failed to remove all entries for username \"%s\" from the alias table: %s", qUtf8Printable(username), qUtf8Printable(ret.text()));
    }

    return ret;
}

/*!
 * \internal
 * \brief Remove the column identified by \a id from the alias table.
 */
QSqlError removeAliasByID(dbid_t id)
{
    QSqlError ret;
    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM alias WHERE id = :id"));
    q.bindValue(QStringLiteral(":id"), id);
    if (Q_UNLIKELY(!q.exec())) {
        ret = q.lastError();
        qCCritical(SK_ACCOUNT, "Failed to remove column identified by ID %u from the alias table: %s", id, qUtf8Printable(ret.text()));
    }

    return ret;
}

/*!
 * \internal
 * \brief Removes the account identified by \a id from the accountuser table.
 */
QSqlError removeAccountByID(dbid_t id)
{
    QSqlError ret;

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM accountuser WHERE id = :id"));
    q.bindValue(QStringLiteral(":id"), id);
    if (Q_UNLIKELY(!q.exec())) {
        ret = q.lastError();
        qCCritical(SK_ACCOUNT, "Failed to remove the user account with ID %u from the database: %s", id, qUtf8Printable(ret.text()));
    }

    return ret;
}

/*!
 * \internal
 * \brief Updates the \a ace_id column in the virtual table for the given \a id.
 */
QSqlError updateAceID(dbid_t id, dbid_t ace_id)
{
    QSqlError ret;

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("UPDATE virtual SET ace_id = :ace_id WHERE id = :id"));
    q.bindValue(QStringLiteral(":ace_id"), ace_id);
    q.bindValue(QStringLiteral(":id"), id);
    if (Q_UNLIKELY(!q.exec())) {
        ret = q.lastError();
        qCCritical(SK_ACCOUNT, "Failed to update relationship between IDN (ID: %u) and ACE (ID: %u) address in the virtual table: %s", id, ace_id, qUtf8Printable(ret.text()));
    }

    return ret;
}

/*!
 * \internal
 * \brief Queries the current list of forwards for the account identified by \a username from the database.
 *
 * First member will contain the list of forward email addresses, second member will be true
 * if incoming emails should be keept in the local mailbox.
 *
 * \param c Current context, used for translations.
 * \param username  Name of the user to quey the forwards for
 * \param e Pointer to an object taking error information.
 * \return List of forward addresses and status of keep local.
 */
std::pair<QStringList, bool> queryFowards(Cutelyst::Context *c, const QString &username, SkaffariError *e = nullptr)
{
    std::pair<QStringList,bool> ret = std::make_pair(QStringList(), false);

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT dest FROM virtual WHERE alias = :username AND username = ''"));
    q.bindValue(QStringLiteral(":username"), username);

    if (Q_UNLIKELY(!q.exec())) {
        if (e) {
            e->setSqlError(q.lastError(), c->translate("Account", "Cannot retrieve current list of forwarding addresses for user account %1 from the database.").arg(username));
        }
        qCCritical(SK_ACCOUNT, "%s failed to query list of forwarding addresses for user account %s from the database: %s", qUtf8Printable(AdminAccount::getUserNameIdString(c)), qUtf8Printable(username), qUtf8Printable(q.lastError().text()));
    } else {
        if (q.next()) {
            const auto fws = q.value(0).toString().split(QLatin1Char(','), QString::SkipEmptyParts);
            ret.first.reserve(fws.size());
            for (const QString &fw : fws) {
                if (fw != username) {
                    ret.first << fw;
                } else {
                    ret.second = true;
                }
            }
        }
    }

    return ret;
}

/*!
 * \brief Queries the current list of email addreses associted with the account identified by \a username from the database.
 *
 * First member will contain the list email addresses, second member will be true if this is
 * a catch-all account.
 *
 * \param c Current context, used for translations.
 * \param username  Name of the user to query the addresses for.
 * \param e Pointer to an object taking error information.
 * \return List of email addresses of the account and status of catch all.
 */
std::pair<QStringList, bool> queryAddresses(Cutelyst::Context *c, const QString &username, SkaffariError *e = nullptr)
{
    std::pair<QStringList,bool> ret = std::make_pair(QStringList(), false);

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT alias FROM virtual WHERE dest = :username AND username = :username AND idn_id = 0 ORDER BY alias ASC"));
    q.bindValue(QStringLiteral(":username"), username);

    if (Q_UNLIKELY(!q.exec())) {
        if (e) {
            e->setSqlError(q.lastError(), c->translate("Account", "Cannot retrieve current list of email addresses for user account %1 from the database.").arg(username));
        }
        qCCritical(SK_ACCOUNT, "%s failed to query list of email addresses for user account %s from the database: %s", qUtf8Printable(AdminAccount::getUserNameIdString(c)), qUtf8Printable(username), qUtf8Printable(q.lastError().text()));
    } else {
        ret.first.reserve(q.size());
        while (q.next()) {
            const QString address = q.value(0).toString();
            if (!address.startsWith(QLatin1Char('@'))) {
                ret.first << address;
            } else {
                ret.second = true;
            }
        }
    }

    return ret;
}

Account Account::create(Cutelyst::Context *c, SkaffariError &e, const QVariantHash &p, const Domain &d, const QStringList &selectedKids)
{
    Account a;

    Q_ASSERT_X(c, "create account", "invalid context object");
    Q_ASSERT_X(!p.empty(), "create account", "empty parameters");
    Q_ASSERT_X(d.isValid(), "create account", "invalid domain object");

    // if domain as prefix is enabled, the username will be the local part of the email address plus the domain separated by a dot
    // if additionally fqun is enabled, it will a fully qualified user name (email address like user@example.com
    // if both are disabled, the username will be the entered username
    const QString username = SkaffariConfig::imapDomainasprefix() ? p.value(QStringLiteral("localpart")).toString().trimmed() + (SkaffariConfig::imapFqun() ? QLatin1Char('@') : QLatin1Char('.')) + d.name() : p.value(QStringLiteral("username")).toString().trimmed();

    // construct the email address from local part and domain name
    const QString localPart = p.value(QStringLiteral("localpart")).toString().trimmed();
    const QString email = localPart + QLatin1Char('@') + d.name();
    const QString emailAce = localPart + QLatin1Char('@') + d.aceName();

    QList<Cutelyst::ValidatorEmail::Diagnose> diags;
    if (!Cutelyst::ValidatorEmail::validate(emailAce, Cutelyst::ValidatorEmail::Valid, Cutelyst::ValidatorEmail::NoOption, &diags)) {
        e.setErrorType(SkaffariError::InputError);
        e.setErrorText(Cutelyst::ValidatorEmail::diagnoseString(c, diags.at(0)));
        return a;
    }

    QSqlError sqlError;
    const bool emailExists = aliasExists(email, sqlError);
    if (Q_UNLIKELY(sqlError.type() != QSqlError::NoError)) {
        e.setSqlError(sqlError, c->translate("Account", "The database query to check if the email address %1 already exists failed.").arg(email));
        return a;
    }
    if (Q_UNLIKELY(emailExists)) {
        e.setErrorText(c->translate("Account", "The email address %1 is already in use by another user.").arg(email));
        e.setErrorType(SkaffariError::InputError);
        return a;
    }

    // for logging
    const QByteArray uniBa = AdminAccount::getUserNameIdString(c).toUtf8();
    const char *uniStr = uniBa.constData();
    const QByteArray aunBa = username.toUtf8();
    const char *aunStr = aunBa.constData();


    // start encrypting the password
    const QString password = p.value(QStringLiteral("password")).toString();
    Password pw(password);
    const QByteArray encpw = pw.encrypt(SkaffariConfig::accPwMethod(), SkaffariConfig::accPwAlgorithm(), SkaffariConfig::accPwRounds());

    if (Q_UNLIKELY(encpw.isEmpty())) {
        e.setErrorText(c->translate("Account", "User password encryption failed. Please check your encryption settings."));
        e.setErrorType(SkaffariError::ConfigError);
        qCCritical(SK_ACCOUNT, "%s failed to encrypt user password for new account %s. Please check your encryption settings.", uniStr, aunStr);
        return a;
    }
    // end encrypting the password

    const bool imap         = p.value(QStringLiteral("imap")).toBool();
    const bool pop          = p.value(QStringLiteral("pop")).toBool();
    const bool sieve        = p.value(QStringLiteral("sieve")).toBool();
    const bool smtpauth     = p.value(QStringLiteral("smtpauth")).toBool();
    const bool _catchAll    = p.value(QStringLiteral("catchall")).toBool();

    const quota_size_t quota = (p.value(QStringLiteral("quota")).value<quota_size_t>() / Q_UINT64_C(1024));

    const QDateTime defDateTime(QDate(2999, 12, 31), QTime(0, 0), QTimeZone::utc());
    const QDateTime currentUtc  = QDateTime::currentDateTimeUtc();
    const QDateTime validUntil  = p.value(QStringLiteral("validUntil"), defDateTime).toDateTime().toUTC();
    const QDateTime pwExpires   = p.value(QStringLiteral("passwordExpires"), defDateTime).toDateTime().toUTC();

    const quint8 accountStatus = Account::calcStatus(validUntil, pwExpires);

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO accountuser (domain_id, username, password, imap, pop, sieve, smtpauth, quota, created_at, updated_at, valid_until, pwd_expire, status) "
                                         "VALUES (:domain_id, :username, :password, :imap, :pop, :sieve, :smtpauth, :quota, :created_at, :updated_at, :valid_until, :pwd_expire, :status)"));

    q.bindValue(QStringLiteral(":domain_id"), d.id());
    q.bindValue(QStringLiteral(":username"), username);
    q.bindValue(QStringLiteral(":password"), encpw);
    q.bindValue(QStringLiteral(":imap"), imap);
    q.bindValue(QStringLiteral(":pop"), pop);
    q.bindValue(QStringLiteral(":sieve"), sieve);
    q.bindValue(QStringLiteral(":smtpauth"), smtpauth);
    q.bindValue(QStringLiteral(":quota"), quota);
    q.bindValue(QStringLiteral(":created_at"), currentUtc);
    q.bindValue(QStringLiteral(":updated_at"), currentUtc);
    q.bindValue(QStringLiteral(":valid_until"), validUntil);
    q.bindValue(QStringLiteral(":pwd_expire"), pwExpires);
    q.bindValue(QStringLiteral(":status"), accountStatus);

    if (Q_UNLIKELY(!q.exec())) {
        e.setSqlError(q.lastError(), c->translate("Account", "New user account could not be created in the database."));
        qCCritical(SK_ACCOUNT, "%s failed to insert new user account %s into the database: %s", uniStr, aunStr, qUtf8Printable(q.lastError().text()));
        return a;
    }

    const dbid_t id = q.lastInsertId().value<dbid_t>();
    const dbid_t idnEmailId = insertVirtual(0, 0, email, username, username, 1, sqlError);

    if (idnEmailId == 0) {
        e.setSqlError(sqlError, c->translate("Account", "Email address for new user account could not be created in the database."));
        qCCritical(SK_ACCOUNT, "%s failed to insert email address %s for new user account %s into the database: %s", uniStr, qUtf8Printable(email), aunStr, qUtf8Printable(sqlError.text()));
        removeAccountByID(id);
        return a;
    }

    if (d.isIdn()) {
        const dbid_t aceEmailId = insertVirtual(idnEmailId, 0, emailAce, username, username, 1, sqlError);
        if (aceEmailId == 0) {
            e.setSqlError(sqlError, c->translate("Account", "ACE email address for new user account could not be created in the database."));
            qCCritical(SK_ACCOUNT, "%s failed to insert ACE email address %s for new user account %s into the database: %s", uniStr, qUtf8Printable(emailAce), aunStr, qUtf8Printable(sqlError.text()));
            removeVirtual(username);
            removeAccountByID(id);
            return a;
        } else {
            sqlError = updateAceID(idnEmailId, aceEmailId);
            if (Q_UNLIKELY(sqlError.type() != QSqlError::NoError)) {
                e.setSqlError(sqlError, c->translate("Account", "Can not set relationship between ACE and IDN representation of email address of new user account in database."));
                return a;
            }
        }
    }

    if (!d.children().empty() && !selectedKids.empty()) {
        for (const SimpleDomain &kid : d.children()) {
            if (selectedKids.contains(QString::number(kid.id()))) {
                SkaffariError cDomError(c);
                const Domain cDom = Domain::get(c, kid.id(), cDomError);
                if (cDom.isValid()) {
                    const QString kidEmail = localPart + QLatin1Char('@') + kid.name();
                    bool exists = aliasExists(kidEmail, sqlError);
                    if (!exists && (sqlError.type() == QSqlError::NoError)) {
                        const dbid_t kidEmailIdnId = insertVirtual(0, 0, kidEmail, username, username, 1, sqlError);
                        if (kidEmailIdnId > 0) {
                            if (cDom.isIdn()) {
                                const QString kidEmailAce = localPart + QLatin1Char('@') + QString::fromLatin1(QUrl::toAce(kid.name()));
                                const dbid_t kidEmailAceId = insertVirtual(kidEmailIdnId, 0, kidEmailAce, username, username, 1, sqlError);
                                if (kidEmailAceId > 0) {
                                    sqlError = updateAceID(kidEmailIdnId, kidEmailAceId);
                                    if (sqlError.type() != QSqlError::NoError) {
                                        removeVirtualByID(kidEmailIdnId);
                                        removeVirtualByID(kidEmailAceId);
                                    }
                                } else {
                                    qCCritical(SK_ACCOUNT, "%s failed to insert ACE email address %s for new user account %s into database: %s", uniStr, qUtf8Printable(kidEmailAce), aunStr, qUtf8Printable(sqlError.text()));
                                    removeVirtualByID(kidEmailIdnId);
                                }
                            }
                        } else {
                            qCCritical(SK_ACCOUNT, "%s failed to insert email address %s for new user account %s into database: %s", uniStr, qUtf8Printable(kidEmail), aunStr, qUtf8Printable(sqlError.text()));
                        }
                    } else {
                        if (exists) {
                            qCWarning(SK_ACCOUNT, "%s tried to insert already existing email address %s for new account %s into database.", uniStr, qUtf8Printable(kidEmail), aunStr);
                        }
                    }
                } else {
                    qCCritical(SK_ACCOUNT, "%s failed to query complete domain data for domain %s from the database while creating child domain addresses for new account %s: %s", uniStr, qUtf8Printable(kid.nameIdString()), aunStr, qUtf8Printable(cDomError.qSqlError().text()));
                }
            }
        }
    }

    // removing old catch-all alias and setting a new one
    if (_catchAll) {
        const QString catchAllAlias = QLatin1Char('@') + d.name();
        const QString catchAllAliasAce = QLatin1Char('@') + QString::fromLatin1(QUrl::toAce(d.name()));

        if (d.isIdn()) {
            q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM virtual WHERE alias = :alias OR alias = :aceAlias"));
            q.bindValue(QStringLiteral(":aceAlias"), catchAllAliasAce);
        } else {
            q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM virtual WHERE alias = :alias"));
        }
        q.bindValue(QStringLiteral(":alias"), catchAllAlias);

        if (Q_UNLIKELY(!q.exec())) {
            e.setSqlError(q.lastError(), c->translate("Account", "Existing catch-all address could not be deleted from the database."));
            qCCritical(SK_ACCOUNT, "%s failed to delete existing catch-all address for domain %s from the database while creating new account %s: %s", uniStr, qUtf8Printable(d.nameIdString()), aunStr, qUtf8Printable(q.lastError().text()));
            removeVirtual(username);
            removeAccountByID(id);
            return a;
        }

        const dbid_t catchAllIdnId = insertVirtual(0, 0, catchAllAlias, username, username, 1, sqlError);

        if (catchAllIdnId == 0) {
            e.setSqlError(sqlError, c->translate("Account", "Account could not be set up as catch-all account."));
            qCCritical(SK_ACCOUNT, "%s failed to setup new account %s as catch-all account for domain %s: %s", uniStr, aunStr, qUtf8Printable(d.nameIdString()), qUtf8Printable(sqlError.text()));
            removeVirtual(username);
            removeAccountByID(id);
            return a;
        }

        if (d.isIdn()) {
            const dbid_t catchAllAceId = insertVirtual(catchAllIdnId, 0, catchAllAliasAce, username, username, 1, sqlError);
            if (catchAllAceId > 0) {
                sqlError = updateAceID(catchAllIdnId, catchAllAceId);
                if (sqlError.type() != QSqlError::NoError) {
                    e.setSqlError(sqlError, c->translate("Account", "Account could not be set up as catch-all account."));
                    removeVirtual(username);
                    removeAccountByID(id);
                    return a;
                }
            } else {
                e.setSqlError(sqlError, c->translate("Account", "Account could not be set up as catch-all account."));
                qCCritical(SK_ACCOUNT, "%s failed to setup new account %s as catch-all account for IDN domain %s: %s", uniStr, aunStr, qUtf8Printable(d.nameIdString()), qUtf8Printable(sqlError.text()));
                removeVirtual(username);
                removeAccountByID(id);
                return a;
            }
        }
    }

    // start creating the mailbox on the IMAP server, according to the skaffari settings
    bool mailboxCreated = true;
    Account::CreateMailbox createMailbox = SkaffariConfig::imapCreatemailbox();

    if (createMailbox != DoNotCreate) {

        SkaffariIMAP imap(c);

        if (createMailbox == LoginAfterCreation) {

            imap.setUser(username);
            imap.setPassword(password);

            mailboxCreated = imap.login();
            if (!mailboxCreated) {
                e.setImapError(imap.lastError(), c->translate("Account", "Logging in to IMAP server for automatic creation of mailbox and folders by the server failed."));
            }
            imap.logout();

        } else if (createMailbox == OnlySetQuota) {

            imap.setUser(username);
            imap.setPassword(password);

            mailboxCreated = imap.login();
            if (!mailboxCreated) {
                e.setImapError(imap.lastError(), c->translate("Account", "Logging in to IMAP server for automatic creation of mailbox and folders by the server failed."));
            }

            imap.logout();

            if (mailboxCreated) {
                imap.setUser(SkaffariConfig::imapUser());
                imap.setPassword(SkaffariConfig::imapPassword());

                if (Q_LIKELY(imap.login())) {

                    if (Q_UNLIKELY(!imap.setQuota(username, quota))) {
                        e.setImapError(imap.lastError(), c->translate("Account", "Storage quota for new user account could not be set."));
                        mailboxCreated = false;
                    }

                    imap.logout();

                } else {
                    e.setImapError(imap.lastError(), c->translate("Account", "Logging in to the IMAP server to set the storage quota for the new user account failed."));
                    mailboxCreated = false;
                }
            }

        } else if (createMailbox == CreateBySkaffari) {

            if (Q_LIKELY(imap.login())) {

                if (Q_LIKELY(imap.createMailbox(username))) {

                    // at this point, the mailbox has been created on the IMAP server
                    // all following actions can fail - if they do, it is not nice,
                    // but base functionality is given, so we only log errors
                    mailboxCreated = true;

                    if(Q_LIKELY(imap.setAcl(username, SkaffariConfig::imapUser()))) {

                        if (Q_UNLIKELY(!imap.setQuota(username, quota))) {
                            qCWarning(SK_ACCOUNT) << "Failed to set IMAP quota for new mailbox" << username;
                        }

                        if (Q_UNLIKELY(!imap.deleteAcl(username, SkaffariConfig::imapUser()))) {
                            qCWarning(SK_ACCOUNT) << "Failed to revoke ACLs for IMAP admin on new mailbox" << username;
                        }

                    } else {
                        qCWarning(SK_ACCOUNT) << "Failed to set ACL for IMAP admin on new mailbox" << username;
                    }

                    imap.logout();

                    if (!d.folders().empty()) {

                        imap.setUser(username);
                        imap.setPassword(password);

                        if (Q_LIKELY(imap.login())) {

                            for (const Folder &folder : d.folders()) {
                                if (Q_UNLIKELY(!imap.createFolder(folder.getName()))) {
                                    qCWarning(SK_ACCOUNT, "%s failed to create default IMAP folder \"%s\" for new account %s: %s", uniStr, qUtf8Printable(folder.getName()), aunStr, qUtf8Printable(imap.lastError().errorText()));
                                }
                            }

                            imap.logout();

                        } else {
                            qCWarning(SK_ACCOUNT, "%s failed to log %s into new account to create default folders: %s", uniStr, aunStr, qUtf8Printable(imap.lastError().errorText()));
                        }
                    }

                } else {
                    e.setImapError(imap.lastError(), c->translate("Account", "Creating a new IMAP mailbox failed."));
                    imap.logout();
                    mailboxCreated = false;
                }

            } else {
                e.setImapError(imap.lastError(), c->translate("Account", "Logging in to IMAP server to create new user account failed."));
                mailboxCreated = false;
            }
        }
    }
    // end creating the mailbox on the IMAP server, according to the skaffari settings

    // revert our changes to the database if mailbox creation failed
    if (!mailboxCreated) {
        removeVirtual(username);
        removeAccountByID(id);
        return a;
    }

    q = CPreparedSqlQueryThread(QStringLiteral("UPDATE domain SET accountcount = accountcount + 1, domainquotaused = domainquotaused + :quota WHERE id = :id"));
    q.bindValue(QStringLiteral(":quota"), quota);
    q.bindValue(QStringLiteral(":id"), d.id());
    if (Q_UNLIKELY(!q.exec())) {
        qCWarning(SK_ACCOUNT, "%s failed to update count of accounts and domain quota usage for domain %s afert creating new account %s: %s", uniStr, qUtf8Printable(d.nameIdString()), aunStr, qUtf8Printable(q.lastError().text()));
    }

    a = Account(id, d.id(), username, imap, pop, sieve, smtpauth, QStringList(email), QStringList(), quota, 0, currentUtc, currentUtc, validUntil, pwExpires, false, _catchAll, Account::calcStatus(validUntil, pwExpires));

    if (SkaffariConfig::useMemcached()) {
        Cutelyst::Memcached::set(MEMC_QUOTA_KEY + QString::number(id), QByteArray::number(0), MEMC_QUOTA_EXP);
    }

    qCInfo(SK_ACCOUNT, "%s created new account %s in domain %s", uniStr, qUtf8Printable(a.nameIdString()), qUtf8Printable(d.nameIdString()));

    return a;
}

bool Account::remove(Cutelyst::Context *c, SkaffariError &e) const
{
    bool ret = false;

    Q_ASSERT_X(c, "remove account", "invalid context object");

    // for logging
    const QByteArray uniBa = AdminAccount::getUserNameIdString(c).toUtf8();
    const char *uniStr = uniBa.constData();
    const QByteArray aniBa = nameIdString().toUtf8();
    const char *aniStr = aniBa.constData();

    SkaffariIMAP imap(c);
    if (Q_UNLIKELY(!imap.login())) {
        e.setImapError(imap.lastError(), c->translate("Account", "Logging in to IMAP server to delete the mailbox %1 failed.").arg(d->username));
        qCCritical(SK_ACCOUNT, "%s failed to login as admin into IMAP server to delete the mailbox of account %s: %s", uniStr, aniStr, qUtf8Printable(imap.lastError().errorText()));
        return ret;
    }

    if (Q_UNLIKELY(!imap.setAcl(d->username, SkaffariConfig::imapUser()))) {
        // if Skaffari is responsible for mailbox creation, direct or indirect,
        // remove will fail if we can not delete the mailbox on the IMAP server
        if (SkaffariConfig::imapCreatemailbox() > DoNotCreate) {
            e.setImapError(imap.lastError(), c->translate("Account", "Setting the access rights for the IMAP administrator to delete the mailbox %1 failed.").arg(d->username));
            qCCritical(SK_ACCOUNT, "%s failed to set the access rights for the IMAP administrator to delete the mailbox of account %s: %s", uniStr, aniStr, qUtf8Printable(imap.lastError().errorText()));
            imap.logout();
            return ret;
        }
    }

    if (!imap.deleteMailbox(d->username) && (SkaffariConfig::imapCreatemailbox() != DoNotCreate)) {
        // if Skaffari is responsible for mailbox creation, direct or indirect,
        // remove will fail if we can not delete the mailbox on the IMAP server
        if (SkaffariConfig::imapCreatemailbox() > DoNotCreate) {
            e.setImapError(imap.lastError(), c->translate("Account", "Mailbox %1 could not be deleted from the IMAP server.").arg(d->username));
            qCCritical(SK_ACCOUNT, "%s failed to delete mailbox of account %s from the IMAP server: %s", uniStr, aniStr, qUtf8Printable(imap.lastError().errorText()));
            imap.logout();
            return ret;
        }
    }

    imap.logout();

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT quota FROM accountuser WHERE username = :username"));
    q.bindValue(QStringLiteral(":username"), d->username);

    const quota_size_t quota = (q.exec() && q.next()) ? q.value(0).value<quota_size_t>() : 0;

    QSqlError sqlError = removeAlias(d->username);
    if (sqlError.type() != QSqlError::NoError) {
        e.setSqlError(sqlError, c->translate("Account", "Alias addresses for user account %1 could not be deleted from the database.").arg(d->username));
        qCCritical(SK_ACCOUNT, "%s failed to delete alias addresses for account %s from the database: %s", uniStr, aniStr, qUtf8Printable(sqlError.text()));
        return ret;
    }

    sqlError = removeVirtual(d->username);
    if (sqlError.type() != QSqlError::NoError) {
        e.setSqlError(sqlError, c->translate("Account", "Email addresses for user account %1 could not be deleted from the database.").arg(d->username));
        qCCritical(SK_ACCOUNT, "%s failed to delete email addresses for account %s from the database: %s", uniStr, aniStr, qUtf8Printable(sqlError.text()));
        return ret;
    }

    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM virtual WHERE alias = :username AND username = ''"));
    q.bindValue(QStringLiteral(":username"), d->username);

    if (Q_UNLIKELY(!q.exec())) {
        e.setSqlError(q.lastError(), c->translate("Account", "Forward addresses for user account %1 could not be deleted from the database.").arg(d->username));
        qCCritical(SK_ACCOUNT, "%s failed to delete forward addresses for account %s from the database: %s", uniStr, aniStr, qUtf8Printable(q.lastError().text()));
        return ret;
    }

    sqlError = removeAccountByID(d->id);
    if (sqlError.type() != QSqlError::NoError) {
        e.setSqlError(sqlError, c->translate("Account", "User account %1 could not be deleted from the database.").arg(d->username));
        qCCritical(SK_ACCOUNT, "%s failed to delete user account %s from the databsae: %s", uniStr, aniStr, qUtf8Printable(sqlError.text()));
        return ret;
    }

    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM log WHERE user = :username"));
    q.bindValue(QStringLiteral(":username"), d->username);

    if (Q_UNLIKELY(!q.exec())) {
        e.setSqlError(q.lastError(), c->translate("Account", "Log entries for user account %1 could not be deleted from the database.").arg(d->username));
        qCWarning(SK_ACCOUNT, "%s failed to delete log entries for user account %s from the database: %s", uniStr, aniStr, qUtf8Printable(q.lastError().text()));
    }

    q = CPreparedSqlQueryThread(QStringLiteral("UPDATE domain SET accountcount = accountcount - 1, domainquotaused = domainquotaused - :quota WHERE id = :id"));
    q.bindValue(QStringLiteral(":quota"), quota);
    q.bindValue(QStringLiteral(":id"), d->domainId);

    if (Q_UNLIKELY(!q.exec())) {
        e.setSqlError(q.lastError(), c->translate("Account", "Number of user accounts in the domain and domain quota used could not be updated in the database."));
        qCWarning(SK_ACCOUNT, "%s failed to update count of domain accounts and used quota for domain ID %u after deleting account %s: %s", uniStr, d->domainId, aniStr, qUtf8Printable(q.lastError().text()));
    }

    qCInfo(SK_ACCOUNT, "%s deleted account %s.", qUtf8Printable(AdminAccount::getUserNameIdString(c)), qUtf8Printable(nameIdString()));

    ret = true;

    return ret;
}

Cutelyst::Pagination Account::list(Cutelyst::Context *c, SkaffariError &e, const Domain &d, const Cutelyst::Pagination &p, const QString &sortBy, const QString &sortOrder, const QString &searchRole, const QString &searchString)
{
    Cutelyst::Pagination pag;
    std::vector<Account> lst;

    Q_ASSERT_X(c, "list accounts", "invalid context object");

    // for logging
    const QByteArray uniBa = AdminAccount::getUserNameIdString(c).toUtf8();
    const char *uniStr = uniBa.constData();
    const QByteArray dniBa = d.nameIdString().toUtf8();
    const char *dniStr = dniBa.constData();

    QSqlQuery q(QSqlDatabase::database(Cutelyst::Sql::databaseNameThread()));

    if (searchString.isEmpty()) {
        q.prepare(QStringLiteral("SELECT SQL_CALC_FOUND_ROWS au.id, au.username, au.imap, au.pop, au.sieve, au.smtpauth, au.quota, au.created_at, au.updated_at, au.valid_until, au.pwd_expire, au.status FROM accountuser au WHERE au.domain_id = :domain_id ORDER BY au.%1 %2 LIMIT %3 OFFSET %4").arg(sortBy, sortOrder, QString::number(p.limit()), QString::number(p.offset())));
    } else {
        const QString _searchString = QLatin1Char('%') + searchString + QLatin1Char('%');
        if (searchRole == QLatin1String("username")) {
            q.prepare(QStringLiteral("SELECT SQL_CALC_FOUND_ROWS au.id, au.username, au.imap, au.pop, au.sieve, au.smtpauth, au.quota, au.created_at, au.updated_at, au.valid_until, au.pwd_expire, au.status FROM accountuser au WHERE au.domain_id = :domain_id AND au.username LIKE '%5' ORDER BY au.%1 %2 LIMIT %3 OFFSET %4").arg(sortBy, sortOrder, QString::number(p.limit()), QString::number(p.offset()), _searchString));
        } else if (searchRole == QLatin1String("email")) {
            q.prepare(QStringLiteral("SELECT SQL_CALC_FOUND_ROWS DISTINCT au.id, au.username, au.imap, au.pop, au.sieve, au.smtpauth, au.quota, au.created_at, au.updated_at, au.valid_until, au.pwd_expire, au.status FROM accountuser au LEFT JOIN virtual vi ON au.username = vi.username WHERE au.domain_id = :domain_id AND vi.dest = au.username AND vi.username = au.username AND vi.alias LIKE '%5' ORDER BY au.%1 %2 LIMIT %3 OFFSET %4").arg(sortBy, sortOrder, QString::number(p.limit()), QString::number(p.offset()), _searchString));
        } else if (searchRole == QLatin1String("forward")) {
            q.prepare(QStringLiteral("SELECT SQL_CALC_FOUND_ROWS DISTINCT au.id, au.username, au.imap, au.pop, au.sieve, au.smtpauth, au.quota, au.created_at, au.updated_at, au.valid_until, au.pwd_expire, au.status FROM accountuser au LEFT JOIN virtual vi ON au.username = vi.alias WHERE au.domain_id = :domain_id AND vi.username = '' AND vi.dest LIKE '%5' ORDER BY au.%1 %2 LIMIT %3 OFFSET %4").arg(sortBy, sortOrder, QString::number(p.limit()), QString::number(p.offset()), _searchString));
        }
    }

    q.bindValue(QStringLiteral(":domain_id"), d.id());

    if (Q_UNLIKELY(!q.exec())) {
        e.setSqlError(q.lastError(), c->translate("Account", "User accounts could not be queried from the database."));
        qCCritical(SK_ACCOUNT, "%s failed to query accounts for domain %s from the database: %s", uniStr, dniStr, qUtf8Printable(q.lastError().text()));
        return pag;
    }

    QSqlQuery countQuery = CPreparedSqlQueryThread(QStringLiteral("SELECT FOUND_ROWS()"));
    if (Q_UNLIKELY(!countQuery.exec())) {
        e.setSqlError(q.lastError(), c->translate("Account", "Total result could not be retrieved from the database."));
        qCCritical(SK_ACCOUNT, "%s failed to query total result for domain %s from the database: %s", uniStr, dniStr, qUtf8Printable(q.lastError().text()));
        return pag;
    }

    quint32 foundRows = 0;
    if (countQuery.next()) {
        foundRows = countQuery.value(0).value<quint32>();
    }

    if (foundRows == 0) {
        return pag;
    }

    pag = Cutelyst::Pagination(foundRows, p.limit(), p.currentPage(), p.pages().size());

    SkaffariIMAP imap(c);
    if (!imap.login()) {
        qCWarning(SK_ACCOUNT, "%s failed to log IMAP admin into IMAP server to query account quotas while listing accounts for domain %s: %s", uniStr, dniStr, qUtf8Printable(imap.lastError().errorText()));
    }

    QCollator col(c->locale());
    lst.reserve(foundRows);

    while (q.next()) {
        const dbid_t _id = q.value(0).value<dbid_t>();
        const QString _username = q.value(1).toString();
        quota_size_t quota = q.value(6).value<quota_size_t>();
        QDateTime accountCreated = q.value(7).toDateTime();
        accountCreated.setTimeSpec(Qt::UTC);
        QDateTime accountUpdated = q.value(8).toDateTime();
        accountUpdated.setTimeSpec(Qt::UTC);
        QDateTime accountValidUntil = q.value(9).toDateTime();
        accountValidUntil.setTimeSpec(Qt::UTC);
        QDateTime accountPwExpires = q.value(10).toDateTime();
        accountPwExpires.setTimeSpec(Qt::UTC);

        std::pair<QStringList,bool> emailAddresses = queryAddresses(c, _username);

        std::pair<QStringList,bool> forwards = queryFowards(c, _username);

        if ((emailAddresses.first.size() > 1) || (forwards.first.size() > 1)) {

            if (emailAddresses.first.size() > 1) {
                std::sort(emailAddresses.first.begin(), emailAddresses.first.end(), col);
            }

            if (forwards.first.size() > 1) {
                std::sort(forwards.first.begin(), forwards.first.end(), col);
            }
        }

        bool gotQuota = false;
        quota_size_t usage = 0;
        if (SkaffariConfig::useMemcached()) {
            const QByteArray usageBa = Cutelyst::Memcached::get(MEMC_QUOTA_KEY + QString::number(_id));
            if (!usageBa.isNull()) {
                bool ok = false;
                usage = usageBa.toULongLong(&ok);
                if (ok) {
                    gotQuota = true;
                }
            }
        }

        if (!gotQuota) {
            if (Q_LIKELY(imap.isLoggedIn())) {
                quota_pair quotaVals = imap.getQuota(_username);
                usage = quotaVals.first;
                quota = quotaVals.second;
                if (SkaffariConfig::useMemcached()) {
                    Cutelyst::Memcached::set(MEMC_QUOTA_KEY + QString::number(_id), QByteArray::number(quotaVals.first), MEMC_QUOTA_EXP);
                }
            }
        }

        lst.emplace_back(_id,
                         d.id(),
                         _username,
                         q.value(2).toBool(),
                         q.value(3).toBool(),
                         q.value(4).toBool(),
                         q.value(5).toBool(),
                         emailAddresses.first,
                         forwards.first,
                         quota,
                         usage,
                         accountCreated,
                         accountUpdated,
                         accountValidUntil,
                         accountPwExpires,
                         forwards.second,
                         emailAddresses.second,
                         q.value(11).value<quint8>());
    }

    imap.logout();

    pag.insert(QStringLiteral("accounts"), QVariant::fromValue<std::vector<Account>>(lst));

    return pag;
}

Account Account::get(Cutelyst::Context *c, SkaffariError &e, dbid_t id)
{
    Account a;

    Q_ASSERT_X(c, "get account", "invalid context object");

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT au.domain_id, au.username, au.imap, au.pop, au.sieve, au.smtpauth, au.quota, au.created_at, au.updated_at, au.valid_until, au.pwd_expire, au.status FROM accountuser au WHERE au.id = :id"));
    q.bindValue(QStringLiteral(":id"), id);

    if (Q_UNLIKELY(!q.exec())) {
        e.setSqlError(q.lastError(), c->translate("Account", "User account data could not be queried from the database."));
        qCCritical(SK_ACCOUNT, "%s failed to query data for user account with ID %u from the database: %s", qUtf8Printable(AdminAccount::getUserNameIdString(c)), id, qUtf8Printable(q.lastError().text()));
        return a;
    }

    if (Q_UNLIKELY(!q.next())) {
        e.setErrorType(SkaffariError::NotFound);
        e.setErrorText(c->translate("Account", "Can not find account with database ID %1.").arg(id));
        qCWarning(SK_ACCOUNT, "%s can not find user account with ID %u in the database.", qUtf8Printable(AdminAccount::getUserNameIdString(c)), id);
        return a;
    }

    const QString userName = q.value(1).toString();
    quota_size_t quota = q.value(6).value<quota_size_t>();
    QDateTime accCreated = q.value(7).toDateTime();
    accCreated.setTimeSpec(Qt::UTC);
    QDateTime accUpdated = q.value(8).toDateTime();
    accUpdated.setTimeSpec(Qt::UTC);
    QDateTime accValidUntil = q.value(9).toDateTime();
    accValidUntil.setTimeSpec(Qt::UTC);
    QDateTime accPwdExpires = q.value(10).toDateTime();
    accPwdExpires.setTimeSpec(Qt::UTC);

    std::pair<QStringList,bool> emailAddresses = queryAddresses(c, userName);

    std::pair<QStringList,bool> forwards = queryFowards(c, userName);

    if ((emailAddresses.first.size() > 1) || (forwards.first.size() > 1)) {
        QCollator col(c->locale());

        if (emailAddresses.first.size() > 1) {
            std::sort(emailAddresses.first.begin(), emailAddresses.first.end(), col);
        }

        if (forwards.first.size() > 1) {
            std::sort(forwards.first.begin(), forwards.first.end(), col);
        }
    }

    bool gotUsage = false;
    quota_size_t usage = 0;
    if (SkaffariConfig::useMemcached()) {
        const QByteArray usageBa = Cutelyst::Memcached::get(MEMC_QUOTA_KEY + QString::number(id));
        if (!usageBa.isNull()) {
            bool ok = false;
            usage = usageBa.toULongLong(&ok);
            if (ok) {
                gotUsage = true;
            }
        }
    }

    if (!gotUsage) {
        SkaffariIMAP imap(c);
        if (imap.login()) {
            quota_pair quotaPair = imap.getQuota(userName);
            usage = quotaPair.first;
            quota = quotaPair.second;
            imap.logout();

            if (SkaffariConfig::useMemcached()) {
                Cutelyst::Memcached::set(MEMC_QUOTA_KEY + QString::number(id), QByteArray::number(quota), MEMC_QUOTA_EXP);
            }
        }
    }

    a = Account(id,
                q.value(0).value<dbid_t>(),
                userName,
                q.value(2).toBool(),
                q.value(3).toBool(),
                q.value(4).toBool(),
                q.value(5).toBool(),
                emailAddresses.first,
                forwards.first,
                quota,
                usage,
                accCreated,
                accUpdated,
                accValidUntil,
                accPwdExpires,
                forwards.second,
                emailAddresses.second,
                Account::calcStatus(accValidUntil, accPwdExpires));

    return a;
}

void Account::toStash(Cutelyst::Context *c, dbid_t accountId)
{
    Q_ASSERT_X(c, "account to stash", "invalid context object");

    SkaffariError e(c);
    Account a = Account::get(c, e, accountId);
    if (Q_LIKELY(a.isValid())) {
        c->stash({
                     {QStringLiteral(ACCOUNT_STASH_KEY), QVariant::fromValue<Account>(a)},
                     {QStringLiteral("site_subtitle"), a.username()}
                 });
    } else {
        e.toStash(c);
        c->detach(c->getAction(QStringLiteral("error")));
    }
}

bool Account::toStash(Cutelyst::Context *c) const
{
    Q_ASSERT_X(c, "account to stash", "invalid context object");
    if (Q_LIKELY(isValid())) {
        c->stash({
                     {QStringLiteral(ACCOUNT_STASH_KEY), QVariant::fromValue<Account>(*this)},
                     {QStringLiteral("site_subtitle"), d->username}
                 });
        return true;
    } else {
        c->res()->setStatus(404);
        c->detach(c->getAction(QStringLiteral("error")));
        return false;
    }
}

Account Account::fromStash(Cutelyst::Context *c)
{
    Account a;
    a = c->stash(QStringLiteral(ACCOUNT_STASH_KEY)).value<Account>();
    return a;
}

bool Account::update(Cutelyst::Context *c, SkaffariError &e, Domain *dom, const QVariantHash &p)
{
    bool ret = false;

    Q_ASSERT_X(c, "update account", "invalid context object");
    Q_ASSERT_X(dom, "update account", "invalid domain object");

    // for logging
    const QByteArray uniBa = AdminAccount::getUserNameIdString(c).toUtf8();
    const char *uniStr = uniBa.constData();
    const QByteArray aniBa = nameIdString().toUtf8();
    const char *aniStr = aniBa.constData();
    const QByteArray dniBa = dom->nameIdString().toUtf8();
    const char *dniStr = dniBa.constData();

    const QString password = p.value(QStringLiteral("password")).toString();
    QByteArray encPw;
    if (!password.isEmpty()) {
        Password pw(password);
        encPw = pw.encrypt(SkaffariConfig::accPwMethod(), SkaffariConfig::accPwAlgorithm(), SkaffariConfig::accPwRounds());
        if (Q_UNLIKELY(encPw.isEmpty())) {
            e.setErrorType(SkaffariError::ApplicationError);
            e.setErrorText(c->translate("Account", "Password encryption failed."));
            qCCritical(SK_ACCOUNT, "%s failed to encrypt user password for account %s. Please check your encryption settings.", uniStr, aniStr);
            return ret;
        }
    }

    const quota_size_t quota = p.contains(QStringLiteral("quota")) ? static_cast<quota_size_t>(p.value(QStringLiteral("quota")).value<quota_size_t>()/Q_UINT64_C(1024)) : d->quota;

    if (quota != d->quota) {
        SkaffariIMAP imap(c);
        if (Q_LIKELY(imap.login())) {
            if (Q_UNLIKELY(!imap.setQuota(d->username, quota))) {
                e.setImapError(imap.lastError(), c->translate("Account", "Changing the storage quota failed."));
                qCCritical(SK_ACCOUNT, "%s failed to set storage quota for account %s on IMAP server: %s", uniStr, aniStr, qUtf8Printable(imap.lastError().errorText()));
                return ret;
            }
        } else {
            e.setImapError(imap.lastError(), c->translate("Account", "Logging in to IMAP server to change the storage quota failed."));
            qCCritical(SK_ACCOUNT, "%s faild to log into IMAP server as %s to change storage quota of account %s: %s", uniStr, qUtf8Printable(SkaffariConfig::imapUser()), aniStr, qUtf8Printable(imap.lastError().errorText()));
            return ret;
        }
    }

    const QDateTime validUntil      = p.value(QStringLiteral("validUntil"), d->validUntil).toDateTime().toUTC();
    const QDateTime pwExpires       = p.value(QStringLiteral("passwordExpires"), d->passwordExpires).toDateTime().toUTC();
    const QDateTime currentTimeUtc  = QDateTime::currentDateTimeUtc();

    const bool imap         = p.value(QStringLiteral("imap")).toBool();
    const bool pop          = p.value(QStringLiteral("pop")).toBool();
    const bool sieve        = p.value(QStringLiteral("sieve")).toBool();
    const bool smtpauth     = p.value(QStringLiteral("smtpauth")).toBool();
    const bool _catchAll    = p.value(QStringLiteral("catchall")).toBool();

    QSqlQuery q;
    if (!password.isEmpty()) {
        q = CPreparedSqlQueryThread(QStringLiteral("UPDATE accountuser SET password = :password, quota = :quota, valid_until = :valid_until, updated_at = :updated_at, imap = :imap, pop = :pop, sieve = :sieve, smtpauth =:smtpauth, pwd_expire = :pwd_expire WHERE id = :id"));
        q.bindValue(QStringLiteral(":password"), encPw);
    } else {
        q = CPreparedSqlQueryThread(QStringLiteral("UPDATE accountuser SET quota = :quota, valid_until = :valid_until, updated_at = :updated_at, imap = :imap, pop = :pop, sieve = :sieve, smtpauth =:smtpauth, pwd_expire = :pwd_expire WHERE id = :id"));
    }
    q.bindValue(QStringLiteral(":quota"), quota);
    q.bindValue(QStringLiteral(":valid_until"), validUntil);
    q.bindValue(QStringLiteral(":id"), d->id);
    q.bindValue(QStringLiteral(":updated_at"), currentTimeUtc);
    q.bindValue(QStringLiteral(":imap"), imap);
    q.bindValue(QStringLiteral(":pop"), pop);
    q.bindValue(QStringLiteral(":sieve"), sieve);
    q.bindValue(QStringLiteral(":smtpauth"), smtpauth);
    q.bindValue(QStringLiteral(":pwd_expire"), pwExpires);


    if (Q_UNLIKELY(!q.exec())) {
        e.setSqlError(q.lastError(), c->translate("Account", "User account could not be updated in the database."));
        qCCritical(SK_ACCOUNT, "%s failed to update user account %s in the database: %s", uniStr, aniStr, qUtf8Printable(q.lastError().text()));
        return ret;
    }

    if (_catchAll != d->catchAll) {
        const QString catchAllAlias = QLatin1Char('@') + dom->name();
        const QString catchAllAliasAce = QLatin1Char('@') + dom->aceName();
        if (_catchAll && !d->catchAll) {
            if (dom->isIdn()) {
                q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM virtual WHERE alias = :alias OR alias = :aliasAce"));
                q.bindValue(QStringLiteral(":aliasAce"), catchAllAliasAce);
            } else {
                q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM virtual WHERE alias = :alias"));
            }
            q.bindValue(QStringLiteral(":alias"), catchAllAlias);

            if (Q_UNLIKELY(!q.exec())) {
                e.setSqlError(q.lastError(), c->translate("Account", "Existing catch-all address could not be deleted from the database."));
                qCWarning(SK_ACCOUNT, "%s failed to delete existing catch-all address of domain %s while updating account %s: %s", uniStr, dniStr, aniStr, qUtf8Printable(q.lastError().text()));;
            }

            QSqlError sqlError;
            const dbid_t catchAllIdnId = insertVirtual(0, 0, catchAllAlias, d->username, d->username, 1, sqlError);
            if (catchAllIdnId == 0) {
                e.setSqlError(sqlError, c->translate("Account", "Account could not be set up as catch-all account."));
                qCWarning(SK_ACCOUNT, "%s failed to setup account %s as catch-all account for domain %s: %s", uniStr, aniStr, dniStr, qUtf8Printable(sqlError.text()));
            } else {
                if (dom->isIdn()) {
                    const dbid_t catchAllAceId = insertVirtual(catchAllIdnId, 0, catchAllAliasAce, d->username, d->username, 1, sqlError);
                    if (catchAllAceId > 0) {
                        q = CPreparedSqlQueryThread(QStringLiteral("UPDATE virtual SET ace_id = :ace_id WHERE id = :id"));
                        q.bindValue(QStringLiteral(":ace_id"), catchAllAceId);
                        q.bindValue(QStringLiteral(":id"), catchAllIdnId);
                        if (Q_LIKELY(q.exec())) {
                            d->catchAll = true;
                        } else {
                            d->catchAll = false;
                            q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM virtual WHERE alias = :alias"));
                            q.bindValue(QStringLiteral(":alias"), catchAllAlias);
                            if (Q_UNLIKELY(!q.exec())) {
                                qCCritical(SK_ACCOUNT, "%s failed to remove previously added IDN catch all address %s from database: %s", uniStr, qUtf8Printable(catchAllAlias), qUtf8Printable(q.lastError().text()));
                            }
                        }
                    } else {
                        e.setSqlError(sqlError, c->translate("Account", "Account could not be set up as catch-all account."));
                        qCWarning(SK_ACCOUNT, "%s failed to setup account %s as catch-all account for domain %s: %s", uniStr, aniStr, dniStr, qUtf8Printable(sqlError.text()));
                    }
                } else {
                    d->catchAll = true;
                }
            }

        } else if (!_catchAll && d->catchAll) {
            q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM virtual WHERE alias = :alias AND username = :username"));
            q.bindValue(QStringLiteral(":alias"), catchAllAlias);
            q.bindValue(QStringLiteral(":username"), d->username);

            if (Q_UNLIKELY(!q.exec())) {
                e.setSqlError(q.lastError(), c->translate("Account", "User account could not be removed as catch-all account for this domain."));
                qCWarning(SK_ACCOUNT, "%s failed to remove account %s as catch-all account for domain %s: %s", uniStr, aniStr, dniStr, qUtf8Printable(q.lastError().text()));
            } else {

                if (dom->isIdn()) {
                    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM virtual WHERE alias = :alias AND username = :username"));
                    q.bindValue(QStringLiteral(":alias"), catchAllAliasAce);
                    q.bindValue(QStringLiteral(":username"), d->username);

                    if (Q_UNLIKELY(!q.exec())) {
                        e.setSqlError(q.lastError(), c->translate("Account", "User account could not be completeley removed as catch-all account for this domain."));
                        qCWarning(SK_ACCOUNT, "%s failed to remove account %s as catch-all account for IDN domain %s: %s", uniStr, aniStr, dniStr, qUtf8Printable(q.lastError().text()));
                    }
                }
                d->catchAll = false;
            }
        }
    }

    q = CPreparedSqlQueryThread(QStringLiteral("UPDATE domain SET domainquotaused = (SELECT SUM(quota) FROM accountuser WHERE domain_id = :domain_id) WHERE id = :domain_id"));
    q.bindValue(QStringLiteral(":domain_id"), d->domainId);
    if (Q_UNLIKELY(!q.exec())) {
        qCWarning(SK_ACCOUNT, "%s failed to update used domain quota for domain %s after updating account %s: %s", uniStr, dniStr, aniStr, qUtf8Printable(q.lastError().text()));
    }

    d->validUntil = validUntil;
    d->passwordExpires = pwExpires;
    d->quota = quota;
    d->updated = currentTimeUtc;
    d->imap = imap;
    d->pop = pop;
    d->sieve = sieve;
    d->smtpauth = smtpauth;

    q = CPreparedSqlQueryThread(QStringLiteral("SELECT domainquotaused FROM domain WHERE id = :domain_id"));
    q.bindValue(QStringLiteral(":domain_id"), dom->id());
    if (Q_UNLIKELY(!q.exec())) {
        qCWarning(SK_ACCOUNT, "%s failed to query used domain quota for domain %s after updating account %s: %s", uniStr, dniStr, aniStr, qUtf8Printable(q.lastError().text()));
    } else {
        if (Q_LIKELY(q.next())) {
            dom->setDomainQuotaUsed(q.value(0).value<quota_size_t>());
        }
    }

    qCInfo(SK_ACCOUNT, "%s updated account %s in domain %s", uniStr, aniStr, dniStr);

    ret = true;

    return ret;
}

#define PAM_ACCT_EXPIRED 1
#define PAM_NEW_AUTHTOK_REQD 2

QStringList Account::check(Cutelyst::Context *c, SkaffariError &e, const Domain &domain, const Cutelyst::ParamsMultiMap &p)
{
    QStringList actions;

    Q_ASSERT_X(c, "check account", "invalid context");

    qCInfo(SK_ACCOUNT, "%s started checking user account %s.", qUtf8Printable(AdminAccount::getUserNameIdString(c)), qUtf8Printable(nameIdString()));

    Domain dom = domain;

    if (!dom.isValid() || (dom.id() != d->domainId)) {
        dom = Domain::fromStash(c);
        if (!dom.isValid() || (dom.id() != d->domainId)) {
            dom = Domain::get(c, d->domainId, e);
            if (!dom.isValid()) {
                return actions;
            }
        }
    }

    // for logging
    const QByteArray uniBa = AdminAccount::getUserNameIdString(c).toUtf8();
    const char *uniStr = uniBa.constData();
    const QByteArray aniBa = nameIdString().toUtf8();
    const char *aniStr = aniBa.constData();
    const QByteArray dniBa = dom.nameIdString().toUtf8();
    const char *dniStr = dniBa.constData();

    SkaffariIMAP imap(c);
    if (Q_UNLIKELY(!imap.login())) {
        e.setImapError(imap.lastError());
        qCCritical(SK_ACCOUNT, "%s failed to login as IMAP admin %s into IMAP server while checking user account %s: %s", uniStr, qUtf8Printable(SkaffariConfig::imapUser()), aniStr, qUtf8Printable(imap.lastError().errorText()));
        return actions;
    }

    const QStringList mboxes = imap.getMailboxes();

    if (mboxes.empty() && (imap.lastError().type() != SkaffariIMAPError::NoError)) {
        e.setImapError(imap.lastError(), c->translate("Account", "Could not retrieve a list of all mailboxes from the IMAP server."));
        qCCritical(SK_ACCOUNT, "%s failed to query a list of all mailboxes from the IMAP server while checking user account %s: %s", uniStr, aniStr, qUtf8Printable(imap.lastError().errorText()));
        imap.logout();
        return actions;
    }

    if ((SkaffariConfig::imapCreatemailbox() != DoNotCreate) && !mboxes.contains(d->username)) {
        if (Q_UNLIKELY(!imap.createMailbox(d->username))) {
            e.setImapError(imap.lastError());
            qCCritical(SK_ACCOUNT, "%s failed to create missing mailbox on IMAP server for user account %s: %s", uniStr, aniStr, qUtf8Printable(imap.lastError().errorText()));
            imap.logout();
            return actions;
        } else {
            qCInfo(SK_ACCOUNT, "%s created missing mailbox on IMAP server for user account %s.", uniStr, aniStr);
            actions.push_back(c->translate("Account", "Missing mailbox created on IMAP server."));
        }
    }

    quota_pair quota = imap.getQuota(d->username);

    if ((dom.domainQuota() > 0) && ((d->quota == 0) || (quota.second == 0))) {
        const quota_size_t newQuota = (dom.quota() > 0) ? dom.quota() : (SkaffariConfig::defQuota() > 0) ? SkaffariConfig::defQuota() : 10240;
        if (quota.second == 0) {
            if (Q_UNLIKELY(!imap.setQuota(d->username, newQuota))) {
                e.setImapError(imap.lastError());
                qCCritical(SK_ACCOUNT, "%s failed to set correct mailbox storage quota of %llu on IMAP sever for user account %s: %s", uniStr, newQuota, aniStr, qUtf8Printable(imap.lastError().errorText()));
                imap.logout();
                return actions;
            } else {
                qCInfo(SK_ACCOUNT, "%s set correct mailbox storage quota of %llu on IMAP server for user account %s.", uniStr, newQuota, aniStr);
                actions.push_back(c->translate("Account", "Storage quota on IMAP server fixed."));
                quota.second = newQuota;
            }
        }

        if (d->quota == 0) {
            QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("UPDATE accountuser SET quota = :quota WHERE id = :id"));
            q.bindValue(QStringLiteral(":quota"), newQuota);
            q.bindValue(QStringLiteral(":id"), d->id);
            if (Q_UNLIKELY(!q.exec())) {
                e.setSqlError(q.lastError());
                qCCritical(SK_ACCOUNT, "%s failed to update quota for account %s in database: %s", uniStr, aniStr, qUtf8Printable(q.lastError().text()));
                return actions;
            } else {
                qCInfo(SK_ACCOUNT, "%s set correct mailbox storage quota of %llu in database for user account %s.",  uniStr, newQuota, aniStr);
                actions.push_back(c->translate("Account", "Storage quota in database fixed."));
                d->quota = newQuota;

                q = CPreparedSqlQueryThread(QStringLiteral("UPDATE domain SET domainquotaused = (SELECT SUM(quota) FROM accountuser WHERE domain_id = :domain_id) WHERE id = :domain_id"));
                q.bindValue(QStringLiteral(":domain_id"), d->domainId);
                if (Q_UNLIKELY(!q.exec())) {
                    qCWarning(SK_ACCOUNT, "%s failed to update used domain quota for domain %s after checking account %s: %s", uniStr, dniStr, aniStr, qUtf8Printable(q.lastError().text()));
                }
            }
        }
    }

    if (quota.second != d->quota) {
        if (Q_UNLIKELY(!imap.setQuota(d->username, d->quota))) {
            e.setImapError(imap.lastError());
            qCCritical(SK_ACCOUNT, "%s failed to set correct mailbox storage quota of %llu on IMAP server for user account %s: %s", uniStr, d->quota, aniStr, qUtf8Printable(imap.lastError().text()));
            imap.logout();
            return actions;
        } else {
            qCInfo(SK_ACCOUNT, "%s set correct mailbox storage quota of %llu on IMAP server for user account %s.", uniStr, d->quota, aniStr);
            actions.push_back(c->translate("Account", "Storage quota on IMAP server fixed."));
            quota.second = d->quota;
        }
    }

    imap.logout();

    const QDateTime now = QDateTime::currentDateTimeUtc();

    quint8 newStatus = 0;
    bool newAccExpired = false;
    if (d->validUntil < now) {
        newStatus |= PAM_ACCT_EXPIRED;
        newAccExpired = true;
    }

    bool newPwExpired = false;
    if (d->passwordExpires < now) {
        newStatus |= PAM_NEW_AUTHTOK_REQD;
        newPwExpired = true;
    }

    if (d->status != newStatus) {
        bool oldAccExpired = ((d->status & PAM_ACCT_EXPIRED) == PAM_ACCT_EXPIRED);
        bool oldPwExpired = ((d->status & PAM_NEW_AUTHTOK_REQD) == PAM_NEW_AUTHTOK_REQD);
        if (oldAccExpired != newAccExpired) {
            if (!oldAccExpired && newAccExpired) {
                //: %1 will be a date and time
                actions.push_back(c->translate("Account", "Account was only valid until %1. The status of the account has been updated to “Expired”.").arg(c->locale().toString(d->validUntil, QLocale::ShortFormat)));
            } else {
                //: %1 will be a date and time
                actions.push_back(c->translate("Account", "Account was marked as “Expired”, but is valid again until %1. The status of the acocunt has been updated.").arg(c->locale().toString(d->validUntil, QLocale::ShortFormat)));
            }
        }

        if (oldPwExpired != newPwExpired) {
            if (!oldPwExpired && newPwExpired) {
                //: %1 will be a date and time
                actions.push_back(c->translate("Account", "Account password was only valid until %1. The status of the account has been updated to “Password Expired”.").arg(c->locale().toString(d->passwordExpires, QLocale::ShortFormat)));
            } else {
                actions.push_back(c->translate("Account", "Account was marked as “Password Expired“, but the password is valid again until %1. The status of the account has been updated.").arg(c->locale().toString(d->passwordExpires, QLocale::ShortFormat)));
            }
        }

        QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("UPDATE accountuser SET status = :status WHERE id = :id"));
        q.bindValue(QStringLiteral(":status"), newStatus);
        q.bindValue(QStringLiteral(":id"), d->id);

        if (Q_UNLIKELY(!q.exec())) {
            qCWarning(SK_ACCOUNT, "%s failed to update status for account %s in database: %s", uniStr, aniStr, qUtf8Printable(q.lastError().text()));
        } else {
            qCInfo(SK_ACCOUNT, "%s set correct status value of %i for user account ID %s.", uniStr, newStatus, aniStr);
        }
    }

    if (Utils::checkCheckbox(p, QStringLiteral("checkChildAddresses")) && !dom.children().empty()) {
        const QStringList addresses = d->addresses;
        if (!addresses.empty()) {
            QSqlQuery q;
            QStringList newAddresses;
            for (const QString &address : addresses) {
                std::pair<QString,QString> parts = addressParts(address);
                if (parts.second == dom.name()) {
                    for (const SimpleDomain &kid : dom.children()) {
                        const QString childAddress = parts.first + QLatin1Char('@') + QString::fromLatin1(QUrl::toAce(kid.name()));
                        q = CPreparedSqlQueryThread(QStringLiteral("SELECT dest FROM virtual WHERE alias = :alias"));
                        q.bindValue(QStringLiteral(":alias"), childAddress);

                        if (Q_LIKELY(q.exec())) {
                            if (!q.next()) {
                                QSqlQuery qq = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO virtual (alias, dest, username, status) VALUES (:alias, :dest, :username, :status)"));
                                qq.bindValue(QStringLiteral(":alias"), childAddress);
                                qq.bindValue(QStringLiteral(":dest"), d->username);
                                qq.bindValue(QStringLiteral(":username"), d->username);
                                qq.bindValue(QStringLiteral(":status"), 1);

                                if (Q_LIKELY(qq.exec())) {
                                    const QString newAddress = parts.first + QLatin1Char('@') + kid.name();
                                    newAddresses.push_back(newAddress);
                                    qCInfo(SK_ACCOUNT, "%s added a new address for child domain %s to account %s.", qUtf8Printable(AdminAccount::getUserNameIdString(c)), qUtf8Printable(kid.nameIdString()), qUtf8Printable(nameIdString()));
                                    actions.push_back(c->translate("Account", "Added new email address for child domain: %1").arg(newAddress));
                                } else {
                                    qCWarning(SK_ACCOUNT, "%s failed to add new email address for child domain %s while checking account %s: %s", uniStr, qUtf8Printable(kid.nameIdString()), aniStr, qUtf8Printable(qq.lastError().text()));
                                }
                            }
                        } else {
                            qCWarning(SK_ACCOUNT, "%s failed to check if email address %s is already in use while checking account %s: %s", uniStr, qUtf8Printable(childAddress), aniStr, qUtf8Printable(q.lastError().text()));
                        }
                    }
                }
            }
            if (!newAddresses.empty()) {
                d->addresses.append(newAddresses);
                if (d->addresses.size() > 1) {
                    QCollator col(c->locale());
                    std::sort(d->addresses.begin(), d->addresses.end(), col);
                }
            }
        }
    }

    if (actions.empty()) {
        qCInfo(SK_ACCOUNT, "Nothing to do for user account %s.", aniStr);
    } else {
        d->usage = quota.first;
        d->status = newStatus;
        markUpdated(c);
    }

    if (SkaffariConfig::useMemcached()) {
        Cutelyst::Memcached::set(MEMC_QUOTA_KEY + QString::number(d->id), QByteArray::number(d->usage), MEMC_QUOTA_EXP);
    }

    qCInfo(SK_ACCOUNT, "%s finished checking user account %s.", qUtf8Printable(AdminAccount::getUserNameIdString(c)), qUtf8Printable(nameIdString()));

    return actions;
}

QString Account::updateEmail(Cutelyst::Context *c, SkaffariError &e, const QVariantHash &p, const QString &oldAddress)
{
    QString ret;

    Q_ASSERT_X(c, "update email", "invalid context object");

    const dbid_t domId = p.value(QStringLiteral("newmaildomain")).value<dbid_t>();
    const Domain dom = Domain::get(c, domId, e);

    if (e.type() != SkaffariError::NoError) {
        return ret;
    }

    // for logging
    const QByteArray uniBa = AdminAccount::getUserNameIdString(c).toUtf8();
    const char *uniStr = uniBa.constData();
    const QByteArray aniBa = nameIdString().toUtf8();
    const char *aniStr = aniBa.constData();

    const QString localPart = p.value(QStringLiteral("newlocalpart")).toString();
    const QString address = localPart + QLatin1Char('@') + dom.name();

    if (address == oldAddress) {
        e.setErrorType(SkaffariError::InputError);
        e.setErrorText(c->translate("Account", "The email address has not been changed."));
        qCWarning(SK_ACCOUNT, "%s failed to update email address for account %s: address %s has not been changed.", uniStr, aniStr, qUtf8Printable(address));
        return ret;
    }

    if (!d->addresses.contains(oldAddress)) {
        e.setErrorType(SkaffariError::InputError);
        e.setErrorText(c->translate("Account", "The email address %1 is not part of this account.").arg(oldAddress));
        qCWarning(SK_ACCOUNT, "%s failed to udpate email address for account %s: address %s is not part of the account.", uniStr, aniStr, qUtf8Printable(oldAddress));
        return ret;
    }

    if (d->addresses.contains(address)) {
        e.setErrorType(SkaffariError::InputError);
        e.setErrorText(c->translate("Account", "The email address %1 is already part of this account.").arg(address));
        qCWarning(SK_ACCOUNT, "%s failed to update email address for account %s: address %s is alread part of the account.", uniStr, aniStr, qUtf8Printable(address));
        return ret;
    }

    if (!d->canAddAddress(c, e, dom, address)) {
        return ret;
    }

    const QString aceAddress = localPart + QLatin1Char('@') + dom.aceName();

    QList<Cutelyst::ValidatorEmail::Diagnose> diags;
    if (!Cutelyst::ValidatorEmail::validate(aceAddress, Cutelyst::ValidatorEmail::Valid, Cutelyst::ValidatorEmail::NoOption, &diags)) {
        e.setErrorType(SkaffariError::InputError);
        e.setErrorText(Cutelyst::ValidatorEmail::diagnoseString(c, diags.at(0)));
        qCWarning(SK_ACCOUNT, "%s failed to update email address for account %s: new address %s is not valid.", uniStr, aniStr, qUtf8Printable(address));
        return ret;
    }

    const EmailAddress a = EmailAddress::get(c, e, oldAddress);
    if (!a) {
        return ret;
    }

    QSqlDatabase db = QSqlDatabase::database(Cutelyst::Sql::databaseNameThread());
    if (Q_UNLIKELY(!db.transaction())) {
        e.setSqlError(db.lastError(), c->translate("Account", "Failed to update email address %1.").arg(oldAddress));
        qCCritical(SK_ACCOUNT, "%s failed to change email address %s of account %s to %s: %s", uniStr, qUtf8Printable(oldAddress), aniStr, qUtf8Printable(address), qUtf8Printable(db.lastError().text()));
        return ret;
    }

    QSqlQuery q = QSqlQuery(db);
    q.prepare(QStringLiteral("UPDATE virtual SET alias = :alias WHERE id = :id"));
    q.bindValue(QStringLiteral(":alias"), address);
    q.bindValue(QStringLiteral(":id"), a.id());

    if (Q_LIKELY(q.exec())) {
        if (a.isIdn() && dom.isIdn()) {
            q.prepare(QStringLiteral("UPDATE virtual SET alias = :alias WHERE id = :id"));
            q.bindValue(QStringLiteral(":alias"), aceAddress);
            q.bindValue(QStringLiteral(":id"), a.aceId());
            q.exec();
        } else if (a.isIdn() && !dom.isIdn()) {
            q.prepare(QStringLiteral("DELETE FROM virtual WHERE id = :id"));
            q.bindValue(QStringLiteral(":id"), a.aceId());
            q.exec();

            q.prepare(QStringLiteral("UPDATE virtual SET ace_id = 0 WHERE id = :id"));
            q.bindValue(QStringLiteral(":id"), a.id());
            q.exec();
        } else if (!a.isIdn() && dom.isIdn()) {
            q.prepare(QStringLiteral("INSERT INTO virtual (idn_id, ace_id, alias, dest, username, status) VALUES (:idn_id, 0, :alias, :dest, :username, 1)"));
            q.bindValue(QStringLiteral(":idn_id"), a.id());
            q.bindValue(QStringLiteral(":alias"), aceAddress);
            q.bindValue(QStringLiteral(":dest"), d->username);
            q.bindValue(QStringLiteral(":username"), d->username);
            q.exec();

            const dbid_t aceId = q.lastInsertId().value<dbid_t>();
            q.prepare(QStringLiteral("UPDATE virtual SET ace_id = :ace_id WHERE id = :id"));
            q.bindValue(QStringLiteral(":ace_id"), aceId);
            q.bindValue(QStringLiteral(":id"), a.id());
            q.exec();
        }
    }

    if (Q_UNLIKELY(!db.commit())) {
        e.setSqlError(db.lastError(), c->translate("Account", "Failed to update email address %1.").arg(oldAddress));
        qCCritical(SK_ACCOUNT, "%s failed to change email address %s of account %s to %s: %s", uniStr, qUtf8Printable(oldAddress), aniStr, qUtf8Printable(address), qUtf8Printable(db.lastError().text()));
        db.rollback();
        return ret;
    }

    d->addresses.removeOne(oldAddress);
    d->addresses.push_back(address);
    if (d->addresses.size() > 1) {
        QCollator col(c->locale());
        std::sort(d->addresses.begin(), d->addresses.end(), col);
    }

    qCInfo(SK_ACCOUNT, "%s updated email address %s of account %s to %s.", uniStr, qUtf8Printable(oldAddress), aniStr, qUtf8Printable(address));

    ret = address;

    markUpdated(c);

    return ret;
}

QString Account::addEmail(Cutelyst::Context *c, SkaffariError &e, const QVariantHash &p)
{
    QString ret;

    Q_ASSERT_X(c, "add email", "invalid context object");

    const dbid_t domId = p.value(QStringLiteral("newmaildomain")).value<dbid_t>();
    const Domain dom = Domain::get(c, domId, e);

    if (e.type() != SkaffariError::NoError) {
        return ret;
    }

    // for loggin
    const QByteArray uniBa = AdminAccount::getUserNameIdString(c).toUtf8();
    const char *uniStr = uniBa.constData();
    const QByteArray aniBa = nameIdString().toUtf8();
    const char *aniStr = aniBa.constData();

    const QString localPart = p.value(QStringLiteral("newlocalpart")).toString();
    const QString address = localPart + QLatin1Char('@') + dom.name();

    if (d->addresses.contains(address)) {
        e.setErrorType(SkaffariError::InputError);
        e.setErrorText(c->translate("Account", "The email address %1 is already part of this account.").arg(address));
        qCWarning(SK_ACCOUNT, "%s failed to add email address to account %s: address %s is already part of the account.", uniStr, aniStr, qUtf8Printable(address));
        return ret;
    }

    if (!d->canAddAddress(c, e, dom, address)) {
        return ret;
    }

    const QString aceAddress = localPart + QLatin1Char('@') + QString::fromLatin1(QUrl::toAce(dom.name()));

    QList<Cutelyst::ValidatorEmail::Diagnose> diags;
    if (!Cutelyst::ValidatorEmail::validate(aceAddress, Cutelyst::ValidatorEmail::Valid, Cutelyst::ValidatorEmail::NoOption, &diags)) {
        e.setErrorType(SkaffariError::InputError);
        e.setErrorText(Cutelyst::ValidatorEmail::diagnoseString(c, diags.at(0)));
        qCWarning(SK_ACCOUNT, "%s failed to add email address to account %s: address %s is not valid.", uniStr, aniStr, qUtf8Printable(address));
        return ret;
    }

    QSqlError sqlError;
    const dbid_t emailIdnId = insertVirtual(0, 0, address, d->username, d->username, 1, sqlError);

    if (emailIdnId == 0) {
        e.setSqlError(sqlError, c->translate("Account", "New email address could not be added to database."));
        qCCritical(SK_ACCOUNT, "%s failed to insert new email address %s for account %s into database: %s", uniStr, qUtf8Printable(address), aniStr, qUtf8Printable(sqlError.text()));
        return ret;
    } else {
        if (dom.isIdn()) {
            const dbid_t emailAceId = insertVirtual(emailIdnId, 0, aceAddress, d->username, d->username, 1, sqlError);
            if (emailAceId == 0) {
                e.setSqlError(sqlError, c->translate("Account", "New email address could not be added to database."));
                qCCritical(SK_ACCOUNT, "%s failed to insert new email address %s for account %s into database: %s", uniStr, qUtf8Printable(address), aniStr, qUtf8Printable(sqlError.text()));
                removeVirtualByID(emailIdnId);
                return ret;
            } else {
                sqlError = updateAceID(emailIdnId, emailAceId);
                if (Q_UNLIKELY(sqlError.type() != QSqlError::NoError)) {
                    e.setSqlError(sqlError, c->translate("Account", "New email address could not be added to database."));
                    qCCritical(SK_ACCOUNT, "%s failed to insert new email address %s for account %s into database: %s", uniStr, qUtf8Printable(address), aniStr, qUtf8Printable(sqlError.text()));
                    removeVirtualByID(emailIdnId);
                    removeVirtualByID(emailAceId);
                    return ret;
                }
            }
        }
    }

    d->addresses.push_back(address);
    if (d->addresses.size() > 1) {
        QCollator col(c->locale());
        std::sort(d->addresses.begin(), d->addresses.end(), col);
    }

    qCInfo(SK_ACCOUNT, "%s added new email address %s to account %s.", uniStr, qUtf8Printable(address), aniStr);

    ret = address;

    markUpdated(c);

    return ret;
}

bool Account::removeEmail(Cutelyst::Context *c, SkaffariError &e, const QString &address)
{
    bool ret = false;

    Q_ASSERT_X(c, "update email", "invalid context object");

    // for loggin
    const QByteArray uniBa = AdminAccount::getUserNameIdString(c).toUtf8();
    const char *uniStr = uniBa.constData();
    const QByteArray aniBa = nameIdString().toUtf8();
    const char *aniStr = aniBa.constData();

    if (d->addresses.size() <= 1) {
        e.setErrorType(SkaffariError::InputError);
        e.setErrorText(c->translate("Account", "You can not remove the last email address for this account. Remove the entire account instead."));
        qCWarning(SK_ACCOUNT, "%s failed to remove email address from account %s: address %s is the last address of the account.", uniStr, aniStr, qUtf8Printable(address));
        return ret;
    }

    if (!d->addresses.contains(address)) {
        e.setErrorType(SkaffariError::InputError);
        e.setErrorText(c->translate("Account", "The email address %1 is not part of this account.").arg(address));
        qCWarning(SK_ACCOUNT, "%s failed to remove email address from account %s: address %s is not part of the account.", uniStr, aniStr, qUtf8Printable(address));
        return ret;
    }

    const EmailAddress a = EmailAddress::get(c, e, address);
    if (!a) {
        return ret;
    }

    const Domain myDomain = Domain::get(c, domainId(), e);
    if (!myDomain.isValid() || (e.type() != SkaffariError::NoError)) {
        return ret;
    }

    if (a.domainPart() == myDomain.name()) {
        int domainAddressCount = 0;
        const QStringList myAddresses = d->addresses;
        for (const QString &addr : myAddresses) {
            if (Account::addressParts(addr).second == myDomain.name()) {
                domainAddressCount++;
            }
        }

        if (domainAddressCount < 2) {
            e.setErrorType(SkaffariError::InputError);
            e.setErrorText(c->translate("Account", "You can not remove the last email address that matches the domain this account belongs to."));
            qCWarning(SK_ACCOUNT, "%s failed to remove email address from acount %s: address %s is the last domain address of the account.", uniStr, aniStr, qUtf8Printable(address));
            return ret;
        }
    }

    QSqlError sqlError;
    if (a.isIdn()) {
        sqlError = removeVirtualByID(a.aceId());
        if (sqlError.type() != QSqlError::NoError) {
            e.setSqlError(sqlError, c->translate("Account", "Email address %1 could not be removed from user account %2.").arg(address, d->username));
            return ret;
        }
    }

    sqlError = removeVirtualByID(a.id());
    if (sqlError.type() != QSqlError::NoError) {
        e.setSqlError(sqlError, c->translate("Account", "Email address %1 could not be removed from user account %2.").arg(address, d->username));
        return ret;
    }

    d->addresses.removeOne(address);

    qCInfo(SK_ACCOUNT, "%s removed email address %s from account %s.", uniStr, qUtf8Printable(address), aniStr);

    ret = true;

    markUpdated(c);

    return ret;
}

bool Account::addForward(Cutelyst::Context *c, SkaffariError &e, const QString &forward)
{
    bool ret = false;

    Q_ASSERT_X(c, "add forward", "invalid context object");
    Q_ASSERT_X(!forward.isEmpty(), "add forward", "empty new forward");

    std::pair<QStringList,bool> forwards = queryFowards(c, username(), &e);
    if (e.type() != SkaffariError::NoError) {
        return ret;
    }

    // used for logging
    const QByteArray uniStrBa = AdminAccount::getUserNameIdString(c).toUtf8();
    const char *uniStr = uniStrBa.constData();
    const QByteArray aniStrBa = nameIdString().toUtf8();
    const char *aniStr = aniStrBa.constData();
    const QByteArray fwStrBa = forward.toUtf8();
    const char *fwStr = fwStrBa.constData();

    const bool oldDataAvailable = !forwards.first.empty();

    if (Q_UNLIKELY(forwards.first.contains(forward, Qt::CaseInsensitive))) {
        e.setErrorType(SkaffariError::InputError);
        e.setErrorText(c->translate("Account", "Emails to account %1 are already forwarded to %2.").arg(d->username, forward));
        qCWarning(SK_ACCOUNT, "%s failed to add new forward address to account %s: forward to %s already exists.", uniStr, aniStr, fwStr);
        return ret;
    }

    forwards.first.append(forward);

    QSqlQuery q;
    if (oldDataAvailable) {
        q = CPreparedSqlQueryThread(QStringLiteral("UPDATE virtual SET dest = :dest WHERE alias = :alias AND username = ''"));
    } else {
        q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO virtual (alias, dest, username) VALUES (:alias, :dest, '')"));
    }
    if (!forwards.second) {
        q.bindValue(QStringLiteral(":dest"), forwards.first.join(QLatin1Char(',')));
    } else {
        QStringList _fws = forwards.first;
        _fws.append(d->username);
        q.bindValue(QStringLiteral(":dest"), _fws.join(QLatin1Char(',')));
    }
    q.bindValue(QStringLiteral(":alias"), d->username);

    if (Q_UNLIKELY(!q.exec())) {
        e.setSqlError(q.lastError(), c->translate("Account", "Cannot update the list of forwarding addresses for user account %1 in the database.").arg(d->username));
        qCCritical(SK_ACCOUNT, "%s failed to update the list of forwarding addresses for user account %s in the database: %s", uniStr, aniStr, qUtf8Printable(q.lastError().text()));
        return ret;
    }

    d->forwards = forwards.first;

    qCInfo(SK_ACCOUNT, "%s added new forward address %s to account %s.", uniStr, fwStr, aniStr);

    ret = true;

    markUpdated(c);

    return ret;
}

bool Account::removeForward(Cutelyst::Context *c, SkaffariError &e, const QString &forward)
{
    bool ret = false;

    Q_ASSERT_X(c, "add forward", "invalid context object");
    Q_ASSERT_X(!forward.isEmpty(), "add forward", "empty input parameters");

    std::pair<QStringList,bool> forwards = queryFowards(c, username(), &e);
    if (e.type() != SkaffariError::NoError) {
        return ret;
    }

    // used for logging
    const QByteArray uniStrBa = AdminAccount::getUserNameIdString(c).toUtf8();
    const char *uniStr = uniStrBa.constData();
    const QByteArray aniStrBa = nameIdString().toUtf8();
    const char *aniStr = aniStrBa.constData();
    const QByteArray fwStrBa = forward.toUtf8();
    const char *fwStr = fwStrBa.constData();

    if (Q_UNLIKELY(!forwards.first.contains(forward, Qt::CaseInsensitive))) {
        e.setErrorType(SkaffariError::InputError);
        e.setErrorText(c->translate("Account", "Forwarding address %1 cannot be removed from user account %2. Forwarding does not exist for this account.").arg(forward, d->username));
        qCWarning(SK_ACCOUNT, "%s failed to remove forward address from account %s: forward to %s does not exist.", uniStr, aniStr, fwStr);
        return ret;
    }

    forwards.first.removeAll(forward);

    QSqlQuery q;
    if (forwards.first.empty() || ((forwards.first.size() == 1) && (forwards.first.at(0) == d->username))) {

        q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM virtual WHERE alias = :username AND username = ''"));
        q.bindValue(QStringLiteral(":username"), d->username);

        if (Q_UNLIKELY(!q.exec())) {
            e.setSqlError(q.lastError(), c->translate("Account", "Forwarding addresses for user account %1 cannot be deleted from the database.").arg(d->username));
            qCCritical(SK_ACCOUNT, "%s failed to remove all forwards of account %s from the database: %s", uniStr, aniStr, qUtf8Printable(q.lastError().text()));
            return ret;
        }

        d->keepLocal = false;

    } else {

        q = CPreparedSqlQueryThread(QStringLiteral("UPDATE virtual SET dest = :dest WHERE alias = :alias AND username = ''"));
        q.bindValue(QStringLiteral(":alias"), d->username);
        if (!forwards.second) {
            q.bindValue(QStringLiteral(":dest"), forwards.first.join(QLatin1Char(',')));
        } else {
            QStringList _fws = forwards.first;
            _fws.append(d->username);
            q.bindValue(QStringLiteral(":dest"), _fws.join(QLatin1Char(',')));
        }

        if (Q_UNLIKELY(!q.exec())) {
            e.setSqlError(q.lastError(), c->translate("Account", "Cannot update the list of forwarding addresses for user account %1 in the database.").arg(d->username));
            qCCritical(SK_ACCOUNT, "%s failed to update list of forward email addresses for account %s in the database after removing one forward address: %s", uniStr, aniStr, qUtf8Printable(q.lastError().text()));
            return ret;
        }

    }

    d->forwards = forwards.first;

    qCInfo(SK_ACCOUNT, "%s removed forward address %s from account %s.", uniStr, fwStr, aniStr);

    ret = true;

    markUpdated(c);

    return ret;
}

bool Account::editForward(Cutelyst::Context *c, SkaffariError &e, const QString &oldForward, const QString &newForward)
{
    bool ret = false;

    Q_ASSERT_X(c, "edit forward", "invalid context object");
    Q_ASSERT_X(!oldForward.isEmpty(), "edit forward", "old forward address can not be empty");
    Q_ASSERT_X(!newForward.isEmpty(), "edit forward", "new forward address can not be empty");

    std::pair<QStringList,bool> forwards = queryFowards(c, username(), &e);
    if (e.type() != SkaffariError::NoError) {
        return ret;
    }

    // used for logging
    const QByteArray uniStrBa = AdminAccount::getUserNameIdString(c).toUtf8();
    const char *uniStr = uniStrBa.constData();
    const QByteArray aniStrBa = nameIdString().toUtf8();
    const char *aniStr = aniStrBa.constData();
    const QByteArray ofwStrBa = oldForward.toUtf8();
    const char *ofwStr = ofwStrBa.constData();
    const QByteArray nfwStrBa = newForward.toUtf8();
    const char *nfwStr = nfwStrBa.constData();

    if (Q_UNLIKELY(!forwards.first.contains(oldForward, Qt::CaseInsensitive))) {
        e.setErrorType(SkaffariError::InputError);
        e.setErrorText(c->translate("Account", "Can not change forward email address %1 from account %2. The forward does not exist.").arg(oldForward, d->username));
        qCWarning(SK_ACCOUNT, "%s failed to change forward address of account %s: forward to %s does not exist.", uniStr, aniStr, ofwStr);
        return ret;
    }

    if (Q_UNLIKELY(forwards.first.contains(newForward, Qt::CaseInsensitive))) {
        e.setErrorType(SkaffariError::InputError);
        e.setErrorText(c->translate("Account", "Forwarding address %1 for user account %2 cannot be changed to %3. The new forwarding already exists.").arg(oldForward, d->username, newForward));
        qCWarning(SK_ACCOUNT, "%s failed to change forward address of account %s: forward to %s already exists.", uniStr, aniStr, nfwStr);
        return ret;
    }

    forwards.first.removeAll(oldForward);
    forwards.first.append(newForward);

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("UPDATE virtual SET dest = :dest WHERE alias = :alias AND username = ''"));
    q.bindValue(QStringLiteral(":alias"), d->username);
    if (!forwards.second) {
        q.bindValue(QStringLiteral(":dest"), forwards.first.join(QLatin1Char(',')));
    } else {
        QStringList _fws = forwards.first;
        _fws.append(d->username);
        q.bindValue(QStringLiteral(":dest"), _fws.join(QLatin1Char(',')));
    }

    if (Q_UNLIKELY(!q.exec())) {
        e.setSqlError(q.lastError(), c->translate("Account", "Cannot update the list of forwarding addresses for user account %1 in the database.").arg(d->username));
        qCCritical(SK_ACCOUNT, "%s failed to update list of forward email addresses for account %s in the database after changing one forward address: %s", uniStr, aniStr, qUtf8Printable(q.lastError().text()));
        return ret;
    }

    d->forwards = forwards.first;

    qCInfo(SK_ACCOUNT, "%s changed forward address %s of account %s to %s.", uniStr, ofwStr, aniStr, nfwStr);

    ret = true;

    markUpdated(c);

    return ret;
}

bool Account::changeKeepLocal(Cutelyst::Context *c, SkaffariError &e, bool keepLocal)
{
    bool ret = false;

    Q_ASSERT_X(c, "edit forward", "invalid context object");

    std::pair<QStringList,bool> forwards = queryFowards(c, username(), &e);
    if (e.type() != SkaffariError::NoError) {
        return ret;
    }

    if ((keepLocal && (!forwards.second)) || (!keepLocal && (forwards.second))) {

        // used for logging
        const QByteArray uniStrBa = AdminAccount::getUserNameIdString(c).toUtf8();
        const char *uniStr = uniStrBa.constData();
        const QByteArray aniStrBa = nameIdString().toUtf8();
        const char *aniStr = aniStrBa.constData();

        if (keepLocal) {
            forwards.first.append(d->username);
        } else {
            forwards.first.removeAll(d->username);
        }

        QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("UPDATE virtual SET dest = :dest WHERE alias = :alias AND username = ''"));
        q.bindValue(QStringLiteral(":alias"), d->username);
        q.bindValue(QStringLiteral(":dest"), forwards.first.join(QLatin1Char(',')));

        if (Q_UNLIKELY(!q.exec())) {
            if (keepLocal) {
                e.setSqlError(q.lastError(), c->translate("Account", "Failed to enable the keeping of forwarded emails in the local mail box for account %1 in the database.").arg(d->username));
                qCCritical(SK_ACCOUNT, "%s failed to enable keeping of forwarded emails in the local mail box for account %s in the database: %s", uniStr, aniStr, qUtf8Printable(q.lastError().text()));
            } else {
                e.setSqlError(q.lastError(), c->translate("Account", "Failed to disable the keeping of forwarded emails in the local mail box for account %1 in the database.").arg(d->username));
                qCCritical(SK_ACCOUNT, "%s failed to disable keeping of forwarded emails in the local mail box for account %s in the database: %s", uniStr, aniStr, qUtf8Printable(q.lastError().text()));
            }
            return ret;
        }

        d->keepLocal = keepLocal;

        qCInfo(SK_ACCOUNT, "%s changed keeping of forwaded email in the local mailbox of account %s to %s.", uniStr, aniStr, d->keepLocal ? "true" : "false");

    }

    ret = true;

    markUpdated(c);

    return ret;
}

QString Account::addressFromACE(const QString &address)
{
    QString addressUtf8;

    const int atIdx = address.lastIndexOf(QLatin1Char('@'));
    const QStringRef addressDomainPart = address.midRef(atIdx + 1);
    const QStringRef addressLocalPart = address.leftRef(atIdx);
    addressUtf8 = addressLocalPart + QLatin1Char('@') + QUrl::fromAce(addressDomainPart.toLatin1());

    return addressUtf8;
}

QString Account::addressToACE(const QString &address)
{
    QString addressACE;

    const int atIdx = address.lastIndexOf(QLatin1Char('@'));
    const QStringRef addressDomainPart = address.midRef(atIdx + 1);
    if (!addressDomainPart.isEmpty()) {
        const QStringRef addressLocalPart = address.leftRef(atIdx);
        if (!addressLocalPart.isEmpty()) {
            addressACE = addressLocalPart + QLatin1Char('@') + QString::fromLatin1(QUrl::toAce(addressDomainPart.toString()));
        }
    }

    return addressACE;
}

quint8 Account::calcStatus(const QDateTime validUntil, const QDateTime pwExpires)
{
    quint8 _stat = 0;

    const QDateTime _validUntil = (validUntil.timeSpec() == Qt::UTC) ? validUntil : validUntil.toUTC();
    const QDateTime _pwExpires = (pwExpires.timeSpec() == Qt::UTC) ? pwExpires : pwExpires.toUTC();
    const QDateTime now = QDateTime::currentDateTimeUtc();

    if (_validUntil < now) {
        _stat |= PAM_ACCT_EXPIRED;
    }

    if (_pwExpires < now) {
        _stat |= PAM_NEW_AUTHTOK_REQD;
    }

    return _stat;
}

std::pair<QString,QString> Account::addressParts(const QString &address)
{
    std::pair<QString,QString> parts;

    const int atIdx = address.lastIndexOf(QLatin1Char('@'));
    parts.first = address.left(atIdx);
    parts.second = address.mid(atIdx + 1);

    return parts;
}

void Account::markUpdated(Cutelyst::Context *c)
{
    const QDateTime current = QDateTime::currentDateTimeUtc();
    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("UPDATE accountuser SET updated_at = :updated_at WHERE id = :id"));
    q.bindValue(QStringLiteral(":updated_at"), current);
    q.bindValue(QStringLiteral(":id"), d->id);

    if (Q_UNLIKELY(!q.exec())) {
        qCCritical(SK_ACCOUNT, "%s failed to update date and time account %s has been last updated in the database: %s", qUtf8Printable(AdminAccount::getUserNameIdString(c)), qUtf8Printable(nameIdString()), qUtf8Printable(q.lastError().text()));
    }

    d->updated = current;
}

QString AccountData::nameIdString() const
{
    QString ret;
    ret = username + QLatin1String(" (ID: ") + QString::number(id) + QLatin1Char(')');
    return ret;
}

bool AccountData::canAddAddress(Cutelyst::Context *c, SkaffariError &e, const Domain &targetDomain, const QString &address) const
{
    bool ret = false;

    Q_ASSERT(c);

    const Domain myDomain = (targetDomain.id() == domainId) ? targetDomain : Domain::get(c, domainId, e);
    if ((e.type() != SkaffariError::NoError) || !myDomain.isValid()) {
        return ret;
    }

    if (!myDomain.isFreeAddressEnabled()) {
        bool addressAllowed = (targetDomain.id() == domainId);
        if (!addressAllowed) {
            for (const SimpleDomain &child : myDomain.children()) {
                if (child.id() == targetDomain.id()) {
                    addressAllowed = true;
                    break;
                }
            }
        }

        if (!addressAllowed) {
            e.setErrorText(c->translate("Account", "You can not create email addresses for other domains as long as free addresses are not allowed for this domain."));
            e.setErrorType(SkaffariError::AuthorizationError);
            qCWarning(SK_ACCOUNT, "%s tried to add email address %s to account %s while free addresses are not allowed for domain %s.", qUtf8Printable(AdminAccount::getUserNameIdString(c)), qUtf8Printable(address), qUtf8Printable(nameIdString()), qUtf8Printable(myDomain.nameIdString()));
            return ret;
        }
    }

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT alias, dest, username FROM virtual WHERE alias = :address"));
    q.bindValue(QStringLiteral(":address"), address);

    if (Q_UNLIKELY(!q.exec())) {
        e.setSqlError(q.lastError(), c->translate("Account", "Unable to check if the new email address %1 is already assigned to another user account.").arg(address));
        qCCritical(SK_ACCOUNT, "%s failed to check if new email address %s for account %s is already assigned to another account: %s", qUtf8Printable(AdminAccount::getUserNameIdString(c)), qUtf8Printable(address), qUtf8Printable(nameIdString()), qUtf8Printable(q.lastError().text()));
        return ret;
    }

    if (Q_UNLIKELY(q.next())) {
        QString otherUser = q.value(2).toString();
        if (otherUser.isEmpty()) {
            otherUser = q.value(1).toString();
        }
        e.setErrorType(SkaffariError::InputError);
        e.setErrorText(c->translate("Account", "Email address %1 is already assigned to another user account.").arg(address));
        qCWarning(SK_ACCOUNT, "%s tried to add email address %s to account %s that is already assigned to %s.", qUtf8Printable(AdminAccount::getUserNameIdString(c)), qUtf8Printable(address), qUtf8Printable(nameIdString()), qUtf8Printable(otherUser));
        return ret;
    }

    ret = true;

    return ret;
}

QDebug operator<<(QDebug dbg, const Account &account)
{
    QDebugStateSaver saver(dbg);
    Q_UNUSED(saver);
    dbg.nospace() << "Account(";
    dbg << "ID: " << account.id();
    dbg << ", Username: " << account.username();
    dbg << ", Domain ID: " << account.domainId();
    dbg << ", Status: " << account.status();
    dbg << ", Quota: " << account.usage() << '/' << account.quota() << "KiB";
    dbg << ", IMAP: " << account.isImapEnabled();
    dbg << ", POP: " << account.isPopEnabled();
    dbg << ", SMTP: " << account.isSmtpauthEnabled();
    dbg << ", SIEVE: " << account.isSieveEnabled();
    dbg << ", Catch All: " << account.catchAll();
    dbg << ", Addresses: " << account.addresses();
    dbg << ", Forwards: " << account.forwards();
    if (!account.forwards().empty()) {
        dbg << ", Keep Local: " << account.keepLocal();
    }
    dbg << ", Created: " << account.created();
    dbg << ", Updated: " << account.updated();
    dbg << ", Valid Until: " << account.validUntil();
    dbg << ", Password Expires: " << account.passwordExpires();
    dbg << ')';
    return dbg.maybeSpace();
}

QDataStream &operator<<(QDataStream &stream, const Account &account)
{
    stream << account.quota() << account.usage() << account.addresses()
           << account.forwards() << account.username() << account.created()
           << account.updated() << account.validUntil() << account.passwordExpires()
           << account.id() << account.domainId() << account.status() << account.isImapEnabled()
           << account.isPopEnabled() << account.isSieveEnabled() << account.isSmtpauthEnabled()
           << account.keepLocal() << account.catchAll();

    return stream;
}

QDataStream &operator>>(QDataStream &stream, Account &account)
{
    stream >> account.d->quota;
    stream >> account.d->usage;
    stream >> account.d->addresses;
    stream >> account.d->forwards;
    stream >> account.d->username;
    stream >> account.d->created;
    stream >> account.d->updated;
    stream >> account.d->validUntil;
    stream >> account.d->passwordExpires;
    stream >> account.d->id;
    stream >> account.d->domainId;
    stream >> account.d->status;
    stream >> account.d->imap;
    stream >> account.d->pop;
    stream >> account.d->sieve;
    stream >> account.d->smtpauth;
    stream >> account.d->keepLocal;
    stream >> account.d->catchAll;

    return stream;
}
