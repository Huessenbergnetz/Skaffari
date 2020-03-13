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

#include "settingseditor.h"
#include "utils/utils.h"
#include "utils/skaffariconfig.h"
#include "objects/language.h"
#include "objects/helpentry.h"
#include "objects/autoconfigserver.h"
#include "objects/skaffarierror.h"
#include "validators/skvalidatoraccountexists.h"
#include "../common/global.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimeZone>
#include <Cutelyst/Plugins/Utils/Validator> // includes the main validator
#include <Cutelyst/Plugins/Utils/ValidatorResult> // includes the validator result
#include <Cutelyst/Plugins/Utils/validatorin.h>
#include <Cutelyst/Plugins/Utils/validatorinteger.h>
#include <Cutelyst/Plugins/Utils/validatorbetween.h>
#include <Cutelyst/Plugins/Utils/validatormin.h>
#include <Cutelyst/Plugins/Utils/validatorfilesize.h>
#include <Cutelyst/Plugins/Utils/validatorcharnotallowed.h>
#include <Cutelyst/Plugins/Utils/validatorboolean.h>
#include <Cutelyst/Plugins/Utils/validatorrequiredif.h>
#include <Cutelyst/Plugins/Utils/validatordomain.h>
#include <Cutelyst/Plugins/Utils/validatorrequired.h>
#include <Cutelyst/Plugins/StatusMessage>
#include <limits>

SettingsEditor::SettingsEditor(QObject *parent) : Controller(parent)
{

}

SettingsEditor::~SettingsEditor()
{

}

bool SettingsEditor::Auto(Context* c)
{
    if (AdminAccount::getUserType(c) < AdminAccount::Administrator) {
        c->res()->setStatus(403);
        c->detach(c->getAction(QStringLiteral("error")));
        return false;
    }

    return true;
}

void SettingsEditor::index(Context *c)
{
    QVariantHash settings = SkaffariConfig::getDefaultsSettings();

    if (c->req()->isPost()) {

        static QString forbiddenFolderChars = SkaffariConfig::imapUnixhierarchysep() ? QStringLiteral("%*#/") : QStringLiteral("%*#.");

        static Validator v({
                               new ValidatorIn(QStringLiteral(SK_CONF_KEY_DEF_LANGUAGE), Language::supportedLangsList()),
                               new ValidatorBetween(QStringLiteral(SK_CONF_KEY_DEF_WARNLEVEL), QMetaType::UInt, 0, 100),
                               new ValidatorBetween(QStringLiteral(SK_CONF_KEY_DEF_MAXDISPLAY), QMetaType::UInt, 15, 255),
                               new ValidatorMin(QStringLiteral(SK_CONF_KEY_DEF_MAXACCOUNTS), QMetaType::UInt, 0),
                               new ValidatorFileSize(QStringLiteral(SK_CONF_KEY_DEF_QUOTA), ValidatorFileSize::ForceBinary, 0, std::numeric_limits<quota_size_t>::max()),
                               new ValidatorFileSize(QStringLiteral(SK_CONF_KEY_DEF_DOMAINQUOTA), ValidatorFileSize::ForceBinary, 0, std::numeric_limits<quota_size_t>::max()),
                               new SkValidatorAccountExists(QStringLiteral(SK_CONF_KEY_DEF_ABUSE_ACC)),
                               new SkValidatorAccountExists(QStringLiteral(SK_CONF_KEY_DEF_NOC_ACC)),
                               new SkValidatorAccountExists(QStringLiteral(SK_CONF_KEY_DEF_SECURITY_ACC)),
                               new SkValidatorAccountExists(QStringLiteral(SK_CONF_KEY_DEF_POSTMASTER_ACC)),
                               new SkValidatorAccountExists(QStringLiteral(SK_CONF_KEY_DEF_HOSTMASTER_ACC)),
                               new SkValidatorAccountExists(QStringLiteral(SK_CONF_KEY_DEF_WEBMASTER_ACC)),
                               new ValidatorCharNotAllowed(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_SENT), forbiddenFolderChars),
                               new ValidatorCharNotAllowed(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_DRAFTS), forbiddenFolderChars),
                               new ValidatorCharNotAllowed(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_TRASH), forbiddenFolderChars),
                               new ValidatorCharNotAllowed(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_JUNK), forbiddenFolderChars),
                               new ValidatorCharNotAllowed(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_ARCHIVE), forbiddenFolderChars),
                               new ValidatorCharNotAllowed(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_OTHERS), forbiddenFolderChars)
                           });

        ValidatorResult vr = v.validate(c, Validator::FillStashOnError|Validator::BodyParamsOnly);
        if (vr) {
            vr.addValue(QStringLiteral(SK_CONF_KEY_DEF_TIMEZONE), c->req()->bodyParam(QStringLiteral(SK_CONF_KEY_DEF_TIMEZONE)));
            vr.addValue(QStringLiteral(SK_CONF_KEY_DEF_DOMAINQUOTA),
                        static_cast<quota_size_t>(vr.value(QStringLiteral(SK_CONF_KEY_DEF_DOMAINQUOTA)).value<quota_size_t>() / Q_UINT64_C(1024)));
            vr.addValue(QStringLiteral(SK_CONF_KEY_DEF_QUOTA),
                        static_cast<quota_size_t>(vr.value(QStringLiteral(SK_CONF_KEY_DEF_QUOTA)).value<quota_size_t>() / Q_UINT64_C(1024)));
            SkaffariConfig::setDefaultsSettings(vr.values());
            c->setStash(QStringLiteral("status_msg"), c->translate("SettingsEditor", "Settings successfully saved."));
            settings = SkaffariConfig::getDefaultsSettings();
        }
    }

    HelpHash help;
    help.reserve(19);
    help.insert(QStringLiteral(SK_CONF_KEY_DEF_LANGUAGE), HelpEntry(c->translate("SettingsEditor", "Language"), c->translate("SettingsEditor", "Default fallback language that will be used if user has no language set and if the language reported by the browser is not supported.")));
    help.insert(QStringLiteral(SK_CONF_KEY_DEF_TIMEZONE), HelpEntry(c->translate("SettingsEditor", "Time zone"), c->translate("SettingsEditor", "Default time zone as fallback that will be used to display localized dates and times if the user has not set one.")));
    help.insert(QStringLiteral(SK_CONF_KEY_DEF_WARNLEVEL), HelpEntry(c->translate("SettingsEditor", "Warn level"), c->translate("SettingsEditor", "Default warn level for account number and quota storage limits if the user has not set one.")));
    help.insert(QStringLiteral(SK_CONF_KEY_DEF_MAXDISPLAY), HelpEntry(c->translate("SettingsEditor", "Maximum display"), c->translate("SettingsEditor", "Default maximum display number of items per page for paginated result lists.")));
    help.insert(QStringLiteral(SK_CONF_KEY_DEF_MAXACCOUNTS), HelpEntry(c->translate("SettingsEditor", "Maximum accounts"), c->translate("SettingsEditor", "Default number of maximum accounts for new domains.")));
    help.insert(QStringLiteral(SK_CONF_KEY_DEF_DOMAINQUOTA), HelpEntry(c->translate("SettingsEditor", "Domain quota"), c->translate("SettingsEditor", "Default domain storage quota for new domains. You can use the multipliers K, KiB, M, MiB, G, GiB, etc.")));
    help.insert(QStringLiteral(SK_CONF_KEY_DEF_QUOTA), HelpEntry(c->translate("SettingsEditor", "Account quota"), c->translate("SettingsEditor", "Default storage quota for new user accounts. You can use the multipliers K, KiB, M, MiB, G, GiB, etc.")));
    help.insert(QStringLiteral(SK_CONF_KEY_DEF_ABUSE_ACC), HelpEntry(c->translate("SettingsEditor", "Abuse account"), c->translate("SettingsEditor", "Used as default account for abuse role email address when creating new domains.")));
    help.insert(QStringLiteral(SK_CONF_KEY_DEF_NOC_ACC), HelpEntry(c->translate("SettingsEditor", "NOC account"), c->translate("SettingsEditor", "Used as default account for NOC role email address when creating new domains.")));
    help.insert(QStringLiteral(SK_CONF_KEY_DEF_SECURITY_ACC), HelpEntry(c->translate("SettingsEditor", "Security account"), c->translate("SettingsEditor", "Used as default account for security role amail address when creating new domains.")));
    help.insert(QStringLiteral(SK_CONF_KEY_DEF_POSTMASTER_ACC), HelpEntry(c->translate("SettingsEditor", "Postmaster account"), c->translate("SettingsEditor", "Used as default account for postmaster role email address when creating new domains.")));
    help.insert(QStringLiteral(SK_CONF_KEY_DEF_HOSTMASTER_ACC), HelpEntry(c->translate("SettingsEditor", "Hostmaster account"), c->translate("SettingsEditor", "Used as default account for hostmaster role email address when creating new domains.")));
    help.insert(QStringLiteral(SK_CONF_KEY_DEF_WEBMASTER_ACC), HelpEntry(c->translate("SettingsEditor", "Webmaster account"), c->translate("SettingsEditor", "Used as default account for webmaster role email address when creating new domains.")));

    help.insert(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_SENT), HelpEntry(c->translate("SettingsEditor", "Sent messages"), c->translate("SettingsEditor", "Folder for sent messages.")));
    help.insert(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_DRAFTS), HelpEntry(c->translate("SettingsEditor", "Drafts"), c->translate("SettingsEditor", "Folder for message drafts.")));
    help.insert(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_TRASH), HelpEntry(c->translate("SettingsEditor", "Trash"), c->translate("SettingsEditor", "Folder for deleted messages.")));
    help.insert(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_JUNK), HelpEntry(c->translate("SettingsEditor", "Junk"), c->translate("SettingsEditor", "Folder for junk messages aka. spam.")));
    help.insert(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_ARCHIVE), HelpEntry(c->translate("SettingsEditor", "Archive"), c->translate("SettingsEditor", "Folder to archive messages.")));
    help.insert(QStringLiteral(SK_CONF_KEY_DEF_FOLDER_OTHERS), HelpEntry(c->translate("SettingsEditor", "Additional folders"), c->translate("SettingsEditor", "Comma-separated list of additional folders.")));

    static QStringList tzs = ([]() -> QStringList {
                                  QStringList _tzs;
                                  _tzs.reserve(QTimeZone::availableTimeZoneIds().size());
                                  for (const QByteArray &tz : QTimeZone::availableTimeZoneIds()) {
                                      _tzs.push_back(QString::fromLatin1(tz));
                                  }
                                  return _tzs;
                              }());

    c->stash(settings);
    c->stash({
                 {QStringLiteral("help"),       QVariant::fromValue<HelpHash>(help)},
                 {QStringLiteral("timezones"),  QVariant::fromValue<QStringList>(tzs)},
                 {QStringLiteral("langs"),      QVariant::fromValue<QVector<Language>>(Language::supportedLangs(c))},
                 {QStringLiteral("site_title"), c->translate("SettingsEditor", "Settings")},
                 {QStringLiteral("template"),   QStringLiteral("settings/index.html")}
             });
}

void SettingsEditor::autoconfig(Context *c)
{
    QVariantHash settings = SkaffariConfig::getAutoconfigSettings();

    if (c->req()->isPost()) {
        static Validator v({
                               new ValidatorBoolean(QStringLiteral(SK_CONF_KEY_AUTOCONF_ENABLED)),
                               new ValidatorRequiredIf(QStringLiteral(SK_CONF_KEY_AUTOCONF_ID), QStringLiteral(SK_CONF_KEY_AUTOCONF_ENABLED), {QStringLiteral("1"), QStringLiteral("on"), QStringLiteral("true")}),
                               new ValidatorDomain(QStringLiteral(SK_CONF_KEY_AUTOCONF_ID)),
                               new ValidatorRequiredIf(QStringLiteral(SK_CONF_KEY_AUTOCONF_DISPLAY), QStringLiteral(SK_CONF_KEY_AUTOCONF_ENABLED), {QStringLiteral("1"), QStringLiteral("on"), QStringLiteral("true")}),
                               new ValidatorRequiredIf(QStringLiteral(SK_CONF_KEY_AUTOCONF_DISPLAY_SHORT), QStringLiteral(SK_CONF_KEY_AUTOCONF_ENABLED), {QStringLiteral("1"), QStringLiteral("on"), QStringLiteral("true")})
                           });

        ValidatorResult vr = v.validate(c, Validator::FillStashOnError|Validator::BodyParamsOnly);
        if (vr) {
            SkaffariConfig::setAutoconfigSettings(vr.values());
            c->setStash(QStringLiteral("status_msg"), c->translate("SettingsEditor", "Settings successfully saved."));
            settings = SkaffariConfig::getAutoconfigSettings();
        }
    }

    HelpHash help;
    help.reserve(4);
    help.insert(QStringLiteral(SK_CONF_KEY_AUTOCONF_ENABLED), HelpEntry(c->translate("SettingsEditor", "Enable auto configuration"), c->translate("SettingsEditor", "If enabled, Skaffari provides access to URLs used for autoconfiguration of mail user agents.")));
    help.insert(QStringLiteral(SK_CONF_KEY_AUTOCONF_ID), HelpEntry(c->translate("SettingsEditor", "Provider ID"), c->translate("SettingsEditor", "This should be your main domain name.")));
    help.insert(QStringLiteral(SK_CONF_KEY_AUTOCONF_DISPLAY), HelpEntry(c->translate("SettingsEditor", "Provider display name"), c->translate("SettingsEditor", "The display name might be used by a mail user agent for automatically configured accounts.")));
    help.insert(QStringLiteral(SK_CONF_KEY_AUTOCONF_DISPLAY_SHORT), HelpEntry(c->translate("SettingsEditor", "Provider display short name"), c->translate("SettingsEditor", "The display short name might be used by a mail user agent for automatically configured accounts.")));

    SkaffariError listAutoconfigServersError(c);
    const std::vector<AutoconfigServer> autoconfigServers = AutoconfigServer::list(c, 0, listAutoconfigServersError);
    if (listAutoconfigServersError.type() != SkaffariError::NoError) {
        c->setStash(QStringLiteral("error_msg"), listAutoconfigServersError.errorText());
    }

    c->stash(settings);
    c->stash({
                 {QStringLiteral("autoconfigServers"), QVariant::fromValue<std::vector<AutoconfigServer>>(autoconfigServers)},
                 {QStringLiteral("help"),       QVariant::fromValue<HelpHash>(help)},
                 {QStringLiteral("site_title"), c->translate("SettingsEditor", "Settings")},
                 {QStringLiteral("template"),   QStringLiteral("settings/autoconfig.html")}
             });
}

void SettingsEditor::add_autoconfig_server(Context *c)
{
    if (c->req()->isPost()) {
        static Validator v({
                               new ValidatorRequired(QStringLiteral("type")),
                               new ValidatorBetween(QStringLiteral("type"), QMetaType::Char, static_cast<qint8>(AutoconfigServer::Imap), static_cast<qint8>(AutoconfigServer::Smtp)),
                               new ValidatorRequired(QStringLiteral("hostname")),
                               new ValidatorDomain(QStringLiteral("hostname")),
                               new ValidatorRequired(QStringLiteral("port")),
                               new ValidatorBetween(QStringLiteral("port"), QMetaType::UShort, 0, 65535),
                               new ValidatorRequired(QStringLiteral("socketType")),
                               new ValidatorBetween(QStringLiteral("socketType"), QMetaType::Char, static_cast<qint8>(AutoconfigServer::Plain), static_cast<qint8>(AutoconfigServer::Ssl)),
                               new ValidatorRequired(QStringLiteral("authentication")),
                               new ValidatorBetween(QStringLiteral("authentication"), QMetaType::Char, static_cast<qint8>(AutoconfigServer::Cleartext), static_cast<qint8>(AutoconfigServer::TlsClientCert)),
                               new ValidatorBetween(QStringLiteral("sorting"), QMetaType::Char, std::numeric_limits<qint8>::min(), std::numeric_limits<qint8>::max())
                           });
        const ValidatorResult vr = v.validate(c, Validator::FillStashOnError|Validator::BodyParamsOnly);
        if (vr) {
            SkaffariError e;
            AutoconfigServer s = AutoconfigServer::create(c, 0, vr.values(), e);
            if (Q_LIKELY(e.type() == SkaffariError::NoError)) {
                c->res()->redirect(c->uriForAction(QStringLiteral("/settings/autoconfig"), QStringList(), QStringList(), StatusMessage::statusQuery(c, c->translate("SettingsEditor", "Successfully added new global autoconfig server “%1” (ID: %2).").arg(s.hostname(), QString::number(s.id())))));
                return;
            } else {
                c->setStash(QStringLiteral("error_msg"), e.errorText());
            }
        }
    }

    HelpHash help;
    help.reserve(6);
    help.insert(QStringLiteral("type"), HelpEntry(c->translate("SettingsEditor", "Type"), c->translate("SettingsEditor", "The type of the server.")));
    help.insert(QStringLiteral("hostname"), HelpEntry(c->translate("SettingsEditor", "Hostname"), c->translate("SettingsEditor", "The hostname of the server.")));
    help.insert(QStringLiteral("port"), HelpEntry(c->translate("SettingsEditor", "Port"), c->translate("SettingsEditor", "The network port of this server.")));
    help.insert(QStringLiteral("socketType"), HelpEntry(c->translate("SettingsEditor", "Socket type"), c->translate("SettingsEditor", "The socket type used to connect to this server.")));
    help.insert(QStringLiteral("authentication"), HelpEntry(c->translate("SettingsEditor", "Authentication"), c->translate("SettingsEditor", "The authentication method used to connect to thsi server.")));
    help.insert(QStringLiteral("sorting"), HelpEntry(c->translate("SettingsEditor", "Sorting value"), c->translate("SettingsEditor", "The sorting value used to sort the servers in the generated autoconfig/autodiscover file. Lower values come first.")));

    c->stash({
                 {QStringLiteral("help"), QVariant::fromValue<HelpHash>(help)},
                 {QStringLiteral("site_title"), c->translate("SettingsEditor", "Add global autoconfig server")},
                 {QStringLiteral("template"),   QStringLiteral("settings/add_autoconfig_server.html")}
             });
}

void SettingsEditor::edit_autoconfig_server(Context *c, const QString &id)
{
    bool ok = true;
    const dbid_t serverId = Utils::strToDbid(id, &ok);
    if (Q_UNLIKELY(!ok)) {
        SkaffariError e(c, SkaffariError::InputError, c->translate("SettingsEditor", "Invalid autoconfig server ID."));
        e.toStash(c);
        c->detach(c->getAction(QStringLiteral("error")));
        return;
    }

    SkaffariError e(c);
    AutoconfigServer server = AutoconfigServer::get(c, 0, serverId, e);
    if (e.type() != SkaffariError::NoError) {
        e.toStash(c);
        c->detach(c->getAction(QStringLiteral("error")));
        return;
    }

    if (c->req()->isPost()) {
        static Validator v({
                               new ValidatorRequired(QStringLiteral("type")),
                               new ValidatorBetween(QStringLiteral("type"), QMetaType::Char, static_cast<qint8>(AutoconfigServer::Imap), static_cast<qint8>(AutoconfigServer::Smtp)),
                               new ValidatorRequired(QStringLiteral("hostname")),
                               new ValidatorDomain(QStringLiteral("hostname")),
                               new ValidatorRequired(QStringLiteral("port")),
                               new ValidatorBetween(QStringLiteral("port"), QMetaType::UShort, 0, 65535),
                               new ValidatorRequired(QStringLiteral("socketType")),
                               new ValidatorBetween(QStringLiteral("socketType"), QMetaType::Char, static_cast<qint8>(AutoconfigServer::Plain), static_cast<qint8>(AutoconfigServer::Ssl)),
                               new ValidatorRequired(QStringLiteral("authentication")),
                               new ValidatorBetween(QStringLiteral("authentication"), QMetaType::Char, static_cast<qint8>(AutoconfigServer::Cleartext), static_cast<qint8>(AutoconfigServer::TlsClientCert)),
                               new ValidatorBetween(QStringLiteral("sorting"), QMetaType::Char, std::numeric_limits<qint8>::min(), std::numeric_limits<qint8>::max())
                           });
        const ValidatorResult vr = v.validate(c, Validator::FillStashOnError|Validator::BodyParamsOnly);
        if (vr) {
            SkaffariError updateError(c);
            if (Q_LIKELY(server.update(c, vr.values(), e))) {
                c->res()->redirect(c->uriForAction(QStringLiteral("/settings/autoconfig"), QStringList(), QStringList(), StatusMessage::statusQuery(c, c->translate("SettingsEditor", "Successfully updated global autoconfig server “%1” (ID: %2).").arg(server.hostname(), QString::number(server.id())))));
                return;
            } else {
                c->setStash(QStringLiteral("error_msg"), e.errorText());
            }
        }
    }

    HelpHash help;
    help.reserve(6);
    help.insert(QStringLiteral("type"), HelpEntry(c->translate("SettingsEditor", "Type"), c->translate("SettingsEditor", "The type of the server.")));
    help.insert(QStringLiteral("hostname"), HelpEntry(c->translate("SettingsEditor", "Hostname"), c->translate("SettingsEditor", "The hostname of the server.")));
    help.insert(QStringLiteral("port"), HelpEntry(c->translate("SettingsEditor", "Port"), c->translate("SettingsEditor", "The network port of this server.")));
    help.insert(QStringLiteral("socketType"), HelpEntry(c->translate("SettingsEditor", "Socket type"), c->translate("SettingsEditor", "The socket type used to connect to this server.")));
    help.insert(QStringLiteral("authentication"), HelpEntry(c->translate("SettingsEditor", "Authentication"), c->translate("SettingsEditor", "The authentication method used to connect to thsi server.")));
    help.insert(QStringLiteral("sorting"), HelpEntry(c->translate("SettingsEditor", "Sorting value"), c->translate("SettingsEditor", "The sorting value used to sort the servers in the generated autoconfig/autodiscover file. Lower values come first.")));

    c->stash({
                 {QStringLiteral("help"), QVariant::fromValue<HelpHash>(help)},
                 {QStringLiteral("server"), QVariant::fromValue<AutoconfigServer>(server)},
                 {QStringLiteral("site_title"), c->translate("SettingsEditor", "Add global autoconfig server")},
                 {QStringLiteral("template"),   QStringLiteral("settings/edit_autoconfig_server.html")}
             });
}

void SettingsEditor::remove_autoconfig_server(Context *c, const QString &id)
{
    const bool isAjax = c->req()->xhr();

    if (Utils::ajaxPostOnly(c, isAjax)) {
        return;
    }

    bool ok = true;
    const dbid_t serverId = Utils::strToDbid(id, &ok, c->translate("SettingsEditor", "Invalid autoconfig server ID."), c);
    if (Q_UNLIKELY(!ok)) {
        return;
    }

    SkaffariError e(c);
    AutoconfigServer server = AutoconfigServer::get(c, 0, serverId, e);
    if (e.type() != SkaffariError::NoError) {
        e.toStash(c, true);
        return;
    }

    QJsonObject json;

    if (c->req()->isPost()) {
        if (Q_UNLIKELY(static_cast<quint32>(c->req()->bodyParam(QStringLiteral("serverId")).toUInt()) != serverId)) {
            Utils::setError(c, json, c->translate("SettingsEditor", "Invalid autoconfig server ID."), Response::BadRequest);
        } else {

            SkaffariError removeError(c);
            if (Q_LIKELY(server.remove(c, removeError))) {
                const QString statusMsg = c->translate("SettingsEditor", "Successfully removed global autoconfig server “%1” (ID: %2)").arg(server.hostname(), QString::number(server.id()));
                if (isAjax) {
                    json.insert(QStringLiteral("status_msg"), statusMsg);
                    json.insert(QStringLiteral("server_hostname"), server.hostname());
                    json.insert(QStringLiteral("server_id"), static_cast<qint64>(server.id()));
                    json.insert(QStringLiteral("server_domain_id"), 0);
                } else {
                    c->res()->redirect(c->uriForAction(QStringLiteral("/settings/autoconfig"), QStringList(), QStringList(), StatusMessage::statusQuery(c, statusMsg)));
                    return;
                }
            } else {
                Utils::setError(c, json, removeError);
            }
        }
    }

    if (isAjax) {
        c->res()->setJsonObjectBody(json);
    } else {
        c->stash({
                     {QStringLiteral("template"), QStringLiteral("settings/remove_autoconfig_server.html")},
                     {QStringLiteral("server"), QVariant::fromValue<AutoconfigServer>(server)}
                 });
    }
}

#include "moc_settingseditor.cpp"
