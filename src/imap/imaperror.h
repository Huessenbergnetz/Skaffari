/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef SKAFFARI_IMAPERROR_H
#define SKAFFARI_IMAPERROR_H

#include <QSharedData>

class QSslError;

class ImapError
{
public:
    enum Type : int {
        NoError,
        NoResponse,
        BadResponse,
        UndefinedResponse,
        EmptyResponse,
        ConnectionTimeout,
        EncryptionError,
        SocketError,
        ResponseError,
        InternalError,
        ConfigError,
        Unknown
    };

    ImapError() noexcept = default;

    ImapError(Type type, const QString &text);
    ImapError(const QSslError &sslError);

    ImapError(const ImapError &other) noexcept = default;
    ImapError(ImapError &&other) noexcept = default;
    ImapError &operator=(const ImapError &other) noexcept = default;
    ImapError &operator=(ImapError &&other) noexcept = default;
    ~ImapError() noexcept = default;

    void swap(ImapError &other) noexcept { data.swap(other.data); }

    [[nodiscard]] Type type() const noexcept;

    [[nodiscard]] QString text() const noexcept;

    [[nodiscard]] bool isError() const noexcept { return type() != NoError; }

    [[nodiscard]] bool isNull() const noexcept { return !data; };

    void clear()
    {
        if (!isNull())
            *this = ImapError();
    }

    explicit operator bool() const noexcept { return isError(); }

private:
    class Data : public QSharedData // NOLINT(cppcoreguidelines-special-member-functions)
    {
    public:
        Data() noexcept = default;

        Data(ImapError::Type _type, QString _text);
        Data(const QSslError &sslError);

        Data(const Data &) noexcept = default;
        Data &operator=(const Data &) = delete;
        ~Data() noexcept = default;

        QString text;
        ImapError::Type type{ImapError::NoError};
    };

    QSharedDataPointer<Data> data;
};

Q_DECLARE_TYPEINFO(ImapError, Q_MOVABLE_TYPE); // NOLINT(modernize-type-traits)

// void swap(Skaffari::ImapError &a, Skaffari::ImapError &b) noexcept
// {
//     a.swap(b);
// }

#endif // SKAFFARI_IMAPERROR_H
