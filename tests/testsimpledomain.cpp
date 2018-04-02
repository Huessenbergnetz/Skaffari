#include "../src/objects/simpledomain.h"

#include <QTest>

class SimpleDomainTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() {}

    void doTest();
    void doTest_data();

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

QTEST_MAIN(SimpleDomainTest)

#include "testsimpledomain.moc"
