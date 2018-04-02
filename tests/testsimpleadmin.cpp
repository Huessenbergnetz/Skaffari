#include "../src/objects/simpleadmin.h"

#include <QTest>

class SimpleAdminTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() {}

    void doTest();
    void doTest_data();

    void cleanupTestCase() {}
};

void SimpleAdminTest::doTest()
{
    QFETCH(int, id);
    QFETCH(QString, name);
    QFETCH(bool, valid);

    SimpleAdmin a(id, name);

    QCOMPARE(a.id(), id);
    QCOMPARE(a.name(), name);
    QCOMPARE(a.isValid(), valid);
}

void SimpleAdminTest::doTest_data()
{
    QTest::addColumn<int>("id");
    QTest::addColumn<QString>("name");
    QTest::addColumn<bool>("valid");

    QTest::newRow("test-00") << 1 << QStringLiteral("admin") << true;
    QTest::newRow("test-01") << 0 << QStringLiteral("admin") << false;
    QTest::newRow("test-02") << 1 << QString() << false;
}

QTEST_MAIN(SimpleAdminTest)

#include "testsimpleadmin.moc"
