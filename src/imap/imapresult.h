/*
 * SPDX-FileCopyrightText: (C) 2024 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef SKAFFARI_IMAPRESULT_H
#define SKAFFARI_IMAPRESULT_H

#include "imap/imaperror.h"

#include <QSharedData>

namespace Skaffari {

class ImapResult {
public:
    enum Type : int {
        OK,
        NO,
        BAD,
        Undefined
    };

    ImapResult() noexcept = default;

    ImapResult(Type type, const ImapError &error);
    ImapResult(Type type, const QString &statusLine, const QStringList &lines = {});

    ImapResult(const ImapResult &other) noexcept = default;
    ImapResult(ImapResult &&other) noexcept = default;
    ImapResult &operator=(const ImapResult &other) noexcept = default;
    ImapResult &operator=(ImapResult &&other) noexcept = default;
    ~ImapResult() noexcept = default;

    void swap(ImapResult &other) noexcept { data.swap(other.data); }

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

        Data(ImapResult::Type _type, const ImapError &_error);
        Data(ImapResult::Type _type, const QString &_statusLine, const QStringList &_lines);

        Data(const Data &) noexcept = default;
        Data &operator=(const Data &) = delete;
        ~Data() noexcept = default;

        ImapError error;
        QStringList lines;
        QString statusLine;
        ImapResult::Type type{ImapResult::Undefined};
    };

    QSharedDataPointer<Data> data;
};

}

Q_DECLARE_TYPEINFO(Skaffari::ImapResult, Q_MOVABLE_TYPE); // NOLINT(modernize-type-traits)

// void swap(Skaffari::ImapResult &a, Skaffari::ImapResult &b) noexcept
// {
//     a.swap(b);
// }

#endif // SKAFFARI_IMAPRESULT_H
