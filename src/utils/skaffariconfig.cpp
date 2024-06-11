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

#include "skaffariconfig.h"
#include "../common/config.h"
#include <Cutelyst/Plugins/Utils/Sql>
#include <Cutelyst/Plugins/Memcached/Memcached>
#include <QSqlQuery>
#include <QSqlError>
#include <QGlobalStatic>
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>
#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
#include <pwquality.h>
#endif

#define MEMC_CONFIG_GROUP_KEY "options"
#define MEMC_CONFIG_STORAGE_DURATION 7200

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

    QString imapHost;
    QString imapUser;
    QString imapPassword;
    QString imapPeername;
    quint16 imapPort = 143;
    QAbstractSocket::NetworkLayerProtocol imapProtocol = static_cast<QAbstractSocket::NetworkLayerProtocol>(SK_DEF_IMAP_PROTOCOL);
    SkaffariIMAP::EncryptionType imapEncryption = static_cast<SkaffariIMAP::EncryptionType>(SK_DEF_IMAP_ENCRYPTION);
    Skaffari::Imap::EncryptionType imapEncryption2 = static_cast<Skaffari::Imap::EncryptionType>(SK_DEF_IMAP_ENCRYPTION);
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
    cfg->imapEncryption2 = static_cast<Skaffari::Imap::EncryptionType>(imap.value(QStringLiteral("encryption"), SK_DEF_IMAP_ENCRYPTION).toInt());
    cfg->imapCreatemailbox = static_cast<Account::CreateMailbox>(imap.value(QStringLiteral("createmailbox"), SK_DEF_IMAP_CREATEMAILBOX).value<quint8>());
    cfg->imapUnixhierarchysep = imap.value(QStringLiteral("unixhierarchysep"), SK_DEF_IMAP_UNIXHIERARCHYSEP).toBool();
    cfg->imapDomainasprefix = imap.value(QStringLiteral("domainasprefix"), SK_DEF_IMAP_DOMAINASPREFIX).toBool();
    cfg->imapFqun = imap.value(QStringLiteral("fqun"), SK_DEF_IMAP_FQUN).toBool();
    cfg->imapAuthMech = static_cast<SkaffariIMAP::AuthMech>(imap.value(QStringLiteral("authmech"), SK_DEF_IMAP_AUTHMECH).value<quint8>());

    cfg->tmplAsyncAccountList = tmpl.value(QStringLiteral("asyncaccountlist"), SK_DEF_TMPL_ASYNCACCOUNTLIST).toBool();
}

void SkaffariConfig::setDefaultsSettings(const QVariantHash &options)
{
    QWriteLocker locker(&cfg->lock);

    setDbOption<quota_size_t>(QStringLiteral(SK_CONF_KEY_DEF_DOMAINQUOTA), options.value(QStringLiteral(SK_CONF_KEY_DEF_DOMAINQUOTA), static_cast<quota_size_t>(SK_DEF_DEF_DOMAINQUOTA)).value<quota_size_t>());
    setDbOption<quota_size_t>(QStringLiteral(SK_CONF_KEY_DEF_QUOTA), options.value(QStringLiteral(SK_CONF_KEY_DEF_QUOTA), static_cast<quota_size_t>(SK_DEF_DEF_QUOTA)).value<quota_size_t>());
    setDbOption<quint32>(QStringLiteral(SK_CONF_KEY_DEF_MAXACCOUNTS), options.value(QStringLiteral(SK_CONF_KEY_DEF_MAXACCOUNTS), static_cast<quota_size_t>(SK_DEF_DEF_MAXACCOUNTS)).value<quint32>());
    setDbOption<QString>(QStringLiteral(SK_CONF_KEY_DEF_LANGUAGE), options.value(QStringLiteral(SK_CONF_KEY_DEF_LANGUAGE), QStringLiteral(SK_DEF_DEF_LANGUAGE)).toString());
    setDbOption<QString>(QStringLiteral(SK_CONF_KEY_DEF_TIMEZONE), options.value(QStringLiteral(SK_CONF_KEY_DEF_TIMEZONE), QStringLiteral(SK_DEF_DEF_TIMEZONE)).toString());
    setDbOption<quint32>(QStringLiteral(SK_CONF_KEY_DEF_MAXDISPLAY), options.value(QStringLiteral(SK_CONF_KEY_DEF_MAXDISPLAY), static_cast<quint8>(SK_DEF_DEF_MAXDISPLAY)).value<quint32>());
    setDbOption<quint32>(QStringLiteral(SK_CONF_KEY_DEF_WARNLEVEL), options.value(QStringLiteral(SK_CONF_KEY_DEF_WARNLEVEL), static_cast<quint8>(SK_DEF_DEF_WARNLEVEL)).value<quint32>());

    setDbOption<QString>(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_SENT), options.value(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_SENT)).toString());
    setDbOption<QString>(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_DRAFTS), options.value(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_DRAFTS)).toString());
    setDbOption<QString>(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_TRASH), options.value(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_TRASH)).toString());
    setDbOption<QString>(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_JUNK), options.value(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_JUNK)).toString());
    setDbOption<QString>(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_ARCHIVE), options.value(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_ARCHIVE)).toString());
    setDbOption<QString>(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_OTHERS), options.value(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_OTHERS)).toString());

    setDefaultAccount(QStringLiteral(SK_CONF_KEY_DEF_ABUSE_ACC), options.value(QStringLiteral(SK_CONF_KEY_DEF_ABUSE_ACC), 0).value<dbid_t>());
    setDefaultAccount(QStringLiteral(SK_CONF_KEY_DEF_NOC_ACC), options.value(QStringLiteral(SK_CONF_KEY_DEF_NOC_ACC), 0).value<dbid_t>());
    setDefaultAccount(QStringLiteral(SK_CONF_KEY_DEF_SECURITY_ACC), options.value(QStringLiteral(SK_CONF_KEY_DEF_SECURITY_ACC), 0).value<dbid_t>());
    setDefaultAccount(QStringLiteral(SK_CONF_KEY_DEF_POSTMASTER_ACC), options.value(QStringLiteral(SK_CONF_KEY_DEF_POSTMASTER_ACC), 0).value<dbid_t>());
    setDefaultAccount(QStringLiteral(SK_CONF_KEY_DEF_HOSTMASTER_ACC), options.value(QStringLiteral(SK_CONF_KEY_DEF_HOSTMASTER_ACC), 0).value<dbid_t>());
    setDefaultAccount(QStringLiteral(SK_CONF_KEY_DEF_WEBMASTER_ACC), options.value(QStringLiteral(SK_CONF_KEY_DEF_WEBMASTER_ACC), 0).value<dbid_t>());
}

QVariantHash SkaffariConfig::getDefaultsSettings()
{
    QReadLocker locker(&cfg->lock);
    QVariantHash s;
    s.reserve(19);

    s.insert(QStringLiteral(SK_CONF_KEY_DEF_DOMAINQUOTA), defDomainquota());
    s.insert(QStringLiteral(SK_CONF_KEY_DEF_QUOTA), defQuota());
    s.insert(QStringLiteral(SK_CONF_KEY_DEF_MAXACCOUNTS), defMaxaccounts());
    s.insert(QStringLiteral(SK_CONF_KEY_DEF_LANGUAGE), defLanguage());
    s.insert(QStringLiteral(SK_CONF_KEY_DEF_TIMEZONE), defTimezone());
    s.insert(QStringLiteral(SK_CONF_KEY_DEF_MAXDISPLAY), defMaxdisplay());
    s.insert(QStringLiteral(SK_CONF_KEY_DEF_WARNLEVEL), defWarnlevel());

    s.insert(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_SENT), defFolderSent());
    s.insert(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_DRAFTS), defFolderDrafts());
    s.insert(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_TRASH), defFolderTrash());
    s.insert(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_JUNK), defFolderJunk());
    s.insert(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_ARCHIVE), defFolderArchive());
    s.insert(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_OTHERS), defFolderOthers());

    s.insert(QStringLiteral(SK_CONF_KEY_DEF_ABUSE_ACC), QVariant::fromValue<SimpleAccount>(defAbuseAccount()));
    s.insert(QStringLiteral(SK_CONF_KEY_DEF_NOC_ACC), QVariant::fromValue<SimpleAccount>(defNocAccount()));
    s.insert(QStringLiteral(SK_CONF_KEY_DEF_SECURITY_ACC), QVariant::fromValue<SimpleAccount>(defSecurityAccount()));
    s.insert(QStringLiteral(SK_CONF_KEY_DEF_POSTMASTER_ACC), QVariant::fromValue<SimpleAccount>(defPostmasterAccount()));
    s.insert(QStringLiteral(SK_CONF_KEY_DEF_HOSTMASTER_ACC), QVariant::fromValue<SimpleAccount>(defHostmasterAccount()));
    s.insert(QStringLiteral(SK_CONF_KEY_DEF_WEBMASTER_ACC), QVariant::fromValue<SimpleAccount>(defWebmasterAccount()));

    return s;
}

void SkaffariConfig::setAutoconfigSettings(const QVariantHash &options)
{
    QWriteLocker locker(&cfg->lock);

    setDbOption<bool>(QStringLiteral(SK_CONF_KEY_AUTOCONF_ENABLED), options.value(QStringLiteral(SK_CONF_KEY_AUTOCONF_ENABLED)).toBool());
    setDbOption<QString>(QStringLiteral(SK_CONF_KEY_AUTOCONF_ID), options.value(QStringLiteral(SK_CONF_KEY_AUTOCONF_ID)).toString());
    setDbOption<QString>(QStringLiteral(SK_CONF_KEY_AUTOCONF_DISPLAY), options.value(QStringLiteral(SK_CONF_KEY_AUTOCONF_DISPLAY)).toString());
    setDbOption<QString>(QStringLiteral(SK_CONF_KEY_AUTOCONF_DISPLAY_SHORT), options.value(QStringLiteral(SK_CONF_KEY_AUTOCONF_DISPLAY_SHORT)).toString());
}

QVariantHash SkaffariConfig::getAutoconfigSettings()
{
    QReadLocker locker(&cfg->lock);
    QVariantHash s;
    s.reserve(4);

    s.insert(QStringLiteral(SK_CONF_KEY_AUTOCONF_ENABLED), autoconfigEnabled());
    s.insert(QStringLiteral(SK_CONF_KEY_AUTOCONF_ID), autoconfigId());
    s.insert(QStringLiteral(SK_CONF_KEY_AUTOCONF_DISPLAY), autoconfigDisplayName());
    s.insert(QStringLiteral(SK_CONF_KEY_AUTOCONF_DISPLAY_SHORT), autoconfigDisplayNameShort());

    return s;
}

QString SkaffariConfig::tmpl() { QReadLocker locker(&cfg->lock); return cfg->tmpl; }
QString SkaffariConfig::tmplBasePath() { QReadLocker locker(&cfg->lock); return cfg->tmplBasePath; }
QString SkaffariConfig::tmplPath(const QString &pathpart)
{
    QReadLocker locker(&cfg->lock);
    return tmplBasePath() + QLatin1Char('/') + pathpart;
}
QString SkaffariConfig::tmplPath(const QStringList &pathparts)
{
    QReadLocker locker(&cfg->lock);
    return tmplBasePath() + QLatin1Char('/') + pathparts.join(QLatin1Char('/'));
}
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

quota_size_t SkaffariConfig::defDomainquota() { QReadLocker locker(&cfg->lock); return getDbOption<quota_size_t>(QStringLiteral(SK_CONF_KEY_DEF_DOMAINQUOTA), static_cast<quota_size_t>(SK_DEF_DEF_DOMAINQUOTA)); }
quota_size_t SkaffariConfig::defQuota() { QReadLocker locker(&cfg->lock); return getDbOption<quota_size_t>(QStringLiteral(SK_CONF_KEY_DEF_QUOTA), static_cast<quota_size_t>(SK_DEF_DEF_QUOTA)); }
quint32 SkaffariConfig::defMaxaccounts() { QReadLocker locker(&cfg->lock); return getDbOption<quint32>(QStringLiteral(SK_CONF_KEY_DEF_MAXACCOUNTS), static_cast<quint32>(SK_DEF_DEF_MAXACCOUNTS)); }
QString SkaffariConfig::defLanguage() { QReadLocker locker(&cfg->lock); return getDbOption<QString>(QStringLiteral(SK_CONF_KEY_DEF_LANGUAGE), QStringLiteral(SK_DEF_DEF_LANGUAGE)); }
QString SkaffariConfig::defTimezone() { QReadLocker locker(&cfg->lock); return getDbOption<QString>(QStringLiteral(SK_CONF_KEY_DEF_TIMEZONE), QStringLiteral(SK_DEF_DEF_TIMEZONE)); }
quint8 SkaffariConfig::defMaxdisplay() { QReadLocker locker(&cfg->lock); return static_cast<quint8>(getDbOption<quint32>(QStringLiteral(SK_CONF_KEY_DEF_MAXDISPLAY), static_cast<quint32>(SK_DEF_DEF_MAXDISPLAY))); }
quint8 SkaffariConfig::defWarnlevel() { QReadLocker locker(&cfg->lock); return static_cast<quint8>(getDbOption<quint32>(QStringLiteral(SK_CONF_KEY_DEF_WARNLEVEL), static_cast<quint32>(SK_DEF_DEF_WARNLEVEL))); }

SimpleAccount SkaffariConfig::defAbuseAccount() { QReadLocker locker(&cfg->lock); return getDefaultAccount(QStringLiteral(SK_CONF_KEY_DEF_ABUSE_ACC)); }
SimpleAccount SkaffariConfig::defNocAccount() { QReadLocker locker(&cfg->lock); return getDefaultAccount(QStringLiteral(SK_CONF_KEY_DEF_NOC_ACC)); }
SimpleAccount SkaffariConfig::defSecurityAccount() { QReadLocker locker(&cfg->lock); return getDefaultAccount(QStringLiteral(SK_CONF_KEY_DEF_SECURITY_ACC)); }
SimpleAccount SkaffariConfig::defPostmasterAccount() { QReadLocker locker(&cfg->lock); return getDefaultAccount(QStringLiteral(SK_CONF_KEY_DEF_POSTMASTER_ACC)); }
SimpleAccount SkaffariConfig::defHostmasterAccount() { QReadLocker locker(&cfg->lock); return getDefaultAccount(QStringLiteral(SK_CONF_KEY_DEF_HOSTMASTER_ACC)); }
SimpleAccount SkaffariConfig::defWebmasterAccount() { QReadLocker locker(&cfg->lock);  return getDefaultAccount(QStringLiteral(SK_CONF_KEY_DEF_WEBMASTER_ACC)); }

QString SkaffariConfig::defFolderSent() { QReadLocker locker(&cfg->lock); return getDbOption<QString>(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_SENT), QString()); }
QString SkaffariConfig::defFolderDrafts() { QReadLocker locker(&cfg->lock); return getDbOption<QString>(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_DRAFTS), QString()); }
QString SkaffariConfig::defFolderTrash() { QReadLocker locker(&cfg->lock); return getDbOption<QString>(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_TRASH), QString()); }
QString SkaffariConfig::defFolderJunk() { QReadLocker locker(&cfg->lock); return getDbOption<QString>(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_JUNK), QString()); }
QString SkaffariConfig::defFolderArchive() { QReadLocker locker(&cfg->lock); return getDbOption<QString>(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_ARCHIVE), QString()); }
QString SkaffariConfig::defFolderOthers() { QReadLocker locker(&cfg->lock); return getDbOption<QString>(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_OTHERS), QString()); }

QString SkaffariConfig::imapHost() { QReadLocker locker(&cfg->lock); return cfg->imapHost; }
quint16 SkaffariConfig::imapPort() { QReadLocker locker(&cfg->lock); return cfg->imapPort; }
QString SkaffariConfig::imapUser() { QReadLocker locker(&cfg->lock); return cfg->imapUser; }
QString SkaffariConfig::imapPassword() { QReadLocker locker(&cfg->lock);  return cfg->imapPassword; }
QString SkaffariConfig::imapPeername() { QReadLocker locker(&cfg->lock); return cfg->imapPeername; }
QAbstractSocket::NetworkLayerProtocol SkaffariConfig::imapProtocol() { QReadLocker locker(&cfg->lock); return cfg->imapProtocol; }
SkaffariIMAP::EncryptionType SkaffariConfig::imapEncryption() { QReadLocker locker(&cfg->lock); return cfg->imapEncryption; }
Skaffari::Imap::EncryptionType SkaffariConfig::imapEncryption2() { QReadLocker locker(&cfg->lock); return cfg->imapEncryption2; }
Account::CreateMailbox SkaffariConfig::imapCreatemailbox() { QReadLocker locker(&cfg->lock); return cfg->imapCreatemailbox; }
bool SkaffariConfig::imapUnixhierarchysep() { QReadLocker locker(&cfg->lock); return cfg->imapUnixhierarchysep; }
bool SkaffariConfig::imapDomainasprefix() { QReadLocker locker(&cfg->lock); return cfg->imapDomainasprefix;}
bool SkaffariConfig::imapFqun() { QReadLocker locker(&cfg->lock); return cfg->imapUnixhierarchysep && cfg->imapDomainasprefix && cfg->imapFqun; }
SkaffariIMAP::AuthMech SkaffariConfig::imapAuthmech() { QReadLocker locker(&cfg->lock); return cfg->imapAuthMech; }

bool SkaffariConfig::autoconfigEnabled() { QReadLocker locker(&cfg->lock); return getDbOption<bool>(QStringLiteral(SK_CONF_KEY_AUTOCONF_ENABLED), false); }
QString SkaffariConfig::autoconfigId() { QReadLocker locker(&cfg->lock); return getDbOption<QString>(QStringLiteral(SK_CONF_KEY_AUTOCONF_ID), QString()); }
QString SkaffariConfig::autoconfigDisplayName() { QReadLocker locker(&cfg->lock); return getDbOption<QString>(QStringLiteral(SK_CONF_KEY_AUTOCONF_DISPLAY), QString()); }
QString SkaffariConfig::autoconfigDisplayNameShort() { QReadLocker locker(&cfg->lock); return getDbOption<QString>(QStringLiteral(SK_CONF_KEY_AUTOCONF_DISPLAY_SHORT), QString()); }

bool SkaffariConfig::tmplAsyncAccountList() { QReadLocker locker(&cfg->lock); return cfg->tmplAsyncAccountList; }

template< typename T >
T SkaffariConfig::getDbOption(const QString &option, const T &defVal)
{
    T retVal = defVal;

    if (cfg->useMemcached) {
        Cutelyst::Memcached::MemcachedReturnType rt;
        retVal = Cutelyst::Memcached::getByKey<T>(QStringLiteral(MEMC_CONFIG_GROUP_KEY), option, nullptr, &rt);
        if (rt == Cutelyst::Memcached::Success) {
            return retVal;
        }
    }

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT option_value FROM options WHERE option_name = :option_name"));
    q.bindValue(QStringLiteral(":option_name"), option);

    if (Q_LIKELY(q.exec())) {
        if (q.next()) {
            retVal = q.value(0).value<T>();
        } else {
            return defVal;
        }
    } else {
        qCCritical(SK_CONFIG, "Failed to query option %s from database, using default: %s", qUtf8Printable(option), qUtf8Printable(q.lastError().text()));
    }

    if (cfg->useMemcached) {
        Cutelyst::Memcached::setByKey<T>(QStringLiteral(MEMC_CONFIG_GROUP_KEY), option, retVal, MEMC_CONFIG_STORAGE_DURATION);
    }

    return retVal;
}

template< typename T >
bool SkaffariConfig::setDbOption(const QString &option, const T &value)
{
    bool rv = false;

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO options (option_name, option_value) "
                                                         "VALUES (:option_name, :option_value) "
                                                         "ON DUPLICATE KEY UPDATE "
                                                         "option_value = :option_value"));
    q.bindValue(QStringLiteral(":option_name"), option);
    q.bindValue(QStringLiteral(":option_value"), QVariant::fromValue<T>(value));

    if (Q_UNLIKELY(!q.exec())) {
        qCCritical(SK_CONFIG) << "Failed to save value" << value << "for option" << option << "in database:" << q.lastError().text();
        return rv;
    }

    rv = true;

    if (cfg->useMemcached) {
        Cutelyst::Memcached::setByKey<T>(QStringLiteral(MEMC_CONFIG_GROUP_KEY), option, value, MEMC_CONFIG_STORAGE_DURATION);
    }

    return rv;
}

SimpleAccount SkaffariConfig::getDefaultAccount(const QString &optionName)
{
    SimpleAccount acc;

    if (cfg->useMemcached) {
        Cutelyst::Memcached::MemcachedReturnType rt;
        acc = Cutelyst::Memcached::getByKey<SimpleAccount>(QStringLiteral(MEMC_CONFIG_GROUP_KEY), optionName, nullptr, &rt);
        if (rt == Cutelyst::Memcached::Success) {
            return acc;
        }
    }

    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT a.id, a.username, d.domain_name FROM accountuser a LEFT JOIN options op ON op.option_value = a.id LEFT JOIN domain d ON a.domain_id = d.id WHERE op.option_name = :option_name"));
    q.bindValue(QStringLiteral(":option_name"), optionName);

    if (Q_LIKELY(q.exec())) {
        if (q.next()) {
            acc = SimpleAccount(q.value(0).value<dbid_t>(), q.value(1).toString(), q.value(2).toString());
        } else {
            return acc;
        }
    } else {
        qCCritical(SK_CONFIG, "Failed to query database for %s: %s", qUtf8Printable(optionName), qUtf8Printable(q.lastError().text()));
    }

    if (cfg->useMemcached) {
        Cutelyst::Memcached::setByKey<SimpleAccount>(QStringLiteral(MEMC_CONFIG_GROUP_KEY), optionName, acc, MEMC_CONFIG_STORAGE_DURATION);
    }

    return acc;
}

bool SkaffariConfig::setDefaultAccount(const QString &option, dbid_t accountId)
{
    bool rv = false;

    if (accountId > 0) {
        QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO options (option_name, option_value) "
                                                             "VALUES (:option_name, :option_value) "
                                                             "ON DUPLICATE KEY UPDATE "
                                                             "option_value = :option_value"));
        q.bindValue(QStringLiteral(":option_name"), option);
        q.bindValue(QStringLiteral(":option_value"), accountId);
        if (Q_UNLIKELY(!q.exec())) {
            qCCritical(SK_CONFIG) << "Failed to save value" << accountId << "for option" << option << "in database:" << q.lastError().text();
            return rv;
        }
    } else {
        QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("DELETE FROM options WHERE option_name = :option_name"));
        q.bindValue(QStringLiteral(":option_name"), option);
        if (Q_UNLIKELY(!q.exec())) {
            qCCritical(SK_CONFIG, "Failed to remove option %s from database: %s", qUtf8Printable(option), qUtf8Printable(q.lastError().text()));
            return rv;
        }
    }

    if (cfg->useMemcached) {
        if (accountId > 0) {
            SimpleAccount a;
            QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT a.id, a.username, d.domain_name FROM accountuser a LEFT JOIN domain d ON a.domain_id = d.id WHERE a.id = :id"));
            q.bindValue(QStringLiteral(":id"), accountId);

            if (Q_LIKELY(q.exec())) {
                if (q.next()) {
                    a = SimpleAccount(q.value(0).value<dbid_t>(), q.value(1).toString(), q.value(2).toString());
                }
            } else {
                qCCritical(SK_SIMPLEACCOUNT, "Failed to query account with ID %u from database: %s", accountId, qUtf8Printable(q.lastError().text()));
            }

            if (a.isValid()) {
                Cutelyst::Memcached::setByKey<SimpleAccount>(QStringLiteral(MEMC_CONFIG_GROUP_KEY), option, a, MEMC_CONFIG_STORAGE_DURATION);
            }
        } else {
            Cutelyst::Memcached::removeByKey(QStringLiteral(MEMC_CONFIG_GROUP_KEY), option);
        }
    }

    return rv;
}
