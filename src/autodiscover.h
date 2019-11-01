/*
 * Skaffari - a mail account administration web interface based on Cutelyst
 * Copyright (C) 2019 Matthias Fehring <mf@huessenbergnetz.de>
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

#ifndef AUTODISCOVER_H
#define AUTODISCOVER_H

#include <Cutelyst/Controller>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(SK_AUTODISCOVER)

using namespace Cutelyst;

class Autodiscover : public Controller
{
    Q_OBJECT
    C_NAMESPACE("autodiscover/autodiscover.xml")
public:
    explicit Autodiscover(QObject *parent = nullptr);

    C_ATTR(index, :Path)
    void index(Context *c);

private:
    void setError(Context *c, Response::HttpStatus status, const QString &msg, int errorCode);
};

#endif // AUTODISCOVER_H
