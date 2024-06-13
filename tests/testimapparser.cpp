#include "../src/imap/imapparser.h"

#include <QTest>

class ImapParserTest : public QObject
{
    Q_OBJECT
public:
    explicit ImapParserTest(QObject *parent = nullptr)
        : QObject{parent}
    {}
    ~ImapParserTest() override = default;

private Q_SLOTS:
    void testParseNamespaceReponse();
    void testParseGetquotarootResponse();
    void testParseIdResponse();

private:
    ImapParser m_parser;
};

void ImapParserTest::testParseNamespaceReponse()
{
    const QVariantList parsed1 = m_parser.parse(QStringLiteral(R"-((("" "/")) (("~" "/")) (("#shared/" "/")("#public/" "/")("#ftp/" "/")("#news." ".")))-"));
    QCOMPARE(parsed1.size(), 3);
    QCOMPARE(parsed1.at(0).toList().size(), 1);
    QCOMPARE(parsed1.at(1).toList().size(), 1);
    QCOMPARE(parsed1.at(2).toList().size(), 4);

    const QVariantList parsed2 = m_parser.parse(QStringLiteral(R"-(NIL (("~" ".")) (("#shared." ".")("#public/" "/")))-"));
    QCOMPARE(parsed2.size(), 3);
    QCOMPARE(parsed2.at(0).type(), QMetaType::QString);
    QVERIFY(parsed2.at(0).toString().isNull());
    QCOMPARE(parsed2.at(1).toList().size(), 1);
    QCOMPARE(parsed2.at(2).toList().size(), 2);
}

void ImapParserTest::testParseGetquotarootResponse()
{
    const QVariantList parsed = m_parser.parse(QStringLiteral("user.test (STORAGE 1478047 4194304)"));
    QCOMPARE(parsed.size(), 2);
    QCOMPARE(parsed.at(0).type(), QMetaType::QString);
    QCOMPARE(parsed.at(0).toString(), QStringLiteral("user.test"));
    QCOMPARE(parsed.at(1).type(), QMetaType::QVariantList);
    QCOMPARE(parsed.at(1).toList().size(), 3);
    QCOMPARE(parsed.at(1).toList().at(0).toString(), QStringLiteral("STORAGE"));
}

void ImapParserTest::testParseIdResponse()
{
    const QVariantList parsed = m_parser.parse(QStringLiteral(R"-(("name" "Cyrus IMAPD" "version" "3.2.12" "vendor" "Project Cyrus"))-"));
    const QVariantList lst = parsed.at(0).toList();
    int i = 0;
    int size = lst.size();
    QMap<QString,QString> id;
    while (i < size) {
        const auto key = lst.at(i).toString();
        ++i;
        const auto value = lst.at(i).toString();
        ++i;
        id.insert(key, value);
    }
    QCOMPARE(id.value(QStringLiteral("version")), QStringLiteral("3.2.12"));
}

QTEST_MAIN(ImapParserTest)

#include "testimapparser.moc"
