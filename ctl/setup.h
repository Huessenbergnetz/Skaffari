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

#ifndef SETUP_H
#define SETUP_H

#include <QFileInfo>
#include <QCoreApplication>

class Setup
{
    Q_DECLARE_TR_FUNCTIONS(Setup)
public:
    Setup(const QString &confFile);

    int exec() const;

    QString readString(const QString &name, const QString &defaultVal, const QString &desc = QString()) const;
    quint16 readPort(const QString &name, quint16 defaultVal, const QString &desc = QString()) const;
    quint8 readChar(const QString &name, quint8 defaultVal, const QString &desc = QString()) const;
    quint32 readInt(const QString &name, quint32 defaultVal, const QString &desc = QString()) const;
    bool readBool(const QString &name, bool defaultVal, const QString &desc = QString()) const;

private:
    QFileInfo m_confFile;
};

#endif // SETUP_H
