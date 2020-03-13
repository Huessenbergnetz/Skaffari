#include "../src/objects/simpleaccount.h"

#include <QTest>
#include <QDataStream>
#include <QMetaObject>
#include <QMetaProperty>

class SimpleAccountTest : public QObject
{
    Q_OBJECT
public:
    SimpleAccountTest(QObject *parent = nullptr) : QObject(parent) {
        qRegisterMetaType<dbid_t>("dbid_t");
    }

private Q_SLOTS:
    void initTestCase() {}

    void doTest();
    void doTest_data();
    void testMove();

    void datastream();

    void cleanupTestCase() {}
};

void SimpleAccountTest::doTest()
{
    QFETCH(dbid_t, id);
    QFETCH(QString, user);
    QFETCH(QString, domain);
    QFETCH(bool, valid);

    SimpleAccount a(id, user, domain);

    QCOMPARE(a.id(), id);
    QCOMPARE(SimpleAccount::staticMetaObject.property(SimpleAccount::staticMetaObject.indexOfProperty("id")).readOnGadget(&a).value<dbid_t>(), id);
    QCOMPARE(a.username(), user);
    QCOMPARE(SimpleAccount::staticMetaObject.property(SimpleAccount::staticMetaObject.indexOfProperty("username")).readOnGadget(&a).toString(), user);
    QCOMPARE(a.domainname(), domain);
    QCOMPARE(SimpleAccount::staticMetaObject.property(SimpleAccount::staticMetaObject.indexOfProperty("domainname")).readOnGadget(&a).toString(), domain);
    QCOMPARE(a.isValid(), valid);
}

void SimpleAccountTest::doTest_data()
{
    QTest::addColumn<dbid_t>("id");
    QTest::addColumn<QString>("user");
    QTest::addColumn<QString>("domain");
    QTest::addColumn<bool>("valid");

    QTest::newRow("test-00") << static_cast<dbid_t>(1) << QStringLiteral("tester") << QStringLiteral("example.com") << true;
    QTest::newRow("test-01") << static_cast<dbid_t>(0) << QStringLiteral("tester") << QStringLiteral("example.com") << false;
    QTest::newRow("test-02") << static_cast<dbid_t>(1) << QString() << QStringLiteral("example.com") << false;
    QTest::newRow("test-03") << static_cast<dbid_t>(1) << QStringLiteral("tester") << QString() << false;
}

void SimpleAccountTest::testMove()
{
    // Test move constructor
    {
        SimpleAccount a1{1, QStringLiteral("tester"), QStringLiteral("example.com")};
        QCOMPARE(a1.username(), QStringLiteral("tester"));
        SimpleAccount a2(std::move(a1));
        QCOMPARE(a2.username(), QStringLiteral("tester"));
    }

    // Test move assignment
    {
        SimpleAccount a1{1, QStringLiteral("tester"), QStringLiteral("example.com")};
        QCOMPARE(a1.username(), QStringLiteral("tester"));
        SimpleAccount a2{2, QStringLiteral("tester2"), QStringLiteral("example.com")};
        a2 = std::move(a1);
        QCOMPARE(a2.username(), QStringLiteral("tester"));
    }
}

void SimpleAccountTest::datastream()
{
    SimpleAccount a1(123, QStringLiteral("bob"), QStringLiteral("example.net"));
    QVERIFY(a1.isValid());

    QByteArray outBa;
    QDataStream out(&outBa, QIODevice::WriteOnly);
    out << a1;

    const QByteArray inBa = outBa;
    QDataStream in(inBa);
    SimpleAccount a2;
    in >> a2;

    QVERIFY(a2.isValid());
    QCOMPARE(a1.id(), a2.id());
    QCOMPARE(a1.username(), a2.username());
    QCOMPARE(a1.domainname(), a2.domainname());
}

QTEST_MAIN(SimpleAccountTest)

#include "testsimpleaccount.moc"
