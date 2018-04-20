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

#include "skaffariconfig.h"
#include "../common/config.h"
#include <Cutelyst/Plugins/Utils/Sql>
#include <QSqlQuery>
#include <QSqlError>
#include <QGlobalStatic>
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>
#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
#include <pwquality.h>
#endif

Q_LOGGING_CATEGORY(SK_CONFIG, "skaffari.config")

struct ConfigValues
{
    mutable QReadWriteLock lock{QReadWriteLock::Recursive};

    Password::Method accPwMethod = static_cast<Password::Method>(SK_DEF_ACC_PWMETHOD);
    Password::Algorithm accPwAlgorithm = static_cast<Password::Algorithm>(SK_DEF_ACC_PWALGORITHM);
    quint32 accPwRounds = SK_DEF_ACC_PWROUNDS;
#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
    QString accPwSettingsFile;
    int accPwThreshold = SK_DEF_ACC_PWTHRESHOLD;
#else
    quint8 SkaffariConfig::m_accPwMinlength = SK_DEF_ACC_PWMINLENGTH;
#endif

    QCryptographicHash::Algorithm admPwAlgorithm = static_cast<QCryptographicHash::Algorithm>(SK_DEF_ADM_PWALGORITHM);
    quint32 admPwRounds = SK_DEF_ADM_PWROUNDS;
#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
    QString admPwSettingsFile;
    int admPwThreshold = SK_DEF_ADM_PWTHRESHOLD;
#else
    quint8 SkaffariConfig::m_admPwMinlength = SK_DEF_ADM_PWMINLENGTH;
#endif

    quota_size_t defDomainquota = SK_DEF_DEF_DOMAINQUOTA;
    quota_size_t defQuota = SK_DEF_DEF_QUOTA;
    quint32 defMaxaccounts = SK_DEF_DEF_MAXACCOUNTS;
    QString defLanguage = QLatin1String(SK_DEF_DEF_LANGUAGE);
    QByteArray defTimezone = QByteArrayLiteral(SK_DEF_DEF_TIMEZONE);
    quint8 defMaxdisplay = SK_DEF_DEF_MAXDISPLAY;
    quint8 defWarnlevel = SK_DEF_DEF_WARNLEVEL;
    SimpleAccount defAbuseAccount;
    SimpleAccount defNocAccount;
    SimpleAccount defPostmasterAccount;
    SimpleAccount defHostmasterAccount;
    SimpleAccount defWebmasterAccount;
    SimpleAccount defSecurityAccount;

    QString imapHost;
    QString imapUser;
    QString imapPassword;
    QString imapPeername;
    quint16 imapPort = 143;
    QAbstractSocket::NetworkLayerProtocol imapProtocol = static_cast<QAbstractSocket::NetworkLayerProtocol>(SK_DEF_IMAP_PROTOCOL);
    SkaffariIMAP::EncryptionType imapEncryption = static_cast<SkaffariIMAP::EncryptionType>(SK_DEF_IMAP_ENCRYPTION);
    SkaffariIMAP::AuthMech imapAuthMech = static_cast<SkaffariIMAP::AuthMech>(SK_DEF_IMAP_AUTHMECH);
    Account::CreateMailbox imapCreatemailbox = static_cast<Account::CreateMailbox>(SK_DEF_IMAP_CREATEMAILBOX);
    bool imapUnixhierarchysep = SK_DEF_IMAP_UNIXHIERARCHYSEP;
    bool imapDomainasprefix = SK_DEF_IMAP_DOMAINASPREFIX;
    bool imapFqun = SK_DEF_IMAP_FQUN;

    QString tmpl = QStringLiteral("default");
    QString tmplBasePath = QStringLiteral(SKAFFARI_TMPLDIR) + QLatin1String("/default");
    bool tmplAsyncAccountList = SK_DEF_TMPL_ASYNCACCOUNTLIST;

    bool useMemcached = false;
    bool useMemcachedSession = false;
};
Q_GLOBAL_STATIC(ConfigValues, cfg)

void SkaffariConfig::load(const QVariantMap &general, const QVariantMap &accounts, const QVariantMap &admins, const QVariantMap &imap, const QVariantMap &tmpl)
{
    QWriteLocker locker(&cfg->lock);

    cfg->tmpl = general.value(QStringLiteral("template"), QStringLiteral("default")).toString();
    cfg->useMemcached = general.value(QStringLiteral("usememcached"), false).toBool();
    cfg->useMemcachedSession = general.value(QStringLiteral("usememcachedsession"), false).toBool();

    cfg->accPwMethod = static_cast<Password::Method>(accounts.value(QStringLiteral("pwmethod"), SK_DEF_ACC_PWMETHOD).value<quint8>());
    cfg->accPwAlgorithm = static_cast<Password::Algorithm>(accounts.value(QStringLiteral("pwalgorithm"), SK_DEF_ACC_PWALGORITHM).value<quint8>());
    cfg->accPwRounds = accounts.value(QStringLiteral("pwrounds"), SK_DEF_ACC_PWROUNDS).value<quint32>();
#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
    cfg->accPwSettingsFile = accounts.value(QStringLiteral("pwsettingsfile")).toString();
    cfg->accPwThreshold = accounts.value(QStringLiteral("pwthreshold"), SK_DEF_ACC_PWTHRESHOLD).toInt();
#else
    cfg->accPwMinlength = accounts.value(QStringLiteral("pwminlength"), SK_DEF_ACC_PWMINLENGTH).value<quint8>();
#endif

    cfg->admPwAlgorithm = static_cast<QCryptographicHash::Algorithm>(admins.value(QStringLiteral("pwalgorithm"), SK_DEF_ADM_PWALGORITHM).value<quint8>());
    cfg->admPwRounds = admins.value(QStringLiteral("pwrounds"), SK_DEF_ADM_PWROUNDS).value<quint32>();
#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
    cfg->admPwSettingsFile = admins.value(QStringLiteral("pwsettingsfile")).toString();
    cfg->admPwThreshold = admins.value(QStringLiteral("pwthreshold"), SK_DEF_ADM_PWTHRESHOLD).toInt();
#else
    cfg->admPwMinlength = admins.value(QStringLiteral("pwminlength"), SK_DEF_ADM_PWMINLENGTH).value<quint8>();
#endif

    cfg->imapHost = imap.value(QStringLiteral("host")).toString();
    cfg->imapUser = imap.value(QStringLiteral("user")).toString();
    cfg->imapPassword = imap.value(QStringLiteral("password")).toString();
    cfg->imapPeername = imap.value(QStringLiteral("peername")).toString();
    cfg->imapPort = imap.value(QStringLiteral("port"), 143).value<quint16>();
    cfg->imapProtocol = static_cast<QAbstractSocket::NetworkLayerProtocol>(imap.value(QStringLiteral("protocol"), SK_DEF_IMAP_PROTOCOL).value<quint8>());
    cfg->imapEncryption = static_cast<SkaffariIMAP::EncryptionType>(imap.value(QStringLiteral("encryption"), SK_DEF_IMAP_ENCRYPTION).value<quint8>());
    cfg->imapCreatemailbox = static_cast<Account::CreateMailbox>(imap.value(QStringLiteral("createmailbox"), SK_DEF_IMAP_CREATEMAILBOX).value<quint8>());
    cfg->imapUnixhierarchysep = imap.value(QStringLiteral("unixhierarchysep"), SK_DEF_IMAP_UNIXHIERARCHYSEP).toBool();
    cfg->imapDomainasprefix = imap.value(QStringLiteral("domainasprefix"), SK_DEF_IMAP_DOMAINASPREFIX).toBool();
    cfg->imapFqun = imap.value(QStringLiteral("fqun"), SK_DEF_IMAP_FQUN).toBool();
    cfg->imapAuthMech = static_cast<SkaffariIMAP::AuthMech>(imap.value(QStringLiteral("authmech"), SK_DEF_IMAP_AUTHMECH).value<quint8>());

    cfg->tmplAsyncAccountList = tmpl.value(QStringLiteral("asyncaccountlist"), SK_DEF_TMPL_ASYNCACCOUNTLIST).toBool();
}

void SkaffariConfig::loadSettingsFromDB()
{
    QWriteLocker locker(&cfg->lock);

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT option_value FROM options WHERE option_name = :option_name"));

    cfg->defDomainquota = loadDbOption(q, QStringLiteral(SK_CONF_KEY_DEF_DOMAINQUOTA), SK_DEF_DEF_DOMAINQUOTA).value<quota_size_t>();
    cfg->defQuota = loadDbOption(q, QStringLiteral(SK_CONF_KEY_DEF_QUOTA), SK_DEF_DEF_QUOTA).value<quota_size_t>();
    cfg->defMaxaccounts = loadDbOption(q, QStringLiteral(SK_CONF_KEY_DEF_MAXACCOUNTS), SK_DEF_DEF_MAXACCOUNTS).value<quint32>();
    cfg->defLanguage = loadDbOption(q, QStringLiteral(SK_CONF_KEY_DEF_LANGUAGE), QLatin1String(SK_DEF_DEF_LANGUAGE)).toString();
    cfg->defTimezone = loadDbOption(q, QStringLiteral(SK_CONF_KEY_DEF_TIMEZONE), QLatin1String(SK_DEF_DEF_TIMEZONE)).toByteArray();
    cfg->defMaxdisplay = loadDbOption(q, QStringLiteral(SK_CONF_KEY_DEF_MAXDISPLAY), SK_DEF_DEF_MAXDISPLAY).value<quint8>();
    cfg->defWarnlevel = loadDbOption(q, QStringLiteral(SK_CONF_KEY_DEF_WARNLEVEL), SK_DEF_DEF_WARNLEVEL).value<quint8>();

    cfg->defAbuseAccount = loadDefaultAccount(QStringLiteral(SK_CONF_KEY_DEF_ABUSE_ACC));
    cfg->defNocAccount = loadDefaultAccount(QStringLiteral(SK_CONF_KEY_DEF_NOC_ACC));
    cfg->defSecurityAccount = loadDefaultAccount(QStringLiteral(SK_CONF_KEY_DEF_SECURITY_ACC));
    cfg->defPostmasterAccount = loadDefaultAccount(QStringLiteral(SK_CONF_KEY_DEF_POSTMASTER_ACC));
    cfg->defHostmasterAccount = loadDefaultAccount(QStringLiteral(SK_CONF_KEY_DEF_HOSTMASTER_ACC));
    cfg->defWebmasterAccount = loadDefaultAccount(QStringLiteral(SK_CONF_KEY_DEF_WEBMASTER_ACC));
}

void SkaffariConfig::saveSettingsToDB(const QVariantHash &options)
{
    QWriteLocker locker(&cfg->lock);

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO options (option_name, option_value) "
                                                         "VALUES (:option_name, :option_value) "
                                                         "ON DUPLICATE KEY UPDATE "
                                                         "option_value = :option_value"));
    if (!options.empty()) {
        auto i = options.constBegin();
        while (i != options.constEnd()) {
            q.bindValue(QStringLiteral(":option_name"), i.key());
            q.bindValue(QStringLiteral(":option_value"), i.value());
            if (Q_UNLIKELY(!q.exec())) {
                qCWarning(SK_CONFIG, "Failed to save value %s for option %s in database: %s", qUtf8Printable(i.value().toString()), qUtf8Printable(i.key()), qUtf8Printable(q.lastError().text()));
            }
            ++i;
        }
    }

    cfg->defDomainquota = options.value(QStringLiteral(SK_CONF_KEY_DEF_DOMAINQUOTA), cfg->defDomainquota).value<quota_size_t>();
    cfg->defQuota = options.value(QStringLiteral(SK_CONF_KEY_DEF_QUOTA), cfg->defQuota).value<quota_size_t>();
    cfg->defMaxaccounts = options.value(QStringLiteral(SK_CONF_KEY_DEF_MAXACCOUNTS), cfg->defMaxaccounts).value<quint32>();
    cfg->defLanguage = options.value(QStringLiteral(SK_CONF_KEY_DEF_LANGUAGE), cfg->defLanguage).toString();
    cfg->defTimezone = options.value(QStringLiteral(SK_CONF_KEY_DEF_TIMEZONE), cfg->defTimezone).toByteArray();
    cfg->defMaxdisplay = options.value(QStringLiteral(SK_CONF_KEY_DEF_MAXDISPLAY), cfg->defMaxdisplay).value<quint8>();
    cfg->defWarnlevel = options.value(QStringLiteral(SK_CONF_KEY_DEF_WARNLEVEL), cfg->defWarnlevel).value<quint8>();

    cfg->defAbuseAccount = loadDefaultAccount(options.value(QStringLiteral(SK_CONF_KEY_DEF_ABUSE_ACC), cfg->defAbuseAccount.id()).value<dbid_t>());
    cfg->defNocAccount = loadDefaultAccount(options.value(QStringLiteral(SK_CONF_KEY_DEF_NOC_ACC), cfg->defNocAccount.id()).value<dbid_t>());
    cfg->defSecurityAccount = loadDefaultAccount(options.value(QStringLiteral(SK_CONF_KEY_DEF_SECURITY_ACC), cfg->defSecurityAccount.id()).value<dbid_t>());
    cfg->defPostmasterAccount = loadDefaultAccount(options.value(QStringLiteral(SK_CONF_KEY_DEF_POSTMASTER_ACC), cfg->defPostmasterAccount.id()).value<dbid_t>());
    cfg->defHostmasterAccount = loadDefaultAccount(options.value(QStringLiteral(SK_CONF_KEY_DEF_HOSTMASTER_ACC), cfg->defHostmasterAccount.id()).value<dbid_t>());
    cfg->defWebmasterAccount = loadDefaultAccount(options.value(QStringLiteral(SK_CONF_KEY_DEF_WEBMASTER_ACC), cfg->defWebmasterAccount.id()).value<dbid_t>());
}

QVariantHash SkaffariConfig::getSettingsFromDB()
{
    QReadLocker locker(&cfg->lock);
    QVariantHash s;
    s.reserve(13);

    s.insert(QStringLiteral(SK_CONF_KEY_DEF_DOMAINQUOTA), cfg->defDomainquota);
    s.insert(QStringLiteral(SK_CONF_KEY_DEF_QUOTA), cfg->defQuota);
    s.insert(QStringLiteral(SK_CONF_KEY_DEF_MAXACCOUNTS), cfg->defMaxaccounts);
    s.insert(QStringLiteral(SK_CONF_KEY_DEF_LANGUAGE), cfg->defLanguage);
    s.insert(QStringLiteral(SK_CONF_KEY_DEF_TIMEZONE), cfg->defTimezone);
    s.insert(QStringLiteral(SK_CONF_KEY_DEF_MAXDISPLAY), cfg->defMaxdisplay);
    s.insert(QStringLiteral(SK_CONF_KEY_DEF_WARNLEVEL), cfg->defWarnlevel);

    s.insert(QStringLiteral(SK_CONF_KEY_DEF_ABUSE_ACC), QVariant::fromValue<SimpleAccount>(cfg->defAbuseAccount));
    s.insert(QStringLiteral(SK_CONF_KEY_DEF_NOC_ACC), QVariant::fromValue<SimpleAccount>(cfg->defNocAccount));
    s.insert(QStringLiteral(SK_CONF_KEY_DEF_SECURITY_ACC), QVariant::fromValue<SimpleAccount>(cfg->defSecurityAccount));
    s.insert(QStringLiteral(SK_CONF_KEY_DEF_POSTMASTER_ACC), QVariant::fromValue<SimpleAccount>(cfg->defPostmasterAccount));
    s.insert(QStringLiteral(SK_CONF_KEY_DEF_HOSTMASTER_ACC), QVariant::fromValue<SimpleAccount>(cfg->defHostmasterAccount));
    s.insert(QStringLiteral(SK_CONF_KEY_DEF_WEBMASTER_ACC), QVariant::fromValue<SimpleAccount>(cfg->defWebmasterAccount));

    return s;
}

QString SkaffariConfig::tmpl() { QReadLocker locker(&cfg->lock); return cfg->tmpl; }
QString SkaffariConfig::tmplBasePath() { QReadLocker locker(&cfg->lock); return cfg->tmplBasePath; }
void SkaffariConfig::setTmplBasePath(const QString &path) { QWriteLocker locker(&cfg->lock); cfg->tmplBasePath = path; }
bool SkaffariConfig::useMemcached() { QReadLocker locker(&cfg->lock); return cfg->useMemcached; }
bool SkaffariConfig::useMemcachedSession() { QReadLocker locker(&cfg->lock); return cfg->useMemcachedSession; }

Password::Method SkaffariConfig::accPwMethod() { QReadLocker locker(&cfg->lock); return cfg->accPwMethod; }
Password::Algorithm SkaffariConfig::accPwAlgorithm() { QReadLocker locker(&cfg->lock); return cfg->accPwAlgorithm; }
quint32 SkaffariConfig::accPwRounds() { QReadLocker locker(&cfg->lock); return cfg->accPwRounds; }
quint8 SkaffariConfig::accPwMinlength()
{
    QReadLocker locker(&cfg->lock);
#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
    pwquality_settings_t *pwq;
    pwq = pwquality_default_settings();
    if (!cfg->accPwSettingsFile.isEmpty()) {
        if (pwquality_read_config(pwq, cfg->accPwSettingsFile.toUtf8().constData(), nullptr) != 0) {
            pwquality_read_config(pwq, nullptr, nullptr);
        }
    } else {
        pwquality_read_config(pwq, nullptr, nullptr);
    }
    int minLen = SK_DEF_ACC_PWMINLENGTH;
    if (pwquality_get_int_value(pwq, PWQ_SETTING_MIN_LENGTH, &minLen) != 0) {
        minLen = SK_DEF_ACC_PWMINLENGTH;
    }
    pwquality_free_settings(pwq);
    return static_cast<quint8>(minLen);
#else
    return cfg->accPwMinlength;
#endif
}
#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
QString SkaffariConfig::accPwSettingsFile() { QReadLocker locker(&cfg->lock); return cfg->accPwSettingsFile; }
int SkaffariConfig::accPwThreshold() { QReadLocker locker(&cfg->lock); return cfg->accPwThreshold; }
#endif

QCryptographicHash::Algorithm SkaffariConfig::admPwAlgorithm() { QReadLocker locker(&cfg->lock); return cfg->admPwAlgorithm; }
quint32 SkaffariConfig::admPwRounds() { QReadLocker locker(&cfg->lock); return cfg->admPwRounds; }
quint8 SkaffariConfig::admPwMinlength()
{
    QReadLocker locker(&cfg->lock);
#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
    pwquality_settings_t *pwq;
    pwq = pwquality_default_settings();
    if (!cfg->admPwSettingsFile.isEmpty()) {
        if (pwquality_read_config(pwq, cfg->admPwSettingsFile.toUtf8().constData(), nullptr) != 0) {
            pwquality_read_config(pwq, nullptr, nullptr);
        }
    } else {
        pwquality_read_config(pwq, nullptr, nullptr);
    }
    int minLen = SK_DEF_ADM_PWMINLENGTH;
    if (pwquality_get_int_value(pwq, PWQ_SETTING_MIN_LENGTH, &minLen) != 0) {
        minLen = SK_DEF_ADM_PWMINLENGTH;
    }
    pwquality_free_settings(pwq);
    return static_cast<quint8>(minLen);
#else
    return cfg->admPwMinlength;
#endif
}
#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
QString SkaffariConfig::admPwSettingsFile() { QReadLocker locker(&cfg->lock); return cfg->admPwSettingsFile; }
int SkaffariConfig::admPwThreshold() { QReadLocker locker(&cfg->lock); return cfg->admPwThreshold; }
#endif

quota_size_t SkaffariConfig::defDomainquota() { QReadLocker locker(&cfg->lock); return cfg->defDomainquota; }
quota_size_t SkaffariConfig::defQuota() { QReadLocker locker(&cfg->lock); return cfg->defQuota; }
quint32 SkaffariConfig::defMaxaccounts() { QReadLocker locker(&cfg->lock); return cfg->defMaxaccounts; }
QString SkaffariConfig::defLanguage() { QReadLocker locker(&cfg->lock); return cfg->defLanguage; }
QByteArray SkaffariConfig::defTimezone() { QReadLocker locker(&cfg->lock); return cfg->defTimezone; }
quint8 SkaffariConfig::defMaxdisplay() { QReadLocker locker(&cfg->lock); return cfg->defMaxdisplay; }
quint8 SkaffariConfig::defWarnlevel() { QReadLocker locker(&cfg->lock); return cfg->defWarnlevel; }
SimpleAccount SkaffariConfig::defAbuseAccount() { QReadLocker locker(&cfg->lock); return cfg->defAbuseAccount; }
SimpleAccount SkaffariConfig::defNocAccount() { QReadLocker locker(&cfg->lock); return cfg->defNocAccount; }
SimpleAccount SkaffariConfig::defPostmasterAccount() { QReadLocker locker(&cfg->lock); return cfg->defPostmasterAccount; }
SimpleAccount SkaffariConfig::defHostmasterAccount() { QReadLocker locker(&cfg->lock); return cfg->defHostmasterAccount; }
SimpleAccount SkaffariConfig::defWebmasterAccount() { QReadLocker locker(&cfg->lock);  return cfg->defWebmasterAccount; }

QString SkaffariConfig::imapHost() { QReadLocker locker(&cfg->lock); return cfg->imapHost; }
quint16 SkaffariConfig::imapPort() { QReadLocker locker(&cfg->lock); return cfg->imapPort; }
QString SkaffariConfig::imapUser() { QReadLocker locker(&cfg->lock); return cfg->imapUser; }
QString SkaffariConfig::imapPassword() { QReadLocker locker(&cfg->lock);  return cfg->imapPassword; }
QString SkaffariConfig::imapPeername() { QReadLocker locker(&cfg->lock); return cfg->imapPeername; }
QAbstractSocket::NetworkLayerProtocol SkaffariConfig::imapProtocol() { QReadLocker locker(&cfg->lock); return cfg->imapProtocol; }
SkaffariIMAP::EncryptionType SkaffariConfig::imapEncryption() { QReadLocker locker(&cfg->lock); return cfg->imapEncryption; }
Account::CreateMailbox SkaffariConfig::imapCreatemailbox() { QReadLocker locker(&cfg->lock); return cfg->imapCreatemailbox; }
bool SkaffariConfig::imapUnixhierarchysep() { QReadLocker locker(&cfg->lock); return cfg->imapUnixhierarchysep; }
bool SkaffariConfig::imapDomainasprefix() { QReadLocker locker(&cfg->lock); return cfg->imapDomainasprefix;}
bool SkaffariConfig::imapFqun() { QReadLocker locker(&cfg->lock); return cfg->imapUnixhierarchysep && cfg->imapDomainasprefix && cfg->imapFqun; }
SkaffariIMAP::AuthMech SkaffariConfig::imapAuthmech() { QReadLocker locker(&cfg->lock); return cfg->imapAuthMech; }

bool SkaffariConfig::tmplAsyncAccountList() { QReadLocker locker(&cfg->lock); return cfg->tmplAsyncAccountList; }

QVariant SkaffariConfig::loadDbOption(QSqlQuery &query, const QString &option, const QVariant &defVal)
{
    QVariant var;

    query.bindValue(QStringLiteral(":option_name"), option);

    if (Q_LIKELY(query.exec())) {
        if (query.next()) {
            var.setValue(query.value(0));
        } else {
            qCWarning(SK_CONFIG, "Can not find option %s in database, using default value %s.", qUtf8Printable(option), qUtf8Printable(defVal.toString()));
            var.setValue(defVal);
        }
    } else {
        qCWarning(SK_CONFIG, "Failed to query option %s from database, using default value %s: %s", qUtf8Printable(option), qUtf8Printable(defVal.toString()), qUtf8Printable(query.lastError().text()));
    }

    return var;
}

SimpleAccount SkaffariConfig::loadDefaultAccount(const QString &optionName)
{
    SimpleAccount acc;

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT a.id, a.username, d.domain_name FROM accountuser a LEFT JOIN options op ON op.option_value = a.id LEFT JOIN domain d ON a.domain_id = d.id WHERE op.option_name = :option_name"));
    q.bindValue(QStringLiteral(":option_name"), optionName);

    if (Q_LIKELY(q.exec())) {
        if (q.next()) {
            acc = SimpleAccount(q.value(0).value<dbid_t>(), q.value(1).toString(), q.value(2).toString());
        }
    } else {
        qCWarning(SK_CONFIG, "Failed to query database for %s: %s", qUtf8Printable(optionName), qUtf8Printable(q.lastError().text()));
    }

    return acc;
}


SimpleAccount SkaffariConfig::loadDefaultAccount(dbid_t accountId)
{
    SimpleAccount acc;

    if (accountId > 0) {
        QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT a.id, a.username, d.domain_name FROM accountuser a LEFT JOIN domain d ON a.domain_id = d.id WHERE a.id = :id"));
        q.bindValue(QStringLiteral(":id"), accountId);

        if (Q_LIKELY(q.exec())) {
            if (q.next()) {
                acc = SimpleAccount(q.value(0).value<dbid_t>(), q.value(1).toString(), q.value(2).toString());
            }
        } else {
            qCWarning(SK_CONFIG, "Failed to query database for accountID %u: %s", accountId, qUtf8Printable(q.lastError().text()));
        }
    }

    return acc;
}
