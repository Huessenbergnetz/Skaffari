/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "imapresult.h"

ImapResponse::Data::Data(ImapResponse::Type _type, const ImapError &_error)
    : QSharedData()
    , error{_error}
    , type{_type}
{
}

ImapResponse::Data::Data(ImapResponse::Type _type, const QString &_statusLine, const QStringList &_lines)
    : QSharedData()
    , lines{_lines}
    , statusLine{_statusLine}
    , type{_type}
{
}

ImapResponse::ImapResponse(ImapResponse::Type type, const ImapError &error)
    : data{new ImapResponse::Data{type, error}}
{
}

ImapResponse::ImapResponse(Type type, const QString &statusLine, const QStringList &lines)
    : data{new ImapResponse::Data{type, statusLine, lines}}
{
}

ImapResponse::Type ImapResponse::type() const noexcept
{
    return data ? data->type : ImapResponse::Undefined;
}

QString ImapResponse::statusLine() const noexcept
{
    return data ? data->statusLine : QString();
}

QStringList ImapResponse::lines() const noexcept
{
    return data ? data->lines : QStringList();
}

ImapError ImapResponse::error() const noexcept
{
    return data ? data->error : ImapError();
}
