/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "imapresult.h"

using namespace Skaffari;

ImapResult::Data::Data(ImapResult::Type _type, const ImapError &_error)
    : QSharedData()
    , error{_error}
    , type{_type}
{
}

ImapResult::Data::Data(ImapResult::Type _type, const QString &_statusLine, const QStringList &_lines)
    : QSharedData()
    , lines{_lines}
    , statusLine{_statusLine}
    , type{_type}
{
}

ImapResult::ImapResult(ImapResult::Type type, const ImapError &error)
    : data{new ImapResult::Data{type, error}}
{
}

ImapResult::ImapResult(Type type, const QString &statusLine, const QStringList &lines)
    : data{new ImapResult::Data{type, statusLine, lines}}
{
}

ImapResult::Type ImapResult::type() const noexcept
{
    return data ? data->type : ImapResult::Undefined;
}

QString ImapResult::statusLine() const noexcept
{
    return data ? data->statusLine : QString();
}

QStringList ImapResult::lines() const noexcept
{
    return data ? data->lines : QStringList();
}

ImapError ImapResult::error() const noexcept
{
    return data ? data->error : ImapError();
}
