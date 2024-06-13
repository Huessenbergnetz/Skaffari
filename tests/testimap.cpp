#include "imap/imap.h"

#include <QTest>

class ImapTest : public QObject
{
    Q_OBJECT
public:
    explicit ImapTest(QObject *parent = nullptr)
        : QObject{parent}
    {}
    ~ImapTest() override = default;

private Q_SLOTS:
    void testUt7ImapConvert();
};

void ImapTest::testUt7ImapConvert()
{
    {
        const QString str = QStringLiteral("Mülleimer");
        const QString utf7 = Imap::toUtf7Imap(str);
        QVERIFY(!utf7.isEmpty());
        const QString utf8 = Imap::fromUtf7Imap(utf7);
        QVERIFY(!utf8.isEmpty());
        QCOMPARE(utf8, str);
    }

    {
        const QString str = QStringLiteral("äüöäüöäüöäüößßßß");
        const QString utf7 = Imap::toUtf7Imap(str);
        QVERIFY(!utf7.isEmpty());
        const QString utf8 = Imap::fromUtf7Imap(utf7);
        QVERIFY(!utf8.isEmpty());
        QCOMPARE(utf8, str);
    }
}

QTEST_MAIN(ImapTest)

#include "testimap.moc"
