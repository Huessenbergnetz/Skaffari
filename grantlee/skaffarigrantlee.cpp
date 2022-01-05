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

#include "skaffarigrantlee.h"

#include "filesizeformattag.h"
#include "timezoneconverttag.h"
#include "admintypetag.h"
#include "urlencodefilter.h"
#include "acedecodefilter.h"
#include "stringlistsortfilter.h"
#include "splitfilter.h"
#include "stringformatfilter.h"

SkaffariGrantlee::SkaffariGrantlee(QObject *parent) : QObject(parent)
{

}


QHash<QString, Grantlee::AbstractNodeFactory *> SkaffariGrantlee::nodeFactories(const QString &name)
{
    Q_UNUSED(name)

    QHash<QString, Grantlee::AbstractNodeFactory *> ret;

    ret.insert(QStringLiteral("sk_fsf"), new FileSizeFormatTag());
    ret.insert(QStringLiteral("sk_tzc"), new TimeZoneConvertTag());
    ret.insert(QStringLiteral("sk_admintypename"), new AdminTypeTag());

    return ret;
}


QHash<QString, Grantlee::Filter *> SkaffariGrantlee::filters(const QString &name)
{
    Q_UNUSED(name)

    QHash<QString, Grantlee::Filter *> ret;

    ret.insert(QStringLiteral("sk_urlencode"), new UrlEncodeFilter());
    ret.insert(QStringLiteral("sk_acedecode"), new AceDecodeFilter());
    ret.insert(QStringLiteral("sk_stringlistsort"), new StringListSortFilter());
    ret.insert(QStringLiteral("sk_split"), new SplitFilter());
    ret.insert(QStringLiteral("sk_stringformat"), new StringformatFilter());

    return ret;
}
