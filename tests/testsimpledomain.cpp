#include "../src/objects/simpledomain.h"

#include <QTest>
#include <QDataStream>
#include <QMetaObject>
#include <QMetaProperty>

class SimpleDomainTest : public QObject
{
    Q_OBJECT
public:
    SimpleDomainTest(QObject *parent = nullptr) : QObject(parent) {
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

void SimpleDomainTest::doTest()
{
    QFETCH(dbid_t, id);
    QFETCH(QString, name);
    QFETCH(QString, nameId);
    QFETCH(bool, valid);

    SimpleDomain d(id, name);

    QCOMPARE(d.id(), id);
    QCOMPARE(SimpleDomain::staticMetaObject.property(SimpleDomain::staticMetaObject.indexOfProperty("id")).readOnGadget(&d).value<dbid_t>(), id);
    QCOMPARE(d.name(), name);
    QCOMPARE(SimpleDomain::staticMetaObject.property(SimpleDomain::staticMetaObject.indexOfProperty("name")).readOnGadget(&d).toString(), name);
    QCOMPARE(d.nameIdString(), nameId);
    QCOMPARE(d.isValid(), valid);
}

void SimpleDomainTest::doTest_data()
{
    QTest::addColumn<dbid_t>("id");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("nameId");
    QTest::addColumn<bool>("valid");

    QTest::newRow("test-00") << static_cast<dbid_t>(1) << QStringLiteral("example.com") << QStringLiteral("example.com (ID: 1)") << true;
    QTest::newRow("test-01") << static_cast<dbid_t>(0) << QStringLiteral("example.com") << QStringLiteral("example.com (ID: 0)") << false;
    QTest::newRow("test-02") << static_cast<dbid_t>(1) << QString() << QStringLiteral(" (ID: 1)") << false;
}

void SimpleDomainTest::testMove()
{
    // Test move constructor
    {
        SimpleDomain d1{1, QStringLiteral("example.com")};
        QCOMPARE(d1.name(), QStringLiteral("example.com"));
        SimpleDomain d2(std::move(d1));
        QCOMPARE(d2.name(), QStringLiteral("example.com"));
    }

    // Test move assignment
    {
        SimpleDomain d1{1, QStringLiteral("example.com")};
        QCOMPARE(d1.name(), QStringLiteral("example.com"));
        SimpleDomain d2{2, QStringLiteral("example.net")};
        d2 = std::move(d1);
        QCOMPARE(d2.name(), QStringLiteral("example.com"));
    }
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
