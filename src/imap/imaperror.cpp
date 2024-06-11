/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "imaperror.h"

#include <QSslError>

using namespace Skaffari;

ImapError::Data::Data(ImapError::Type _type, QString _text)
    : QSharedData()
    , text{std::move(_text)}
    , type{_type}
{
}

ImapError::Data::Data(const QSslError &sslError)
    : QSharedData()
    , text{sslError.errorString()}
    , type{ImapError::EncryptionError}
{
}

ImapError::ImapError(Type type, const QString &text)
    : data{new Data{type, text}}
{
}

ImapError::ImapError(const QSslError &sslError)
    : data{new Data{sslError}}
{

}

ImapError::Type ImapError::type() const noexcept
{
    return data ? data->type : ImapError::NoError;
}

QString ImapError::text() const noexcept
{
    return data ? data->text : QString();
}
