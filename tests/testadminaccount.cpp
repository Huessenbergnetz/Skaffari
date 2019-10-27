#include "../src/objects/adminaccount.h"
#include <QTest>
#include <QObject>
#include <QDataStream>

class AdminAccountTest : public QObject
{
    Q_OBJECT
public:
    AdminAccountTest(QObject *parent = nullptr) : QObject(parent) {}

private Q_SLOTS:
    void initTestCase() {}

    void isNotValid();
    void constructor();
    void isValid();
    void id();
    void username();
    void domains();
    void type();
    void typeInt();
    void typeStr();
    void isSuperUser();
    void lang();
    void tz();
    void created();
    void updated();
    void warnLevel();
    void maxDisplay();
    void nameIdString();

    void isValidTest();
    void isValidTest_data();

    void isSuperUserTest();
    void isSuperUserTest_data();

    void allowedTypesTest();
    void allowedTypesTest_data();

    void maxAllowedTypeTest();
    void maxAllowedTypeTest_data();

    void getUserTypeByIntTest();
    void getUserTypeByIntTest_data();

    void typeIntTest();
    void typeIntTest_data();

    void datastream();

    void cleanupTestCase() {}

private:
    AdminAccount m_acc;
    const QDateTime baseTime = QDateTime::currentDateTime();
    const QList<dbid_t> domIdList {static_cast<dbid_t>(1), static_cast<dbid_t>(2), static_cast<dbid_t>(3)};
};

void AdminAccountTest::isNotValid()
{
    QVERIFY(!m_acc.isValid());
}

void AdminAccountTest::constructor()
{
    m_acc = AdminAccount(123, QStringLiteral("admin"), AdminAccount::DomainMaster, domIdList, QStringLiteral("Europe/Berlin"), QStringLiteral("de_DE"), QStringLiteral("default"), 50, 85, baseTime.addMonths(-1), baseTime);
}

void AdminAccountTest::isValid()
{
    QVERIFY(m_acc.isValid());
}

void AdminAccountTest::id()
{
    QCOMPARE(m_acc.id(), static_cast<dbid_t>(123));
}

void AdminAccountTest::username()
{
    QCOMPARE(m_acc.username(), QStringLiteral("admin"));
}

void AdminAccountTest::domains()
{
    QCOMPARE(m_acc.domains(), domIdList);
}

void AdminAccountTest::type()
{
    QCOMPARE(m_acc.type(), AdminAccount::DomainMaster);
}

void AdminAccountTest::typeInt()
{
    QCOMPARE(m_acc.typeInt(), static_cast<quint8>(127));
}

void AdminAccountTest::typeStr()
{
    QCOMPARE(m_acc.typeStr(), QStringLiteral("127"));
}

void AdminAccountTest::isSuperUser()
{
    QVERIFY(!m_acc.isSuperUser());
}

void AdminAccountTest::lang()
{
    QCOMPARE(m_acc.lang(), QStringLiteral("de_DE"));
}

void AdminAccountTest::tz()
{
    QCOMPARE(m_acc.tz(), QStringLiteral("Europe/Berlin"));
}

void AdminAccountTest::created()
{
    QCOMPARE(m_acc.created(), baseTime.addMonths(-1));
}

void AdminAccountTest::updated()
{
    QCOMPARE(m_acc.updated(), baseTime);
}

void AdminAccountTest::warnLevel()
{
    QCOMPARE(m_acc.warnLevel(), static_cast<quint8>(85));
}

void AdminAccountTest::maxDisplay()
{
    QCOMPARE(m_acc.maxDisplay(), static_cast<quint8>(50));
}

void AdminAccountTest::nameIdString()
{
    QCOMPARE(m_acc.nameIdString(), QStringLiteral("admin (ID: 123)"));
}

void AdminAccountTest::isValidTest()
{
    QFETCH(QString, username);
    QFETCH(dbid_t, id);
    QFETCH(bool, result);

    AdminAccount aa(id, username, AdminAccount::Administrator, QList<dbid_t>(), QStringLiteral("UTC"), QStringLiteral("en"), QStringLiteral("default"), 25, 90, baseTime, baseTime);
    QCOMPARE(aa.isValid(), result);
}

void AdminAccountTest::isValidTest_data()
{
    QTest::addColumn<QString>("username");
    QTest::addColumn<dbid_t>("id");
    QTest::addColumn<bool>("result");

    QTest::newRow("username and id invalid") << QString() << static_cast<dbid_t>(0) << false;
    QTest::newRow("username invalid") << QString() << static_cast<dbid_t>(12) << false;
    QTest::newRow("id invalid") << QStringLiteral("admin") << static_cast<dbid_t>(0) << false;
    QTest::newRow("username and id valid") << QStringLiteral("admin") << static_cast<dbid_t>(1) << true;
}

void AdminAccountTest::isSuperUserTest()
{
    QFETCH(AdminAccount::AdminAccountType, type);
    QFETCH(bool, result);

    AdminAccount aa(12, QStringLiteral("bigadmin"), type, QList<dbid_t>(), QStringLiteral("UTC"), QStringLiteral("en"), QStringLiteral("default"), 25, 90, baseTime, baseTime);

    QCOMPARE(aa.isSuperUser(), result);
}

void AdminAccountTest::isSuperUserTest_data()
{
    QTest::addColumn<AdminAccount::AdminAccountType>("type");
    QTest::addColumn<bool>("result");

    QTest::newRow("Disabled") << AdminAccount::Disabled << false;
    QTest::newRow("DomainMaster") << AdminAccount::DomainMaster << false;
    QTest::newRow("Administrator") << AdminAccount::Administrator << false;
    QTest::newRow("SuperUser") << AdminAccount::SuperUser << true;
}

void AdminAccountTest::allowedTypesTest()
{
    QFETCH(AdminAccount::AdminAccountType, type);
    QFETCH(QStringList, result);

    const QStringList ats = AdminAccount::allowedTypes(type);

    QCOMPARE(ats, result);
}

void AdminAccountTest::allowedTypesTest_data()
{
    QTest::addColumn<AdminAccount::AdminAccountType>("type");
    QTest::addColumn<QStringList>("result");

    QTest::newRow("Disabled") << AdminAccount::Disabled << QStringList();
    QTest::newRow("DomainMaster") << AdminAccount::DomainMaster << QStringList();
    QTest::newRow("Administrator") << AdminAccount::Administrator << QStringList(QStringLiteral("127"));
    QTest::newRow("SuperUser") << AdminAccount::SuperUser << QStringList({QStringLiteral("127"), QStringLiteral("254"), QStringLiteral("255")});
}

void AdminAccountTest::maxAllowedTypeTest()
{
    QFETCH(AdminAccount::AdminAccountType, type);
    QFETCH(AdminAccount::AdminAccountType, result);

    QCOMPARE(AdminAccount::maxAllowedType(type), result);
}

void AdminAccountTest::maxAllowedTypeTest_data()
{
    QTest::addColumn<AdminAccount::AdminAccountType>("type");
    QTest::addColumn<AdminAccount::AdminAccountType>("result");

    QTest::newRow("Disabled") << AdminAccount::Disabled << AdminAccount::Disabled;
    QTest::newRow("DomainMaster") << AdminAccount::DomainMaster << AdminAccount::Disabled;
    QTest::newRow("Administrator") << AdminAccount::Administrator << AdminAccount::DomainMaster;
    QTest::newRow("SuperUser") << AdminAccount::SuperUser << AdminAccount::SuperUser;
}

void AdminAccountTest::getUserTypeByIntTest()
{
    QFETCH(quint8, type);
    QFETCH(AdminAccount::AdminAccountType, result);

    QCOMPARE(AdminAccount::getUserType(type), result);
}

void AdminAccountTest::getUserTypeByIntTest_data()
{
    QTest::addColumn<quint8>("type");
    QTest::addColumn<AdminAccount::AdminAccountType>("result");

    QTest::newRow("Disabled") << static_cast<quint8>(0) << AdminAccount::Disabled;
    QTest::newRow("DomainMaster") << static_cast<quint8>(127) << AdminAccount::DomainMaster;
    QTest::newRow("Administrator") << static_cast<quint8>(254) << AdminAccount::Administrator;
    QTest::newRow("SuperUser") << static_cast<quint8>(255) << AdminAccount::SuperUser;
}

void AdminAccountTest::typeIntTest()
{
    QFETCH(AdminAccount::AdminAccountType, type);
    QFETCH(quint8, result);

    AdminAccount a(1, QStringLiteral("admin"), type, QList<dbid_t>());

    QCOMPARE(a.typeInt(), result);
}

void AdminAccountTest::typeIntTest_data()
{
    QTest::addColumn<AdminAccount::AdminAccountType>("type");
    QTest::addColumn<quint8>("result");

    QTest::newRow("Disabled") << AdminAccount::Disabled << static_cast<quint8>(0);
    QTest::newRow("DomainMaster") << AdminAccount::DomainMaster << static_cast<quint8>(127);
    QTest::newRow("Administrator") << AdminAccount::Administrator << static_cast<quint8>(254);
    QTest::newRow("SuperUser") << AdminAccount::SuperUser << static_cast<quint8>(255);
}

void AdminAccountTest::datastream()
{
    QVERIFY(m_acc.isValid());

    QByteArray outBa;
    QDataStream out(&outBa, QIODevice::WriteOnly);
    out << m_acc;

    const QByteArray inBa = outBa;
    QDataStream in(inBa);
    AdminAccount a2;
    in >> a2;

    QVERIFY(a2.isValid());

    QCOMPARE(m_acc.id(), a2.id());
    QCOMPARE(m_acc.username(), a2.username());
    QCOMPARE(m_acc.nameIdString(), a2.nameIdString());
    QCOMPARE(m_acc.domains(), a2.domains());
    QCOMPARE(m_acc.type(), a2.type());
    QCOMPARE(m_acc.typeInt(), a2.typeInt());
    QCOMPARE(m_acc.typeStr(), a2.typeStr());
    QCOMPARE(m_acc.isSuperUser(), a2.isSuperUser());
    QCOMPARE(m_acc.lang(), a2.lang());
    QCOMPARE(m_acc.tz(), a2.tz());
    QCOMPARE(m_acc.created(), a2.created());
    QCOMPARE(m_acc.updated(), a2.updated());
    QCOMPARE(m_acc.maxDisplay(), a2.maxDisplay());
    QCOMPARE(m_acc.warnLevel(), a2.warnLevel());
    QCOMPARE(m_acc.getTemplate(), a2.getTemplate());
}

QTEST_MAIN(AdminAccountTest)

#include "testadminaccount.moc"
