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
#include "domain.h"
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

Q_LOGGING_CATEGORY(SK_ACCOUNT, "skaffari.account")

Account::Account() :
    d(new AccountData)
{

}


Account::Account(quint32 id, quint32 domainId, const QString& username, const QString &prefix, const QString &domainName, bool imap, bool pop, bool sieve, bool smtpauth, const QStringList &addresses, const QStringList &forwards, qint32 quota, qint32 usage, const QDateTime &created, const QDateTime &updated, const QDateTime &validUntil, bool keepLocal) :
    d(new AccountData(id, domainId, username, prefix, domainName, imap, pop, sieve, smtpauth, addresses, forwards, quota, usage, created, updated, validUntil, keepLocal))
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

quint32 Account::getId() const
{
    return d->id;
}


void Account::setId(quint32 nId)
{
    d->id = nId;
}


quint32 Account::getDomainId() const
{
    return d->domainId;
}


void Account::setDomainId(quint32 nDomainId)
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




qint32 Account::getQuota() const
{
	return d->quota;
}


void Account::setQuota(qint32 nQuota)
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


qint32 Account::getUsage() const
{
	return d->usage;
}


void Account::setUsage(qint32 nUsage)
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


Account Account::create(Cutelyst::Context *c, SkaffariError *e, const Cutelyst::ParamsMultiMap &p, const Domain &d)
{
    Account a;

    Q_ASSERT_X(c, "create account", "invalid context object");
    Q_ASSERT_X(e, "create account", "invalid error object");
    Q_ASSERT_X(!p.empty(), "create account", "empty parameters");
    Q_ASSERT_X(d.isValid(), "create account", "invalid domain object");

    const QString username = SkaffariConfig::imapDomainasprefix() ? p.value(QStringLiteral("localpart")).trimmed() + (SkaffariConfig::imapFqun() ? QLatin1Char('@') : QLatin1Char('.')) + d.getName() : p.value(QStringLiteral("username")).trimmed();

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT id FROM accountuser WHERE username = :username"));
    q.bindValue(QStringLiteral(":username"), username);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to query for already existing username."));
        return a;
    }

    if (Q_UNLIKELY(q.next())) {
        e->setErrorType(SkaffariError::InputError);
        e->setErrorText(c->translate("Account", "Username %1 is already in use.").arg(username));
        return a;
    }

    const QString email = p.value(QStringLiteral("localpart")) + QLatin1Char('@') + d.getName();

    q = CPreparedSqlQueryThread(QStringLiteral("SELECT username FROM virtual WHERE alias = :email"));
    q.bindValue(QStringLiteral(":email"), email);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to query for already existing email address."));
        return a;
    }

    if (Q_UNLIKELY(q.next())) {
        e->setErrorText(c->translate("Account", "Email address %1 is already in use by user %2.").arg(email, q.value(0).toString()));
        e->setErrorType(SkaffariError::InputError);
        return a;
    }

    const QString password = p.value(QStringLiteral("password"));
    Password pw(password);
    const QByteArray encpw = pw.encrypt(SkaffariConfig::accPwMethod(), SkaffariConfig::accPwAlgorithm(), SkaffariConfig::accPwRounds());

    if (Q_UNLIKELY(encpw.isEmpty())) {
        e->setErrorText(c->translate("Account", "Failed to encrypt user password. Please check your encryption settings."));
        e->setErrorType(SkaffariError::ConfigError);
        return a;
    }

    bool imap = p.contains(QStringLiteral("imap"));
    bool pop = p.contains(QStringLiteral("pop"));
    bool sieve = p.contains(QStringLiteral("sieve"));
    bool smtpauth = p.contains(QStringLiteral("smtpauth"));
    const quint32 quota = p.value(QStringLiteral("quota"), QStringLiteral("0")).toULong();
    const QDateTime currentUtc = QDateTime::currentDateTimeUtc();

    QDateTime validUntil = QDateTime::fromString(p.value(QStringLiteral("validUntil"), QStringLiteral("2998-12-31 23:59:59")), QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    QTimeZone userTz(Cutelyst::Session::value(c, QStringLiteral("tz"), QStringLiteral("UTC")).toByteArray());
    if (userTz == QTimeZone::utc()) {
        validUntil.setTimeSpec(Qt::UTC);
    } else {
        validUntil.setTimeZone(userTz);
        validUntil = validUntil.toUTC();
    }

    q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO accountuser (domain_id, username, password, prefix, domain_name, imap, pop, sieve, smtpauth, quota, created_at, updated_at, valid_until) "
                                         "VALUES (:domain_id, :username, :password, :prefix, :domain_name, :imap, :pop, :sieve, :smtpauth, :quota, :created_at, :updated_at, :valid_until)"));

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
    q.bindValue(QStringLiteral(":created_at"), currentUtc.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")));
    q.bindValue(QStringLiteral(":updated_at"), currentUtc);
    q.bindValue(QStringLiteral(":valid_until"), validUntil);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to create new user account in database."));
        return a;
    }

    const quint32 id = q.lastInsertId().value<quint32>();

    q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO virtual (alias, dest, username, status) VALUES (:alias, :dest, :username, :status)"));
    q.bindValue(QStringLiteral(":alias"), email);
    q.bindValue(QStringLiteral(":dest"), username);
    q.bindValue(QStringLiteral(":username"), username);
    q.bindValue(QStringLiteral(":status"), 1);

    if (!q.exec()) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to create new user account in database."));
        q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM accountuser WHERE id = :id"));
        q.bindValue(QStringLiteral(":id"), id);
        q.exec();
        return a;
    }

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
    q.exec();

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
    a.setQuota(p.value(QStringLiteral("quota")).toULong());
    const QDateTime currentUserTime = Utils::toUserTZ(c, currentUtc);
    a.setCreated(currentUserTime);
    a.setUpdated(currentUserTime);
    a.setValidUntil(Utils::toUserTZ(c, validUntil));
    a.setHumanQuota(Utils::humanBinarySize(c, (quint64)a.getQuota() * 1024));
    a.setHumanUsage(Utils::humanBinarySize(c, (quint64)a.getUsage() * 1024));

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
        return ret;
    }

    if (Q_UNLIKELY(!imap.setAcl(username, SkaffariConfig::imapUser()))) {
        e->setImapError(imap.lastError(), c->translate("Account", "Failed to set ACL for IMAP admin to delete account %1.").arg(username));
    }

    if (Q_UNLIKELY(!imap.deleteMailbox(username))) {
        e->setImapError(imap.lastError(), c->translate("Account", "Failed to delete account %1 from IMAP server.").arg(username));
        imap.logout();
        return ret;
    }

    imap.logout();

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT quota FROM accountuser WHERE username = :username"));
    q.bindValue(QStringLiteral(":username"), username);

    const quint32 quota = (q.exec() && q.next()) ? q.value(0).value<quint32>() : 0;

    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM alias WHERE username = :username"));
    q.bindValue(QStringLiteral(":username"), username);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to remove account's alias addresses from the database."));
        return ret;
    }

    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM virtual WHERE username = :username"));
    q.bindValue(QStringLiteral(":username"), username);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to remove account's addresses from the database."));
        return ret;
    }

    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM virtual WHERE alias = :username AND username = ''"));
    q.bindValue(QStringLiteral(":username"), username);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to remove account's forward addresses from the database"));
        return ret;
    }

    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM accountuser WHERE username = :username"));
    q.bindValue(QStringLiteral(":username"), username);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to remove account from the database."));
        return ret;
    }

    q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM log WHERE user = :username"));
    q.bindValue(QStringLiteral(":username"), username);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to delete log entries for this user from the database."));
        return ret;
    }

    q = CPreparedSqlQueryThread(QStringLiteral("UPDATE domain SET accountcount = accountcount - 1, domainquotaused = domainquotaused - :quota WHERE id = :id"));
    q.bindValue(QStringLiteral(":quota"), quota);
    q.bindValue(QStringLiteral(":id"), domain->id());

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to update count of domain accounts and used quota in database."));
    }

    domain->setAccounts(domain->getAccounts() - 1);
    domain->setDomainQuotaUsed(domain->getDomainQuotaUsed() - quota);

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
        q.prepare(QStringLiteral("SELECT au.id, au.username, au.imap, au.pop, au.sieve, au.smtpauth, au.quota, au.created_at, au.updated_at, au.valid_until FROM accountuser au WHERE domain_id = :domain_id ORDER BY %1 %2 LIMIT %3 OFFSET %4").arg(sortBy, sortOrder, QString::number(p.limit()), QString::number(p.offset())));
    } else {

    }

    q.bindValue(QStringLiteral(":domain_id"), d.id());

    if (!q.exec()) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to query accounts from database."));
        return pag;
    }

    pag = Cutelyst::Pagination(q.size(), p.limit(), p.currentPage(), p.pages().size());

    SkaffariIMAP imap(c);
    if (!imap.login()) {
        qCWarning(SK_ACCOUNT) << "Failed to login to IMAP server. Omitting quota query.";
    }

    while (q.next()) {
        const QString username = q.value(1).toString();

        QSqlQuery q2 = CPreparedSqlQueryThread(QStringLiteral("SELECT alias FROM virtual WHERE dest = :username AND username = :username"));
        q2.bindValue(QStringLiteral(":username"), username);
        q2.exec();
        QStringList emailAddresses;
        while (q2.next()) {
            emailAddresses << q2.value(0).toString();
        }

        q2 = CPreparedSqlQueryThread(QStringLiteral("SELECT dest FROM virtual WHERE alias = :username AND username = ''"));
        q2.bindValue(QStringLiteral(":username"), username);
        q2.exec();
        QStringList aliases;
        bool _keepLocal = false;
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

        Account a(q.value(0).value<quint32>(),
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
                  q.value(6).value<quint32>(),
                  0,
                  Utils::toUserTZ(c, q.value(7).toDateTime()),
                  Utils::toUserTZ(c, q.value(8).toDateTime()),
                  Utils::toUserTZ(c, q.value(9).toDateTime()),
                  _keepLocal);

        if (Q_LIKELY(imap.isLoggedIn())) {
            std::pair<quint32,quint32> quota = imap.getQuota(a.getUsername());
            a.setUsage(quota.first);
            a.setQuota(quota.second);
        }

        a.setHumanQuota(Utils::humanBinarySize(c, (quint64)a.getQuota() * 1024));
        a.setHumanUsage(Utils::humanBinarySize(c, (quint64)a.getUsage() * 1024));

        lst.push_back(a);
    }

    imap.logout();

    pag.insert(QStringLiteral("accounts"), QVariant::fromValue<std::vector<Account>>(lst));

    return pag;
}


Account Account::get(Cutelyst::Context *c, SkaffariError *e, const Domain &d, quint32 id)
{
    Account a;

    Q_ASSERT_X(c, "get account", "invalid context object");
    Q_ASSERT_X(e, "get account", "invalid error object");

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT au.id, au.username, au.imap, au.pop, au.sieve, au.smtpauth, au.quota, au.created_at, au.updated_at, au.valid_until FROM accountuser au WHERE id = :id"));
    q.bindValue(QStringLiteral(":id"), id);

    if (!q.exec()) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to query account from database."));
        return a;
    }

    if (!q.next()) {
        return a;
    }

    a.setId(q.value(0).value<quint32>());
    a.setUsername(q.value(1).toString());
    a.setImapEnabled(q.value(2).toBool());
    a.setPopEnabled(q.value(3).toBool());
    a.setSieveEnabled(q.value(4).toBool());
    a.setSmtpauthEnabled(q.value(5).toBool());
    a.setQuota(q.value(6).value<quint32>());
    a.setCreated(Utils::toUserTZ(c, q.value(7).toDateTime()));
    a.setUpdated(Utils::toUserTZ(c, q.value(8).toDateTime()));
    a.setValidUntil(Utils::toUserTZ(c, q.value(9).toDateTime()));
    a.setDomainId(d.id());
    a.setPrefix(d.getPrefix());
    a.setDomainName(d.getName());

    q = CPreparedSqlQueryThread(QStringLiteral("SELECT alias FROM virtual WHERE dest = :username AND username = :username ORDER BY alias ASC"));
    q.bindValue(QStringLiteral(":username"), a.getUsername());
    q.exec();
    QStringList emailAddresses;
    while (q.next()) {
        emailAddresses << q.value(0).toString();
    }

    a.setAddresses(emailAddresses);

    q = CPreparedSqlQueryThread(QStringLiteral("SELECT dest FROM virtual WHERE alias = :username AND username = ''"));
    q.bindValue(QStringLiteral(":username"), a.getUsername());
    q.exec();
    QStringList aliases;
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

    a.setForwards(aliases);

    SkaffariIMAP imap(c);
    if (imap.login()) {
        std::pair<quint32,quint32> quota = imap.getQuota(a.getUsername());
        a.setUsage(quota.first);
        a.setQuota(quota.second);
        imap.logout();
    }

    a.setHumanQuota(Utils::humanBinarySize(c, (quint64)a.getQuota() * 1024));
    a.setHumanUsage(Utils::humanBinarySize(c, (quint64)a.getUsage() * 1024));

    return a;
}


void Account::toStash(Cutelyst::Context *c, const Domain &d, quint32 accountId)
{
    Q_ASSERT_X(c, "account to stash", "invalid context object");

    SkaffariError e(c);
    Account a = Account::get(c, &e, d, accountId);
    if (Q_LIKELY(a.isValid())) {
        c->stash({
                     {QStringLiteral("account"), QVariant::fromValue<Account>(a)},
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


Account Account::fromStash(Cutelyst::Context *c)
{
    Account a;
    a = c->stash(QStringLiteral("account")).value<Account>();
    return a;
}



bool Account::update(Cutelyst::Context *c, SkaffariError *e, Account *a, const Cutelyst::ParamsMultiMap &p)
{
    bool ret = false;

    Q_ASSERT_X(c, "update account", "invalid context object");
    Q_ASSERT_X(e, "update account", "invalid error object");
    Q_ASSERT_X(a, "update account", "invalid account object");

    const QString password = p.value(QStringLiteral("password"));
    QByteArray encPw;
    if (!password.isEmpty()) {
        Password pw(password);
        encPw = pw.encrypt(SkaffariConfig::accPwMethod(), SkaffariConfig::accPwAlgorithm(), SkaffariConfig::accPwRounds());
        if (Q_UNLIKELY(encPw.isEmpty())) {
            e->setErrorType(SkaffariError::ApplicationError);
            e->setErrorText(c->translate("Account", "Failed to encrypt password."));
            qCWarning(SK_ACCOUNT) << "Failed to encrypt password with method" << SkaffariConfig::accPwMethod() << ", algorithm" << SkaffariConfig::accPwAlgorithm() << "and" << SkaffariConfig::accPwRounds() << "rounds";
            return ret;
        }
    }

    const quint32 quota = p.value(QStringLiteral("quota")).toULong();

    if (quota != (quint32)a->getQuota()) {
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

    QDateTime validUntil = QDateTime::fromString(p.value(QStringLiteral("validUntil"), QStringLiteral("2998-12-31 23:59:59")), QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    QTimeZone userTz(Cutelyst::Session::value(c, QStringLiteral("tz"), SkaffariConfig::defTimezone()).toByteArray());
    if (userTz == QTimeZone::utc()) {
        validUntil.setTimeSpec(Qt::UTC);
    } else {
        validUntil.setTimeZone(userTz);
        validUntil = validUntil.toUTC();
    }
    const QDateTime currentTimeUtc = QDateTime::currentDateTimeUtc();

    QSqlQuery q;
    if (!password.isEmpty()) {
        q = CPreparedSqlQueryThread(QStringLiteral("UPDATE accountuser SET password = :password, quota = :quota, valid_until = :validUntil, updated_at = :updated_at WHERE id = :id"));
        q.bindValue(QStringLiteral(":password"), encPw);
    } else {
        q = CPreparedSqlQueryThread(QStringLiteral("UPDATE accountuser SET quota = :quota, valid_until = :validUntil, updated_at = :updated_at WHERE id = :id"));
    }
    q.bindValue(QStringLiteral(":quota"), quota);
    q.bindValue(QStringLiteral(":validUntil"), validUntil);
    q.bindValue(QStringLiteral(":id"), a->getId());
    q.bindValue(QStringLiteral(":updated_at"), currentTimeUtc);

    if (!q.exec()) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to update user account in database."));
        return ret;
    }

    q = CPreparedSqlQueryThread(QStringLiteral("UPDATE domain SET domainquotaused = (SELECT SUM(quota) FROM accountuser WHERE domain_id = :domain_id) WHERE id = :domain_id"));
    q.bindValue(QStringLiteral(":domain_id"), a->getDomainId());
    if (!q.exec()) {
        qCWarning(SK_ACCOUNT) << "Failed to update used domain quota for domain ID" << a->getDomainId();
        qCDebug(SK_ACCOUNT) << q.lastError().text();
    }

    a->setValidUntil(Utils::toUserTZ(c, validUntil));
    a->setQuota(quota);
    a->setUpdated(Utils::toUserTZ(c, currentTimeUtc));

    ret = true;

    return ret;
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

    if (!a->getAddresses().contains(address) && (address != oldAddress)) {

        QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT alias, dest, username FROM virtual WHERE alias = :address"));
        q.bindValue(QStringLiteral(":address"), address);

        if (Q_UNLIKELY(!q.exec())) {
            e->setSqlError(q.lastError(), c->translate("Account", "Failed to check if the new email address %1 is already in use by another account.").append(address));
            return ret;
        }

        if (Q_UNLIKELY(q.next())) {
            const QString user = q.value(2).toString();
            if (user != a->getUsername()) {
                e->setErrorType(SkaffariError::InputError);
                if (!user.isEmpty()) {
                    e->setErrorText(c->translate("Account", "The email address %1 is already in use by user %2.").arg(address, user));
                } else {
                    e->setErrorText(c->translate("Account", "The email address %1 is already in use for destination %2.").arg(address, q.value(1).toString()));
                }
                return ret;
            }
        }

        q = CPreparedSqlQueryThread(QStringLiteral("UPDATE virtual SET alias = :aliasnew WHERE alias = :aliasold AND username = :username"));
        q.bindValue(QStringLiteral(":aliasnew"), address);
        q.bindValue(QStringLiteral(":aliasold"), oldAddress);
        q.bindValue(QStringLiteral(":username"), a->getUsername());

        if (Q_UNLIKELY(!q.exec())) {
            e->setSqlError(q.lastError(), c->translate("Account", "Failed to update email address %1.").arg(oldAddress));
            return ret;
        }

        QStringList addresses = a->getAddresses();
        addresses.removeOne(oldAddress);
        addresses.append(address);
        a->setAddresses(addresses);
    }

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
    q.bindValue(QStringLiteral(":address"), address);

    if (Q_UNLIKELY(!q.exec())) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to check if the new email address %1 is already in use by another account.").append(address));
        return ret;
    }

    if (Q_UNLIKELY(q.next())) {
        const QString user = q.value(2).toString();
        if (user != a->getUsername()) {
            e->setErrorType(SkaffariError::InputError);
            if (!user.isEmpty()) {
                e->setErrorText(c->translate("Account", "The email address %1 is already in use by user %2.").arg(address, user));
            } else {
                e->setErrorText(c->translate("Account", "The email address %1 is already in use for destination %2.").arg(address, q.value(1).toString()));
            }
            return ret;
        }
    }

    q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO virtual (alias, dest, username, status) VALUES (:alias, :dest, :username, :status)"));
    q.bindValue(QStringLiteral(":alias"), address);
    q.bindValue(QStringLiteral(":dest"), a->getUsername());
    q.bindValue(QStringLiteral(":username"), a->getUsername());
    q.bindValue(QStringLiteral(":status"), 1);

    if (!q.exec()) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to insert new email address into database."));
        return ret;
    }

    QStringList addresses = a->getAddresses();
    addresses.append(address);
    a->setAddresses(addresses);

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
    q.bindValue(QStringLiteral(":address"), address);

    if (!q.exec()) {
        e->setSqlError(q.lastError(), c->translate("Account", "Failed to remove email address %1 from account %2.").arg(address, a->getUsername()));
        return ret;
    }

    QStringList addresses = a->getAddresses();
    addresses.removeOne(address);
    a->setAddresses(addresses);

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

    if (!q.exec()) {
        e->setSqlError(q.lastError(), c->translate("Account", "Faild to update forwards for account %1 in database.").arg(a->getUsername()));
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
            return ret;
        }

        if (_keepLocal) {
            checkedForwards.removeLast();
        }

        a->setForwards(checkedForwards);
        a->setKeepLocal(_keepLocal);

    } else {
        a->setForwards(QStringList());
        a->setKeepLocal(false);
    }

    ret = true;

    return ret;
}

