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

#ifndef FILESIZEFORMATTAG_H
#define FILESIZEFORMATTAG_H

#include <grantlee/filter.h>
#include <grantlee/safestring.h>
#include <grantlee/util.h>
#include <grantlee/node.h>

class FileSizeFormatTag : public Grantlee::AbstractNodeFactory
{
    Q_OBJECT
public:
    Grantlee::Node *getNode(const QString &tagContent, Grantlee::Parser *p) const override;
};


class FileSizeFormat : public Grantlee::Node
{
    Q_OBJECT
public:
    explicit FileSizeFormat(const Grantlee::FilterExpression &size, const Grantlee::FilterExpression &base, const Grantlee::FilterExpression &decimal, const Grantlee::FilterExpression &multiplier, Grantlee::Parser *parser = nullptr);

    void render(Grantlee::OutputStream *stream, Grantlee::Context *gc) const override;

private:
    mutable QString m_cutelystContext = QStringLiteral("c");
    Grantlee::FilterExpression m_size;
    Grantlee::FilterExpression m_base;
    Grantlee::FilterExpression m_precision;
    Grantlee::FilterExpression m_multiplier;

};

#endif // FILESIZEFORMATTAG_H
