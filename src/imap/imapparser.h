#ifndef IMAPPARSER_H
#define IMAPPARSER_H

#include <QVariantList>
#include <QString>

class ImapParser
{
public:
    QVariantList parse(const QString &line);

private:
    bool atListEnd();
    bool atLiteralEnd() const;
    bool hasList();
    bool hasLiteral();
    bool hasString();
    QString parseQuotedString();
    QString readLiteralPart();
    qint64 readNumber(bool *ok = nullptr);
    QVariantList readParenthesizedList();
    QString readString();
    void stripLeadingSpaces();

    QString m_line;
    qint64 m_literalSize{-1};
    int m_position{0};
    int m_size{0};
};

#endif // IMAPPARSER_H
