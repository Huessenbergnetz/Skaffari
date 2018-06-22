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

#ifndef SKAFFARIGRANTLEE_H
#define SKAFFARIGRANTLEE_H

#include <grantlee/taglibraryinterface.h>

/*!
 * \defgroup skaffarigrantlee SkaffariGrantlee
 * \brief %Skaffari specific Grantlee template engine plugins
 */

/*!
 * \ingroup skaffarigrantlee
 * \brief Skaffari specific plugin for the Grantlee template engine.
 *
 * Provides Skaffari specific tags and filters for the Grantlee template engine.
 *
 * The following tags and filters are available:
 * \li sk_fsf - FileSizeFormat
 * \li sk_tzc - TimeZoneConvert
 * \li sk_acedecode - AceDecodeFilter
 * \li sk_urlencode - UrlEncodeFilter
 */
class SkaffariGrantlee : public QObject, public Grantlee::TagLibraryInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.grantlee.TagLibraryInterface/1.0")
    Q_INTERFACES(Grantlee::TagLibraryInterface)
public:
    /*!
     * \brief Constructs a new SkaffariGrantlee object.
     */
    explicit SkaffariGrantlee(QObject *parent = nullptr);

    /*!
     * \brief Returns the Skaffari specific Grantlee template tags.
     */
    virtual QHash<QString, Grantlee::AbstractNodeFactory *> nodeFactories(const QString &name = QString())  override;

    /*!
     * \brief Returns the Skaffari specific Grantlee template filters.
     */
    virtual QHash<QString, Grantlee::Filter *> filters(const QString &name = QString()) override;
};

#endif // SKAFFARIGRANTLEE_H
