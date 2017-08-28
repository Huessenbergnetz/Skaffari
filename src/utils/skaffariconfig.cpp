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

Q_LOGGING_CATEGORY(SK_CONFIG, "skaffari.config")

Password::Method SkaffariConfig::m_accPwMethod = static_cast<Password::Method>(SK_DEF_ACC_PWMETHOD);
Password::Algorithm SkaffariConfig::m_accPwAlgorithm = static_cast<Password::Algorithm>(SK_DEF_ACC_PWALGORITHM);
quint32 SkaffariConfig::m_accPwRounds = SK_DEF_ACC_PWROUNDS;
quint8 SkaffariConfig::m_accPwMinlength = SK_DEF_ACC_PWMINLENGTH;

QCryptographicHash::Algorithm SkaffariConfig::m_admPwAlgorithm = static_cast<QCryptographicHash::Algorithm>(SK_DEF_ADM_PWALGORITHM);
quint32 SkaffariConfig::m_admPwRounds = SK_DEF_ADM_PWROUNDS;
quint8 SkaffariConfig::m_admPwMinlength = SK_DEF_ADM_PWMINLENGTH;

quota_size_t SkaffariConfig::m_defDomainquota = SK_DEF_DEF_DOMAINQUOTA;
quota_size_t SkaffariConfig::m_defQuota = SK_DEF_DEF_QUOTA;
quint32 SkaffariConfig::m_defMaxaccounts = SK_DEF_DEF_MAXACCOUNTS;
QString SkaffariConfig::m_defLanguage = QLatin1String(SK_DEF_DEF_LANGUAGE);
QByteArray SkaffariConfig::m_defTimezone = QByteArrayLiteral(SK_DEF_DEF_TIMEZONE);
quint8 SkaffariConfig::m_defMaxdisplay = SK_DEF_DEF_MAXDISPLAY;
quint8 SkaffariConfig::m_defWarnlevel = SK_DEF_DEF_WARNLEVEL;

QString SkaffariConfig::m_imapHost;
QString SkaffariConfig::m_imapUser;
QString SkaffariConfig::m_imapPassword;
QString SkaffariConfig::m_imapPeername;
quint16 SkaffariConfig::m_imapPort = 143;
QAbstractSocket::NetworkLayerProtocol SkaffariConfig::m_imapProtocol = static_cast<QAbstractSocket::NetworkLayerProtocol>(SK_DEF_IMAP_PROTOCOL);
SkaffariIMAP::EncryptionType SkaffariConfig::m_imapEncryption = static_cast<SkaffariIMAP::EncryptionType>(SK_DEF_IMAP_ENCRYPTION);
Account::CreateMailbox SkaffariConfig::m_imapCreatemailbox = static_cast<Account::CreateMailbox>(SK_DEF_IMAP_CREATEMAILBOX);
bool SkaffariConfig::m_imapUnixhierarchysep = SK_DEF_IMAP_UNIXHIERARCHYSEP;
bool SkaffariConfig::m_imapDomainasprefix = SK_DEF_IMAP_DOMAINASPREFIX;
bool SkaffariConfig::m_imapFqun = SK_DEF_IMAP_FQUN;

bool SkaffariConfig::m_tmplAsyncAccountList = SK_DEF_TMPL_ASYNCACCOUNTLIST;


SkaffariConfig::SkaffariConfig()
{

}

void SkaffariConfig::load(const QVariantMap &accounts, const QVariantMap &admins, const QVariantMap &imap, const QVariantMap &tmpl)
{
    SkaffariConfig::m_accPwMethod = static_cast<Password::Method>(accounts.value(QStringLiteral("pwmethod"), SK_DEF_ACC_PWMETHOD).value<quint8>());
    SkaffariConfig::m_accPwAlgorithm = static_cast<Password::Algorithm>(accounts.value(QStringLiteral("pwalgorithm"), SK_DEF_ACC_PWALGORITHM).value<quint8>());
    SkaffariConfig::m_accPwRounds = accounts.value(QStringLiteral("pwrounds"), SK_DEF_ACC_PWROUNDS).value<quint32>();
    SkaffariConfig::m_accPwMinlength = accounts.value(QStringLiteral("pwminlength"), SK_DEF_ACC_PWMINLENGTH).value<quint8>();

    SkaffariConfig::m_admPwAlgorithm = static_cast<QCryptographicHash::Algorithm>(admins.value(QStringLiteral("pwalgorithm"), SK_DEF_ADM_PWALGORITHM).value<quint8>());
    SkaffariConfig::m_admPwRounds = admins.value(QStringLiteral("pwrounds"), SK_DEF_ADM_PWROUNDS).value<quint32>();
    SkaffariConfig::m_admPwMinlength = admins.value(QStringLiteral("pwminlength"), SK_DEF_ADM_PWMINLENGTH).value<quint8>();

    SkaffariConfig::m_imapHost = imap.value(QStringLiteral("host")).toString();
    SkaffariConfig::m_imapUser = imap.value(QStringLiteral("user")).toString();
    SkaffariConfig::m_imapPassword = imap.value(QStringLiteral("password")).toString();
    SkaffariConfig::m_imapPeername = imap.value(QStringLiteral("peername")).toString();
    SkaffariConfig::m_imapPort = imap.value(QStringLiteral("port"), 143).value<quint16>();
    SkaffariConfig::m_imapProtocol = static_cast<QAbstractSocket::NetworkLayerProtocol>(imap.value(QStringLiteral("protocol"), SK_DEF_IMAP_PROTOCOL).value<quint8>());
    SkaffariConfig::m_imapEncryption = static_cast<SkaffariIMAP::EncryptionType>(imap.value(QStringLiteral("encryption"), SK_DEF_IMAP_ENCRYPTION).value<quint8>());
    SkaffariConfig::m_imapCreatemailbox = static_cast<Account::CreateMailbox>(imap.value(QStringLiteral("createmailbox"), SK_DEF_IMAP_CREATEMAILBOX).value<quint8>());
    SkaffariConfig::m_imapUnixhierarchysep = imap.value(QStringLiteral("unixhierarchysep"), SK_DEF_IMAP_UNIXHIERARCHYSEP).toBool();
    SkaffariConfig::m_imapDomainasprefix = imap.value(QStringLiteral("domainasprefix"), SK_DEF_IMAP_DOMAINASPREFIX).toBool();
    SkaffariConfig::m_imapFqun = imap.value(QStringLiteral("fqun"), SK_DEF_IMAP_FQUN).toBool();

    SkaffariConfig::m_tmplAsyncAccountList = tmpl.value(QStringLiteral("asyncaccountlist"), SK_DEF_TMPL_ASYNCACCOUNTLIST).toBool();
}

void SkaffariConfig::loadSettingsFromDB()
{
    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("SELECT option_value FROM options WHERE option_name = :option_name"));

    SkaffariConfig::m_defDomainquota = loadDbOption(q, QStringLiteral(SK_CONF_KEY_DEF_DOMAINQUOTA), SK_DEF_DEF_DOMAINQUOTA).value<quota_size_t>();
    SkaffariConfig::m_defQuota = loadDbOption(q, QStringLiteral(SK_CONF_KEY_DEF_QUOTA), SK_DEF_DEF_QUOTA).value<quota_size_t>();
    SkaffariConfig::m_defMaxaccounts = loadDbOption(q, QStringLiteral(SK_CONF_KEY_DEF_MAXACCOUNTS), SK_DEF_DEF_MAXACCOUNTS).value<quint32>();
    SkaffariConfig::m_defLanguage = loadDbOption(q, QStringLiteral(SK_CONF_KEY_DEF_LANGUAGE), QLatin1String(SK_DEF_DEF_LANGUAGE)).toString();
    SkaffariConfig::m_defTimezone = loadDbOption(q, QStringLiteral(SK_CONF_KEY_DEF_TIMEZONE), QLatin1String(SK_DEF_DEF_TIMEZONE)).toByteArray();
    SkaffariConfig::m_defMaxdisplay = loadDbOption(q, QStringLiteral(SK_CONF_KEY_DEF_MAXDISPLAY), SK_DEF_DEF_MAXDISPLAY).value<quint8>();
    SkaffariConfig::m_defWarnlevel = loadDbOption(q, QStringLiteral(SK_CONF_KEY_DEF_WARNLEVEL), SK_DEF_DEF_WARNLEVEL).value<quint8>();
}

void SkaffariConfig::saveSettingsToDB(const QVariantHash &options)
{
    QSqlQuery q = CPreparedSqlQueryThread(QStringLiteral("INSERT INTO options (option_name, option_value) "
                                                         "VALUES (:option_name, :option_value) "
                                                         "ON DUPLICATE KEY UPDATE "
                                                         "option_value = :option_value"));
    if (!options.empty()) {
        QVariantHash::const_iterator i = options.constBegin();
        while (i != options.constEnd()) {
            if (!i.value().isNull() && i.value().isValid()) {
                q.bindValue(QStringLiteral(":option_name"), i.key());
                q.bindValue(QStringLiteral(":option_value"), i.value());
                if (Q_UNLIKELY(!q.exec())) {
                    qCWarning(SK_CONFIG, "Failed to save value %s for option %s in database: %s", qUtf8Printable(i.value().toString()), qUtf8Printable(i.key()), qUtf8Printable(q.lastError().text()));
                }
            } else {
                qCWarning(SK_CONFIG, "Can not save invalid value for option %s.", qUtf8Printable(i.key()));
            }
            ++i;
        }
    }

    SkaffariConfig::m_defDomainquota = options.value(QStringLiteral(SK_CONF_KEY_DEF_DOMAINQUOTA), SkaffariConfig::m_defDomainquota).value<quota_size_t>();
    SkaffariConfig::m_defQuota = options.value(QStringLiteral(SK_CONF_KEY_DEF_QUOTA), SkaffariConfig::m_defQuota).value<quota_size_t>();
    SkaffariConfig::m_defMaxaccounts = options.value(QStringLiteral(SK_CONF_KEY_DEF_MAXACCOUNTS), SkaffariConfig::m_defMaxaccounts).value<quint32>();
    SkaffariConfig::m_defLanguage = options.value(QStringLiteral(SK_CONF_KEY_DEF_LANGUAGE), SkaffariConfig::m_defLanguage).toString();
    SkaffariConfig::m_defTimezone = options.value(QStringLiteral(SK_CONF_KEY_DEF_TIMEZONE), SkaffariConfig::m_defTimezone).toByteArray();
    SkaffariConfig::m_defMaxdisplay = options.value(QStringLiteral(SK_CONF_KEY_DEF_MAXDISPLAY), SkaffariConfig::m_defMaxdisplay).value<quint8>();
    SkaffariConfig::m_defWarnlevel = options.value(QStringLiteral(SK_CONF_KEY_DEF_WARNLEVEL), SkaffariConfig::m_defWarnlevel).value<quint8>();
}

QVariantHash SkaffariConfig::getSettingsFromDB()
{
    QVariantHash s;

    s.insert(QStringLiteral(SK_CONF_KEY_DEF_DOMAINQUOTA), SkaffariConfig::m_defDomainquota);
    s.insert(QStringLiteral(SK_CONF_KEY_DEF_QUOTA), SkaffariConfig::m_defQuota);
    s.insert(QStringLiteral(SK_CONF_KEY_DEF_MAXACCOUNTS), SkaffariConfig::m_defMaxaccounts);
    s.insert(QStringLiteral(SK_CONF_KEY_DEF_LANGUAGE), SkaffariConfig::m_defLanguage);
    s.insert(QStringLiteral(SK_CONF_KEY_DEF_TIMEZONE), SkaffariConfig::m_defTimezone);
    s.insert(QStringLiteral(SK_CONF_KEY_DEF_MAXDISPLAY), SkaffariConfig::m_defMaxdisplay);
    s.insert(QStringLiteral(SK_CONF_KEY_DEF_WARNLEVEL), SkaffariConfig::m_defWarnlevel);

    return s;
}

Password::Method SkaffariConfig::accPwMethod() { return m_accPwMethod; }
Password::Algorithm SkaffariConfig::accPwAlgorithm() { return m_accPwAlgorithm; }
quint32 SkaffariConfig::accPwRounds() { return m_accPwRounds; }
quint8 SkaffariConfig::accPwMinlength() { return m_accPwMinlength; }

QCryptographicHash::Algorithm SkaffariConfig::admPwAlgorithm() { return m_admPwAlgorithm; }
quint32 SkaffariConfig::admPwRounds() { return m_admPwRounds; }
quint8 SkaffariConfig::admPwMinlength() { return m_admPwMinlength; }

quota_size_t SkaffariConfig::defDomainquota() { return SkaffariConfig::m_defDomainquota; }
quota_size_t SkaffariConfig::defQuota() { return SkaffariConfig::m_defQuota; }
quint32 SkaffariConfig::defMaxaccounts() { return SkaffariConfig::m_defMaxaccounts; }
QString SkaffariConfig::defLanguage() { return SkaffariConfig::m_defLanguage; }
QByteArray SkaffariConfig::defTimezone() { return SkaffariConfig::m_defTimezone; }
quint8 SkaffariConfig::defMaxdisplay() { return SkaffariConfig::m_defMaxdisplay; }
quint8 SkaffariConfig::defWarnlevel() { return SkaffariConfig::m_defWarnlevel; }

QString SkaffariConfig::imapHost() { return SkaffariConfig::m_imapHost; }
quint16 SkaffariConfig::imapPort() { return SkaffariConfig::m_imapPort; }
QString SkaffariConfig::imapUser() { return SkaffariConfig::m_imapUser; }
QString SkaffariConfig::imapPassword() { return SkaffariConfig::m_imapPassword; }
QString SkaffariConfig::imapPeername() { return SkaffariConfig::m_imapPeername; }
QAbstractSocket::NetworkLayerProtocol SkaffariConfig::imapProtocol() { return SkaffariConfig::m_imapProtocol; }
SkaffariIMAP::EncryptionType SkaffariConfig::imapEncryption() { return SkaffariConfig::m_imapEncryption; }
Account::CreateMailbox SkaffariConfig::imapCreatemailbox() { return SkaffariConfig::m_imapCreatemailbox; }
bool SkaffariConfig::imapUnixhierarchysep() { return SkaffariConfig::m_imapUnixhierarchysep; }
bool SkaffariConfig::imapDomainasprefix() { return (SkaffariConfig::m_imapUnixhierarchysep && SkaffariConfig::m_imapDomainasprefix); }
bool SkaffariConfig::imapFqun() { return (SkaffariConfig::m_imapUnixhierarchysep && SkaffariConfig::m_imapDomainasprefix && SkaffariConfig::m_imapFqun); }

bool SkaffariConfig::tmplAsyncAccountList() { return SkaffariConfig::m_tmplAsyncAccountList; }

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
        qCCritical(SK_CONFIG, "Failed to query option %s from database, using default value %s: %s", qUtf8Printable(option), qUtf8Printable(defVal.toString()), qUtf8Printable(query.lastError().text()));
    }

    return var;
}
