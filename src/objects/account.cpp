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
#include <Cutelyst/Plugins/Memcached/Memcached>
#include <Cutelyst/Plugins/Utils/validatoremail.h>
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

#define MEMC_QUOTA_EXP 900
#define MEMC_QUOTA_KEY QLatin1String("sk_quotausage_")

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

dbid_t Account::id() const
{
    return d->id;
}


void Account::setId(dbid_t nId)
{
    d->id = nId;
}


dbid_t Account::domainId() const
{
    return d->domainId;
}


void Account::setDomainId(dbid_t nDomainId)
{
    d->domainId = nDomainId;
}


QString Account::username() const
{
    return d->username;
}


void Account::setUsername(const QString& nUsername)
{
    d->username = nUsername;
}



QString Account::prefix() const
{
    return d->prefix;
}



void Account::setPrefix(const QString &nPrefix)
{
    d->prefix = nPrefix;
}



QString Account::domainName() const
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



QStringList Account::addresses() const
{
    return d->addresses;
}


void Account::setAddresses(const QStringList &nAddresses)
{
    d->addresses = nAddresses;
}



QStringList Account::forwards() const
{
    return d->forwards;
}



void Account::setForwards(const QStringList &nForwards)
{
    d->forwards = nForwards;
}




quota_size_t Account::quota() const
{
    return d->quota;
}


void Account::setQuota(quota_size_t nQuota)
{
    d->quota = nQuota;
}


quota_size_t Account::usage() const
{
    return d->usage;
}


void Account::setUsage(quota_size_t nUsage)
{
    d->usage = nUsage;
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

void Account::setCreated(const QDateTime &created)
{
    d->created = created;
}


QDateTime Account::updated() const
{
    return d->updated;
}

void Account::setUpdated(const QDateTime &updated)
{
    d->updated = updated;
}


QDateTime Account::validUntil() const
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


bool Account::catchAll() const
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


Account Account::create(Cutelyst::Context *c, SkaffariError *e, const QVariantHash &p, const Domain &d, const QStringList &selectedKids)
{
    Account a;

    Q_ASSERT_X(c, "create account", "invalid context object");
    Q_ASSERT_X(e, "create account", "invalid error object");
    Q_ASSERT_X(!p.empty(), "create account", "empty parameters");
    Q_ASSERT_X(d.isValid(), "create account", "invalid domain object");

    // if domain as prefix is enabled, the username will be the local part of the email address plus the domain separated by a dot
    // if additionally fqun is enabled, it will a fully qualified user name (email address like user@example.com
    // if both are disabled, the username will be the entered username
    const QString username = SkaffariConfig::imapDomainasprefix() ? p.value(QStringLiteral("localpart")).toString().trimmed() + (SkaffariConfig::imapFqun() ? QLatin1Char('@') : QLatin1Char('.')) + d.name() : p.value(QStringLiteral("username")).toString().trimmed();

    // construct the email address from local part and domain name
    const QString localPart = p.value(QStringLiteral("localpart")).toString().trimmed();
    const QString email = localPart + QLatin1Char('@') + QString::fromLatin1(QUrl::toAce(d.name()));

    QList<Cutelyst::ValidatorEmail::Diagnose> diags;
    if (!Cutelyst::ValidatorEmail::validate(email, Cutelyst::ValidatorEmail::Valid, false, &diags)) {
        e->setErrorType(SkaffariError::InputError);
        e->setErrorText(Cutelyst::ValidatorEmail::diagnoseString(c, diags.at(0)));
        return a;
    }

    // start checking if the email address is already in use
    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT username FROM virtual WHERE alias = :email"));
    q.bindValue(QStringLiteral(":email"), email);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "The database query to check if the email address %1 already exists failed.").arg(Account::addressFromACE(email)));
        qCCritical(SK_ACCOUNT, "The database query to check if the email address %s already exists failed: %s", qUtf8Printable(Account::addressFromACE(email)), qUtf8Printable(q.lastError().text()));
        return a;
    }

    if (Q_UNLIKELY(q.next())) {
        e->setErrorText(c->translate("Account", "The email address %1 is already in use by another user.").arg(Account::addressFromACE(email)));
        e->setErrorType(SkaffariError::InputError);
        return a;
    }
    // end checking if the email address is already in use

    // start encrypting the password
    const QString password = p.value(QStringLiteral("password")).toString();
    Password pw(password);
    const QByteArray encpw = pw.encrypt(SkaffariConfig::accPwMethod(), SkaffariConfig::accPwAlgorithm(), SkaffariConfig::accPwRounds());

    if (Q_UNLIKELY(encpw.isEmpty())) {
        e->setErrorText(c->translate("Account", "User password encryption failed. Please check your encryption settings."));
        e->setErrorType(SkaffariError::ConfigError);
        qCCritical(SK_ACCOUNT, "User password encryption failed. Please check your encryption settings.");
        return a;
    }
    // end encrypting the password

    const bool imap = p.value(QStringLiteral("imap")).toBool();
    const bool pop = p.value(QStringLiteral("pop")).toBool();
    const bool sieve = p.value(QStringLiteral("sieve")).toBool();
    const bool smtpauth = p.value(QStringLiteral("smtpauth")).toBool();
    const bool _catchAll = p.value(QStringLiteral("catchall")).toBool();

    const quota_size_t quota = (p.value(QStringLiteral("quota")).value<quota_size_t>() / Q_UINT64_C(1024));

    const QDateTime currentUtc = QDateTime::currentDateTimeUtc();
    const QDateTime defDateTime(QDate(2999, 12, 31), QTime(0, 0), QTimeZone::utc());
    const QDateTime validUntil = p.value(QStringLiteral("validUntil"), defDateTime).toDateTime().toUTC();
    const QDateTime pwExpires = p.value(QStringLiteral("passwordExpires"), defDateTime).toDateTime().toUTC();

    const quint8 accountStatus = Account::calcStatus(validUntil, pwExpires);

    q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO accountuser (domain_id, username, password, prefix, domain_name, imap, pop, sieve, smtpauth, quota, created_at, updated_at, valid_until, pwd_expire, status) "
                                         "VALUES (:domain_id, :username, :password, :prefix, :domain_name, :imap, :pop, :sieve, :smtpauth, :quota, :created_at, :updated_at, :valid_until, :pwd_expire, :status)"));

    q.bindValue(QStringLiteral(":domain_id"), d.id());
    q.bindValue(QStringLiteral(":username"), username);
    q.bindValue(QStringLiteral(":password"), encpw);
    q.bindValue(QStringLiteral(":prefix"), d.prefix());
    q.bindValue(QStringLiteral(":domain_name"), QUrl::toAce(d.name()));
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
        e->setSqlError(q.lastError(), c->translate("Account", "New user account could not be created in the database."));
        qCCritical(SK_ACCOUNT, "New user account could not be created in the database: %s", qUtf8Printable(q.lastError().text()));
        return a;
    }

    const dbid_t id = q.lastInsertId().value<dbid_t>();

    q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO virtual (alias, dest, username, status) VALUES (:alias, :dest, :username, :status)"));
    q.bindValue(QStringLiteral(":alias"), email);
    q.bindValue(QStringLiteral(":dest"), username);
    q.bindValue(QStringLiteral(":username"), username);
    q.bindValue(QStringLiteral(":status"), 1);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Email address for new user account could not be created in the database."));
        qCCritical(SK_ACCOUNT, "Email address for new user account could not be created in the database.: %s", qUtf8Printable(q.lastError().text()));
        q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM accountuser WHERE id = :id"));
        q.bindValue(QStringLiteral(":id"), id);
        q.exec();
        return a;
    }

    if (!d.children().empty()) {
//        const QStringList selectedKids = p.values(QStringLiteral("children"));
        if (!selectedKids.empty()) {
            const QVector<SimpleDomain> thekids = d.children();
            for (const SimpleDomain &kid : thekids) {
                if (selectedKids.contains(QString::number(kid.id()))) {
                    const QString kidEmail = localPart + QLatin1Char('@') + QString::fromLatin1(QUrl::toAce(kid.name()));
                    q = CPreparedSqlQueryThread(QStringLiteral("SELECT username FROM virtual WHERE alias = :email"));
                    q.bindValue(QStringLiteral(":email"), kidEmail);

                    if (Q_UNLIKELY(!q.exec())) {
                        qCCritical(SK_ACCOUNT, "The database query to check if the email address %s already exists failed: %s", qUtf8Printable(addressFromACE(kidEmail)), qUtf8Printable(q.lastError().text()));
                    } else {
                        if (Q_LIKELY(!q.next())) {
                            QSqlQuery qq = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO virtual (alias, dest, username, status) VALUES (:alias, :dest, :username, :status)"));
                            qq.bindValue(QStringLiteral(":alias"), kidEmail);
                            qq.bindValue(QStringLiteral(":dest"), username);
                            qq.bindValue(QStringLiteral(":username"), username);
                            qq.bindValue(QStringLiteral(":status"), 1);

                            if (Q_UNLIKELY(!qq.exec())) {
                                qCCritical(SK_ACCOUNT, "Failed to insert email address %s for new user account into database: %s", qUtf8Printable(addressFromACE(kidEmail)), qUtf8Printable(qq.lastError().text()));
                            }
                        } else {
                            qCWarning(SK_ACCOUNT, "Email address %s for child domain %s already exists when creating new account %s.", qUtf8Printable(addressFromACE(kidEmail)), qUtf8Printable(kid.name()), qUtf8Printable(username));
                        }
                    }
                }
            }
        }
    }

    // removing old catch-all alias and setting a new one
    if (_catchAll) {
        const QString catchAllAlias = QLatin1Char('@') + QString::fromLatin1(QUrl::toAce(d.name()));

        q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM virtual WHERE alias = :alias"));
        q.bindValue(QStringLiteral(":alias"), catchAllAlias);

        if (Q_UNLIKELY(!q.exec())) {
            e->setSqlError(q.lastError(), c->translate("Account", "Existing catch-all address could not be deleted from the database."));
            qCCritical(SK_ACCOUNT, "Existing catch-all address for domain %s could not be deleted from the database: %s", qUtf8Printable(d.name()), qUtf8Printable(q.lastError().text()));

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
            e->setSqlError(q.lastError(), c->translate("Account", "Account could not be set up as catch-all account."));
            qCCritical(SK_ACCOUNT, "Account %s could not be set up as catch-all account for domain %s: %s", qUtf8Printable(username), qUtf8Printable(d.name()), qUtf8Printable(q.lastError().text()));

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
                e->setImapError(imap.lastError(), c->translate("Account", "Logging in to IMAP server for automatic creation of mailbox and folders by the server failed."));
            }
            imap.logout();

        } else if (createMailbox == OnlySetQuota) {

            imap.setUser(username);
            imap.setPassword(password);

            mailboxCreated = imap.login();
            if (!mailboxCreated) {
                e->setImapError(imap.lastError(), c->translate("Account", "Logging in to IMAP server for automatic creation of mailbox and folders by the server failed."));
            }

            imap.logout();

            if (mailboxCreated) {
                imap.setUser(SkaffariConfig::imapUser());
                imap.setPassword(SkaffariConfig::imapPassword());

                if (Q_LIKELY(imap.login())) {

                    if (Q_UNLIKELY(!imap.setQuota(username, quota))) {
                        e->setImapError(imap.lastError(), c->translate("Account", "Storage quota for new user account could not be set."));
                        mailboxCreated = false;
                    }

                    imap.logout();

                } else {
                    e->setImapError(imap.lastError(), c->translate("Account", "Logging in to the IMAP server to set the storage quota for the new user account failed."));
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

                            const QVector<Folder> folders = d.folders();

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
                    e->setImapError(imap.lastError(), c->translate("Account", "Creating a new IMAP mailbox failed."));
                    imap.logout();
                    mailboxCreated = false;
                }

            } else {
                e->setImapError(imap.lastError(), c->translate("Account", "Logging in to IMAP server to create new user account failed."));
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
        qCWarning(SK_ACCOUNT, "Failed to update count of accounts and domain quota usage after adding new account to domain ID %u (%s): %s", d.id(), qUtf8Printable(d.name()), qUtf8Printable(q.lastError().text()));
    }

    a.setId(id);
    a.setDomainId(d.id());
    a.setUsername(username);
    a.setPrefix(d.prefix());
    a.setDomainName(d.name());
    a.setImapEnabled(imap);
    a.setPopEnabled(pop);
    a.setSieveEnabled(sieve);
    a.setSmtpauthEnabled(smtpauth);
    a.setQuota(quota);
    a.setAddresses(QStringList(email));
    a.setCreated(currentUtc);
    a.setUpdated(currentUtc);
    a.setValidUntil(validUntil);
    a.setPasswordExpires(pwExpires);
    a.setCatchAll(_catchAll);

    if (SkaffariConfig::useMemcached()) {
        Cutelyst::Memcached::set(MEMC_QUOTA_KEY + QString::number(id), QByteArray::number(0), MEMC_QUOTA_EXP);
    }

    qCInfo(SK_ACCOUNT, "%s created new account %s for domain %s", qUtf8Printable(Utils::getUserName(c)), qUtf8Printable(username), qUtf8Printable(d.name()));

    return a;
}



bool Account::remove(Cutelyst::Context *c, SkaffariError *e) const
{
    bool ret = false;

    Q_ASSERT_X(c, "remove account", "invalid context object");
    Q_ASSERT_X(e, "remove account", "invalid error object");

    SkaffariIMAP imap(c);
    if (Q_UNLIKELY(!imap.login())) {
        e->setImapError(imap.lastError(), c->translate("Account", "Logging in to IMAP server to delete the mailbox %1 failed.").arg(d->username));
        qCCritical(SK_ACCOUNT, "Logging in to IMAP server to delete the mailbox %s failed: %s", qUtf8Printable(d->username), qUtf8Printable(imap.lastError().errorText()));
        return ret;
    }

    if (Q_UNLIKELY(!imap.setAcl(d->username, SkaffariConfig::imapUser()))) {
        // if Skaffari is responsible for mailbox creation, direct or indirect,
        // remove will fail if we can not delete the mailbox on the IMAP server
        if (SkaffariConfig::imapCreatemailbox() > DoNotCreate) {
            e->setImapError(imap.lastError(), c->translate("Account", "Setting the access rights for the IMAP administrator to delete the mailbox %1 failed.").arg(d->username));
            qCCritical(SK_ACCOUNT, "Setting the access rights for the IMAP administrator to delete the mailbox %s failed: %s", qUtf8Printable(d->username), qUtf8Printable(imap.lastError().errorText()));
            imap.logout();
            return ret;
        }
    }

    if (!imap.deleteMailbox(d->username) && (SkaffariConfig::imapCreatemailbox() != DoNotCreate)) {
        // if Skaffari is responsible for mailbox creation, direct or indirect,
        // remove will fail if we can not delete the mailbox on the IMAP server
        if (SkaffariConfig::imapCreatemailbox() > DoNotCreate) {
            e->setImapError(imap.lastError(), c->translate("Account", "Mailbox %1 could not be deleted from the IMAP server.").arg(d->username));
            qCCritical(SK_ACCOUNT, "Mailbox %s could not be deleted from the IMAP server: %s", qUtf8Printable(d->username), qUtf8Printable(imap.lastError().errorText()));
            imap.logout();
            return ret;
        }
    }

    imap.logout();

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT quota FROM accountuser WHERE username = :username"));
    q.bindValue(QStringLiteral(":username"), d->username);

    const quota_size_t quota = (q.exec() && q.next()) ? q.value(0).value<quota_size_t>() : 0;

    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM alias WHERE username = :username"));
    q.bindValue(QStringLiteral(":username"), d->username);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Alias addresses for user account %1 could not be deleted from the database.").arg(d->username));
        qCCritical(SK_ACCOUNT, "Alias addresses for user account %s could not be deleted from the database: %s", qUtf8Printable(d->username), qUtf8Printable(q.lastError().text()));
        return ret;
    }

    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM virtual WHERE username = :username"));
    q.bindValue(QStringLiteral(":username"), d->username);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Email addresses for user account %1 could not be deleted from the database.").arg(d->username));
        qCCritical(SK_ACCOUNT, "Email addresses for user account %s could not be deleted from the database: %s", qUtf8Printable(d->username), qUtf8Printable(q.lastError().text()));
        return ret;
    }

    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM virtual WHERE alias = :username AND username = ''"));
    q.bindValue(QStringLiteral(":username"), d->username);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Forward addresses for user account %1 could not be deleted from the database.").arg(d->username));
        qCCritical(SK_ACCOUNT, "Forward addresses for user account %s could not be deleted from the database: %s", qUtf8Printable(d->username), qUtf8Printable(q.lastError().text()));
        return ret;
    }

    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM accountuser WHERE username = :username"));
    q.bindValue(QStringLiteral(":username"), d->username);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "User account %1 could not be deleted from the database.").arg(d->username));
        qCCritical(SK_ACCOUNT, "User account %s could not be deleted from the database.: %s", qUtf8Printable(d->username), qUtf8Printable(q.lastError().text()));
        return ret;
    }

    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM log WHERE user = :username"));
    q.bindValue(QStringLiteral(":username"), d->username);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Log entries for user account %1 could not be deleted from the database.").arg(d->username));
        qCWarning(SK_ACCOUNT, "Log entries for user account %s could not be deleted from the database.: %s", qUtf8Printable(d->username), qUtf8Printable(q.lastError().text()));
    }

    q = CPreparedSqlQueryThread(QStringLiteral("UPDATE domain SET accountcount = accountcount - 1, domainquotaused = domainquotaused - :quota WHERE id = :id"));
    q.bindValue(QStringLiteral(":quota"), quota);
    q.bindValue(QStringLiteral(":id"), d->domainId);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Number of user accounts in the domain and domain quota used could not be updated in the database."));
        qCWarning(SK_ACCOUNT, "Failed to update count of domain accounts and used quota for domain ID %lu in database: %s", d->domainId, qUtf8Printable(q.lastError().text()));
    }

    qCInfo(SK_ACCOUNT, "%s deleted account %s (ID: %lu, domain ID: %lu).", qUtf8Printable(Utils::getUserName(c)), qUtf8Printable(d->username), d->id, d->domainId);

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
        e->setSqlError(q.lastError(), c->translate("Account", "User accounts could not be queried from the database."));
        qCCritical(SK_ACCOUNT, "User accounts for domain %s (ID: %lu) could not be queried from the database: %s", qUtf8Printable(d.name()), d.id(), qUtf8Printable(q.lastError().text()));
        return pag;
    }

    QSqlQuery countQuery = CPreparedSqlQueryThread(QStringLiteral("SELECT FOUND_ROWS()"));
    if (Q_UNLIKELY(!countQuery.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Total result could not be retrieved from the database."));
        qCCritical(SK_ACCOUNT, "Total result for domain %s (ID: %lu) could not be retrieved from the database: %s", qUtf8Printable(d.name()), d.id(), qUtf8Printable(q.lastError().text()));
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
        qCWarning(SK_ACCOUNT, "Failed to login to IMAP server. Omitting quota query while listing accounts for domain ID %u (%s).", d.id(), qUtf8Printable(d.name()));
    }

    QCollator col(c->locale());
    const QString catchAllAlias = QLatin1Char('@') + QString::fromLatin1(QUrl::toAce(d.name()));

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

        QDateTime accountCreated = q.value(7).toDateTime();
        accountCreated.setTimeSpec(Qt::UTC);
        QDateTime accountUpdated = q.value(8).toDateTime();
        accountUpdated.setTimeSpec(Qt::UTC);
        QDateTime accountValidUntil = q.value(9).toDateTime();
        accountValidUntil.setTimeSpec(Qt::UTC);
        QDateTime accountPwExpires = q.value(10).toDateTime();
        accountPwExpires.setTimeSpec(Qt::UTC);
        Account a(q.value(0).value<dbid_t>(),
                  d.id(),
                  q.value(1).toString(),
                  d.prefix(),
                  d.name(),
                  q.value(2).toBool(),
                  q.value(3).toBool(),
                  q.value(4).toBool(),
                  q.value(5).toBool(),
                  emailAddresses,
                  aliases,
                  q.value(6).value<quota_size_t>(),
                  0,
                  accountCreated,
                  accountUpdated,
                  accountValidUntil,
                  accountPwExpires,
                  _keepLocal,
                  _catchAll,
                  q.value(11).value<quint8>());


        bool gotQuota = false;
        if (SkaffariConfig::useMemcached()) {
            const QByteArray usage = Cutelyst::Memcached::get(MEMC_QUOTA_KEY + QString::number(a.id()));
            if (!usage.isNull()) {
                bool ok = false;
                a.setUsage(usage.toULongLong(&ok));
                if (ok) {
                    gotQuota = true;
                }
            }
        }

        if (!gotQuota) {
            if (Q_LIKELY(imap.isLoggedIn())) {
                quota_pair quota = imap.getQuota(a.username());
                a.setUsage(quota.first);
                a.setQuota(quota.second);
                if (SkaffariConfig::useMemcached()) {
                    Cutelyst::Memcached::set(MEMC_QUOTA_KEY + QString::number(a.id()), QByteArray::number(quota.first), MEMC_QUOTA_EXP);
                }
            }
        }

        lst.push_back(a);
    }

    imap.logout();

    pag.insert(QStringLiteral("accounts"), QVariant::fromValue<std::vector<Account>>(lst));

    return pag;
}

Account Account::get(Cutelyst::Context *c, SkaffariError *e, dbid_t id)
{
    Account a;

    Q_ASSERT_X(c, "get account", "invalid context object");
    Q_ASSERT_X(e, "get account", "invalid error object");

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT au.id, au.username, au.imap, au.pop, au.sieve, au.smtpauth, au.quota, au.created_at, au.updated_at, au.valid_until, au.pwd_expire, au.status, au.domain_id, au.prefix, au.domain_name FROM accountuser au WHERE id = :id"));
    q.bindValue(QStringLiteral(":id"), id);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "User account data could not be queried from the database."));
        qCCritical(SK_ACCOUNT, "Data for user account with ID %lu could not be queried from the database: %s", id, qUtf8Printable(q.lastError().text()));
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
    QDateTime accCreated = q.value(7).toDateTime();
    accCreated.setTimeSpec(Qt::UTC);
    a.setCreated(accCreated);
    QDateTime accUpdated = q.value(8).toDateTime();
    accUpdated.setTimeSpec(Qt::UTC);
    a.setUpdated(accUpdated);
    QDateTime accValidUntil = q.value(9).toDateTime();
    accValidUntil.setTimeSpec(Qt::UTC);
    a.setValidUntil(accValidUntil);
    QDateTime accPwdExpires = q.value(10).toDateTime();
    accPwdExpires.setTimeSpec(Qt::UTC);
    a.setPasswordExpires(accPwdExpires);
    a.setStatus(q.value(11).value<quint8>());
    a.setDomainId(q.value(12).value<dbid_t>());
    a.setPrefix(q.value(13).toString());
    const QString domainNameAce = q.value(14).toString();
    a.setDomainName(QUrl::fromAce(domainNameAce.toLatin1()));

    QStringList emailAddresses;
    q = CPreparedSqlQueryThread(QStringLiteral("SELECT alias FROM virtual WHERE dest = :username AND username = :username ORDER BY alias ASC"));
    q.bindValue(QStringLiteral(":username"), a.username());
    if (Q_LIKELY(q.exec())) {
        const QString catchAllAlias = QLatin1Char('@') + domainNameAce;
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
        qCWarning(SK_ACCOUNT, "Failed to query email addresses for account ID %u (%s) from the database: %s", id, qUtf8Printable(a.username()), qUtf8Printable(q.lastError().text()));
    }

    QStringList aliases;
    q = CPreparedSqlQueryThread(QStringLiteral("SELECT dest FROM virtual WHERE alias = :username AND username = ''"));
    q.bindValue(QStringLiteral(":username"), a.username());
    if (Q_LIKELY(q.exec())) {
        while (q.next()) {
            const QStringList destinations = q.value(0).toString().split(QLatin1Char(','));
            if (!destinations.empty()) {
                for (const QString &dest : destinations) {
                    if (dest != a.username()) {
                        aliases << dest;
                    } else {
                        a.setKeepLocal(true);
                    }
                }
            }
        }
    } else {
        qCWarning(SK_ACCOUNT, "Failed to query email forwards for account ID %u (%s) from the database: %s", id, qUtf8Printable(a.username()), qUtf8Printable(q.lastError().text()));
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

    bool gotQuota = false;
    if (SkaffariConfig::useMemcached()) {
        const QByteArray usage = Cutelyst::Memcached::get(MEMC_QUOTA_KEY + QString::number(id));
        if (!usage.isNull()) {
            bool ok = false;
            a.setUsage(usage.toULongLong(&ok));
            if (ok) {
                gotQuota = true;
            }
        }
    }

    if (!gotQuota) {
        SkaffariIMAP imap(c);
        if (imap.login()) {
            quota_pair quota = imap.getQuota(a.username());
            a.setUsage(quota.first);
            a.setQuota(quota.second);
            imap.logout();

            if (SkaffariConfig::useMemcached()) {
                Cutelyst::Memcached::set(MEMC_QUOTA_KEY + QString::number(id), QByteArray::number(quota.first), MEMC_QUOTA_EXP);
            }
        }
    }

    return a;
}

void Account::toStash(Cutelyst::Context *c, dbid_t accountId)
{
    Q_ASSERT_X(c, "account to stash", "invalid context object");

    SkaffariError e(c);
    Account a = Account::get(c, &e, accountId);
    if (Q_LIKELY(a.isValid())) {
        c->stash({
                     {QStringLiteral(ACCOUNT_STASH_KEY), QVariant::fromValue<Account>(a)},
                     {QStringLiteral("site_subtitle"), a.username()}
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
                 {QStringLiteral("site_subtitle"), a.username()}
             });
}

Account Account::fromStash(Cutelyst::Context *c)
{
    Account a;
    a = c->stash(QStringLiteral(ACCOUNT_STASH_KEY)).value<Account>();
    return a;
}

bool Account::update(Cutelyst::Context *c, SkaffariError *e, Domain *dom, const QVariantHash &p)
{
    bool ret = false;

    Q_ASSERT_X(c, "update account", "invalid context object");
    Q_ASSERT_X(e, "update account", "invalid error object");
    Q_ASSERT_X(dom, "update account", "invalid domain object");

    const QString password = p.value(QStringLiteral("password")).toString();
    QByteArray encPw;
    if (!password.isEmpty()) {
        Password pw(password);
        encPw = pw.encrypt(SkaffariConfig::accPwMethod(), SkaffariConfig::accPwAlgorithm(), SkaffariConfig::accPwRounds());
        if (Q_UNLIKELY(encPw.isEmpty())) {
            e->setErrorType(SkaffariError::ApplicationError);
            e->setErrorText(c->translate("Account", "Password encryption failed."));
            qCCritical(SK_ACCOUNT) << "Failed to encrypt user password with method" << SkaffariConfig::accPwMethod() << ", algorithm" << SkaffariConfig::accPwAlgorithm() << "and" << SkaffariConfig::accPwRounds() << "rounds";
            return ret;
        }
    }

    const quota_size_t quota = p.contains(QStringLiteral("quota")) ? static_cast<quota_size_t>(p.value(QStringLiteral("quota")).value<quota_size_t>()/Q_UINT64_C(1024)) : d->quota;

    if (quota != d->quota) {
        SkaffariIMAP imap(c);
        if (Q_LIKELY(imap.login())) {
            if (Q_UNLIKELY(!imap.setQuota(d->username, quota))) {
                e->setImapError(imap.lastError(), c->translate("Account", "Changing the storage quota failed."));
                return ret;
            }
        } else {
            e->setImapError(imap.lastError(), c->translate("Account", "Logging in to IMAP server to change the storage quota failed."));
            return ret;
        }
    }

    const QDateTime validUntil = p.value(QStringLiteral("validUntil")).toDateTime().toUTC();
    const QDateTime pwExpires = p.value(QStringLiteral("passwordExpires")).toDateTime().toUTC();
    const QDateTime currentTimeUtc = QDateTime::currentDateTimeUtc();

    const bool imap = p.value(QStringLiteral("imap")).toBool();
    const bool pop = p.value(QStringLiteral("pop")).toBool();
    const bool sieve = p.value(QStringLiteral("sieve")).toBool();
    const bool smtpauth = p.value(QStringLiteral("smtpauth")).toBool();
    const bool _catchAll = p.value(QStringLiteral("catchall")).toBool();

    QSqlQuery q;
    if (!password.isEmpty()) {
        q = CPreparedSqlQueryThread(QStringLiteral("UPDATE accountuser SET password = :password, quota = :quota, valid_until = :validUntil, updated_at = :updated_at, imap = :imap, pop = :pop, sieve = :sieve, smtpauth =:smtpauth, pwd_expire = :pwd_expire WHERE id = :id"));
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
        e->setSqlError(q.lastError(), c->translate("Account", "User account could not be updated in the database."));
        qCCritical(SK_ACCOUNT, "User account %s (ID: %lu) could not be updated in the database: %s", qUtf8Printable(d->username), d->id, qUtf8Printable(q.lastError().text()));
        return ret;
    }

    if (_catchAll != d->catchAll) {
        const QString catchAllAlias = QLatin1Char('@') + QString::fromLatin1(QUrl::toAce(dom->name()));
        if (_catchAll && !d->catchAll) {
            q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM virtual WHERE alias = :alias"));
            q.bindValue(QStringLiteral(":alias"), catchAllAlias);

            if (Q_UNLIKELY(!q.exec())) {
                e->setSqlError(q.lastError(), c->translate("Account", "Existing catch-all address could not be deleted from the database."));
                qCWarning(SK_ACCOUNT, "Existing catch-all address of domain %s (ID: %lu) could not be deleted from the database: %s", qUtf8Printable(dom->name()), dom->id(), qUtf8Printable(q.lastError().text()));
            }

            q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO virtual (alias, dest, username, status) VALUES (:alias, :dest, :username, :status)"));
            q.bindValue(QStringLiteral(":alias"), catchAllAlias);
            q.bindValue(QStringLiteral(":dest"), d->username);
            q.bindValue(QStringLiteral(":username"), d->username);
            q.bindValue(QStringLiteral(":status"), 1);

            if (Q_UNLIKELY(!q.exec())) {
                e->setSqlError(q.lastError(), c->translate("Account", "Account could not be set up as catch-all account."));
                qCWarning(SK_ACCOUNT, "Account %s could not be set up as catch-all account for domain %s: %s", qUtf8Printable(d->username), qUtf8Printable(dom->name()), qUtf8Printable(q.lastError().text()));
            } else {
                setCatchAll(true);
            }

        } else if (!_catchAll && d->catchAll) {
            q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM virtual WHERE alias = :alias AND username = :username"));
            q.bindValue(QStringLiteral(":alias"), catchAllAlias);
            q.bindValue(QStringLiteral(":username"), d->username);

            if (Q_UNLIKELY(!q.exec())) {
                e->setSqlError(q.lastError(), c->translate("Account", "User account could not be removed as a catch-all account for this domain."));
                qCWarning(SK_ACCOUNT, "User account %s could not be removed as a catch-all account for domain %s: %s", qUtf8Printable(d->username), qUtf8Printable(d->domainName), qUtf8Printable(q.lastError().text()));
            } else {
                setCatchAll(false);
            }
        }
    }

    q = CPreparedSqlQueryThread(QStringLiteral("UPDATE domain SET domainquotaused = (SELECT SUM(quota) FROM accountuser WHERE domain_id = :domain_id) WHERE id = :domain_id"));
    q.bindValue(QStringLiteral(":domain_id"), d->domainId);
    if (Q_UNLIKELY(!q.exec())) {
        qCWarning(SK_ACCOUNT, "Failed to update used domain quota for domain ID %u: %s", d->domainId, qUtf8Printable(q.lastError().text()));
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
        qCWarning(SK_ACCOUNT) << "Failed to query used domain quota after updating account ID" << d->id << "in domain ID" << dom->id();
        qCDebug(SK_ACCOUNT) << q.lastError().text();
    } else {
        if (Q_LIKELY(q.next())) {
            dom->setDomainQuotaUsed(q.value(0).value<quota_size_t>());
        }
    }

    qCInfo(SK_ACCOUNT, "%s updated account %s for domain %s", qUtf8Printable(Utils::getUserName(c)), qUtf8Printable(d->username), qUtf8Printable(d->domainName));

    ret = true;

    return ret;
}

#define PAM_ACCT_EXPIRED 1
#define PAM_NEW_AUTHTOK_REQD 2

QStringList Account::check(Cutelyst::Context *c, SkaffariError *e, const Domain &domain, const Cutelyst::ParamsMultiMap &p)
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
        e->setImapError(imap.lastError(), c->translate("Account", "Could not retrieve a list of all mailboxes from the IMAP server."));
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
            actions.push_back(c->translate("Account", "Missing mailbox created on IMAP server."));
        }
    }

    quota_pair quota = imap.getQuota(d->username);

    if ((dom.domainQuota() > 0) && ((d->quota == 0) || (quota.second == 0))) {
        const quota_size_t newQuota = (dom.quota() > 0) ? dom.quota() : (SkaffariConfig::defQuota() > 0) ? SkaffariConfig::defQuota() : 10240;
        if (quota.second == 0) {
            if (Q_UNLIKELY(!imap.setQuota(d->username, newQuota))) {
                e->setImapError(imap.lastError());
                imap.logout();
                return actions;
            } else {
                qCInfo(SK_ACCOUNT, "%s set correct mailbox storage quota of %llu on IMAP server for user account ID %u.", qUtf8Printable(c->stash(QStringLiteral("userName")).toString()), newQuota, d->id);
                actions.push_back(c->translate("Account", "Storage quota on IMAP server fixed."));
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
                qCInfo(SK_ACCOUNT, "%s set correct mailbox storage quota of %llu in database for user account ID %u.", qUtf8Printable(c->stash(QStringLiteral("userName")).toString()), newQuota, d->id);
                actions.push_back(c->translate("Account", "Storage quota in database fixed."));
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
            qCInfo(SK_ACCOUNT, "%s set correct mailbox storage quota of %llu on IMAP server for user account ID %u.", qUtf8Printable(c->stash(QStringLiteral("userName")).toString()), d->quota, d->id);
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
            qCWarning(SK_ACCOUNT, "Failed to update status for account ID %u in the database: %s", d->id, qUtf8Printable(q.lastError().text()));
        } else {
            qCInfo(SK_ACCOUNT, "%s set correct status value of %i for user account ID %u.", qUtf8Printable(Utils::getUserName(c)), newStatus, d->id);
        }
    }

    if (Utils::checkCheckbox(p, QStringLiteral("checkChildAddresses")) && !domain.children().empty()) {
        const QStringList addresses = d->addresses;
        if (!addresses.empty()) {
            QSqlQuery q;
            QStringList newAddresses;
            for (const QString &address : addresses) {
                std::pair<QString,QString> parts = addressParts(address);
                if (parts.second == d->domainName) {
                    const QVector<SimpleDomain> thekids = domain.children();
                    for (const SimpleDomain &kid : thekids) {
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
                                    qCInfo(SK_ACCOUNT, "%s added a new address for child domain %s to account ID %u.", qUtf8Printable(Utils::getUserName(c)), qUtf8Printable(kid.name()), d->id);
                                    actions.push_back(c->translate("Account", "Added new email address for child domain: %1").arg(newAddress));
                                } else {
                                    qCWarning(SK_ACCOUNT, "Failed to add new email address for child domain while checking account ID %u: %s", d->id, qUtf8Printable(qq.lastError().text()));
                                }
                            }
                        } else {
                            qCWarning(SK_ACCOUNT, "Failed to check if email address is already in use by another account: %s", qUtf8Printable(q.lastError().text()));
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
        qCInfo(SK_ACCOUNT, "Nothing to do for user account ID %u.", d->id);
    } else {
        d->usage = quota.first;
        d->status = newStatus;
    }

    if (SkaffariConfig::useMemcached()) {
        Cutelyst::Memcached::set(MEMC_QUOTA_KEY + QString::number(d->id), QByteArray::number(d->usage), MEMC_QUOTA_EXP);
    }

    qCInfo(SK_ACCOUNT, "%s finished checking user account ID %u.", qUtf8Printable(Utils::getUserName(c)), d->id);

    return actions;
}

QString Account::updateEmail(Cutelyst::Context *c, SkaffariError *e, const QVariantHash &p, const QString &oldAddress)
{
    QString ret;

    Q_ASSERT_X(c, "update email", "invalid context object");
    Q_ASSERT_X(e, "update email", "invalid error object");

    const dbid_t domId = p.value(QStringLiteral("newmaildomain")).value<dbid_t>();
    const Domain dom = Domain::get(c, domId, e);

    if (e->type() != SkaffariError::NoError) {
        return ret;
    }

    const QString address = p.value(QStringLiteral("newlocalpart")).toString() + QLatin1Char('@') + dom.name();

    if (!dom.isFreeAddressEnabled() && (dom.id() != d->domainId)) {
        e->setErrorText(c->translate("Account", "You can not create email addresses for other domains as long as free addresses are not allowed for this domain."));
        e->setErrorType(SkaffariError::AutorizationError);
        qCWarning(SK_ACCOUNT, "Updating email address failed: can not create email address %s because domain %s (ID: %lu) is not allowed to have free addresses.", qUtf8Printable(address), qUtf8Printable(dom.name()), dom.id());
        return ret;
    }

    if (address == oldAddress) {
        e->setErrorType(SkaffariError::InputError);
        e->setErrorText(c->translate("Account", "The email address has not been changed."));
        qCWarning(SK_ACCOUNT, "Updating email address failed: address %s has not been changed.", qUtf8Printable(address));
        return ret;
    }

    if (!d->addresses.contains(oldAddress)) {
        e->setErrorType(SkaffariError::InputError);
        e->setErrorText(c->translate("Account", "The email address %1 is not part of this account.").arg(oldAddress));
        qCWarning(SK_ACCOUNT, "Updating email address failed: address %s is not part of account %s (ID: %lu).", qUtf8Printable(oldAddress), qUtf8Printable(d->username), d->id);
        return ret;
    }

    if (d->addresses.contains(address)) {
        e->setErrorType(SkaffariError::InputError);
        e->setErrorText(c->translate("Account", "The email address %1 is already part of this account.").arg(address));
        qCWarning(SK_ACCOUNT, "Updating email address failed: address %s is already part of account %s (ID: %lu).", qUtf8Printable(address), qUtf8Printable(d->username), d->id);
        return ret;
    }

    const QString aceAddress = Account::addressToACE(address);
    if (aceAddress.isEmpty()) {
        e->setErrorText(c->translate("Account", "Can not convert email address %1 into a ACE string.").arg(address));
        e->setErrorType(SkaffariError::InputError);
        return ret;
    }

    QList<Cutelyst::ValidatorEmail::Diagnose> diags;
    if (!Cutelyst::ValidatorEmail::validate(aceAddress, Cutelyst::ValidatorEmail::Valid, false, &diags)) {
        e->setErrorType(SkaffariError::InputError);
        e->setErrorText(Cutelyst::ValidatorEmail::diagnoseString(c, diags.at(0)));
        qCWarning(SK_ACCOUNT, "Updating email address failed: new address %s is not valid.", qUtf8Printable(address));
        return ret;
    }

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT alias, dest, username FROM virtual WHERE alias = :address"));
    q.bindValue(QStringLiteral(":address"), aceAddress);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Unable to check if the new email address %1 is already assigned to another user account.").arg(address));
        qCCritical(SK_ACCOUNT, "Unable to check if the new email address %s is already assigned to another user account: %s", qUtf8Printable(address), qUtf8Printable(q.lastError().text()));
        return ret;
    }

    if (Q_UNLIKELY(q.next())) {
        QString otherUser = q.value(2).toString();
        if (otherUser.isEmpty()) {
            otherUser = q.value(1).toString();
        }
        e->setErrorType(SkaffariError::InputError);
        e->setErrorText(c->translate("Account", "Email address %1 is already assigned to another user account.").arg(address));
        qCWarning(SK_ACCOUNT, "%s tried to change email address %s to %s that is already assigned to %s.", qUtf8Printable(Utils::getUserName(c)), qUtf8Printable(oldAddress), qUtf8Printable(address), qUtf8Printable(otherUser));
        return ret;
    }

    q = CPreparedSqlQueryThread(QStringLiteral("UPDATE virtual SET alias = :aliasnew WHERE alias = :aliasold AND username = :username"));
    q.bindValue(QStringLiteral(":aliasnew"), aceAddress);
    q.bindValue(QStringLiteral(":aliasold"), addressToACE(oldAddress));
    q.bindValue(QStringLiteral(":username"), d->username);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to update email address %1.").arg(oldAddress));
        qCCritical(SK_ACCOUNT, "Failed to change email address %s of account ID %u (%s) to %s: %s", qUtf8Printable(oldAddress), d->id, qUtf8Printable(d->username), qUtf8Printable(address), qUtf8Printable(q.lastError().text()));
        return ret;
    }

    d->addresses.removeOne(oldAddress);
    d->addresses.push_back(address);
    if (d->addresses.size() > 1) {
        QCollator col(c->locale());
        std::sort(d->addresses.begin(), d->addresses.end(), col);
    }

    qCInfo(SK_ACCOUNT, "%s updated email address %s of account %s (ID %lu) to %s.", qUtf8Printable(Utils::getUserName(c)), qUtf8Printable(oldAddress), qUtf8Printable(d->username), d->id, qUtf8Printable(address));

    ret = address;

    return ret;
}

QString Account::addEmail(Cutelyst::Context *c, SkaffariError *e, const QVariantHash &p)
{
    QString ret;

    Q_ASSERT_X(c, "update email", "invalid context object");
    Q_ASSERT_X(e, "update email", "invalid error object");

    const dbid_t domId = p.value(QStringLiteral("newmaildomain")).value<dbid_t>();
    const Domain dom = Domain::get(c, domId, e);

    if (e->type() != SkaffariError::NoError) {
        return ret;
    }

    const QString address = p.value(QStringLiteral("newlocalpart")).toString() + QLatin1Char('@') + dom.name();

    if (!dom.isFreeAddressEnabled() && (dom.id() != d->domainId)) {
        e->setErrorText(c->translate("Account", "You can not create email addresses for other domains as long as free addresses are not allowed for this domain."));
        e->setErrorType(SkaffariError::AutorizationError);
        qCWarning(SK_ACCOUNT, "Adding email address failed: can not create email address %s because domain %s (ID: %lu) is not allowed to have free addresses.", qUtf8Printable(address), qUtf8Printable(dom.name()), dom.id());
        return ret;
    }

    const QString aceAddress = Account::addressToACE(address);

    if (aceAddress.isEmpty()) {
        e->setErrorText(c->translate("Account", "Can not convert email address %1 into a ACE string.").arg(address));
        e->setErrorType(SkaffariError::InputError);
        return ret;
    }

    if (d->addresses.contains(address)) {
        e->setErrorType(SkaffariError::InputError);
        e->setErrorText(c->translate("Account", "The email address %1 is already part of this account.").arg(address));
        qCWarning(SK_ACCOUNT, "Updating email address failed: address %s is already part of account %s.", qUtf8Printable(address), qUtf8Printable(d->username));
        return ret;
    }

    QList<Cutelyst::ValidatorEmail::Diagnose> diags;
    if (!Cutelyst::ValidatorEmail::validate(aceAddress, Cutelyst::ValidatorEmail::Valid, false, &diags)) {
        e->setErrorType(SkaffariError::InputError);
        e->setErrorText(Cutelyst::ValidatorEmail::diagnoseString(c, diags.at(0)));
        qCWarning(SK_ACCOUNT, "Adding email address failed: new address %s is not valid.", qUtf8Printable(address));
        return ret;
    }

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT alias, dest, username FROM virtual WHERE alias = :address"));
    q.bindValue(QStringLiteral(":address"), aceAddress);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Unable to check if the new email address %1 is already assigned to another user account.").arg(address));
        qCCritical(SK_ACCOUNT, "Unable to check if the new email address %s for account %s (ID: %lu) is already assigned to another user account: %s", qUtf8Printable(address), qUtf8Printable(d->username), d->id, qUtf8Printable(q.lastError().text()));
        return ret;
    }

    if (Q_UNLIKELY(q.next())) {
        QString otherUser = q.value(2).toString();
        if (otherUser.isEmpty()) {
            otherUser = q.value(1).toString();
        }
        e->setErrorType(SkaffariError::InputError);
        e->setErrorText(c->translate("Account", "Email address %1 is already assigned to another user account.").arg(address));
        qCWarning(SK_ACCOUNT, "%s tried to add email address %s to account %s (ID: %lu) that is already assigned to %s.", qUtf8Printable(Utils::getUserName(c)), qUtf8Printable(address), qUtf8Printable(d->username), d->id, qUtf8Printable(otherUser));
        return ret;
    }

    q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO virtual (alias, dest, username, status) VALUES (:alias, :dest, :username, :status)"));
    q.bindValue(QStringLiteral(":alias"), aceAddress);
    q.bindValue(QStringLiteral(":dest"), d->username);
    q.bindValue(QStringLiteral(":username"), d->username);
    q.bindValue(QStringLiteral(":status"), 1);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "New email address could not be added to database."));
        qCCritical(SK_ACCOUNT, "Failed to insert new email address %s for account ID %u (%s) into database: %s", qUtf8Printable(address), d->id, qUtf8Printable(d->username), qUtf8Printable(q.lastError().text()));
        return ret;
    }

    d->addresses.push_back(address);
    if (d->addresses.size() > 1) {
        QCollator col(c->locale());
        std::sort(d->addresses.begin(), d->addresses.end(), col);
    }

    qCInfo(SK_ACCOUNT, "%s added new email address %s to account %s (ID: %lu).", qUtf8Printable(Utils::getUserName(c)), qUtf8Printable(address), qUtf8Printable(d->username), d->id);

    ret = address;

    return ret;
}

bool Account::removeEmail(Cutelyst::Context *c, SkaffariError *e, const QString &address)
{
    bool ret = false;

    Q_ASSERT_X(c, "update email", "invalid context object");
    Q_ASSERT_X(e, "update email", "invalid error object");

    if (!d->addresses.contains(address)) {
        e->setErrorType(SkaffariError::InputError);
        e->setErrorText(c->translate("Account", "The email address %1 is not part of this account.").arg(address));
        qCWarning(SK_ACCOUNT, "Removing email address failed: address %s is not part of account %s (ID: %lu).", qUtf8Printable(address), qUtf8Printable(d->username), d->id);
        return ret;
    }

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM virtual WHERE alias = :address"));
    q.bindValue(QStringLiteral(":address"), addressToACE(address));

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Email address %1 could not be removed from user account %2.").arg(address, d->username));
        qCCritical(SK_ACCOUNT, "Email address %s could not be removed from user account %s (ID: %lu): %s", qUtf8Printable(address), qUtf8Printable(d->username), d->id, qUtf8Printable(q.lastError().text()));
        return ret;
    }

    d->addresses.removeOne(address);

    qCInfo(SK_ACCOUNT, "%s removed email address %s from account %s (ID: %lu).", qUtf8Printable(Utils::getUserName(c)), qUtf8Printable(address), qUtf8Printable(d->username), d->id);

    ret = true;

    return ret;
}

bool Account::addForward(Cutelyst::Context *c, SkaffariError *e, Account *a, const Cutelyst::ParamsMultiMap &p)
{
    bool ret = false;

    Q_ASSERT_X(c, "add forward", "invalid context object");
    Q_ASSERT_X(e, "add forward", "invalid error object");
    Q_ASSERT_X(a, "add forward", "invalid account object");
    Q_ASSERT_X(!p.empty(), "add forward", "empty input parameters");

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT dest FROM virtual WHERE alias = :username AND username = ''"));
    q.bindValue(QStringLiteral(":username"), a->username());

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Cannot retrieve current list of forwarding addresses for user account %1 from the database.").arg(a->username()));
        qCCritical(SK_ACCOUNT, "Cannot retrieve current list of forwarding addresses for user account %s (ID: %lu) from the database: %s", qUtf8Printable(a->username()), a->id(), qUtf8Printable(q.lastError().text()));
        return ret;
    }

    QStringList forwards;
    bool oldDataAvailable = false;
    if (q.next()) {
        oldDataAvailable = true;
        forwards = q.value(0).toString().split(QLatin1Char(','), QString::SkipEmptyParts);
    }

    const QString newForward = p.value(QStringLiteral("newforward"));

    if (forwards.contains(newForward, Qt::CaseInsensitive)) {
        e->setErrorType(SkaffariError::InputError);
        e->setErrorText(c->translate("Account", "Emails to account %1 are already forwarded to %2.").arg(a->username(), newForward));
        qCWarning(SK_ACCOUNT, "%s tried to add already existing forward email address to account %s (ID: %u).", qUtf8Printable(c->stash(QStringLiteral("userName")).toString()), qUtf8Printable(a->username()), a->id());
        return ret;
    }

    forwards.prepend(newForward);

    if (oldDataAvailable) {
        q = CPreparedSqlQueryThread(QStringLiteral("UPDATE virtual SET dest = :dest WHERE alias = :alias AND username = ''"));
    } else {
        q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO virtual (alias, dest, username) VALUES (:alias, :dest, '')"));
    }
    q.bindValue(QStringLiteral(":dest"), forwards.join(QLatin1Char(',')));
    q.bindValue(QStringLiteral(":alias"), a->username());

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Cannot update the list of forwarding addresses for user account %1 in the database.").arg(a->username()));
        qCCritical(SK_ACCOUNT, "Cannot update the list of forwarding addresses for user account %s (ID: %u) in the database: %s", qUtf8Printable(a->username()), a->id(), qUtf8Printable(q.lastError().text()));
        return ret;
    }

    forwards.removeAll(a->username());

    a->setForwards(forwards);

    qCInfo(SK_ACCOUNT, "%s added new forward address %s to account %s (ID: %u)", qUtf8Printable(c->stash(QStringLiteral("userName")).toString()), qUtf8Printable(newForward), qUtf8Printable(a->username()), a->id());

    ret = true;

    return ret;
}

bool Account::removeForward(Cutelyst::Context *c, SkaffariError *e, Account *a, const QString &forward)
{
    bool ret = false;

    Q_ASSERT_X(c, "add forward", "invalid context object");
    Q_ASSERT_X(e, "add forward", "invalid error object");
    Q_ASSERT_X(a, "add forward", "invalid account object");
    Q_ASSERT_X(!forward.isEmpty(), "add forward", "empty input parameters");

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT dest FROM virtual WHERE alias = :username AND username = ''"));
    q.bindValue(QStringLiteral(":username"), a->username());

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Cannot retrieve current list of forwarding addresses for user account %1 from the database.").arg(a->username()));
        qCCritical(SK_ACCOUNT, "Cannot retrieve current list of forwarding addresses for user account %s (ID: %lu) from the database: %s", qUtf8Printable(a->username()), a->id(), qUtf8Printable(q.lastError().text()));
        return ret;
    }

    QStringList forwards;
    bool oldDataAvailable = false;
    if (q.next()) {
        forwards = q.value(0).toString().split(QLatin1Char(','), QString::SkipEmptyParts);
        oldDataAvailable = true;
    }

    if (Q_UNLIKELY(!forwards.contains(forward, Qt::CaseInsensitive))) {
        e->setErrorType(SkaffariError::InputError);
        e->setErrorText(c->translate("Account", "Forwarding address %1 cannot be removed from user account %2. Forwarding does not exist for this account.").arg(forward, a->username()));
        qCWarning(SK_ACCOUNT, "%s tried to remove not existing forward email address from account %s (ID: %u).", qUtf8Printable(c->stash(QStringLiteral("userName")).toString()), qUtf8Printable(a->username()), a->id());
        return ret;
    }

    forwards.removeAll(forward);

    if (forwards.empty() || ((forwards.size() == 1) && (forwards.at(0) == a->username()))) {

        q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM virtual WHERE alias = :username AND username = ''"));
        q.bindValue(QStringLiteral(":username"), a->username());

        if (Q_UNLIKELY(!q.exec())) {
            e->setSqlError(q.lastError(), c->translate("Account", "Forwarding addresses for user account %1 cannot be deleted from the database.").arg(a->username()));
            qCCritical(SK_ACCOUNT, "Failed to remove all forwards of account %s (ID: %u) in the database: %s", qUtf8Printable(a->username()), a->id(), qUtf8Printable(q.lastError().text()));
            return ret;
        }

        a->setKeepLocal(false);

    } else {

        if (oldDataAvailable) {
            q = CPreparedSqlQueryThread(QStringLiteral("UPDATE virtual SET dest = :dest WHERE alias = :alias AND username = ''"));
        } else {
            q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO virtual (alias, dest, username) VALUES (:alias, :dest, '')"));
        }
        q.bindValue(QStringLiteral(":alias"), a->username());
        q.bindValue(QStringLiteral(":dest"), forwards.join(QLatin1Char(',')));

        if (Q_UNLIKELY(!q.exec())) {
            e->setSqlError(q.lastError(), c->translate("Account", "Cannot update the list of forwarding addresses for user account %1 in the database.").arg(a->username()));
            qCCritical(SK_ACCOUNT, "Failed to update list of forward email addresses for account %s (ID: %u) in the database after removing one forward address: %s", qUtf8Printable(a->username()), a->id(), qUtf8Printable(q.lastError().text()));
            return ret;
        }

    }

    forwards.removeAll(a->username());

    a->setForwards(forwards);

    qCInfo(SK_ACCOUNT, "%s removed forward address %s from account %s (ID: %u).", qUtf8Printable(c->stash(QStringLiteral("userName")).toString()), qUtf8Printable(forward), qUtf8Printable(a->username()), a->id());

    ret = true;

    return ret;
}

bool Account::editForward(Cutelyst::Context *c, SkaffariError *e, Account *a, const QString &oldForward, const QString &newForward)
{
    bool ret = false;

    Q_ASSERT_X(c, "edit forward", "invalid context object");
    Q_ASSERT_X(e, "edit forward", "invalid error object");
    Q_ASSERT_X(a, "edit forward", "invalid account object");
    Q_ASSERT_X(!oldForward.isEmpty(), "edit forward", "old forward address can not be empty");
    Q_ASSERT_X(!newForward.isEmpty(), "edit forward", "new forward address can not be empty");

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT dest FROM virtual WHERE alias = :username AND username = ''"));
    q.bindValue(QStringLiteral(":username"), a->username());

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Cannot retrieve current list of forwarding addresses for user account %1 from the database.").arg(a->username()));
        qCCritical(SK_ACCOUNT, "Cannot retrieve current list of forwarding addresses for user account %s (ID: %lu) from the database: %s", qUtf8Printable(a->username()), a->id(), qUtf8Printable(q.lastError().text()));
        return ret;
    }

    QStringList forwards;
    if (q.next()) {
        forwards = q.value(0).toString().split(QLatin1Char(','), QString::SkipEmptyParts);
    }

    if (Q_UNLIKELY(!forwards.contains(oldForward, Qt::CaseInsensitive))) {
        e->setErrorType(SkaffariError::InputError);
        e->setErrorText(c->translate("Account", "Can not change forward email address %1 from account %2. The forward does not exist.").arg(oldForward, a->username()));
        qCWarning(SK_ACCOUNT, "%s tried to change not existing forward email address %s on account %s (ID: %u).", qUtf8Printable(c->stash(QStringLiteral("userName")).toString()), qUtf8Printable(oldForward), qUtf8Printable(a->username()), a->id());
        return ret;
    }

    if (Q_UNLIKELY(forwards.contains(newForward, Qt::CaseInsensitive))) {
        e->setErrorType(SkaffariError::InputError);
        e->setErrorText(c->translate("Account", "Forwarding address %1 for user account %2 cannot be changed to %3. The new forwarding already exists.").arg(oldForward, a->username(), newForward));
        qCWarning(SK_ACCOUNT, "%s tried to change forward email address %s to already existing forward %s on account %s (ID: %u).", qUtf8Printable(c->stash(QStringLiteral("userName")).toString()), qUtf8Printable(oldForward), qUtf8Printable(newForward), qUtf8Printable(a->username()), a->id());
        return ret;
    }

    forwards.removeAll(oldForward);
    forwards.prepend(newForward);

    q = CPreparedSqlQueryThread(QStringLiteral("UPDATE virtual SET dest = :dest WHERE alias = :alias AND username = ''"));
    q.bindValue(QStringLiteral(":alias"), a->username());
    q.bindValue(QStringLiteral(":dest"), forwards.join(QLatin1Char(',')));

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Cannot update the list of forwarding addresses for user account %1 in the database.").arg(a->username()));
        qCCritical(SK_ACCOUNT, "Failed to update list of forward email addresses for account %s (ID: %u) in the database after changing one forward address: %s", qUtf8Printable(a->username()), a->id(), qUtf8Printable(q.lastError().text()));
        return ret;
    }

    forwards.removeAll(a->username());

    a->setForwards(forwards);

    qCInfo(SK_ACCOUNT, "%s changed forward address %s to %s for account %s (ID: %u).", qUtf8Printable(c->stash(QStringLiteral("userName")).toString()), qUtf8Printable(oldForward), qUtf8Printable(newForward), qUtf8Printable(a->username()), a->id());

    ret = true;

    return ret;
}

bool Account::changeKeepLocal(Cutelyst::Context *c, SkaffariError *e, Account *a, bool keepLocal)
{
    bool ret = false;

    Q_ASSERT_X(c, "edit forward", "invalid context object");
    Q_ASSERT_X(e, "edit forward", "invalid error object");
    Q_ASSERT_X(a, "edit forward", "invalid account object");

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT dest FROM virtual WHERE alias = :username AND username = ''"));
    q.bindValue(QStringLiteral(":username"), a->username());

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Cannot retrieve current list of forwarding addresses for user account %1 from the database.").arg(a->username()));
        qCCritical(SK_ACCOUNT, "Cannot retrieve current list of forwarding addresses for user account %s (ID: %lu) from the database: %s", qUtf8Printable(a->username()), a->id(), qUtf8Printable(q.lastError().text()));
        return ret;
    }

    QStringList forwards;
    if (q.next()) {
        forwards = q.value(0).toString().split(QLatin1Char(','), QString::SkipEmptyParts);
    }

    if ((keepLocal && (!forwards.contains(a->username()))) || (!keepLocal && (forwards.contains(a->username())))) {

        if (keepLocal) {
            forwards.append(a->username());
        } else {
            forwards.removeAll(a->username());
        }

        q = CPreparedSqlQueryThread(QStringLiteral("UPDATE virtual SET dest = :dest WHERE alias = :alias AND username = ''"));
        q.bindValue(QStringLiteral(":alias"), a->username());
        q.bindValue(QStringLiteral(":dest"), forwards.join(QLatin1Char(',')));

        if (Q_UNLIKELY(!q.exec())) {
            if (keepLocal) {
                e->setSqlError(q.lastError(), c->translate("Account", "Failed to enable the keeping of forwarded emails in the local mail box for account %1 in the database.").arg(a->username()));
                qCCritical(SK_ACCOUNT, "Failed to enable the keeping of forwarded emails in the local mail box for account %s (ID: %u) in the database: %s", qUtf8Printable(a->username()), a->id(), qUtf8Printable(q.lastError().text()));
            } else {
                e->setSqlError(q.lastError(), c->translate("Account", "Failed to disable the keeping of forwarded emails in the local mail box for account %1 in the database.").arg(a->username()));
                qCCritical(SK_ACCOUNT, "Failed to disable the keeping of forwarded emails in the local mail box for account %s (ID: %u) in the database: %s", qUtf8Printable(a->username()), a->id(), qUtf8Printable(q.lastError().text()));
            }
            return ret;
        }

        a->setKeepLocal(keepLocal);

        qCInfo(SK_ACCOUNT, "%s changed keep local of account %s (ID: %u) to %s.", qUtf8Printable(c->stash(QStringLiteral("userName")).toString()), qUtf8Printable(a->username()), a->id(), a->keepLocal() ? "true" : "false");

    }

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
