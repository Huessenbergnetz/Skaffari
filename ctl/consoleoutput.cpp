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

#include "consoleoutput.h"
#include <cstdio>
#include <iostream>
#include <string>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>
#include <QStringList>

ConsoleOutput::ConsoleOutput()
{

}


ConsoleOutput::~ConsoleOutput()
{

}


int ConsoleOutput::error(const QString &message, int code) const
{
    printError(message);
    return code;
}

int ConsoleOutput::inputError(const QString &message) const
{
    return error(message, 1);
}

int ConsoleOutput::configError(const QString &message) const
{
    return error(message, 2);
}


int ConsoleOutput::dbError(const QString &message) const
{
    return error(message, 3);
}


int ConsoleOutput::dbError(const QSqlDatabase &db) const
{
    return dbError(db.lastError().text());
}


int ConsoleOutput::dbError(const QSqlQuery &q) const
{
    return dbError(q.lastError().text());
}


int ConsoleOutput::dbError(const QSqlError &e) const
{
    return dbError(e.text());
}


int ConsoleOutput::fileError(const QString &message) const
{
    return error(message, 4);
}


int ConsoleOutput::imapError(const QString &message) const
{
    return error(message, 5);
}


void ConsoleOutput::printStatus(const QString &status) const
{
    printf("%-100s", status.toUtf8().constData());
}


void ConsoleOutput::printDone(const QString &done) const
{
    if (done.isEmpty()) {
        printf("\x1b[32m%s\x1b[0m\n", QCoreApplication::translate("ConsoleOutput", "Done").toUtf8().constData());
    } else {
        printf("\x1b[32m%s\x1b[0m\n", done.toUtf8().constData());
    }
}


void ConsoleOutput::printFailed(const QString &failed) const
{
    if (failed.isEmpty()) {
        printf("\x1b[31m%s\x1b[0m\n", QCoreApplication::translate("ConsoleOutput", "Failed").toUtf8().constData());
    } else {
        printf("\x1b[31m%s\x1b[0m\n", failed.toUtf8().constData());
    }
}


void ConsoleOutput::printError(const QString &error) const
{
    printf("\x1b[33m%s\x1b[0m\n", error.toUtf8().constData());
}


void ConsoleOutput::printError(const QStringList &errors) const
{
    if (!errors.empty()) {
        for (const QString &error : errors) {
            printError(error);
        }
    }
}


void ConsoleOutput::printMessage(const QString &message) const
{
    printf("%s\n", message.toUtf8().constData());
}


void ConsoleOutput::printSuccess(const QString &message) const
{
    printf("\x1b[32m%s\x1b[0m\n", message.toUtf8().constData());
}


void ConsoleOutput::printDesc(const QString &desc) const
{
    if (desc.length() <= 95) {
        printf("# %s\n", desc.toUtf8().constData());
    } else {
        const QStringList parts = desc.split(QStringLiteral(" "));
        QStringList out;
        int outSize = 0;
        for (const QString &part : parts) {
            if ((outSize + part.length() + 1) <= 95) {
                out << part;
                outSize += (part.length() + 1);
            } else {
                printf("# %s\n", out.join(QStringLiteral(" ")).toUtf8().constData());
                out.clear();
                out << part;
                outSize = (part.length() + 1);
            }
        }
        if (!out.isEmpty()) {
            printf("# %s\n", out.join(QStringLiteral(" ")).toUtf8().constData());
        }
    }
}


void ConsoleOutput::printDesc(const QStringList &descList) const
{
    if (!descList.empty()) {
        for (const QString &desc : descList) {
            printDesc(desc);
        }
    }
}


void ConsoleOutput::printTable(std::initializer_list<std::pair<QString, QString> > table, const QString &header) const
{
    if (table.size() == 0) {
        return;
    }

    int maxLabelLength = 0;
    int maxValueLength = 0;

    for (const std::pair<QString,QString> &col : table) {
        if (maxLabelLength < col.first.length()) {
            maxLabelLength = col.first.length();
        }
        if (maxValueLength < col.second.length()) {
            maxValueLength = col.second.length();
        }
    }

    maxLabelLength += 5;
    maxValueLength += 5;

    int fullLength = maxLabelLength + maxValueLength;
    if ((fullLength % 2) > 0) {
        fullLength++;
    }

    QString devider = QChar('+');
    for (int i = 0; i < fullLength; ++i) {
        devider.append(QChar('-'));
    }
    devider.append(QChar('+'));
    printf("%s\n", devider.toUtf8().constData());

    if (!header.isEmpty()) {
        int centerHeader = fullLength / 2 + header.length() / 2;
        printf("|%*s%*s\n", centerHeader, qUtf8Printable(header), fullLength-centerHeader+1, "|");
        printf("%s\n", qUtf8Printable(devider));
    }

    for (const std::pair<QString,QString> &col : table) {
        printf("| %-*s| %-*s|\n", maxLabelLength-1, qUtf8Printable(col.first), maxValueLength-2, qUtf8Printable(col.second));
    }

    printf("%s\n", qUtf8Printable(devider));
}




QString ConsoleOutput::readString(const QString &name, const QString &defaultVal, const QStringList &desc, const QStringList acceptableInput) const
{
    QString inputVal;

    printDesc(desc);

    if (acceptableInput.empty()) {
        while (inputVal.isEmpty()) {
            if (!defaultVal.isEmpty()) {
                printf("%s [%s]: ", qUtf8Printable(name), qUtf8Printable(defaultVal));
            } else {
                printf("%s: ", qUtf8Printable(name));
            }
            std::string in;
            std::getline(std::cin, in);
            inputVal = QString::fromStdString(in);
            if (inputVal.isEmpty() && !defaultVal.isEmpty()) {
                inputVal = defaultVal;
            }
            if (inputVal.isEmpty()) {
                printf("%s\n", qUtf8Printable(tr("Can not be empty.")));
            }
        }
    } else {
        while (!acceptableInput.contains(inputVal)) {
            if (!defaultVal.isEmpty()) {
                printf("%s [%s]: ", qUtf8Printable(name), qUtf8Printable(defaultVal));
            } else {
                printf("%s: ", qUtf8Printable(name));
            }
            std::string in;
            std::getline(std::cin, in);
            inputVal = QString::fromStdString(in);
            if (inputVal.isEmpty() && !defaultVal.isEmpty()) {
                inputVal = defaultVal;
            }
        }
    }

    return inputVal;
}


quint16 ConsoleOutput::readPort(const QString &name, quint16 defaultVal, const QStringList &desc) const
{
    quint16 inputVal = 0;

    printDesc(desc);

    printf("%s [%i]: ", qUtf8Printable(name), defaultVal);
    std::string in;
    std::getline(std::cin, in);
    QString _inputVal = QString::fromStdString(in);
    if (_inputVal.isEmpty()) {
        inputVal = defaultVal;
    } else {
        inputVal = _inputVal.toUInt();
    }

    return inputVal;
}


quint8 ConsoleOutput::readChar(const QString &name, quint8 defaultVal, const QStringList &desc, const QList<quint8> acceptableInput) const
{
    quint8 inputVal = 255;

    printDesc(desc);

    if (acceptableInput.empty()) {
        printf("%s [%i]: ", qUtf8Printable(name), defaultVal);
        std::string in;
        std::getline(std::cin, in);
        QString _inputVal = QString::fromStdString(in);
        if (_inputVal.isEmpty()) {
            inputVal = defaultVal;
        } else {
            inputVal = _inputVal.toUShort();
        }
    } else {
        while(!acceptableInput.contains(inputVal)) {
            printf("%s [%i]: ", qUtf8Printable(name), defaultVal);
            std::string in;
            std::getline(std::cin, in);
            QString _inputVal = QString::fromStdString(in);
            if (_inputVal.isEmpty()) {
                inputVal = defaultVal;
            } else {
                inputVal = _inputVal.toUShort();
            }
        }
    }

    return inputVal;
}


quint32 ConsoleOutput::readInt(const QString &name, quint32 defaultVal, const QStringList &desc) const
{
    quint32 inputVal = 0;

    printDesc(desc);

    printf("%s [%i]: ", qUtf8Printable(name), defaultVal);
    std::string in;
    std::getline(std::cin, in);
    QString _inputVal = QString::fromStdString(in);
    if (_inputVal.isEmpty()) {
        inputVal = defaultVal;
    } else {
        inputVal = _inputVal.toULong();
    }

    return inputVal;
}


bool ConsoleOutput::readBool(const QString &name, bool defaultVal, const QStringList &desc) const
{
    bool retVal = false;

    printDesc(desc);

    static const QStringList posVals({
                                         //: short for yes
                                         tr("Y"),
                                         //: short for no
                                         tr("N"),
                                         tr("Yes"),
                                         tr("No")
                                     });
    QString inStr;
    const QString defAnswer = defaultVal ? tr("Yes") : tr("No");
    while (!posVals.contains(inStr, Qt::CaseInsensitive)) {
        printf("%s [%s]: ", qUtf8Printable(name), qUtf8Printable(defAnswer));
        std::string in;
        std::getline(std::cin, in);
        inStr = QString::fromStdString(in);
        if (inStr.isEmpty()) {
            inStr = defAnswer;
        }
    }

    retVal = ((inStr.compare(tr("Y"), Qt::CaseInsensitive) == 0) || (inStr.compare(tr("Yes"), Qt::CaseInsensitive) == 0));

    return retVal;
}
