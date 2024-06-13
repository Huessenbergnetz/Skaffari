#include "imap/imapparser.h"

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
    void testParseGetDelimeterResponse();
    void testParseGetQuotaResponse();
    void testParseGetListResponse();

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

void ImapParserTest::testParseGetDelimeterResponse()
{
    const QVariantList parsed = m_parser.parse(QStringLiteral(R"-((\Noselect) "." "")-"));
    QCOMPARE(parsed.size(), 3);
    QCOMPARE(parsed.at(1).toString(), QStringLiteral("."));
    QVERIFY(parsed.at(2).toString().isEmpty());
}

void ImapParserTest::testParseGetQuotaResponse()
{
    const QVariantList parsed = m_parser.parse(QStringLiteral("user.joe (STORAGE 1478122 4194304)"));
    QCOMPARE(parsed.size(), 2);
    QCOMPARE(parsed.at(0).type(), QMetaType::QString);
    QCOMPARE(parsed.at(1).type(), QMetaType::QVariantList);
    const QVariantList quotaLst = parsed.at(1).toList();
    QVERIFY(quotaLst.size() % 3 == 0);
    std::pair<quint64,quint64> quota{0, 0};
    for (int i = 0; i < quotaLst.size(); ++i) {
        const auto quotaType = quotaLst.at(i).toString();
        if (quotaType.compare(QLatin1String("STORAGE"), Qt::CaseInsensitive) == 0) {
            if ((i + 2) < quotaLst.size()) {
                quota.first = quotaLst.at(i + 1).toString().toULongLong();
                quota.second = quotaLst.at(i + 2).toString().toULongLong();
                break;
            } else {
                break;
            }
        }
    }
    QCOMPARE(quota.first, Q_INT64_C(1478122));
    QCOMPARE(quota.second, Q_INT64_C(4194304));
}

void ImapParserTest::testParseGetListResponse()
{
    const QVariantList parsed = m_parser.parse(QStringLiteral("(\\HasNoChildren) \".\" user.joe.Listen.kde-announce"));
    QCOMPARE(parsed.size(), 3);
    QCOMPARE(parsed.at(0).toList().at(0).toString(), QStringLiteral(R"(\HasNoChildren)"));
    QCOMPARE(parsed.at(1).toString(), QStringLiteral("."));
    QCOMPARE(parsed.at(2).toString(), QStringLiteral("user.joe.Listen.kde-announce"));
}

QTEST_MAIN(ImapParserTest)

#include "testimapparser.moc"
