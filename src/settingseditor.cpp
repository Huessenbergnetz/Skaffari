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

SettingsEditor::SettingsEditor(QObject *parent) : Controller(parent)
{

}

SettingsEditor::~SettingsEditor()
{

}

void SettingsEditor::index(Context *c)
{
    if (checkAccess(c)) {

        if (c->req()->isPost()) {

        }

        HelpHash help;
        help.insert(QStringLiteral("defaultLanguage"), HelpEntry(c->translate("SettingsEditor", "Default language"), c->translate("SettingsEditor", "Default fallback language that will be used if user has no language set and if the language reported by the browser is not supported.")));
        help.insert(QStringLiteral("defaultTimezone"), HelpEntry(c->translate("SettingsEditor", "Default time zone"), c->translate("SettingsEditor", "Default time zone as fallback taht will be used to display localized dates and times if the user has not set one.")));

        c->stash(SkaffariConfig::getSettingsFromDB());
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
