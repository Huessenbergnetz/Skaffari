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

#ifndef SKAFFARIGRANTLEE_H
#define SKAFFARIGRANTLEE_H

#include <grantlee/taglibraryinterface.h>

class SkaffariGrantlee : public QObject, public Grantlee::TagLibraryInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.grantlee.TagLibraryInterface/1.0")
    Q_INTERFACES(Grantlee::TagLibraryInterface)
public:
    explicit SkaffariGrantlee(QObject *parent = nullptr);

    virtual QHash<QString, Grantlee::AbstractNodeFactory *> nodeFactories(const QString &name = QString())  override;
};

#endif // SKAFFARIGRANTLEE_H