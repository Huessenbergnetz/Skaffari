/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef SKAFFARI_IMAPRESPONSE_H
#define SKAFFARI_IMAPRESPONSE_H

#include "imap/imaperror.h"

#include <QSharedData>

class ImapResponse {
public:
    enum Type : int {
        OK,
        NO,
        BAD,
        Undefined
    };

    ImapResponse() noexcept = default;

    ImapResponse(Type type, const ImapError &error);
    ImapResponse(Type type, const QString &statusLine, const QStringList &lines = {});

    ImapResponse(const ImapResponse &other) noexcept = default;
    ImapResponse(ImapResponse &&other) noexcept = default;
    ImapResponse &operator=(const ImapResponse &other) noexcept = default;
    ImapResponse &operator=(ImapResponse &&other) noexcept = default;
    ~ImapResponse() noexcept = default;

    void swap(ImapResponse &other) noexcept { data.swap(other.data); }

    [[nodiscard]] Type type() const noexcept;

    [[nodiscard]] QString statusLine() const noexcept;

    [[nodiscard]] QStringList lines() const noexcept;

    [[nodiscard]] ImapError error() const noexcept;

    [[nodiscard]] bool hasError() const noexcept { return data->error.isError(); }

    explicit operator bool() const noexcept { return !hasError(); }

private:
    class Data : public QSharedData // NOLINT(cppcoreguidelines-special-member-functions)
    {
    public:
        Data() noexcept = default;

        Data(ImapResponse::Type _type, const ImapError &_error);
        Data(ImapResponse::Type _type, const QString &_statusLine, const QStringList &_lines);

        Data(const Data &) noexcept = default;
        Data &operator=(const Data &) = delete;
        ~Data() noexcept = default;

        ImapError error;
        QStringList lines;
        QString statusLine;
        ImapResponse::Type type{ImapResponse::Undefined};
    };

    QSharedDataPointer<Data> data;
};

Q_DECLARE_TYPEINFO(ImapResponse, Q_MOVABLE_TYPE); // NOLINT(modernize-type-traits)

// void swap(Skaffari::ImapResponse &a, Skaffari::ImapResponse &b) noexcept
// {
//     a.swap(b);
// }

#endif // SKAFFARI_IMAPRESPONSE_H
