#include "../src/objects/emailaddress.h"

#include <QTest>

class EmailAddressTest : public QObject
{
    Q_OBJECT
public:
    EmailAddressTest(QObject *parent = nullptr) : QObject(parent) {}

private Q_SLOTS:
    void initTestCase() {}

    void doTest();
    void doTest_data();

    void cleanupTestCase() {}
};

void EmailAddressTest::doTest()
{
    QFETCH(dbid_t, id);
    QFETCH(dbid_t, aceId);
    QFETCH(QString, address);
    QFETCH(QString, full);
    QFETCH(QString, local);
    QFETCH(QString, domain);
    QFETCH(bool, isIdn);
    QFETCH(bool, isValid);

    EmailAddress a(id, aceId, address);

    QCOMPARE(a.id(), id);
    QCOMPARE(a.aceId(), aceId);
    QCOMPARE(a.name(), full);
    QCOMPARE(a.localPart(), local);
    QCOMPARE(a.domainPart(), domain);
    QCOMPARE(a.isIdn(), isIdn);
    QCOMPARE(a.isValid(), isValid);
}

void EmailAddressTest::doTest_data()
{
    QTest::addColumn<dbid_t>("id");
    QTest::addColumn<dbid_t>("aceId");
    QTest::addColumn<QString>("address");
    QTest::addColumn<QString>("full");
    QTest::addColumn<QString>("local");
    QTest::addColumn<QString>("domain");
    QTest::addColumn<bool>("isIdn");
    QTest::addColumn<bool>("isValid");

    QTest::newRow("test-00") << static_cast<dbid_t>(1) << static_cast<dbid_t>(0) << QStringLiteral("test@example.com") << QStringLiteral("test@example.com") << QStringLiteral("test") << QStringLiteral("example.com") << false << true;
    QTest::newRow("test-01") << static_cast<dbid_t>(0) << static_cast<dbid_t>(0) << QStringLiteral("test@example.com") << QStringLiteral("test@example.com") << QStringLiteral("test") << QStringLiteral("example.com") << false << false;
    QTest::newRow("test-02") << static_cast<dbid_t>(1) << static_cast<dbid_t>(1) << QStringLiteral("test@example.com") << QStringLiteral("test@example.com") << QStringLiteral("test") << QStringLiteral("example.com") << true << true;
    QTest::newRow("test-03") << static_cast<dbid_t>(1) << static_cast<dbid_t>(0) << QStringLiteral("@example.com") << QStringLiteral("@") << QString() << QString() << false << false;
    QTest::newRow("test-04") << static_cast<dbid_t>(1) << static_cast<dbid_t>(0) << QStringLiteral("test@") << QStringLiteral("@") << QString() << QString() << false << false;
    QTest::newRow("test-05") << static_cast<dbid_t>(1) << static_cast<dbid_t>(0) << QStringLiteral("test") << QStringLiteral("@") << QString() << QString() << false << false;
}

QTEST_MAIN(EmailAddressTest)

#include "testemailaddress.moc"
