#include "../src/objects/simpleadmin.h"

#include <QTest>
#include <QDataStream>
#include <QMetaObject>
#include <QMetaProperty>

class SimpleAdminTest : public QObject
{
    Q_OBJECT
public:
    SimpleAdminTest(QObject *parent = nullptr) : QObject(parent) {
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

void SimpleAdminTest::doTest()
{
    QFETCH(dbid_t, id);
    QFETCH(QString, name);
    QFETCH(bool, valid);

    SimpleAdmin a(id, name);

    QCOMPARE(a.id(), id);
    QCOMPARE(SimpleAdmin::staticMetaObject.property(SimpleAdmin::staticMetaObject.indexOfProperty("id")).readOnGadget(&a).value<dbid_t>(), id);
    QCOMPARE(a.name(), name);
    QCOMPARE(SimpleAdmin::staticMetaObject.property(SimpleAdmin::staticMetaObject.indexOfProperty("name")).readOnGadget(&a).toString(), name);
    QCOMPARE(a.isValid(), valid);
}

void SimpleAdminTest::doTest_data()
{
    QTest::addColumn<dbid_t>("id");
    QTest::addColumn<QString>("name");
    QTest::addColumn<bool>("valid");

    QTest::newRow("test-00") << static_cast<dbid_t>(1) << QStringLiteral("admin") << true;
    QTest::newRow("test-01") << static_cast<dbid_t>(0) << QStringLiteral("admin") << false;
    QTest::newRow("test-02") << static_cast<dbid_t>(1) << QString() << false;
}

void SimpleAdminTest::testMove()
{
    // Test move constructor
    {
        SimpleAdmin a1{1, QStringLiteral("admin")};
        QCOMPARE(a1.name(), QStringLiteral("admin"));
        SimpleAdmin a2(std::move(a1));
        QCOMPARE(a2.name(), QStringLiteral("admin"));
    }

    // Test move assignment
    {
        SimpleAdmin a1{1, QStringLiteral("admin")};
        QCOMPARE(a1.name(), QStringLiteral("admin"));
        SimpleAdmin a2{2, QStringLiteral("admin2")};
        a2 = std::move(a1);
        QCOMPARE(a2.name(), QStringLiteral("admin"));
    }
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
