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

#include "settingseditor.h"
#include "utils/utils.h"
#include "utils/skaffariconfig.h"
#include "utils/language.h"
#include "objects/helpentry.h"
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
    QVariantHash settings = SkaffariConfig::getSettingsFromDB();

    if (c->req()->isPost()) {
        static Validator v({
                               new ValidatorIn(QStringLiteral(SK_CONF_KEY_DEF_LANGUAGE), Language::supportedLangsList()),
                               new ValidatorBetween(QStringLiteral(SK_CONF_KEY_DEF_WARNLEVEL), QMetaType::UChar, 0, 100),
                               new ValidatorBetween(QStringLiteral(SK_CONF_KEY_DEF_MAXDISPLAY), QMetaType::UChar, 15, 255),
                               new ValidatorMin(QStringLiteral(SK_CONF_KEY_DEF_MAXACCOUNTS), QMetaType::UInt, 0),
                               new ValidatorFileSize(QStringLiteral(SK_CONF_KEY_DEF_QUOTA), ValidatorFileSize::ForceBinary, 0, std::numeric_limits<quota_size_t>::max()),
                               new ValidatorFileSize(QStringLiteral(SK_CONF_KEY_DEF_DOMAINQUOTA), ValidatorFileSize::ForceBinary, 0, std::numeric_limits<quota_size_t>::max()),
                               new SkValidatorAccountExists(QStringLiteral(SK_CONF_KEY_DEF_ABUSE_ACC)),
                               new SkValidatorAccountExists(QStringLiteral(SK_CONF_KEY_DEF_NOC_ACC)),
                               new SkValidatorAccountExists(QStringLiteral(SK_CONF_KEY_DEF_SECURITY_ACC)),
                               new SkValidatorAccountExists(QStringLiteral(SK_CONF_KEY_DEF_POSTMASTER_ACC)),
                               new SkValidatorAccountExists(QStringLiteral(SK_CONF_KEY_DEF_HOSTMASTER_ACC)),
                               new SkValidatorAccountExists(QStringLiteral(SK_CONF_KEY_DEF_WEBMASTER_ACC))
                           });

        ValidatorResult vr = v.validate(c, Validator::FillStashOnError|Validator::BodyParamsOnly);
        if (vr) {
            vr.addValue(QStringLiteral(SK_CONF_KEY_DEF_DOMAINQUOTA), static_cast<quota_size_t>(vr.value(QStringLiteral(SK_CONF_KEY_DEF_DOMAINQUOTA)).value<quota_size_t>() / Q_UINT64_C(1024)));
            vr.addValue(QStringLiteral(SK_CONF_KEY_DEF_QUOTA), static_cast<quota_size_t>(vr.value(QStringLiteral(SK_CONF_KEY_DEF_QUOTA)).value<quota_size_t>() / Q_UINT64_C(1024)));
            SkaffariConfig::saveSettingsToDB(vr.values());
            c->setStash(QStringLiteral("status_msg"), c->translate("SettingsEditor", "Settings successfully saved."));
            settings = SkaffariConfig::getSettingsFromDB();
        }
    }

    HelpHash help;
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

    c->stash(settings);
    c->stash({
                 {QStringLiteral("help"), QVariant::fromValue<HelpHash>(help)},
                 {QStringLiteral("timezones"), QVariant::fromValue<QList<QByteArray>>(QTimeZone::availableTimeZoneIds())},
                 {QStringLiteral("langs"), QVariant::fromValue<QVector<Language>>(Language::supportedLangs(c))},
                 {QStringLiteral("site_title"), c->translate("SettingsEditor", "Settings")},
                 {QStringLiteral("template"), QStringLiteral("settings/index.html")}
             });
}

#include "moc_settingseditor.cpp"
