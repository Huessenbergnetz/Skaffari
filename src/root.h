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

#ifndef ROOT_H
#define ROOT_H

#include <Cutelyst/Controller>
#include <map>

using namespace Cutelyst;
class SkaffariEngine;

/*!
 * \ingroup skaffaricontrollers
 * \brief Routes for the root/empty namespace.
 */
class Root : public Controller
{
    Q_OBJECT
    C_NAMESPACE("")
public:
    explicit Root(QObject *parent = nullptr);
    ~Root();

    C_ATTR(index, :Path :Args(0))
    void index(Context *c);

    C_ATTR(about, :Global :Args(0))
    void about(Context *c);

    C_ATTR(defaultPage, :Path)
    void defaultPage(Context *c);

    C_ATTR(csrfdenied, :Local :Private :AutoArgs :ActionClass("RenderView"))
    void csrfdenied(Context *c);

    C_ATTR(error, :Local :Private :AutoArgs :ActionClass("RenderView"))
    void error(Context *c);

private:
    C_ATTR(End, :ActionClass("RenderView"))
    void End(Context *c) { Q_UNUSED(c); }

    C_ATTR(Auto, :Private)
    bool Auto(Context *c);

    /*!
     * \brief Returns the version string of the ICU library in use.
     */
    QString getICUversion() const;

    /*!
     * \brief Creates a new map containing information about used core components.
     * \param name          Name of the component.
     * \param version       Version of the component.
     * \param url           URL to the component's website.
     * \param author        Name of the component's author.
     * \param authorUrl     URL to the website of the component's author.
     * \param license       Name of the license of the component.
     * \param licenseUrl    URL to the component's license text.
     */
    std::map<QString,QString> createCoreComponentInfo(const QString &name,
                                                  const QString &version,
                                                  const QString &url,
                                                  const QString &author,
                                                  const QString &authorUrl,
                                                  const QString &license,
                                                  const QString &licenseUrl) const;
};

#endif //ROOT_H

