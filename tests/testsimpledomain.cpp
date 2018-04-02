#include "../src/objects/simpledomain.h"

#include <QTest>
#include <QDataStream>

class SimpleDomainTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() {}

    void doTest();
    void doTest_data();
    void datastream();

    void cleanupTestCase() {}
};

void SimpleDomainTest::doTest()
{
    QFETCH(int, id);
    QFETCH(QString, name);
    QFETCH(QString, nameId);
    QFETCH(bool, valid);

    SimpleDomain d(id, name);

    QCOMPARE(d.id(), id);
    QCOMPARE(d.name(), name);
    QCOMPARE(d.nameIdString(), nameId);
    QCOMPARE(d.isValid(), valid);
}

void SimpleDomainTest::doTest_data()
{
    QTest::addColumn<int>("id");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("nameId");
    QTest::addColumn<bool>("valid");

    QTest::newRow("test-00") << 1 << QStringLiteral("example.com") << QStringLiteral("example.com (ID: 1)") << true;
    QTest::newRow("test-01") << 0 << QStringLiteral("example.com") << QStringLiteral("example.com (ID: 0)") << false;
    QTest::newRow("test-02") << 1 << QString() << QStringLiteral(" (ID: 1)") << false;
}

void SimpleDomainTest::datastream()
{
    SimpleDomain d1(123, QStringLiteral("example.com"));
    QVERIFY(d1.isValid());

    QByteArray outBa;
    QDataStream out(&outBa, QIODevice::WriteOnly);
    out << d1;

    const QByteArray inBa = outBa;
    QDataStream in(inBa);
    SimpleDomain d2;
    in >> d2;

    QCOMPARE(d1.id(), d2.id());
    QCOMPARE(d1.name(), d2.name());
    QCOMPARE(d1.nameIdString(), d2.nameIdString());
    QVERIFY(d2.isValid());
}

QTEST_MAIN(SimpleDomainTest)

#include "testsimpledomain.moc"
