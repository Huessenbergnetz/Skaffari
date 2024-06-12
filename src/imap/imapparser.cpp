#include "imapparser.h"

QVariantList ImapParser::parse(const QString &line)
{
    QVariantList lst;

    m_line = line;
    m_position = 0;
    m_size = m_line.size();

    while (m_position < m_size) {
        if (hasString()) {
            const QString str = readString();
            if (str == QLatin1String("NIL")) {
                lst << QString();
            } else {
                lst << str;
            }
        } else if (hasList()) {
            const QVariantList lst2 = readParenthesizedList();
            lst << QVariant::fromValue(lst2);
        } else if (hasLiteral()) {
            QString literal;
            while (!atLiteralEnd()) {
                literal += readLiteralPart();
            }
            lst << literal;
        }
    }

    return lst;
}

bool ImapParser::atListEnd()
{
    const int savedPos = m_position;
    stripLeadingSpaces();
    const int pos = m_position;
    m_position = savedPos;
    if (m_line.at(pos) == QLatin1Char(')')) {
        m_position = pos + 1;
        return true;
    }
    return false;
}

bool ImapParser::atLiteralEnd() const
{
    return m_literalSize == 0;
}

bool ImapParser::hasList()
{
    const int savedPos = m_position;
    stripLeadingSpaces();
    const int pos = m_position;
    m_position = savedPos;
    if (m_line.at(pos) == QLatin1Char('(')) {
        return true;
    }

    return false;
}

bool ImapParser::hasLiteral()
{
    int savedPos = m_position;
    stripLeadingSpaces();
    if (m_line.at(m_position) == QLatin1Char('{')) {
        const int end = m_line.indexOf(QLatin1Char('}'), m_position);
        m_literalSize = m_line.mid(m_position + 1, end - m_position -1).toLongLong();
        m_position = end + 1;
        return true;
    } else {
        m_position = savedPos;
        return false;
    }
}

bool ImapParser::hasString()
{
    int savedPos = m_position;
    stripLeadingSpaces();
    int pos = m_position;
    m_position = savedPos;
    const QChar dataChar = m_line.at(pos);
    if (dataChar == QLatin1Char('{')) {
        return true; // literal string
    } else if (dataChar == QLatin1Char('"')) {
        return true; // quoted stsring
    } else if (dataChar != QLatin1Char(' ') &&
               dataChar != QLatin1Char('(') &&
               dataChar != QLatin1Char(')') &&
               dataChar != QLatin1Char('[') &&
               dataChar != QLatin1Char(']') &&
               dataChar != QLatin1Char('\n') &&
               dataChar != QLatin1Char('\r')) {
        return true; // unquoted string
    }

    return false; // something else, not a string
}

QString ImapParser::parseQuotedString()
{
    QString result;

    stripLeadingSpaces();

    int end = m_position;
    bool foundSlash = false;

    // quoted string
    if (m_line.at(m_position) == QLatin1Char('"')) {
        ++m_position;
        int i = m_position;
        for (;;) {
            if (m_line.at(i) == QLatin1Char('\\')) {
                i += 2;
                foundSlash = true;
                continue;
            }
            if (m_line.at(i) == QLatin1Char('"')) {
                result = m_line.mid(m_position, i - m_position);
                end = i + 1; // skip the '"'
                break;
            }
            ++i;
        }
    }

    // unquoted string
    else {
        bool reachedInputEnd = true;
        int i = m_position;
        for (;;) {
            const QChar ch = m_line.at(i);
            if (ch == QLatin1Char(' ') ||
                ch == QLatin1Char('(') ||
                ch == QLatin1Char(')') ||
                ch == QLatin1Char('[') ||
                ch == QLatin1Char(']') ||
                ch == QLatin1Char('\n') ||
                ch == QLatin1Char('\r') ||
                ch == QLatin1Char('"')) {

                end = i;
                reachedInputEnd = false;
                break;
            }
            if (ch == QLatin1Char('\\')) {
                foundSlash = true;
            }
            i++;
        }
        if (reachedInputEnd) {
            end = m_size;
        }

        result = m_line.mid(m_position, end - m_position);
    }

    // strip quotes
    if (foundSlash) {
        while (result.contains(QLatin1String("\\\""))) {
            result.replace(QLatin1String("\\\""), QStringLiteral("\""));
        }
        while (result.contains(QLatin1String("\\\\"))) {
            result.replace(QLatin1String("\\\\"), QStringLiteral("\\"));
        }
    }

    m_position = end;
    return result;
}

qint64 ImapParser::readNumber(bool *ok)
{
    qint64 result{0};
    if (ok) {
        *ok = false;
    }
    stripLeadingSpaces();
    int i = m_position;
    for (;;) {
        if (!m_line.at(i).isDigit()) {
            break;
        }
        ++i;
    }

    const QString tmp = m_line.mid(m_position, i - m_position);
    result = tmp.toLongLong(ok);
    m_position = i;
    return result;
}

QVariantList ImapParser::readParenthesizedList()
{
    QVariantList result;

    stripLeadingSpaces();
    if (m_line.at(m_position) != QLatin1Char('(')) {
        return result; // no list found
    }

    bool concatToLast = false;
    int count = 0;
    int sublistbegin = m_position;
    int i = m_position + 1;

    for (;;) {
        const QChar ch = m_line.at(i);

        if (ch == QLatin1Char('(')) {
            ++count;
            if (count == 1) {
                sublistbegin = i;
            }
            ++i;
            continue;
        }

        if (ch == QLatin1Char(')')) {
            if (count <= 0) {
                m_position = i + 1;
                return result;
            }
            if (count == 1) {
                result.append(m_line.mid(sublistbegin, i - sublistbegin + 1));
            }
            --count;
            ++i;
            continue;
        }

        if (ch == QLatin1Char(' ')) {
            ++i;
            continue;
        }

        if (ch == QLatin1Char('"')) {
            if (count > 0) {
                m_position = i;
                parseQuotedString();
                i = m_position;
                continue;
            }
        }

        if (ch == QLatin1Char('[')) {
            concatToLast = true;
            if (result.isEmpty()) {
                result.append(QStringLiteral("["));
            } else {
                const QString tmp = result.last().toString() + QLatin1Char('[');
                result.append(tmp);
            }
            ++i;
            continue;
        }

        if (ch == QLatin1Char(']')) {
            concatToLast = false;
            const QString tmp = result.last().toString() + QLatin1Char(']');
            result.replace(result.size() - 1, tmp);
            ++i;
            continue;
        }

        if (count == 0) {
            m_position = i;
            QString str;
            if (hasLiteral()) {
                while (!atLiteralEnd()) {
                    str += readLiteralPart();
                }
            } else {
                str = readString();
            }

            while ((m_position < m_size) && (m_line.at(m_position) == QLatin1Char('\r') || m_line.at(m_position) == QLatin1Char('\n'))) {
                m_position++;
            }

            i = m_position - 1;
            if (concatToLast) {
                const QString tmp = result.last().toString() + str;
                result.replace(result.size() - 1, tmp);
            } else {
                result.append(str);
            }
        }
        ++i;
    }
}

QString ImapParser::readString()
{
    QString result;
    stripLeadingSpaces();

    if (hasLiteral()) {
        while (!atLiteralEnd()) {
            result += readLiteralPart();
        }
        return result;
    }

    return parseQuotedString();
}

QString ImapParser::readLiteralPart()
{
    static const qint64 maxLiteralPartSize{4096};
    int size = std::min(maxLiteralPartSize, m_literalSize);

    const QString result = m_line.mid(m_position, size);
    m_position += size;
    m_literalSize -= size;
    return result;
}

void ImapParser::stripLeadingSpaces()
{
    for (int i = m_position; i < m_size; ++i) {
        if (m_line.at(i) != QLatin1Char(' ')) {
            m_position = i;
            return;
        }
    }
    m_position = m_size;
}
