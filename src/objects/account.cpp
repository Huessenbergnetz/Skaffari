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
#include "../utils/utils.h"
#include "../imap/skaffariimap.h"
#include "../../common/password.h"
#include "../utils/skaffariconfig.h"
#include <Cutelyst/Context>
#include <Cutelyst/Response>
#include <Cutelyst/Plugins/Utils/Sql>
#include <Cutelyst/Plugins/Session/Session>
#include <QSqlError>
#include <QSqlQuery>
#include <QTimeZone>
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

Account::Account() :
    d(new AccountData)
{

}


Account::Account(dbid_t id, dbid_t domainId, const QString& username, const QString &prefix, const QString &domainName, bool imap, bool pop, bool sieve, bool smtpauth, const QStringList &addresses, const QStringList &forwards, quota_size_t quota, quota_size_t usage, const QDateTime &created, const QDateTime &updated, const QDateTime &validUntil, const QDateTime &pwdExpiration, bool keepLocal, bool catchAll, quint8 status) :
    d(new AccountData(id, domainId, username, prefix, domainName, imap, pop, sieve, smtpauth, addresses, forwards, quota, usage, created, updated, validUntil, pwdExpiration, keepLocal, catchAll, status))
{

}


Account::Account(const Account &other) :
    d(other.d)
{

}


Account& Account::operator=(const Account &other)
{
    d = other.d;
    return *this;
}


Account::~Account()
{

}

dbid_t Account::getId() const
{
    return d->id;
}


void Account::setId(dbid_t nId)
{
    d->id = nId;
}


dbid_t Account::getDomainId() const
{
    return d->domainId;
}


void Account::setDomainId(dbid_t nDomainId)
{
    d->domainId = nDomainId;
}


QString Account::getUsername() const
{
    return d->username;
}


void Account::setUsername(const QString& nUsername)
{
    d->username = nUsername;
}



QString Account::getPrefix() const
{
    return d->prefix;
}



void Account::setPrefix(const QString &nPrefix)
{
    d->prefix = nPrefix;
}



QString Account::getDomainName() const
{
    return d->domainName;
}


void Account::setDomainName(const QString &nDomainName)
{
    d->domainName = nDomainName;
}



bool Account::isImapEnabled() const
{
    return d->imap;
}


void Account::setImapEnabled(bool nImap)
{
    d->imap = nImap;
}



bool Account::isPopEnabled() const
{
    return d->pop;
}


void Account::setPopEnabled(bool nPop)
{
    d->pop = nPop;
}



bool Account::isSieveEnabled() const
{
    return d->sieve;
}


void Account::setSieveEnabled(bool nSieve)
{
    d->sieve = nSieve;
}



bool Account::isSmtpauthEnabled() const
{
    return d->smtpauth;
}


void Account::setSmtpauthEnabled(bool nSmtpauth)
{
    d->smtpauth = nSmtpauth;
}



QStringList Account::getAddresses() const
{
    return d->addresses;
}


void Account::setAddresses(const QStringList &nAddresses)
{
    d->addresses = nAddresses;
}



QStringList Account::getForwards() const
{
    return d->forwards;
}



void Account::setForwards(const QStringList &nForwards)
{
    d->forwards = nForwards;
}




quota_size_t Account::getQuota() const
{
	return d->quota;
}


void Account::setQuota(quota_size_t nQuota)
{
	d->quota = nQuota;
}


QString Account::getHumanQuota() const
{
    return d->humanQuota;
}


void Account::setHumanQuota(const QString &humanQuota)
{
    d->humanQuota = humanQuota;
}


quota_size_t Account::getUsage() const
{
	return d->usage;
}


void Account::setUsage(quota_size_t nUsage)
{
	d->usage = nUsage;
}


QString Account::getHumanUsage() const
{
    return d->humanUsage;
}


void Account::setHumanUsage(const QString &humanUsage)
{
    d->humanUsage = humanUsage;
}


float Account::getUsagePercent() const
{
    if ((getQuota() == 0) && (getUsage() == 0)) {
		return 0;
	}
	return ((float)getUsage() / (float)getQuota()) * (float)100;
}


bool Account::isValid() const
{
    return ((d->id > 0) && (d->domainId > 0));
}


QDateTime Account::getCreated() const
{
    return d->created;
}

void Account::setCreated(const QDateTime &created)
{
    d->created = created;
}


QDateTime Account::getUpdated() const
{
    return d->updated;
}

void Account::setUpdated(const QDateTime &updated)
{
    d->updated = updated;
}


QDateTime Account::getValidUntil() const
{
    return d->validUntil;
}


void Account::setValidUntil(const QDateTime &validUntil)
{
    d->validUntil = validUntil;
}


bool Account::keepLocal() const
{
    return d->keepLocal;
}


void Account::setKeepLocal(bool nKeepLocal)
{
    d->keepLocal = nKeepLocal;
}


bool Account::cathAll() const
{
    return d->catchAll;
}


void Account::setCatchAll(bool nCatchAll)
{
    d->catchAll = nCatchAll;
}


QDateTime Account::passwordExpires() const
{
    return d->passwordExpires;
}


void Account::setPasswordExpires(const QDateTime &expirationDate)
{
    d->passwordExpires = expirationDate;
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


void Account::setStatus(quint8 status)
{
    d->status = status;
}


QJsonObject Account::toJson() const
{
    QJsonObject ao;

    ao.insert(QStringLiteral("id"), static_cast<qint64>(d->id));
    ao.insert(QStringLiteral("domainId"), static_cast<qint64>(d->domainId));
    ao.insert(QStringLiteral("username"), d->username);
    ao.insert(QStringLiteral("prefix"), d->prefix);
    ao.insert(QStringLiteral("domainName"), d->domainName);
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


Account Account::create(Cutelyst::Context *c, SkaffariError *e, const Cutelyst::ParamsMultiMap &p, const Domain &d)
{
    Account a;

    Q_ASSERT_X(c, "create account", "invalid context object");
    Q_ASSERT_X(e, "create account", "invalid error object");
    Q_ASSERT_X(!p.empty(), "create account", "empty parameters");
    Q_ASSERT_X(d.isValid(), "create account", "invalid domain object");

    // if domain as prefix is enabled, the username will be the local part of the email address
    // if additionally fqun is enabled, it will a fully qualified user name (email address like user@example.com
    // if both are disabled, the username will be the entered username
    const QString username = SkaffariConfig::imapDomainasprefix() ? p.value(QStringLiteral("localpart")).trimmed() + (SkaffariConfig::imapFqun() ? QLatin1Char('@') : QLatin1Char('.')) + d.getName() : p.value(QStringLiteral("username")).trimmed();

    // start checking if the username is already in use
    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT id FROM accountuser WHERE username = :username"));
    q.bindValue(QStringLiteral(":username"), username);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to query for already existing username."));
        qCCritical(SK_ACCOUNT, "Failed to query database for already existing username %s: %s", qUtf8Printable(username), qUtf8Printable(q.lastError().text()));
        return a;
    }

    if (Q_UNLIKELY(q.next())) {
        e->setErrorType(SkaffariError::InputError);
        e->setErrorText(c->translate("Account", "Username %1 is already in use.").arg(username));
        return a;
    }
    // end checking if the username is already in use

    // construct the email address from local part and domain name
    const QString email = p.value(QStringLiteral("localpart")) + QLatin1Char('@') + d.getName();

    // start checking if the email address is already in use
    q = CPreparedSqlQueryThread(QStringLiteral("SELECT username FROM virtual WHERE alias = :email"));
    q.bindValue(QStringLiteral(":email"), email);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to query for already existing email address."));
        qCCritical(SK_ACCOUNT, "Failed to query database for already existing email address %s: %s", qUtf8Printable(email), qUtf8Printable(q.lastError().text()));
        return a;
    }

    if (Q_UNLIKELY(q.next())) {
        e->setErrorText(c->translate("Account", "Email address %1 is already in use by user %2.").arg(email, q.value(0).toString()));
        e->setErrorType(SkaffariError::InputError);
        return a;
    }
    // end checking if the email address is already in use

    // start encrypting the password
    const QString password = p.value(QStringLiteral("password"));
    Password pw(password);
    const QByteArray encpw = pw.encrypt(SkaffariConfig::accPwMethod(), SkaffariConfig::accPwAlgorithm(), SkaffariConfig::accPwRounds());

    if (Q_UNLIKELY(encpw.isEmpty())) {
        e->setErrorText(c->translate("Account", "Failed to encrypt user password. Please check your encryption settings."));
        e->setErrorType(SkaffariError::ConfigError);
        qCCritical(SK_ACCOUNT, "Failed to encrypt user password. Please check your encryption settings.");
        return a;
    }
    // end encrypting the password

    const bool imap = (p.value(QStringLiteral("imap"), QStringLiteral("0")) == QLatin1String("1"));
    const bool pop = (p.value(QStringLiteral("pop"), QStringLiteral("0")) == QLatin1String("1"));
    const bool sieve = (p.value(QStringLiteral("sieve"), QStringLiteral("0")) == QLatin1String("1"));
    const bool smtpauth = (p.value(QStringLiteral("smtpauth"), QStringLiteral("0")) == QLatin1String("1"));
    const bool _catchAll = (p.value(QStringLiteral("catchall"), QStringLiteral("0")) == QLatin1String("1"));

    quota_size_t quota = 0;
    if (p.contains(QStringLiteral("humanQuota"))) {
        bool quotaOk = true;
        quota = Utils::humanToIntSize(c, p.value(QStringLiteral("humanQuota")), &quotaOk);
        if (!quotaOk) {
            e->setErrorType(SkaffariError::InputError);
            e->setErrorText(c->translate("Account", "Failed to convert human readable quota size string into valid integer value."));
            return a;
        }
    } else {
        quota = p.value(QStringLiteral("quota"), QStringLiteral("0")).toULongLong();
    }

    const QDateTime currentUtc = QDateTime::currentDateTimeUtc();

    // start converting the entered valid until time into UTC
    QTimeZone userTz(Cutelyst::Session::value(c, QStringLiteral("tz"), QStringLiteral("UTC")).toByteArray());
    QDateTime validUntil = QDateTime::fromString(p.value(QStringLiteral("validUntil"), QStringLiteral("2998-12-31 23:59:59")), QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    QDateTime pwExpires = QDateTime::fromString(p.value(QStringLiteral("passwordExpires"), QStringLiteral("2998-12-31 23:59:59")), QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    if (userTz == QTimeZone::utc()) {
        validUntil.setTimeSpec(Qt::UTC);
        pwExpires.setTimeSpec(Qt::UTC);
    } else {
        validUntil.setTimeZone(userTz);
        pwExpires.setTimeZone(userTz);
    }
    const QDateTime validUntilUTC = validUntil.toUTC();
    const QDateTime pwExpiresUTC = validUntil.toUTC();

    const quint8 accountStatus = Account::calcStatus(validUntilUTC, pwExpiresUTC);
    // end converting the entered valid until time into UTC

    q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO accountuser (domain_id, username, password, prefix, domain_name, imap, pop, sieve, smtpauth, quota, created_at, updated_at, valid_until, pwd_expire, status) "
                                         "VALUES (:domain_id, :username, :password, :prefix, :domain_name, :imap, :pop, :sieve, :smtpauth, :quota, :created_at, :updated_at, :valid_until, :pwd_expire, :status)"));

    q.bindValue(QStringLiteral(":domain_id"), d.id());
    q.bindValue(QStringLiteral(":username"), username);
    q.bindValue(QStringLiteral(":password"), encpw);
    q.bindValue(QStringLiteral(":prefix"), d.getPrefix());
    q.bindValue(QStringLiteral(":domain_name"), QUrl::toAce(d.getName()));
    q.bindValue(QStringLiteral(":imap"), imap);
    q.bindValue(QStringLiteral(":pop"), pop);
    q.bindValue(QStringLiteral(":sieve"), sieve);
    q.bindValue(QStringLiteral(":smtpauth"), smtpauth);
    q.bindValue(QStringLiteral(":quota"), quota);
    q.bindValue(QStringLiteral(":created_at"), currentUtc);
    q.bindValue(QStringLiteral(":updated_at"), currentUtc);
    q.bindValue(QStringLiteral(":valid_until"), validUntilUTC);
    q.bindValue(QStringLiteral(":pwd_expire"), pwExpiresUTC);
    q.bindValue(QStringLiteral(":status"), accountStatus);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to create new user account in database."));
        qCCritical(SK_ACCOUNT, "Failed to create new user account in database: %s", qUtf8Printable(q.lastError().text()));
        return a;
    }

    const dbid_t id = q.lastInsertId().value<dbid_t>();

    q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO virtual (alias, dest, username, status) VALUES (:alias, :dest, :username, :status)"));
    q.bindValue(QStringLiteral(":alias"), addressToACE(email));
    q.bindValue(QStringLiteral(":dest"), username);
    q.bindValue(QStringLiteral(":username"), username);
    q.bindValue(QStringLiteral(":status"), 1);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to insert email address new user account in database."));
        qCCritical(SK_ACCOUNT, "Failed to insert email address for new user account into database: %s", qUtf8Printable(q.lastError().text()));
        q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM accountuser WHERE id = :id"));
        q.bindValue(QStringLiteral(":id"), id);
        q.exec();
        return a;
    }

    // removing old catch all alias and setting a new one
    if (_catchAll) {
        const QString catchAllAlias = QLatin1Char('@') + QString::fromLatin1(QUrl::toAce(d.getName()));

        q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM virtual WHERE alias = :alias"));
        q.bindValue(QStringLiteral(":alias"), catchAllAlias);

        if (Q_UNLIKELY(!q.exec())) {
            e->setSqlError(q.lastError(), c->translate("Account", "Failed to remove old catch all address from database."));
            qCCritical(SK_ACCOUNT, "Failed to remove old catch all address for domain %s from database: %s", qUtf8Printable(d.getName()), qUtf8Printable(q.lastError().text()));

            q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM virtual WHERE username = :username"));
            q.bindValue(QStringLiteral(":username"), username);
            q.exec();

            q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM accountuser WHERE id = :id"));
            q.bindValue(QStringLiteral(":id"), id);
            q.exec();

            return a;
        }

        q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO virtual (alias, dest, username, status) VALUES (:alias, :dest, :username, :status)"));
        q.bindValue(QStringLiteral(":alias"), catchAllAlias);
        q.bindValue(QStringLiteral(":dest"), username);
        q.bindValue(QStringLiteral(":username"), username);
        q.bindValue(QStringLiteral(":status"), 1);

        if (Q_UNLIKELY(!q.exec())) {
            e->setSqlError(q.lastError(), c->translate("Account", "Failed to set this account as catch all account."));
            qCCritical(SK_ACCOUNT, "Failed to set new account %s as catch all account for domain %s: %s", qUtf8Printable(username), qUtf8Printable(d.getName()), qUtf8Printable(q.lastError().text()));

            q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM virtual WHERE username = :username"));
            q.bindValue(QStringLiteral(":username"), username);
            q.exec();

            q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM accountuser WHERE id = :id"));
            q.bindValue(QStringLiteral(":id"), id);
            q.exec();

            return a;
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
                e->setImapError(imap.lastError(), c->translate("Account", "Failed to login to IMAP server to let the server automatically create the mailbox and folders."));
            }
            imap.logout();

        } else if (createMailbox == OnlySetQuota) {

            imap.setUser(username);
            imap.setPassword(password);

            mailboxCreated = imap.login();
            if (!mailboxCreated) {
                e->setImapError(imap.lastError(), c->translate("Account", "Failed to login to IMAP server to let the server automatically create the mailbox and folders."));
            }

            imap.logout();

            if (mailboxCreated) {
                imap.setUser(SkaffariConfig::imapUser());
                imap.setPassword(SkaffariConfig::imapPassword());

                if (Q_LIKELY(imap.login())) {

                    if (Q_UNLIKELY(!imap.setQuota(username, quota))) {
                        e->setImapError(imap.lastError(), c->translate("Account", "Failed to set quota for new account."));
                        mailboxCreated = false;
                    }

                    imap.logout();

                } else {
                    e->setImapError(imap.lastError(), c->translate("Account", "Failed to login to IMAP server to set quota."));
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

                    if (!d.getFolders().empty()) {

                        imap.setUser(username);
                        imap.setPassword(password);

                        if (Q_LIKELY(imap.login())) {

                            const QVector<Folder> folders = d.getFolders();

                            for (const Folder &folder : folders) {
                                if (Q_UNLIKELY(!imap.createFolder(folder.getName()))) {
                                    qCWarning(SK_ACCOUNT) << "Failed to create default folder" << folder.getName() << "for new IMAP account" << username;
                                }
                            }

                            imap.logout();

                        } else {
                            qCWarning(SK_ACCOUNT) << "Failed to login into new IMAP account" << username << "to create default folders.";
                        }
                    }

                } else {
                    e->setImapError(imap.lastError(), c->translate("Account", "Failed to create new IMAP mailbox."));
                    imap.logout();
                    mailboxCreated = false;
                }

            } else {
                e->setImapError(imap.lastError(), c->translate("Account", "Failed to login to IMAP server to create new mailbox."));
                mailboxCreated = false;
            }
        }
    }
    // end creating the mailbox on the IMAP server, according to the skaffari settings

    // revert our changes to the database if mailbox creation failed
    if (!mailboxCreated) {
        q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM accountuser WHERE id = :id"));
        q.bindValue(QStringLiteral(":id"), id);
        q.exec();

        q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM virtual WHERE username = :username"));
        q.bindValue(QStringLiteral(":username"), username);
        q.exec();

        return a;
    }

    q = CPreparedSqlQueryThread(QStringLiteral("UPDATE domain SET accountcount = accountcount + 1, domainquotaused = domainquotaused + :quota WHERE id = :id"));
    q.bindValue(QStringLiteral(":quota"), quota);
    q.bindValue(QStringLiteral(":id"), d.id());
    if (Q_UNLIKELY(!q.exec())) {
        qCWarning(SK_ACCOUNT, "Failed to update count of accounts and domain quota usage after adding new acocunt to domain ID %u (%s): %s", d.id(), qUtf8Printable(d.getName()), qUtf8Printable(q.lastError().text()));
    }

    a.setId(id);
    a.setDomainId(d.id());
    a.setUsername(username);
    a.setPrefix(d.getPrefix());
    a.setDomainName(d.getName());
    a.setImapEnabled(imap);
    a.setPopEnabled(pop);
    a.setSieveEnabled(sieve);
    a.setSmtpauthEnabled(smtpauth);
    a.setQuota(quota);
    a.setAddresses(QStringList(email));
    const QDateTime currentUserTime = Utils::toUserTZ(c, currentUtc);
    a.setCreated(currentUserTime);
    a.setUpdated(currentUserTime);
    a.setValidUntil(Utils::toUserTZ(c, validUntil));
    a.setPasswordExpires(Utils::toUserTZ(c, pwExpires));
    a.setHumanQuota(Utils::humanBinarySize(c, a.getQuota() * 1024));
    a.setHumanUsage(Utils::humanBinarySize(c, a.getUsage() * 1024));
    a.setCatchAll(_catchAll);

    qCInfo(SK_ACCOUNT) << c->stash(QStringLiteral("userName")).toString() << "created a new account" << username << "for domain" << d.getName();

    return a;
}



bool Account::remove(Cutelyst::Context *c, SkaffariError *e, const QString &username, Domain *domain)
{
    bool ret = false;

    Q_ASSERT_X(c, "remove account", "invalid context object");
    Q_ASSERT_X(e, "remove account", "invalid error object");

    SkaffariIMAP imap(c);
    if (Q_UNLIKELY(!imap.login())) {
        e->setImapError(imap.lastError(), c->translate("Account", "Failed to login to IMAP server to delete account %1.").arg(username));
        qCCritical(SK_ACCOUNT, "Failed to login to IMAP server to delete acount %s: %s", qUtf8Printable(username), qUtf8Printable(imap.lastError().errorText()));
        return ret;
    }

    if (Q_UNLIKELY(!imap.setAcl(username, SkaffariConfig::imapUser()))) {
        // if Skaffari is responsible for mailbox creation, direct or indirect,
        // remove will fail if we can not delete the mailbox on the IMAP server
        if (SkaffariConfig::imapCreatemailbox() > DoNotCreate) {
            e->setImapError(imap.lastError(), c->translate("Account", "Failed to set ACL for IMAP admin to delete account %1.").arg(username));
            qCCritical(SK_ACCOUNT, "Failed to set ACL for IMAP admin to delete account %s: %s", qUtf8Printable(username), qUtf8Printable(imap.lastError().errorText()));
            imap.logout();
            return ret;
        }
    }

    if (!imap.deleteMailbox(username) && (SkaffariConfig::imapCreatemailbox() != DoNotCreate)) {
        // if Skaffari is responsible for mailbox creation, direct or indirect,
        // remove will fail if we can not delete the mailbox on the IMAP server
        if (SkaffariConfig::imapCreatemailbox() > DoNotCreate) {
            e->setImapError(imap.lastError(), c->translate("Account", "Failed to delete account %1 from IMAP server.").arg(username));
            qCCritical(SK_ACCOUNT, "Failed to delete account %s from IMAP server: %s", qUtf8Printable(username), qUtf8Printable(imap.lastError().errorText()));
            imap.logout();
            return ret;
        }
    }

    imap.logout();

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT quota FROM accountuser WHERE username = :username"));
    q.bindValue(QStringLiteral(":username"), username);

    const quota_size_t quota = (q.exec() && q.next()) ? q.value(0).value<quota_size_t>() : 0;

    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM alias WHERE username = :username"));
    q.bindValue(QStringLiteral(":username"), username);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to remove account's alias addresses from the database."));
        qCCritical(SK_ACCOUNT, "Failed to remove alias addresses from account %s from the database: %s", qUtf8Printable(username), qUtf8Printable(q.lastError().text()));
        return ret;
    }

    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM virtual WHERE username = :username"));
    q.bindValue(QStringLiteral(":username"), username);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to remove account's addresses from the database."));
        qCCritical(SK_ACCOUNT, "Failed to remove the addresses from account %s from the database: %s", qUtf8Printable(username), qUtf8Printable(q.lastError().text()));
        return ret;
    }

    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM virtual WHERE alias = :username AND username = ''"));
    q.bindValue(QStringLiteral(":username"), username);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to remove account's forward addresses from the database"));
        qCCritical(SK_ACCOUNT, "Failed to remove the forward addresses from account %s from the database: %s", qUtf8Printable(username), qUtf8Printable(q.lastError().text()));
        return ret;
    }

    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM accountuser WHERE username = :username"));
    q.bindValue(QStringLiteral(":username"), username);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to remove account from the database."));
        qCCritical(SK_ACCOUNT, "Failed to remove the account %s fromt the database: %s", qUtf8Printable(username), qUtf8Printable(q.lastError().text()));
        return ret;
    }

    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM log WHERE user = :username"));
    q.bindValue(QStringLiteral(":username"), username);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to delete log entries for this user from the database."));
        qCWarning(SK_ACCOUNT, "Failed to remove log entries for account %s from the database: %s", qUtf8Printable(username), qUtf8Printable(q.lastError().text()));
    }

    q = CPreparedSqlQueryThread(QStringLiteral("UPDATE domain SET accountcount = accountcount - 1, domainquotaused = domainquotaused - :quota WHERE id = :id"));
    q.bindValue(QStringLiteral(":quota"), quota);
    q.bindValue(QStringLiteral(":id"), domain->id());

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to update count of domain accounts and used quota in database."));
        qCWarning(SK_ACCOUNT, "Failed to update count of domain accounts and used quota for domain %s in database: %s", qUtf8Printable(domain->getName()), qUtf8Printable(q.lastError().text()));
    }

    domain->setAccounts(domain->getAccounts() - 1);
    domain->setDomainQuotaUsed(domain->getDomainQuotaUsed() - quota);

    qCInfo(SK_ACCOUNT) << c->stash(QStringLiteral("userName")).toString() << "removed account" << username << "from domain" << domain->getName();

    ret = true;

    return ret;
}


bool Account::remove(Cutelyst::Context *c, SkaffariError *e, Domain *d)
{
    bool ret = false;

    Q_ASSERT_X(c, "remove account", "invalid context object");
    Q_ASSERT_X(e, "remove account", "invalid error object");

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT username FROM accountuser WHERE domain_id = :domain_id"));
    q.bindValue(QStringLiteral(":domain_id"), d->id());

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to query the user accounts that belong to this domain."));
        qCCritical(SK_ACCOUNT, "Failed to query accounts to delete that belong to domain ID %u (%s): %s", d->id(), qUtf8Printable(d->getName()), qUtf8Printable(q.lastError().text()));
        return ret;
    }

    QStringList usernames;
    while (q.next()) {
        usernames << q.value(0).toString();
    }

    if (!usernames.empty()) {
        SkaffariError e2(c);
        for (int i = 0; i < usernames.size(); ++i) {
            if (!Account::remove(c, &e2, usernames.at(i), d)) {
                e->setErrorType(SkaffariError::ApplicationError);
                e->setErrorText(c->translate("Account", "Abort removing user accounts for the domain %1 because of the following error: %2").arg(d->getName(), e2.errorText()));
                return ret;
            }
        }
    }

    ret = true;

    return ret;
}



Cutelyst::Pagination Account::list(Cutelyst::Context *c, SkaffariError *e, const Domain &d, const Cutelyst::Pagination &p, const QString &sortBy, const QString &sortOrder, const QString &searchRole, const QString &searchString)
{
    Cutelyst::Pagination pag;
    std::vector<Account> lst;

    Q_ASSERT_X(c, "list accounts", "invalid context object");
    Q_ASSERT_X(e, "list accounts", "invalid error object");

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
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to query accounts from database."));
        qCCritical(SK_ACCOUNT, "Failed to query accounts of domain ID %u (%s) from the database: %s", d.id(), qUtf8Printable(d.getName()), qUtf8Printable(q.lastError().text()));
        return pag;
    }

    QSqlQuery countQuery = CPreparedSqlQueryThread(QStringLiteral("SELECT FOUND_ROWS()"));
    if (Q_UNLIKELY(!countQuery.exec())) {
        e->setSqlError(q.lastError(), c->translate("Acocunt", "Failed to query total result count."));
        qCCritical(SK_ACCOUNT, "Failed to query total result count of domain %s (ID: %u) from the database: %s", qUtf8Printable(d.getName()), d.id(), qUtf8Printable(q.lastError().text()));
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
        qCWarning(SK_ACCOUNT, "Failed to login to IMAP server. Omitting quota query while listing accounts for domain ID %u (%s).", d.id(), qUtf8Printable(d.getName()));
    }

    QCollator col(c->locale());
    const QString catchAllAlias = QLatin1Char('@') + QString::fromLatin1(QUrl::toAce(d.getName()));

    while (q.next()) {
        const dbid_t id = q.value(0).value<dbid_t>();
        const QString username = q.value(1).toString();

        QStringList emailAddresses;
        bool _catchAll = false;

        QSqlQuery q2 = CPreparedSqlQueryThread(QStringLiteral("SELECT alias FROM virtual WHERE dest = :username AND username = :username"));
        q2.bindValue(QStringLiteral(":username"), username);

        if (Q_LIKELY(q2.exec())) {
            while (q2.next()) {
                const QString address = q2.value(0).toString();
                if (!address.startsWith(QLatin1Char('@'))) {
                    emailAddresses << addressFromACE(address);
                } else {
                    if (address == catchAllAlias) {
                        _catchAll = true;
                    }
                }
            }
        } else {
            qCWarning(SK_ACCOUNT, "Failed to query email addresses of account ID %u (%s) from the database: %s", id, qUtf8Printable(username), qUtf8Printable(q.lastError().text()));
        }


        QStringList aliases;
        bool _keepLocal = false;

        q2 = CPreparedSqlQueryThread(QStringLiteral("SELECT dest FROM virtual WHERE alias = :username AND username = ''"));
        q2.bindValue(QStringLiteral(":username"), username);
        if (Q_LIKELY(q2.exec())) {
            while (q2.next()) {
                const QStringList destinations = q2.value(0).toString().split(QLatin1Char(','));
                if (!destinations.empty()) {
                    for (const QString &dest : destinations) {
                        if (dest != username) {
                            aliases << dest;
                        } else {
                            _keepLocal = true;
                        }
                    }
                }
            }
        } else {
            qCWarning(SK_ACCOUNT, "Failed to query forward email addresses of account ID %u (%s) from the database: %s", id, qUtf8Printable(username), qUtf8Printable(q.lastError().text()));
        }

        if ((emailAddresses.size() > 1) || (aliases.size() > 1)) {

            if (emailAddresses.size() > 1) {
                std::sort(emailAddresses.begin(), emailAddresses.end(), col);
            }

            if (aliases.size() > 1) {
                std::sort(aliases.begin(), aliases.end(), col);
            }
        }


        Account a(q.value(0).value<dbid_t>(),
                  d.id(),
                  q.value(1).toString(),
                  d.getPrefix(),
                  d.getName(),
                  q.value(2).toBool(),
                  q.value(3).toBool(),
                  q.value(4).toBool(),
                  q.value(5).toBool(),
                  emailAddresses,
                  aliases,
                  q.value(6).value<quota_size_t>(),
                  0,
                  Utils::toUserTZ(c, q.value(7).toDateTime()),
                  Utils::toUserTZ(c, q.value(8).toDateTime()),
                  Utils::toUserTZ(c, q.value(9).toDateTime()),
                  Utils::toUserTZ(c, q.value(10).toDateTime()),
                  _keepLocal,
                  _catchAll,
                  q.value(11).value<quint8>());

        if (Q_LIKELY(imap.isLoggedIn())) {
            quota_pair quota = imap.getQuota(a.getUsername());
            a.setUsage(quota.first);
            a.setQuota(quota.second);
        }

        a.setHumanQuota(Utils::humanBinarySize(c, a.getQuota() * 1024));
        a.setHumanUsage(Utils::humanBinarySize(c, a.getUsage() * 1024));

        lst.push_back(a);
    }

    imap.logout();

    pag.insert(QStringLiteral("accounts"), QVariant::fromValue<std::vector<Account>>(lst));

    return pag;
}


Account Account::get(Cutelyst::Context *c, SkaffariError *e, const Domain &d, dbid_t id)
{
    Account a;

    Q_ASSERT_X(c, "get account", "invalid context object");
    Q_ASSERT_X(e, "get account", "invalid error object");

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT au.id, au.username, au.imap, au.pop, au.sieve, au.smtpauth, au.quota, au.created_at, au.updated_at, au.valid_until, au.pwd_expire, au.status FROM accountuser au WHERE id = :id"));
    q.bindValue(QStringLiteral(":id"), id);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to query account from database."));
        qCCritical(SK_ACCOUNT, "Failed to query data for account ID %u from the database: %s", id, qUtf8Printable(q.lastError().text()));
        return a;
    }

    if (!q.next()) {
        qCWarning(SK_ACCOUNT, "Account with ID %u not found in database.", id);
        return a;
    }


    a.setId(q.value(0).value<dbid_t>());
    a.setUsername(q.value(1).toString());
    a.setImapEnabled(q.value(2).toBool());
    a.setPopEnabled(q.value(3).toBool());
    a.setSieveEnabled(q.value(4).toBool());
    a.setSmtpauthEnabled(q.value(5).toBool());
    a.setQuota(q.value(6).value<quota_size_t>());
    a.setCreated(Utils::toUserTZ(c, q.value(7).toDateTime()));
    a.setUpdated(Utils::toUserTZ(c, q.value(8).toDateTime()));
    a.setValidUntil(Utils::toUserTZ(c, q.value(9).toDateTime()));
    a.setPasswordExpires(Utils::toUserTZ(c, q.value(10).toDateTime()));
    a.setDomainId(d.id());
    a.setPrefix(d.getPrefix());
    a.setDomainName(d.getName());
    a.setStatus(q.value(11).value<quint8>());

    QStringList emailAddresses;
    q = CPreparedSqlQueryThread(QStringLiteral("SELECT alias FROM virtual WHERE dest = :username AND username = :username ORDER BY alias ASC"));
    q.bindValue(QStringLiteral(":username"), a.getUsername());
    if (Q_LIKELY(q.exec())) {
        const QString catchAllAlias = QLatin1Char('@') + QString::fromLatin1(QUrl::toAce(d.getName()));
        while (q.next()) {
            const QString address = q.value(0).toString();
            if (!address.startsWith(QLatin1Char('@'))) {
                emailAddresses << addressFromACE(address);
            } else {
                if (address == catchAllAlias) {
                    a.setCatchAll(true);
                }
            }
        }
    } else {
        qCWarning(SK_ACCOUNT, "Failed to query email addresses for account ID %u (%s) from the database: %s", id, qUtf8Printable(a.getUsername()), qUtf8Printable(q.lastError().text()));
    }

    QStringList aliases;
    q = CPreparedSqlQueryThread(QStringLiteral("SELECT dest FROM virtual WHERE alias = :username AND username = ''"));
    q.bindValue(QStringLiteral(":username"), a.getUsername());
    if (Q_LIKELY(q.exec())) {
        while (q.next()) {
            const QStringList destinations = q.value(0).toString().split(QLatin1Char(','));
            if (!destinations.empty()) {
                for (const QString &dest : destinations) {
                    if (dest != a.getUsername()) {
                        aliases << dest;
                    } else {
                        a.setKeepLocal(true);
                    }
                }
            }
        }
    } else {
        qCWarning(SK_ACCOUNT, "Failed to query email forwards for account ID %u (%s) from the database: %s", id, qUtf8Printable(a.getUsername()), qUtf8Printable(q.lastError().text()));
    }

    if ((emailAddresses.size() > 1) || (aliases.size() > 1)) {
        QCollator col(c->locale());

        if (emailAddresses.size() > 1) {
            std::sort(emailAddresses.begin(), emailAddresses.end(), col);
        }

        if (aliases.size() > 1) {
            std::sort(aliases.begin(), aliases.end(), col);
        }
    }

    a.setAddresses(emailAddresses);
    a.setForwards(aliases);

    SkaffariIMAP imap(c);
    if (imap.login()) {
        quota_pair quota = imap.getQuota(a.getUsername());
        a.setUsage(quota.first);
        a.setQuota(quota.second);
        imap.logout();
    }

    a.setHumanQuota(Utils::humanBinarySize(c, a.getQuota() * 1024));
    a.setHumanUsage(Utils::humanBinarySize(c, a.getUsage() * 1024));

    return a;
}


void Account::toStash(Cutelyst::Context *c, const Domain &d, dbid_t accountId)
{
    Q_ASSERT_X(c, "account to stash", "invalid context object");

    SkaffariError e(c);
    Account a = Account::get(c, &e, d, accountId);
    if (Q_LIKELY(a.isValid())) {
        c->stash({
                     {QStringLiteral(ACCOUNT_STASH_KEY), QVariant::fromValue<Account>(a)},
                     {QStringLiteral("site_subtitle"), a.getUsername()}
                 });
    } else {
        c->stash({
                     {QStringLiteral("template"), QStringLiteral("404.html")},
                     {QStringLiteral("site_title"), c->translate("Account", "Not found")},
                     {QStringLiteral("not_found_text"), c->translate("Account", "There is no account with database ID %1.").arg(accountId)}
                 });
        c->res()->setStatus(404);
    }
}


void Account::toStash(Cutelyst::Context *c, const Account &a)
{
    Q_ASSERT_X(c, "account to stash", "invalid context object");
    Q_ASSERT_X(a.isValid(), "account to stash", "invalid account object");
    c->stash({
                 {QStringLiteral(ACCOUNT_STASH_KEY), QVariant::fromValue<Account>(a)},
                 {QStringLiteral("site_subtitle"), a.getUsername()}
             });
}


Account Account::fromStash(Cutelyst::Context *c)
{
    Account a;
    a = c->stash(QStringLiteral(ACCOUNT_STASH_KEY)).value<Account>();
    return a;
}



bool Account::update(Cutelyst::Context *c, SkaffariError *e, Account *a, Domain *d, const Cutelyst::ParamsMultiMap &p)
{
    bool ret = false;

    Q_ASSERT_X(c, "update account", "invalid context object");
    Q_ASSERT_X(e, "update account", "invalid error object");
    Q_ASSERT_X(a, "update account", "invalid account object");
    Q_ASSERT_X(d, "update account", "invalid domain object");

    const QString password = p.value(QStringLiteral("password"));
    QByteArray encPw;
    if (!password.isEmpty()) {
        Password pw(password);
        encPw = pw.encrypt(SkaffariConfig::accPwMethod(), SkaffariConfig::accPwAlgorithm(), SkaffariConfig::accPwRounds());
        if (Q_UNLIKELY(encPw.isEmpty())) {
            e->setErrorType(SkaffariError::ApplicationError);
            e->setErrorText(c->translate("Account", "Failed to encrypt password."));
            qCCritical(SK_ACCOUNT) << "Failed to encrypt user password with method" << SkaffariConfig::accPwMethod() << ", algorithm" << SkaffariConfig::accPwAlgorithm() << "and" << SkaffariConfig::accPwRounds() << "rounds";
            return ret;
        }
    }

    quota_size_t quota = 0;
    if (p.contains(QStringLiteral("humanQuota"))) {
        bool quotaOk = true;
        quota = Utils::humanToIntSize(c, p.value(QStringLiteral("humanQuota")), &quotaOk);
        if (!quotaOk) {
            e->setErrorType(SkaffariError::InputError);
            e->setErrorText(c->translate("Account", "Failed to convert human readable quota size string into valid integer value."));
            return ret;
        }
    } else {
        bool quotaOk = true;
        quota = p.value(QStringLiteral("quota"), QStringLiteral("0")).toULongLong(&quotaOk);
        if (!quotaOk) {
            e->setErrorType(SkaffariError::InputError);
            e->setErrorText(c->translate("Account", "Failed to parse quota string into integer value."));
        }
    }

    if (quota != a->getQuota()) {
        SkaffariIMAP imap(c);
        if (Q_LIKELY(imap.login())) {
            if (Q_UNLIKELY(!imap.setQuota(a->getUsername(), quota))) {
                e->setImapError(imap.lastError(), c->translate("Account", "Failed to change user account quota."));
                return ret;
            }
        } else {
            e->setImapError(imap.lastError(), c->translate("Account", "Failed to change user account quota."));
            return ret;
        }
    }

    QTimeZone userTz(Cutelyst::Session::value(c, QStringLiteral("tz"), QStringLiteral("UTC")).toByteArray());
    QDateTime validUntil = QDateTime::fromString(p.value(QStringLiteral("validUntil"), QStringLiteral("2998-12-31 23:59:59")), QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    QDateTime pwExpires = QDateTime::fromString(p.value(QStringLiteral("passwordExpires"), QStringLiteral("2998-12-31 23:59:59")), QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    if (userTz == QTimeZone::utc()) {
        validUntil.setTimeSpec(Qt::UTC);
        pwExpires.setTimeSpec(Qt::UTC);
    } else {
        validUntil.setTimeZone(userTz);
        validUntil = validUntil.toUTC();
        pwExpires.setTimeZone(userTz);
        pwExpires = pwExpires.toUTC();
    }
    const QDateTime currentTimeUtc = QDateTime::currentDateTimeUtc();

    const bool imap = (p.value(QStringLiteral("imap"), QStringLiteral("0")) == QLatin1String("1"));
    const bool pop = (p.value(QStringLiteral("pop"), QStringLiteral("0")) == QLatin1String("1"));
    const bool sieve = (p.value(QStringLiteral("sieve"), QStringLiteral("0")) == QLatin1String("1"));
    const bool smtpauth = (p.value(QStringLiteral("smtpauth"), QStringLiteral("0")) == QLatin1String("1"));
    const bool _catchAll = (p.value(QStringLiteral("catchall"), QStringLiteral("0")) == QLatin1String("1"));

    QSqlQuery q;
    if (!password.isEmpty()) {
        q = CPreparedSqlQueryThread(QStringLiteral("UPDATE accountuser SET password = :password, quota = :quota, valid_until = :validUntil, updated_at = :updated_at, imap = :imap, pop = :pop, sieve = :sieve, smtpauth =:smtpauth, pwd_expire = :pwd_expire WHERE id = :id"));
        q.bindValue(QStringLiteral(":password"), encPw);
    } else {
        q = CPreparedSqlQueryThread(QStringLiteral("UPDATE accountuser SET quota = :quota, valid_until = :validUntil, updated_at = :updated_at, imap = :imap, pop = :pop, sieve = :sieve, smtpauth =:smtpauth, pwd_expire = :pwd_expire WHERE id = :id"));
    }
    q.bindValue(QStringLiteral(":quota"), quota);
    q.bindValue(QStringLiteral(":validUntil"), validUntil);
    q.bindValue(QStringLiteral(":id"), a->getId());
    q.bindValue(QStringLiteral(":updated_at"), currentTimeUtc);
    q.bindValue(QStringLiteral(":imap"), imap);
    q.bindValue(QStringLiteral(":pop"), pop);
    q.bindValue(QStringLiteral(":sieve"), sieve);
    q.bindValue(QStringLiteral(":smtpauth"), smtpauth);
    q.bindValue(QStringLiteral(":pwd_expire"), pwExpires);


    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to update user account in database."));
        qCCritical(SK_ACCOUNT, "Failed to update user account ID %u (%s) in database: %s", a->getId(), qUtf8Printable(a->getUsername()), qUtf8Printable(q.lastError().text()));
        return ret;
    }

    if (_catchAll != a->cathAll()) {
        const QString catchAllAlias = QLatin1Char('@') + QString::fromLatin1(QUrl::toAce(d->getName()));
        if (_catchAll && !a->cathAll()) {
            q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM virtual WHERE alias = :alias"));
            q.bindValue(QStringLiteral(":alias"), catchAllAlias);

            if (Q_UNLIKELY(!q.exec())) {
                e->setSqlError(q.lastError(), c->translate("Account", "Failed to remove old catch all address from database."));
                qCWarning(SK_ACCOUNT, "Failed to remove old catch all address for domain %s from database: %s", qUtf8Printable(d->getName()), qUtf8Printable(q.lastError().text()));
            }

            q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO virtual (alias, dest, username, status) VALUES (:alias, :dest, :username, :status)"));
            q.bindValue(QStringLiteral(":alias"), catchAllAlias);
            q.bindValue(QStringLiteral(":dest"), a->getUsername());
            q.bindValue(QStringLiteral(":username"), a->getUsername());
            q.bindValue(QStringLiteral(":status"), 1);

            if (Q_UNLIKELY(!q.exec())) {
                e->setSqlError(q.lastError(), c->translate("Account", "Failed to set this account as catch all account."));
                qCWarning(SK_ACCOUNT, "Failed to set account %s as catch all account for domain %s: %s", qUtf8Printable(a->getUsername()), qUtf8Printable(d->getName()), qUtf8Printable(q.lastError().text()));
            } else {
                a->setCatchAll(true);
            }

        } else if (!_catchAll && a->cathAll()) {
            q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM virtual WHERE alias = :alias AND username = :username"));
            q.bindValue(QStringLiteral(":alias"), catchAllAlias);
            q.bindValue(QStringLiteral(":username"), a->getUsername());

            if (Q_UNLIKELY(!q.exec())) {
                e->setSqlError(q.lastError(), c->translate("Account", "Failed to deselect this account as catch all account for this domain."));
                qCWarning(SK_ACCOUNT, "Failed to deselect account %s as catch all account for domain %s: %s", qUtf8Printable(a->getUsername()), qUtf8Printable(a->getDomainName()), qUtf8Printable(q.lastError().text()));
            } else {
                a->setCatchAll(false);
            }
        }
    }

    q = CPreparedSqlQueryThread(QStringLiteral("UPDATE domain SET domainquotaused = (SELECT SUM(quota) FROM accountuser WHERE domain_id = :domain_id) WHERE id = :domain_id"));
    q.bindValue(QStringLiteral(":domain_id"), a->getDomainId());
    if (Q_UNLIKELY(!q.exec())) {
        qCWarning(SK_ACCOUNT, "Failed to update used domain quota for domain ID %u: %s", a->getDomainId(), qUtf8Printable(q.lastError().text()));
    }

    a->setValidUntil(Utils::toUserTZ(c, validUntil));
    a->setPasswordExpires(Utils::toUserTZ(c, pwExpires));
    a->setQuota(quota);
    a->setHumanQuota(Utils::humanBinarySize(c, quota * Q_UINT64_C(1024)));
    a->setUpdated(Utils::toUserTZ(c, currentTimeUtc));
    a->setImapEnabled(imap);
    a->setPopEnabled(pop);
    a->setSieveEnabled(sieve);
    a->setSmtpauthEnabled(smtpauth);

    q = CPreparedSqlQueryThread(QStringLiteral("SELECT domainquotaused FROM domain WHERE id = :domain_id"));
    q.bindValue(QStringLiteral(":domain_id"), d->id());
    if (Q_UNLIKELY(!q.exec())) {
        qCWarning(SK_ACCOUNT) << "Failed to query used domain quota after updating account ID" << a->getId() << "in domain ID" << d->id();
        qCDebug(SK_ACCOUNT) << q.lastError().text();
    } else {
        if (Q_LIKELY(q.next())) {
            d->setDomainQuotaUsed(q.value(0).value<quota_size_t>());
            d->setHumanDomainQuotaUsed(Utils::humanBinarySize(c, d->getDomainQuotaUsed() * Q_UINT64_C(1024)));
        }
    }

    qCInfo(SK_ACCOUNT) << c->stash(QStringLiteral("userName")).toString() << "updated account" << a->getUsername() << "for domain" << a->getDomainName();

    ret = true;

    return ret;
}

#define PAM_ACCT_EXPIRED 1
#define PAM_NEW_AUTHTOK_REQD 2

QStringList Account::check(Cutelyst::Context *c, SkaffariError *e, const Domain &domain)
{
    QStringList actions;

    Q_ASSERT_X(c, "check account", "invalid context");
    Q_ASSERT_X(e, "check account", "invalid error object");

    qCInfo(SK_ACCOUNT, "%s started checking user account ID %u.", qUtf8Printable(c->stash(QStringLiteral("userName")).toString()), d->id);

    Domain dom = domain;

    if (!dom.isValid()) {
        dom = Domain::fromStash(c);
        if (!dom.isValid()) {
            dom = Domain::get(c, d->domainId, e);
            if (!dom.isValid()) {
                return actions;
            }
        }
    }

    SkaffariIMAP imap(c);
    if (Q_UNLIKELY(!imap.login())) {
        e->setImapError(imap.lastError());
        return actions;
    }

    const QStringList mboxes = imap.getMailboxes();

    if (mboxes.empty() && (imap.lastError().type() != SkaffariIMAPError::NoError)) {
        e->setImapError(imap.lastError(), c->translate("Account", "Failed to get a list of all IMAP maiboxes from the IMAP server."));
        imap.logout();
        return actions;
    }

    if ((SkaffariConfig::imapCreatemailbox() != DoNotCreate) && !mboxes.contains(d->username)) {
        if (Q_UNLIKELY(!imap.createMailbox(d->username))) {
            e->setImapError(imap.lastError());
            imap.logout();
            return actions;
        } else {
            qCInfo(SK_ACCOUNT, "%s created missing mailbox on IMAP server for user account ID %u.", qUtf8Printable(c->stash(QStringLiteral("userName")).toString()), d->id);
            actions.append(c->translate("Account", "Created missing mailbox on IMAP server."));
        }
    }

    quota_pair quota = imap.getQuota(d->username);

    if ((dom.getDomainQuota() > 0) && ((d->quota == 0) || (quota.second == 0))) {
        const quota_size_t newQuota = (dom.getQuota() > 0) ? dom.getQuota() : (SkaffariConfig::defQuota() > 0) ? SkaffariConfig::defQuota() : 10240;
        if (quota.second == 0) {
            if (Q_UNLIKELY(!imap.setQuota(d->username, newQuota))) {
                e->setImapError(imap.lastError());
                imap.logout();
                return actions;
            } else {
                qCInfo(SK_ACCOUNT, "%s set correct mailbox storage quota of %u on IMAP server for user account ID %u.", qUtf8Printable(c->stash(QStringLiteral("userName")).toString()), newQuota, d->id);
                actions.append(c->translate("Account", "Set correct mailbox storage quota on IMAP server."));
                quota.second = newQuota;
            }
        }

        if (d->quota == 0) {
            QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("UPDATE accountuser SET quota = :quota WHERE id = :id"));
            q.bindValue(QStringLiteral(":quota"), newQuota);
            q.bindValue(QStringLiteral(":id"), d->id);
            if (Q_UNLIKELY(!q.exec())) {
                e->setSqlError(q.lastError());
                return actions;
            } else {
                qCInfo(SK_ACCOUNT, "%s set correct mailbox storage quota of %u in database for user account ID %u.", qUtf8Printable(c->stash(QStringLiteral("userName")).toString()), newQuota, d->id);
                actions.append(c->translate("Account", "Set correct mailbox storage quota in database."));
                d->quota = newQuota;

                q = CPreparedSqlQueryThread(QStringLiteral("UPDATE domain SET domainquotaused = (SELECT SUM(quota) FROM accountuser WHERE domain_id = :domain_id) WHERE id = :domain_id"));
                q.bindValue(QStringLiteral(":domain_id"), d->domainId);
                if (Q_UNLIKELY(!q.exec())) {
                    qCWarning(SK_ACCOUNT, "Failed to update used domain quota in database for domain ID %u: %s", d->domainId, qUtf8Printable(q.lastError().text()));
                }
            }
        }
    }

    if (quota.second != d->quota) {
        if (Q_UNLIKELY(!imap.setQuota(d->username, d->quota))) {
            e->setImapError(imap.lastError());
            imap.logout();
            return actions;
        } else {
            qCInfo(SK_ACCOUNT, "%s set correct mailbox storage quota of %u on IMAP server for user account ID %u.", qUtf8Printable(c->stash(QStringLiteral("userName")).toString()), d->quota, d->id);
            actions.append(c->translate("Account", "Set correct mailbox storage quota on IMAP server."));
            quota.second = d->quota;
        }
    }

    imap.logout();

    const QDateTime now = QDateTime::currentDateTimeUtc();
    const QDateTime validUntil = d->validUntil.toUTC();
    const QDateTime pwdExpires = d->passwordExpires.toUTC();

    quint8 newStatus = 0;
    bool newAccExpired = false;
    if (validUntil < now) {
        newStatus |= PAM_ACCT_EXPIRED;
        newAccExpired = true;
    }

    bool newPwExpired = false;
    if (pwdExpires < now) {
        newStatus |= PAM_NEW_AUTHTOK_REQD;
        newPwExpired = true;
    }

    if (d->status != newStatus) {
        bool oldAccExpired = ((d->status & PAM_ACCT_EXPIRED) == PAM_ACCT_EXPIRED);
        bool oldPwExpired = ((d->status & PAM_NEW_AUTHTOK_REQD) == PAM_NEW_AUTHTOK_REQD);
        if (oldAccExpired != newAccExpired) {
            if (!oldAccExpired && newAccExpired) {
                actions.append(c->translate("Account", "The account was only valid until %1 and has been expired.").arg(c->locale().toString(d->validUntil, QLocale::ShortFormat)));
            } else {
                actions.append(c->translate("Account", "The account was marked as expired but is now valid again until %1.").arg(c->locale().toString(d->validUntil, QLocale::ShortFormat)));
            }
        }

        if (oldPwExpired != newPwExpired) {
            if (!oldPwExpired && newPwExpired) {
                actions.append(c->translate("Accont", "The password of the account was only valid until %1 and has been expired.").arg(c->locale().toString(d->passwordExpires, QLocale::ShortFormat)));
            } else {
                actions.append(c->translate("Account", "The password of the account was marked as expired but is now valid again until %1.").arg(c->locale().toString(d->passwordExpires, QLocale::ShortFormat)));
            }
        }

        QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("UPDATE accountuser SET status = :status WHERE id = :id"));
        q.bindValue(QStringLiteral(":status"), newStatus);
        q.bindValue(QStringLiteral(":id"), d->id);

        if (Q_UNLIKELY(!q.exec())) {
            qCWarning(SK_ACCOUNT, "Failed to update status for account ID %u in the database: %s", d->id, qUtf8Printable(q.lastError().text()));
        } else {
            qCInfo(SK_ACCOUNT, "%s seth correct status value of %i for user account ID %u.", qUtf8Printable(Utils::getUserName(c)), newStatus, d->id);
        }
    }

    if (actions.empty()) {
        qCInfo(SK_ACCOUNT, "Nothing to do for user account ID %u.", d->id);
    } else {
        d->humanQuota = Utils::humanBinarySize(c, d->quota * Q_UINT64_C(1024));
        d->usage = quota.first;
        d->humanUsage = Utils::humanBinarySize(c, d->usage * Q_UINT64_C(1024));
        d->status = newStatus;
    }

    qCInfo(SK_ACCOUNT, "%s finished checking user account ID %u.", qUtf8Printable(c->stash(QStringLiteral("userName")).toString()), d->id);

    return actions;
}



bool Account::updateEmail(Cutelyst::Context *c, SkaffariError *e, Account *a, const Domain &d, const Cutelyst::ParamsMultiMap &p, const QString &oldAddress)
{
    bool ret = false;

    Q_ASSERT_X(c, "update email", "invalid context object");
    Q_ASSERT_X(e, "update email", "invalid error object");
    Q_ASSERT_X(a, "update email", "invalid account object");

    QString address;
    if (d.isFreeNamesEnabled()) {
        address = p.value(QStringLiteral("newlocalpart")) + QLatin1Char('@') + p.value(QStringLiteral("newmaildomain"));
    } else {
        address = p.value(QStringLiteral("newlocalpart")) + QLatin1Char('@') + a->getDomainName();
    }

    if (a->getAddresses().contains(address)) {
        e->setErrorType(SkaffariError::InputError);
        e->setErrorText(c->translate("Account", "The email address %1 is already part of account %2.").arg(address, a->getUsername()));
        qCWarning(SK_ACCOUNT, "Updating email address failed: address %s is already part of account %s.", qUtf8Printable(address), qUtf8Printable(a->getUsername()));
        return ret;
    }

    if (address == oldAddress) {
        e->setErrorType(SkaffariError::InputError);
        e->setErrorText(c->translate("Account", "The email address has not been changed."));
        qCWarning(SK_ACCOUNT, "Updating email address failed: address %s has not been changed.", qUtf8Printable(address));
        return ret;
    }

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT alias, dest, username FROM virtual WHERE alias = :address"));
    q.bindValue(QStringLiteral(":address"), addressToACE(address));

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to check if the new email address %1 is already in use by another account.").arg(address));
        qCCritical(SK_ACCOUNT, "Failed to check if the new email address %s is already in use by another account: %s", qUtf8Printable(address), qUtf8Printable(q.lastError().text()));
        return ret;
    }

    if (Q_UNLIKELY(q.next())) {
        const QString user = q.value(2).toString();
        e->setErrorType(SkaffariError::InputError);
        if (!user.isEmpty()) {
            e->setErrorText(c->translate("Account", "The email address %1 is already in use by user %2.").arg(address, user));
        } else {
            e->setErrorText(c->translate("Account", "The email address %1 is already in use for destination %2.").arg(address, q.value(1).toString()));
        }
        qCWarning(SK_ACCOUNT) << c->stash(QStringLiteral("userName")).toString() << "tried to change the email address" << oldAddress << "to already existing address" << address;
        return ret;
    }

    q = CPreparedSqlQueryThread(QStringLiteral("UPDATE virtual SET alias = :aliasnew WHERE alias = :aliasold AND username = :username"));
    q.bindValue(QStringLiteral(":aliasnew"), addressToACE(address));
    q.bindValue(QStringLiteral(":aliasold"), addressToACE(oldAddress));
    q.bindValue(QStringLiteral(":username"), a->getUsername());

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to update email address %1.").arg(oldAddress));
        qCCritical(SK_ACCOUNT, "Failed to change email address %s of account ID %u (%s) to %s: %s", qUtf8Printable(oldAddress), a->getId(), qUtf8Printable(a->getUsername()), qUtf8Printable(address), qUtf8Printable(q.lastError().text()));
        return ret;
    }

    QStringList addresses = a->getAddresses();
    addresses.removeOne(oldAddress);
    addresses.append(address);
    if (addresses.size() > 1) {
        QCollator col(c->locale());
        std::sort(addresses.begin(), addresses.end(), col);
    }
    a->setAddresses(addresses);

    qCInfo(SK_ACCOUNT) << c->stash(QStringLiteral("userName")).toString() << "updated email address" << oldAddress << "of account" << a->getUsername();

    ret = true;

    return ret;
}


bool Account::addEmail(Cutelyst::Context *c, SkaffariError *e, Account *a, const Domain &d, const Cutelyst::ParamsMultiMap &p)
{
    bool ret = false;

    Q_ASSERT_X(c, "update email", "invalid context object");
    Q_ASSERT_X(e, "update email", "invalid error object");
    Q_ASSERT_X(a, "update email", "invalid account object");

    QString address;
    if (d.isFreeNamesEnabled()) {
        address = p.value(QStringLiteral("newlocalpart")) + QLatin1Char('@') + p.value(QStringLiteral("newmaildomain"));
    } else {
        address = p.value(QStringLiteral("newlocalpart")) + QLatin1Char('@') + a->getDomainName();
    }

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT alias, dest, username FROM virtual WHERE alias = :address"));
    q.bindValue(QStringLiteral(":address"), addressToACE(address));

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to check if the new email address %1 is already in use by another account.").arg(address));
        qCCritical(SK_ACCOUNT, "Failed to check if new email address %s for account ID %u (%s) is already in use by another account: %s", qUtf8Printable(address), a->getId(), qUtf8Printable(a->getUsername()), qUtf8Printable(q.lastError().text()));
        return ret;
    }

    if (Q_UNLIKELY(q.next())) {
        const QString user = q.value(2).toString();
        e->setErrorType(SkaffariError::InputError);
        if (!user.isEmpty()) {
            e->setErrorText(c->translate("Account", "The email address %1 is already in use by user %2.").arg(address, user));
        } else {
            e->setErrorText(c->translate("Account", "The email address %1 is already in use for destination %2.").arg(address, q.value(1).toString()));
        }
        qCWarning(SK_ACCOUNT) << c->stash(QStringLiteral("userName")).toString() << "tried to add already in use email address" << address << "to account" << a->getUsername();
        return ret;
    }

    q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO virtual (alias, dest, username, status) VALUES (:alias, :dest, :username, :status)"));
    q.bindValue(QStringLiteral(":alias"), addressToACE(address));
    q.bindValue(QStringLiteral(":dest"), a->getUsername());
    q.bindValue(QStringLiteral(":username"), a->getUsername());
    q.bindValue(QStringLiteral(":status"), 1);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to insert new email address into database."));
        qCCritical(SK_ACCOUNT, "Failed to insert new email address %s for account ID %u (%s) into database: %s", qUtf8Printable(address), a->getId(), qUtf8Printable(a->getUsername()), qUtf8Printable(q.lastError().text()));
        return ret;
    }

    QStringList addresses = a->getAddresses();
    addresses.append(address);
    if (address.size() > 1) {
        addresses.sort();
    }
    a->setAddresses(addresses);

    qCInfo(SK_ACCOUNT) << c->stash(QStringLiteral("userName")).toString() << "added email address" << address << "to account" << a->getUsername();

    ret = true;

    return ret;
}


bool Account::removeEmail(Cutelyst::Context *c, SkaffariError *e, Account *a, const QString &address)
{
    bool ret = false;

    Q_ASSERT_X(c, "update email", "invalid context object");
    Q_ASSERT_X(e, "update email", "invalid error object");
    Q_ASSERT_X(a, "update email", "invalid account object");

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM virtual WHERE alias = :address"));
    q.bindValue(QStringLiteral(":address"), addressToACE(address));

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to remove email address %1 from account %2.").arg(address, a->getUsername()));
        qCCritical(SK_ACCOUNT, "Failed to remove email address %s from account %s: %s", qUtf8Printable(address), qUtf8Printable(a->getUsername()), qUtf8Printable(q.lastError().text()));
        return ret;
    }

    QStringList addresses = a->getAddresses();
    addresses.removeOne(address);
    a->setAddresses(addresses);

    qCInfo(SK_ACCOUNT) << c->stash(QStringLiteral("userName")).toString() << "removed email address" << address << "from account" << a->getUsername();

    ret = true;

    return ret;
}



bool Account::updateForwards(Cutelyst::Context *c, SkaffariError *e, Account *a, const Cutelyst::ParamsMultiMap &p)
{
    bool ret = false;

    Q_ASSERT_X(c, "update forwards", "invalid context object");
    Q_ASSERT_X(e, "update forwards", "invalid error object");
    Q_ASSERT_X(a, "update forwards", "invalid account object");

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM virtual WHERE alias = :username AND username = ''"));
    q.bindValue(QStringLiteral(":username"), a->getUsername());

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to update forwards for account %1 in database.").arg(a->getUsername()));
        qCCritical(SK_ACCOUNT, "Failed to update forwards for account ID %u (%s) in database: %s", a->getId(), qUtf8Printable(a->getUsername()), qUtf8Printable(q.lastError().text()));
        return ret;
    }

    const QStringList forwards = p.values(QStringLiteral("forward"));

    if (!forwards.empty()) {

        QStringList checkedForwards;
        QStringList invalidForwards;
        for (const QString &forward : forwards) {
            if (forward.contains(QRegularExpression(QStringLiteral("(?(DEFINE)"
                                                                   "(?<addr_spec> (?&local_part) @ (?&domain) )"
                                                                   "(?<local_part> (?&dot_atom) | (?&quoted_string) | (?&obs_local_part) )"
                                                                   "(?<domain> (?&dot_atom) | (?&domain_literal) | (?&obs_domain) )"
                                                                   "(?<domain_literal> (?&CFWS)? \\[ (?: (?&FWS)? (?&dtext) )* (?&FWS)? \\] (?&CFWS)? )"
                                                                   "(?<dtext> [\\x21-\\x5a] | [\\x5e-\\x7e] | (?&obs_dtext) )"
                                                                   "(?<quoted_pair> \\\\ (?: (?&VCHAR) | (?&WSP) ) | (?&obs_qp) )"
                                                                   "(?<dot_atom> (?&CFWS)? (?&dot_atom_text) (?&CFWS)? )"
                                                                   "(?<dot_atom_text> (?&atext) (?: \\. (?&atext) )* )"
                                                                   "(?<atext> [a-zA-Z0-9!#$%&'*+\\/=?^_`{|}~-]+ )"
                                                                   "(?<atom> (?&CFWS)? (?&atext) (?&CFWS)? )"
                                                                   "(?<word> (?&atom) | (?&quoted_string) )"
                                                                   "(?<quoted_string> (?&CFWS)? \" (?: (?&FWS)? (?&qcontent) )* (?&FWS)? \" (?&CFWS)? )"
                                                                   "(?<qcontent> (?&qtext) | (?&quoted_pair) )"
                                                                   "(?<qtext> \\x21 | [\\x23-\\x5b] | [\\x5d-\\x7e] | (?&obs_qtext) )"
                                                                   "(?<FWS> (?: (?&WSP)* \\r\\n )? (?&WSP)+ | (?&obs_FWS) )"
                                                                   "(?<CFWS> (?: (?&FWS)? (?&comment) )+ (?&FWS)? | (?&FWS) )"
                                                                   "(?<comment> \\( (?: (?&FWS)? (?&ccontent) )* (?&FWS)? \\) )"
                                                                   "(?<ccontent> (?&ctext) | (?&quoted_pair) | (?&comment) )"
                                                                   "(?<ctext> [\\x21-\\x27] | [\\x2a-\\x5b] | [\\x5d-\\x7e] | (?&obs_ctext) )"
                                                                   "(?<obs_domain> (?&atom) (?: \\. (?&atom) )* )"
                                                                   "(?<obs_local_part> (?&word) (?: \\. (?&word) )* )"
                                                                   "(?<obs_dtext> (?&obs_NO_WS_CTL) | (?&quoted_pair) )"
                                                                   "(?<obs_qp> \\\\ (?: \\x00 | (?&obs_NO_WS_CTL) | \\n | \\r ) )"
                                                                   "(?<obs_FWS> (?&WSP)+ (?: \\r\\n (?&WSP)+ )* )"
                                                                   "(?<obs_ctext> (?&obs_NO_WS_CTL) )"
                                                                   "(?<obs_qtext> (?&obs_NO_WS_CTL) )"
                                                                   "(?<obs_NO_WS_CTL> [\\x01-\\x08] | \\x0b | \\x0c | [\\x0e-\\x1f] | \\x7f )"
                                                                   "(?<VCHAR> [\\x21-\\x7E] )"
                                                                   "(?<WSP> [ \\t] )"
                                                               ")"
                                                               "^(?&addr_spec)$"), QRegularExpression::ExtendedPatternSyntaxOption))) {
                checkedForwards << forward;

            } else {
                invalidForwards << forward;
            }
        }

        if (!invalidForwards.empty()) {
            e->setErrorType(SkaffariError::InputError);
            e->setErrorText(c->translate("Account", "The following addresses seem not to be valid: %1").arg(invalidForwards.join(QStringLiteral(", "))));
            return ret;
        }

        const bool _keepLocal = p.contains(QStringLiteral("keeplocal"));

        if (_keepLocal) {
            checkedForwards << a->getUsername();
        }

        q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO virtual (alias, dest) VALUES (:username, :forwards)"));
        q.bindValue(QStringLiteral(":username"), a->getUsername());
        q.bindValue(QStringLiteral(":forwards"), checkedForwards.join(QLatin1Char(',')));

        if (Q_UNLIKELY(!q.exec())) {
            e->setSqlError(q.lastError(), c->translate("Account", "Failed to update forwards for account %1 in database.").arg(a->getUsername()));
            qCCritical(SK_ACCOUNT, "Failed to update forwards for account ID %u (%s) in the database: %s", a->getId(), qUtf8Printable(a->getUsername()), qUtf8Printable(q.lastError().text()));
            return ret;
        }

        if (_keepLocal) {
            checkedForwards.removeLast();
        }

        if (checkedForwards.size() > 1) {
            checkedForwards.sort();
        }

        a->setForwards(checkedForwards);
        a->setKeepLocal(_keepLocal);

    } else {
        a->setForwards(QStringList());
        a->setKeepLocal(false);
    }

    qCInfo(SK_ACCOUNT) << c->stash(QStringLiteral("userName")).toString() << "updated the forwards for account" << a->getUsername();

    ret = true;

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
    const QStringRef addressLocalPart = address.leftRef(atIdx);
    addressACE = addressLocalPart + QLatin1Char('@') + QString::fromLatin1(QUrl::toAce(addressDomainPart.toString()));

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
