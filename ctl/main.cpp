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

#include "setup.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setOrganizationName(QStringLiteral("Buschtrommel"));
    app.setOrganizationDomain(QStringLiteral("buschmann23.de"));
    app.setApplicationName(QStringLiteral("skaffarictl"));
    app.setApplicationVersion(QStringLiteral(SKAFFARI_VERSION));

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption setup(QStringList({QStringLiteral("setup"), QStringLiteral("s")}), QCoreApplication::translate("main", "Create new Skaffari setup."));
    parser.addOption(setup);

    QString confFilePath = QStringLiteral(SKAFFARI_CONFDIR);
    confFilePath.append(QLatin1String("/skaffari.ini"));
    QCommandLineOption iniPath(QStringLiteral("ini-path"), QCoreApplication::translate("main", "Path to the configuration file. Default: %1").arg(confFilePath), QStringLiteral("ini-file"), confFilePath);
    parser.addOption(iniPath);

    parser.process(app);

    if (parser.isSet(setup)) {

        Setup s(parser.value(iniPath));
        return s.exec();

    } else {
        parser.showHelp(1);
    }

    return 0;
}
