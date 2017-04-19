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

#ifndef CONSOLEOUTPUT_H
#define CONSOLEOUTPUT_H

#include <QString>
#include <QCoreApplication>

class QSqlDatabase;
class QSqlQuery;
class QSqlError;
class QStringList;

class ConsoleOutput {

    Q_DECLARE_TR_FUNCTIONS(ConsoleOutput)
public:
    ConsoleOutput();
    ~ConsoleOutput();

protected:
    int error(const QString &message, int code = 99) const;
    int inputError(const QString &message) const;
    int configError(const QString &message) const;
    int dbError(const QString &message) const;
    int dbError(const QSqlDatabase &db) const;
    int dbError(const QSqlQuery &q) const;
    int dbError(const QSqlError &e) const;
    int fileError(const QString &message) const;
    int imapError(const QString &message) const;
    void printStatus(const QString &status) const;
    void printDone(const QString &done = QString()) const;
    void printFailed(const QString &failed = QString()) const;
    void printError(const QString &error) const;
    void printError(const QStringList &errors) const;
    void printMessage(const QString &message) const;
    void printSuccess(const QString &message) const;
    void printDesc(const QString &desc) const;
    void printDesc(const QStringList &descList) const;
    void printTable(std::initializer_list<std::pair<QString, QString>> table, const QString &header = QString()) const;
};

#endif // CONSOLEOUTPUT_H
