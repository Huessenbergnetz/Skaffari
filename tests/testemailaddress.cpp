#include "../src/objects/emailaddress.h"

#include <QTest>
#include <QMetaObject>
#include <QMetaProperty>

class EmailAddressTest : public QObject
{
    Q_OBJECT
public:
    EmailAddressTest(QObject *parent = nullptr) : QObject(parent) {
        qRegisterMetaType<dbid_t>("dbid_t");
    }

private Q_SLOTS:
    void initTestCase() {}

    void doTest();
    void doTest_data();
    void testMove();

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
    QCOMPARE(EmailAddress::staticMetaObject.property(EmailAddress::staticMetaObject.indexOfProperty("id")).readOnGadget(&a).value<dbid_t>(), id);
    QCOMPARE(a.aceId(), aceId);
    QCOMPARE(a.name(), full);
    QCOMPARE(EmailAddress::staticMetaObject.property(EmailAddress::staticMetaObject.indexOfProperty("name")).readOnGadget(&a).toString(), full);
    QCOMPARE(a.localPart(), local);
    QCOMPARE(a.domainPart(), domain);
    QCOMPARE(a.isIdn(), isIdn);
    QCOMPARE(EmailAddress::staticMetaObject.property(EmailAddress::staticMetaObject.indexOfProperty("isIdn")).readOnGadget(&a).toBool(), isIdn);
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

void EmailAddressTest::testMove()
{
    // Test move constructor
    {
        EmailAddress a1{1, 2, QStringLiteral("test@example.com")};
        QCOMPARE(a1.name(), QStringLiteral("test@example.com"));
        EmailAddress a2(std::move(a1));
        QCOMPARE(a2.name(), QStringLiteral("test@example.com"));
    }

    // Test move assignment
    {
        EmailAddress a1{1, 2, QStringLiteral("test@example.com")};
        QCOMPARE(a1.name(), QStringLiteral("test@example.com"));
        EmailAddress a2{3, 4, QStringLiteral("test2@example.com")};
        a2 = std::move(a1);
        QCOMPARE(a2.name(), QStringLiteral("test@example.com"));
    }
}

QTEST_MAIN(EmailAddressTest)

#include "testemailaddress.moc"
