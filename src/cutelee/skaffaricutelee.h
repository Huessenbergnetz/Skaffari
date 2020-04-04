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

#ifndef SKAFFARICUTELEE_H
#define SKAFFARICUTELEE_H

#include <cutelee/taglibraryinterface.h>

/*!
 * \defgroup skaffaricutelee Cutelee
 * \brief %Skaffari specific Cutelee template engine plugins
 */

/*!
 * \ingroup skaffaricutelee
 * \defgroup skaffaricuteleetags Tags
 * \brief %Skaffari specific Cutelee tags
 */

/*!
 * \ingroup skaffaricutelee
 * \defgroup skaffaricuteleefilters Filters
 * \brief %Skaffari specific Cutelee filters
 */

/*!
 * \ingroup skaffaricutelee
 * \brief %Skaffari specific plugin for the Cutelee template engine.
 *
 * Provides Skaffari specific tags and filters for the Cutelee template engine.
 *
 * The following tags and filters are available:
 * \li sk_fsf - FileSizeFormat
 * \li sk_tzc - TimeZoneConvert
 * \li sk_acedecode - AceDecodeFilter
 * \li sk_urlencode - UrlEncodeFilter
 */
class SkaffariCutelee : public QObject, public Cutelee::TagLibraryInterface
{
    Q_OBJECT
    Q_INTERFACES(Cutelee::TagLibraryInterface)
public:
    /*!
     * \brief Constructs a new SkaffariCutelee object.
     */
    explicit SkaffariCutelee(QObject *parent = nullptr);

    /*!
     * \brief Returns the Skaffari specific Cutelee template tags.
     */
    virtual QHash<QString, Cutelee::AbstractNodeFactory *> nodeFactories(const QString &name = QString())  override;

    /*!
     * \brief Returns the Skaffari specific Cutelee template filters.
     */
    virtual QHash<QString, Cutelee::Filter *> filters(const QString &name = QString()) override;
};

#endif // SKAFFARICUTELEE_H
