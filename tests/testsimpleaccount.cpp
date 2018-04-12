#include "../src/objects/simpleaccount.h"

#include <QTest>
#include <QDataStream>

class SimpleAccountTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() {}

    void doTest();
    void doTest_data();

    void datastream();

    void cleanupTestCase() {}
};

void SimpleAccountTest::doTest()
{
    QFETCH(int, id);
    QFETCH(QString, user);
    QFETCH(QString, domain);
    QFETCH(bool, valid);

    SimpleAccount a(id, user, domain);

    QCOMPARE(a.id(), id);
    QCOMPARE(a.username(), user);
    QCOMPARE(a.domainname(), domain);
    QCOMPARE(a.isValid(), valid);
}

void SimpleAccountTest::doTest_data()
{
    QTest::addColumn<int>("id");
    QTest::addColumn<QString>("user");
    QTest::addColumn<QString>("domain");
    QTest::addColumn<bool>("valid");

    QTest::newRow("test-00") << 1 << QStringLiteral("tester") << QStringLiteral("example.com") << true;
    QTest::newRow("test-01") << 0 << QStringLiteral("tester") << QStringLiteral("example.com") << false;
    QTest::newRow("test-02") << 1 << QString() << QStringLiteral("example.com") << false;
    QTest::newRow("test-03") << 1 << QStringLiteral("tester") << QString() << false;
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
