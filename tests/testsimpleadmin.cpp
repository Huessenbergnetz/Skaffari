#include "../src/objects/simpleadmin.h"

#include <QTest>
#include <QDataStream>

class SimpleAdminTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() {}

    void doTest();
    void doTest_data();
    void datastream();

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

void SimpleAdminTest::datastream()
{
    SimpleAdmin a1(3245, QStringLiteral("admin"));
    QVERIFY(a1.isValid());

    QByteArray outBa;
    QDataStream out(&outBa, QIODevice::WriteOnly);
    out << a1;

    const QByteArray inBa = outBa;
    QDataStream in(inBa);
    SimpleAdmin a2;
    in >> a2;

    QCOMPARE(a1.id(), a2.id());
    QCOMPARE(a1.name(), a2.name());
    QVERIFY(a2.isValid());
}

QTEST_MAIN(SimpleAdminTest)

#include "testsimpleadmin.moc"
