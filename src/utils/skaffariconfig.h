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

#ifndef SKAFFARICONFIG_H
#define SKAFFARICONFIG_H

#include <QCryptographicHash>
#include <QAbstractSocket>
#include <QLoggingCategory>
#include "../../common/password.h"
#include "../../common/global.h"
#include "../imap/skaffariimap.h"
#include "../objects/account.h"
#include "../objects/simpleaccount.h"

Q_DECLARE_LOGGING_CATEGORY(SK_CONFIG)

#define SK_CONF_KEY_DEF_DOMAINQUOTA "default_domainquota"
#define SK_CONF_KEY_DEF_QUOTA "default_quota"
#define SK_CONF_KEY_DEF_MAXACCOUNTS "default_maxaccounts"
#define SK_CONF_KEY_DEF_LANGUAGE "default_language"
#define SK_CONF_KEY_DEF_TIMEZONE "default_timezone"
#define SK_CONF_KEY_DEF_MAXDISPLAY "default_maxdisplay"
#define SK_CONF_KEY_DEF_WARNLEVEL "default_warnlevel"
#define SK_CONF_KEY_DEF_ABUSE_ACC "default_abuse_account"
#define SK_CONF_KEY_DEF_NOC_ACC "default_noc_account"
#define SK_CONF_KEY_DEF_SECURITY_ACC "default_security_account"
#define SK_CONF_KEY_DEF_POSTMASTER_ACC "default_postmaster_account"
#define SK_CONF_KEY_DEF_HOSTMASTER_ACC "default_hostmaster_account"
#define SK_CONF_KEY_DEF_WEBMASTER_ACC "default_webmaster_account"

class QSqlQuery;

/*!
 * \ingroup skaffaricore
 * \brief Static interface to access Skaffari and template settings in read only mode.
 *
 * This class contains the settings from the Skaffari configuration file as well as the settings
 * from the current template and specific settings stored in the database.
 *
 * All configuration values are saved static to be accessible globally.
 */
class SkaffariConfig
{
public:
    /*!
     * \brief Constructs a new SkaffariConfig object.
     */
    SkaffariConfig();

    /*!
     * \brief Loads the different configuration areas.
     * \param general   Entries from the \a General section.
     * \param accounts  Entries from the \a Accounts section.
     * \param admins    Entries from the \a Admins section.
     * \param defaults  Entries from the \a Defaults section.
     * \param imap      Entries from the \a IMAP section.
     * \param tmpl      Entrief from the current template.
     */
    static void load(const QVariantMap &general, const QVariantMap &accounts, const QVariantMap &admins, const QVariantMap &imap, const QVariantMap &tmpl);

    /*!
     * \brief Loads specific settings from the database options table.
     */
    static void loadSettingsFromDB();

    /*!
     * \brief Saves specific settings into the database options table.
     *
     * \par Currently supported keys
     * \li default_domainquota
     * \li default_quota
     * \li default_maxaccounts
     * \li default_language
     * \li default_timezone
     * \li default_maxdisplay
     * \li default_warnlevel
     */
    static void saveSettingsToDB(const QVariantHash &options);

    /*!
     * \brief Returns specific settings stored in the database.
     *
     * \par Currently supported keys
     * \li default_domainquota
     * \li default_quota
     * \li default_maxaccounts
     * \li default_language
     * \li default_timezone
     * \li default_maxdisplay
     * \li default_warnlevel
     */
    static QVariantHash getSettingsFromDB();

    /*!
     * \brief Password encryption method for the user accounts.
     *
     * The basic method to encrypt the user's password. Some methods support further settings that are defined in pwmethod.
     * If it is possible to use for you, the recommended type is to use the crypt(3) function, because it supports modern
     * hashing algorithms together with salts and an extensible storage format. The other encryption methods are there for
     * backwards compatibility.
     *
     * \par Config file key
     * Accounts/pwmethod
     */
    static Password::Method accPwMethod();
    /*!
     * \brief Password encryption algorithm for the user accounts.
     *
     * The MySQL and crypt(3) password encryption methods support different algorithms to derive a key from a password
     * string. To see which algorithms are supported by crypt(3) on your system, use man crypt. Especially the bcrypt
     * algorithm that uses Blowfish is not available on every system because it is not part of the default crypt(3) distribution.
     * The not recommended hashing methods are provided for backwards compatibility and if you have to store passwords for
     * use across different operating systems.
     *
     * \par Config file key
     * Accounts/pwalgorithm
     */
    static Password::Algorithm accPwAlgorithm();
    /*!
     * \brief Password encryption iterations for the user accounts.
     *
     * If you are using the crypt(3) to create passwords together with the SHA-256, SHA-512 or bcrypt algorithm, you can
     * specify an iteration count to increase the cost for deriving the key from the password. This hardens the password
     * against brute force attacks.
     *
     * For SHA-256 and SHA-512 you can choose values from 1000 to 999999999 - the default is 32000. The iteration count passed
     * to the crypt(3) function when using bcrypt is the base-2 logarithm of the acutal iteration count. Supported values for
     * bcrypt are between 4 and 32, the default is 12.
     *
     * \par Config file key
     * Accounts/pwrounds
     */
    static quint32 accPwRounds();
    /*!
     * \brief Minimum length for user account passwords.
     *
     * The required minimum length for user account passwords created or changed via Skaffari.
     *
     * \par Config file key
     * Accounts/pwminlength
     */
    static quint8 accPwMinlength();
#ifdef PWQUALITY_ENABLED
    static QString accPwSettingsFile();
    static int accPwThreshold();
#endif

    /*!
     * \brief Password encryption algorithm for admin accounts.
     *
     * Skaffari uses <A HREF="https://en.wikipedia.org/wiki/PBKDF2">PBKDF2</A> to secure the administrator passwords. PBKDF2 can use
     * different hashing algorithms and iteration counts to produce a derived key and to increase the cost for the derivation. To
     * better secure your administartor passwords you should use values that lead to a time consumption of around 0.5s on your system
     * for creating the derived key. This might be a good compromise between security and user experience. To test  different settings
     * with the PBKDF2 implementation of Cutelyst/Skaffari you can use <A HREF="https://github.com/Buschtrommel/pbkdf2test">pbkdf2test</A>.
     *
     * \par Config file key
     * Admins/pwalgorithm
     */
    static QCryptographicHash::Algorithm admPwAlgorithm();
    /*!
     * \brief Password encryption iterations for admin accounts.
     *
     * The iteration count used by the PBKDF2 implementation to increase the cost for the key derivation.
     *
     * \par Config file key
     * Admins/pwrounds
     */
    static quint32 admPwRounds();
    /*!
     * \brief Minimum length for admin account passwords.
     *
     * The required minimum length for adiminstrator account passwords.
     *
     * \par Config file key
     * Admins/pwminlength
     */
    static quint8 admPwMinlength();
#ifdef PWQUALITY_ENABLED
    static QString admPwSettingsFile();
    static int admPwThreshold();
#endif

    /*!
     * \brief The default domain quota for new domains.
     *
     * \par Database options table key
     * default_domainquota
     */
    static quota_size_t defDomainquota();
    /*!
     * \brief The default quota for new accounts
     *
     * \par Database options table key
     * default_quota
     */
    static quota_size_t defQuota();
    /*!
     * \brief The default maximum accounts value for new domains.
     *
     * \par Database options table key
     * default_maxaccounts
     */
    static quint32 defMaxaccounts();
    /*!
     * \brief The default language if user has no language set.
     *
     * \par Database options table key
     * default_language
     */
    static QString defLanguage();
    /*!
     * \brief The default time zone if user has no time zone set.
     *
     * \par Database options table key
     * default_timezone
     */
    static QByteArray defTimezone();
    /*!
     * \brief The default maximum display value for paginated lists.
     *
     * \par Database options table key
     * default_maxdisplay
     */
    static quint8 defMaxdisplay();
    /*!
     * \brief The default warn level for quota and account limits.
     *
     * \par Database options table key
     * default_warnlevel
     */
    static quint8 defWarnlevel();
    /*!
     * \brief The default account for the abuse role address.
     *
     * \par Database options table key
     * default_abuse_account
     */
    static SimpleAccount defAbuseAccount();
    /*!
     * \brief The default account for the NOC role address.
     *
     * \par Database options table key
     * default_noc_account
     */
    static SimpleAccount defNocAccount();
    /*!
     * \brief The default account for the security role address.
     */
    static SimpleAccount defSecurityAccount();
    /*!
     * \brief The default account for the postmaster role address.
     *
     * \par Database options table key
     * default_postmaster_account
     */
    static SimpleAccount defPostmasterAccount();
    /*!
     * \brief The default account for the hostmaster role address.
     *
     * \par Database options table key
     * default_hostmaster_account
     */
    static SimpleAccount defHostmasterAccount();
    /*!
     * \brief The default account for the webmaster role address.
     *
     * \par Database options table key
     * default_webmaster_account
     */
    static SimpleAccount defWebmasterAccount();

    /*!
     * \brief IMAP server address.
     *
     * The host the IMAP server is running on.
     *
     * \par Config file key
     * IMAP/host
     */
    static QString imapHost();
    /*!
     * \brief IMAP server port.
     *
     * The port the IMAP server is listening on.
     *
     * \par Config file key
     * IMAP/port
     */
    static quint16 imapPort();
    /*!
     * \brief IMAP admin user name.
     *
     * The user name of the IAMP admin user. This user has to be defined as administrator in the configuration of your IMAP
     * server. For Cyrus-IMAP this is one of the users defined in the admins: key in the imapd.conf(5) configuration file.
     *
     * \par Config file key
     * IMAP/user
     */
    static QString imapUser();
    /*!
     * \brief IMAP admin user password.
     *
     * Password for the IMAP server administrator.
     *
     * \par Config file key
     * IMAP/password
     */
    static QString imapPassword();
    /*!
     * \brief IMAP server peer name.
     *
     * If you use a different host name to connect to your IMAP server than the one used in the certificate of the IMAP server,
     * you can define this different peer name here. This can for example be used to establish an encrypted connection to an
     * IMAP server running on your local host.
     *
     * \par Config file key
     * IMAP/peername
     */
    static QString imapPeername();
    /*!
     * \brief IMAP network layer protocol.
     *
     * The network layer protocol to connect to the IMAP server.
     *
     * \par Config file key
     * IMAP/protocol
     */
    static QAbstractSocket::NetworkLayerProtocol imapProtocol();
    /*!
     * \brief IMAP connection encryption.
     *
     * The method to encrypt the connection to the IMAP server.
     *
     * \par Config file key
     * IMAP/encryption
     */
    static SkaffariIMAP::EncryptionType imapEncryption();
    /*!
     * \brief IMAP mailbox creation strategy.
     *
     * Skaffari can create the mailboxes and all default folders on the IMAP server after creating a new user account. Alternatively
     * the IMAP server can create default folders and account quotas on the first user login or first incoming email for the new account
     * (has to be configured in your imapd.conf file). Skaffari is more flexible on creating different default folders for different domains.
     *
     * \par Config file key
     * IMAP/createmailbox
     */
    static Account::CreateMailbox imapCreatemailbox();
    /*!
     * \brief The IMAP server uses the UNIX hierarchy separator.
     *
     * This setting should correspond to the value of the same setting in your imapd.conf(5) file and indicates that your imap server uses
     * the UNIX separator character '/' for delimiting levels of mailbox hierarchy instead of the netnews separator character '.'. Up to
     * Cyrus-IMAP 2.5.x the default value for this value in the IMAP server configuration is \c off, beginning with version 3.0.0 of Cyrus-IMAP
     * the default has changed to \c on.
     *
     * \par Config file key
     * IMAP/unixhierarchysep
     */
    static bool imapUnixhierarchysep();
    /*!
     * \brief Use dot separated email address like user names.
     *
     * If enabled, usernames will be composed from the email local part and the domain name, separated by a dot instead of an @ sign. Like
     * \c user.example.com. If you want to use real email addresses (fully qualified user names aka. fqun) like \c user@example.com as user
     * imapFqun() must return \c true, too.
     *
     * \note This will only return \c true, if imapUnixhierarchysep() returns \c true as well.
     *
     * \par Config file key
     * IMAP/domainasprefix
     */
    static bool imapDomainasprefix();
    /*!
     * \brief Use fully qualified user names.
     *
     * If enabled, usernames will be email addresses like john.doe@example.com.
     *
     * \note This will only return \c true, if imapUnixhierarchysep() and imapDomainasprefix() return \c true as well.
     */
    static bool imapFqun();

    /*!
     * \brief Returns the directory name of the template currently in use.
     */
    static QString tmpl();

    /*!
     * \brief Returns the absout base path to the template currently in use.
     */
    static QString tmplBasePath();

    /*!
     * \brief Sets the absolute base path to the template currently in use.
     */
    static void setTmplBasePath(const QString &path);

    /*!
     * \brief Returns \c true if the current template uses asynchronous/AJAX requests to load the list of accounts.
     */
    static bool tmplAsyncAccountList();

    /*!
     * \brief Returns \c true if Memcached should be used to cache database and IMAP queries.
     */
    static bool useMemcached();

    /*!
     * \brief Returns \c true if Memcached should be used to store session data for logged in users.
     */
    static bool useMemcachedSession();

#ifdef PWQUALITY_ENABLED
    static int pwqDifok();
#endif

private:
    static Password::Method m_accPwMethod;
    static Password::Algorithm m_accPwAlgorithm;
    static quint32 m_accPwRounds;
    static quint8 m_accPwMinlength;

    static QCryptographicHash::Algorithm m_admPwAlgorithm;
    static quint32 m_admPwRounds;
    static quint8 m_admPwMinlength;

    static quota_size_t m_defDomainquota;
    static quota_size_t m_defQuota;
    static quint32 m_defMaxaccounts;
    static QString m_defLanguage;
    static QByteArray m_defTimezone;
    static quint8 m_defMaxdisplay;
    static quint8 m_defWarnlevel;
    static SimpleAccount m_defAbuseAccount;
    static SimpleAccount m_defNocAccount;
    static SimpleAccount m_defSecurityAccount;
    static SimpleAccount m_defPostmasterAccount;
    static SimpleAccount m_defHostmasterAccount;
    static SimpleAccount m_defWebmasterAccount;

    static QString m_imapHost;
    static quint16 m_imapPort;
    static QString m_imapUser;
    static QString m_imapPassword;
    static QString m_imapPeername;
    static QAbstractSocket::NetworkLayerProtocol m_imapProtocol;
    static SkaffariIMAP::EncryptionType m_imapEncryption;
    static Account::CreateMailbox m_imapCreatemailbox;
    static bool m_imapUnixhierarchysep;
    static bool m_imapDomainasprefix;
    static bool m_imapFqun;

    static QString m_template;
    static QString m_tmplBasePath;
    static bool m_tmplAsyncAccountList;

    static bool m_useMemcached;
    static bool m_useMemcachedSession;

#ifdef PWQUALITY_ENABLED
    static QString m_accPwSettingsFile;
    static int m_accPwThreshold;
    static QString m_admPwSettingsFile;
    static int m_admPwThreshold;
#endif

    static QVariant loadDbOption(QSqlQuery &query, const QString &option, const QVariant &defVal = QVariant());
    static SimpleAccount loadDefaultAccount(const QString &optionName);
    static SimpleAccount loadDefaultAccount(dbid_t accountId);
};

#endif // SKAFFARICONFIG_H
