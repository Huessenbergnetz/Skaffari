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
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimeZone>
#include <Cutelyst/Plugins/Utils/Validator> // includes the main validator
#include <Cutelyst/Plugins/Utils/Validators> // includes all validator rules
#include <Cutelyst/Plugins/Utils/ValidatorResult> // includes the validator result

SettingsEditor::SettingsEditor(QObject *parent) : Controller(parent)
{

}

SettingsEditor::~SettingsEditor()
{

}

void SettingsEditor::index(Context *c)
{
    if (checkAccess(c)) {

        QVariantHash settings = SkaffariConfig::getSettingsFromDB();

        if (c->req()->isPost()) {
            static Validator v({
                                   new ValidatorIn(QStringLiteral(SK_CONF_KEY_DEF_LANGUAGE), Language::supportedLangsList()),
                                   new ValidatorInteger(QStringLiteral(SK_CONF_KEY_DEF_WARNLEVEL)),
                                   new ValidatorBetween(QStringLiteral(SK_CONF_KEY_DEF_WARNLEVEL), QMetaType::UInt, 0.0, 100.0),
                                   new ValidatorInteger(QStringLiteral(SK_CONF_KEY_DEF_MAXDISPLAY)),
                                   new ValidatorBetween(QStringLiteral(SK_CONF_KEY_DEF_MAXDISPLAY), QMetaType::UInt, 15.0, 255.0),
                                   new ValidatorInteger(QStringLiteral(SK_CONF_KEY_DEF_MAXACCOUNTS)),
                                   new ValidatorMin(QStringLiteral(SK_CONF_KEY_DEF_MAXACCOUNTS), QMetaType::UInt, 0.0),
                                   new ValidatorRegularExpression(QStringLiteral(SK_CONF_KEY_DEF_QUOTA), QRegularExpression(QStringLiteral("^\\d+[,.٫]?\\d*\\s*[KMGT]?i?B?"), QRegularExpression::CaseInsensitiveOption)),
                                   new ValidatorRegularExpression(QStringLiteral(SK_CONF_KEY_DEF_DOMAINQUOTA), QRegularExpression(QStringLiteral("^\\d+[,.٫]?\\d*\\s*[KMGT]?i?B?"), QRegularExpression::CaseInsensitiveOption)),
                               });

            const ValidatorResult vr = v.validate(c, Validator::FillStashOnError);
            if (vr) {
                const ParamsMultiMap p = c->req()->bodyParams();
                settings.insert(QStringLiteral(SK_CONF_KEY_DEF_LANGUAGE), p.value(QStringLiteral(SK_CONF_KEY_DEF_LANGUAGE), settings.value(QStringLiteral(SK_CONF_KEY_DEF_LANGUAGE)).toString()));
                settings.insert(QStringLiteral(SK_CONF_KEY_DEF_TIMEZONE), p.value(QStringLiteral(SK_CONF_KEY_DEF_TIMEZONE), settings.value(QStringLiteral(SK_CONF_KEY_DEF_TIMEZONE)).toString()).toLatin1());
                settings.insert(QStringLiteral(SK_CONF_KEY_DEF_WARNLEVEL), p.value(QStringLiteral(SK_CONF_KEY_DEF_WARNLEVEL), settings.value(QStringLiteral(SK_CONF_KEY_DEF_WARNLEVEL)).toString()).toUInt());
                settings.insert(QStringLiteral(SK_CONF_KEY_DEF_MAXDISPLAY), p.value(QStringLiteral(SK_CONF_KEY_DEF_MAXDISPLAY), settings.value(QStringLiteral(SK_CONF_KEY_DEF_MAXDISPLAY)).toString()).toUInt());
                settings.insert(QStringLiteral(SK_CONF_KEY_DEF_MAXACCOUNTS), QVariant::fromValue<ulong>(p.value(QStringLiteral(SK_CONF_KEY_DEF_MAXACCOUNTS), settings.value(QStringLiteral(SK_CONF_KEY_DEF_MAXACCOUNTS)).toString()).toULong()));
                bool convertQuotas = true;
                settings.insert(QStringLiteral(SK_CONF_KEY_DEF_QUOTA), Utils::humanToIntSize(c, p.value(QStringLiteral(SK_CONF_KEY_DEF_QUOTA), Utils::humanBinarySize(c, settings.value(QStringLiteral(SK_CONF_KEY_DEF_QUOTA)).value<quota_size_t>())), &convertQuotas));
                settings.insert(QStringLiteral(SK_CONF_KEY_DEF_DOMAINQUOTA), Utils::humanToIntSize(c, p.value(QStringLiteral(SK_CONF_KEY_DEF_DOMAINQUOTA), Utils::humanBinarySize(c, settings.value(QStringLiteral(SK_CONF_KEY_DEF_DOMAINQUOTA)).value<quota_size_t>())), &convertQuotas));

                SkaffariConfig::saveSettingsToDB(settings);
                c->setStash(QStringLiteral("status_msg"), c->translate("SettingsEditor", "Settings successfully saved."));
            }
        }

        HelpHash help;
        help.insert(QStringLiteral(SK_CONF_KEY_DEF_LANGUAGE), HelpEntry(c->translate("SettingsEditor", "Default language"), c->translate("SettingsEditor", "Default fallback language that will be used if user has no language set and if the language reported by the browser is not supported.")));
        help.insert(QStringLiteral(SK_CONF_KEY_DEF_TIMEZONE), HelpEntry(c->translate("SettingsEditor", "Default time zone"), c->translate("SettingsEditor", "Default time zone as fallback that will be used to display localized dates and times if the user has not set one.")));
        help.insert(QStringLiteral(SK_CONF_KEY_DEF_WARNLEVEL), HelpEntry(c->translate("SettingsEditor", "Default warn level"), c->translate("SettingsEditor", "Default warn level for account number and quota storage limits if the user has not set one.")));
        help.insert(QStringLiteral(SK_CONF_KEY_DEF_MAXDISPLAY), HelpEntry(c->translate("SettingsEditor", "Default maximum display"), c->translate("SettingsEditor", "Default maximum display numer of items per page for paginated result lists.")));
        help.insert(QStringLiteral(SK_CONF_KEY_DEF_MAXACCOUNTS), HelpEntry(c->translate("SettingsEditor", "Default maximum accounts"), c->translate("SettingsEditor", "Default number of maximum accounts for new domains.")));
        help.insert(QStringLiteral(SK_CONF_KEY_DEF_DOMAINQUOTA), HelpEntry(c->translate("SettingsEditor", "Default domain quota"), c->translate("SettingsEditor", "Default amount of domain quota for new domains. You can use the multipliers M, MiB, G, GiB, T and TiB. Without a multiplier, KiB is the default.")));
        help.insert(QStringLiteral(SK_CONF_KEY_DEF_QUOTA), HelpEntry(c->translate("SettingsEditor", "Default account quota"), c->translate("SettingsEditor", "Default amount of default account storage quota for new domains. You can use the multipliers M, MiB, G, GiB, T and TiB. Without a multiplier, KiB is the default.")));

        c->stash(settings);
        c->stash({
                     {QStringLiteral("help"), QVariant::fromValue<HelpHash>(help)},
                     {QStringLiteral("timezones"), QVariant::fromValue<QList<QByteArray>>(QTimeZone::availableTimeZoneIds())},
                     {QStringLiteral("langs"), QVariant::fromValue<QVector<Language>>(Language::supportedLangs())},
                     {QStringLiteral("site_title"), c->translate("SettingsEditor", "Settings")},
                     {QStringLiteral("template"), QStringLiteral("settings/index.html")}
                 });
    }
}

bool SettingsEditor::checkAccess(Context *c)
{
    if (Q_LIKELY(c->stash(QStringLiteral("userType")).value<qint16>() == 0)) {
        return true;
    }

    if (Utils::isAjax(c)) {
        QJsonObject json({
                             {QStringLiteral("error_msg"), c->translate("SettingsEditor", "You are not authorized to access this resource or to perform this action.")}
                         });
        c->res()->setJsonBody(QJsonDocument(json));
    } else {
        c->stash({
                     {QStringLiteral("site_title"), c->translate("SettingsEditor", "Access denied")},
                     {QStringLiteral("template"), QStringLiteral("403.html")}
                 });
    }
    c->res()->setStatus(Response::Forbidden);

    return false;
}

bool SettingsEditor::accessGranted(Context *c)
{
    const quint16 status = c->res()->status();
    return ((status != 404) && (status != 403));
}
