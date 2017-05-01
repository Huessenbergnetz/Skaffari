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

#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QSharedDataPointer>
#include <QString>
#include <QStringList>
#include <grantlee5/grantlee/metatype.h>
#include <Cutelyst/ParamsMultiMap>
#include <Cutelyst/Plugins/Utils/Pagination>
#include <QLoggingCategory>
#include <QDateTime>
#include <math.h>

Q_DECLARE_LOGGING_CATEGORY(SK_ACCOUNT)

namespace Cutelyst {
class Context;
}

class AccountData;
class SkaffariError;
class Domain;

/*!
 * \brief Represents an user account
 */
class Account
{
public:
    Account();
    Account(quint32 id, quint32 domainId, const QString &username, const QString &prefix, const QString &domainName, bool imap, bool pop, bool sieve, bool smtpauth, const QStringList &addresses, const QStringList &forwards, qint32 quota, qint32 usage, const QDateTime &created, const QDateTime &updated, const QDateTime &validUntil, bool keepLocal);
    Account(const Account &other);
    Account& operator=(const Account &other);
    ~Account();

    enum CreateMailbox : quint8 {
        DoNotCreate         = 0,
        LoginAfterCreation  = 1,
        OnlySetQuota        = 2,
        CreateBySkaffari    = 3
    };

    quint32 getId() const;
    quint32 getDomainId() const;
    QString getUsername() const;
    QString getPrefix() const;
    QString getDomainName() const;
    bool isImapEnabled() const;
    bool isPopEnabled() const;
    bool isSieveEnabled() const;
    bool isSmtpauthEnabled() const;
    QStringList getAddresses() const;
    QStringList getForwards() const;
	qint32 getQuota() const;
    QString getHumanQuota() const;
    qint32 getUsage() const;
    QString getHumanUsage() const;
	float getUsagePercent() const;
    bool isValid() const;
    QDateTime getCreated() const;
    QDateTime getUpdated() const;
    QDateTime getValidUntil() const;
    bool keepLocal() const;

    void setId(quint32 nId);
    void setDomainId(quint32 nDomainId);
    void setUsername(const QString &nUsername);
    void setPrefix(const QString &nPrefix);
    void setDomainName(const QString &nDomainName);
    void setImapEnabled(bool nImap);
    void setPopEnabled(bool nPop);
    void setSieveEnabled(bool nSieve);
    void setSmtpauthEnabled(bool nSmtpauth);
    void setAddresses(const QStringList &nAddresses);
    void setForwards(const QStringList &nForwards);
    void setQuota(qint32 nQuota);
    void setHumanQuota(const QString &humanQuota);
    void setUsage(qint32 nUsage);
    void setHumanUsage(const QString &humanUsage);
    void setCreated(const QDateTime &created);
    void setUpdated(const QDateTime &updated);
    void setValidUntil(const QDateTime &validUntil);
    void setKeepLocal(bool nKeepLocal);

    static Account create(Cutelyst::Context *c, SkaffariError *e, const Cutelyst::ParamsMultiMap &p, const Domain &d, quint8 pwType, quint8 pwMethod, quint32 pwRounds, const QVariantMap imapConfig);
    static bool remove(Cutelyst::Context *c, SkaffariError *e, const QString &username, Domain *domain);
    static bool remove(Cutelyst::Context *c, SkaffariError *e, Domain *d);
    static Cutelyst::Pagination list(Cutelyst::Context *c, SkaffariError *e, const Domain &d, const Cutelyst::Pagination &p, const QVariantMap &imapConfig, const QString &sortBy = QStringLiteral("username"), const QString &sortOrder = QStringLiteral("ASC"), const QString &searchRole = QStringLiteral("username"), const QString &searchString = QString());
    static Account get(Cutelyst::Context *c, SkaffariError *e, const Domain &d, quint32 id, const QVariantMap &imapConfig);
    static void toStash(Cutelyst::Context *c, const Domain &d, quint32 accountId, const QVariantMap &imapConfig);
    static Account fromStash(Cutelyst::Context *c);
    static bool update(Cutelyst::Context *c, SkaffariError *e, Account *a, const Cutelyst::ParamsMultiMap &p, quint8 pwType, quint8 pwMethod, quint32 pwRounds);
    static bool updateEmail(Cutelyst::Context *c, SkaffariError *e, Account *a, const Domain &d, const Cutelyst::ParamsMultiMap &p, const QString &oldAddress);
    static bool addEmail(Cutelyst::Context *c, SkaffariError *e, Account *a, const Domain &d, const Cutelyst::ParamsMultiMap &p);
    static bool removeEmail(Cutelyst::Context *c, SkaffariError *e, Account *a, const QString &address);
    static bool updateForwards(Cutelyst::Context *c, SkaffariError *e, Account *a, const Cutelyst::ParamsMultiMap &p);

protected:
    QSharedDataPointer<AccountData> d;
};

Q_DECLARE_METATYPE(Account)
Q_DECLARE_TYPEINFO(Account, Q_MOVABLE_TYPE);

GRANTLEE_BEGIN_LOOKUP(Account)
QVariant var;
if (property == QLatin1String("id")) {
    var.setValue(object.getId());
} else if (property == QLatin1String("domainId")) {
    var.setValue(object.getDomainId());
} else if (property == QLatin1String("username")) {
    var.setValue(object.getUsername());
} else if (property == QLatin1String("prefix")) {
    var.setValue(object.getPrefix());
} else if (property == QLatin1String("domainName")) {
    var.setValue(object.getDomainName());
} else if (property == QLatin1String("imap")) {
    var.setValue(object.isImapEnabled());
} else if (property == QLatin1String("pop")) {
    var.setValue(object.isPopEnabled());
} else if (property == QLatin1String("sieve")) {
    var.setValue(object.isSieveEnabled());
} else if (property == QLatin1String("smtpauth")) {
    var.setValue(object.isSmtpauthEnabled());
} else if (property == QLatin1String("addresses")) {
    var.setValue(object.getAddresses());
} else if (property == QLatin1String("forwards")) {
    var.setValue(object.getForwards());
} else if (property == QLatin1String("quota")) {
    var.setValue(object.getQuota());
} else if (property == QLatin1String("humanQuota")) {
    var.setValue(object.getHumanQuota());
} else if (property == QLatin1String("usage")) {
    var.setValue(object.getUsage());
} else if (property == QLatin1String("humanUsage")) {
    var.setValue(object.getHumanUsage());
} else if (property == QLatin1String("usagePercent")) {
    var.setValue(object.getUsagePercent());
} else if (property == QLatin1String("usagePercentFlat")) {
    var.setValue(lround(object.getUsagePercent()));
} else if (property == QLatin1String("isValid")) {
    var.setValue(object.isValid());
} else if (property == QLatin1String("created")) {
    var.setValue(object.getCreated());
} else if (property == QLatin1String("updated")) {
    var.setValue(object.getUpdated());
} else if (property == QLatin1String("validUntil")) {
    var.setValue(object.getValidUntil());
} else if (property == QLatin1String("validUntilString")) {
    var.setValue(object.getValidUntil().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")));
} else if (property == QLatin1String("keepLocal")) {
    var.setValue(object.keepLocal());
}
return var;
GRANTLEE_END_LOOKUP

#endif // ACCOUNT_H
