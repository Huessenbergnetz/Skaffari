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

#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QFileInfo>

#include <cstdio>
#include <iostream>
#include <string>

#include "../common/config.h"
#include "setup.h"
#include "webcyradmimporter.h"
#include "tester.h"
#include "accountstatusupdater.h"

/*!
 * \defgroup skaffaricmd SkaffariCMD
 * \brief The %Skaffari command line utility skaffaricmd.
 */

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setOrganizationName(QStringLiteral("Buschtrommel"));
    app.setOrganizationDomain(QStringLiteral("buschmann23.de"));
    app.setApplicationName(QStringLiteral("skaffaricmd"));
    app.setApplicationVersion(QStringLiteral(SKAFFARI_VERSION));

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption setup(QStringLiteral("setup"), QCoreApplication::translate("main", "Create new Skaffari setup."));
    parser.addOption(setup);

    QCommandLineOption import(QStringLiteral("import-web-cyradm"), QCoreApplication::translate("main", "Import web-cyradm configuration and database."), QStringLiteral("web-cyradm conf file"));
    parser.addOption(import);

    QCommandLineOption test(QStringList({QStringLiteral("test"), QStringLiteral("t")}), QCoreApplication::translate("main", "Test the Skaffari settings."));
    parser.addOption(test);

    const QString confFilePath = QStringLiteral(SKAFFARI_CONFDIR) + QLatin1String("/skaffari.ini");
    QCommandLineOption iniPath(QStringList({QStringLiteral("ini"), QStringLiteral("i")}), QCoreApplication::translate("main", "Path to the configuration file. Default: %1").arg(confFilePath), QStringLiteral("ini-file"), confFilePath);
    parser.addOption(iniPath);

    QCommandLineOption quiet(QStringList({QStringLiteral("quiet"), QStringLiteral("q")}), QCoreApplication::translate("main", "Do not print any output."));
    parser.addOption(quiet);

    QCommandLineOption updateAccountStatus(QStringLiteral("update-account-status"), QCoreApplication::translate("main", "Checks and updates the status column of every account."));
    parser.addOption(updateAccountStatus);

    parser.process(app);

    if (parser.isSet(setup)) {

        Setup s(parser.value(iniPath));
        return s.exec();

    } else if (parser.isSet(import)) {

        WebCyradmImporter importer(parser.value(import), parser.value(iniPath));
        return importer.exec();

    } else if (parser.isSet(test)) {

        Tester tester(parser.value(iniPath));
        return tester.exec();

    } else if (parser.isSet(updateAccountStatus)) {

        AccountStatusUpdater asu(parser.value(iniPath), parser.isSet(quiet));
        return asu.exec();

    } else {
        parser.showHelp(1);
    }

    return 0;
}
