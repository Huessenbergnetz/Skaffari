#include "../src/objects/account.h"

#include <QTest>
#include <QDataStream>
#include <QJsonObject>
#include <QJsonArray>
#include <QMetaObject>
#include <QMetaProperty>

#define TA_INIT_ID static_cast<dbid_t>(1)
#define TA_INIT_DOMAINID static_cast<dbid_t>(1)
#define TA_INIT_USERNAME "test"
#define TA_INIT_QUOTA static_cast<quota_size_t>(102400)
#define TA_INIT_USAGE static_cast<quota_size_t>(10240)
#define TA_INIT_KEEPLOCAL true
#define TA_INIT_CATCHALL false
#define TA_INIT_STATUS static_cast<quint8>(0)

class AccountTest : public QObject
{
    Q_OBJECT
public:
    AccountTest(QObject *parent = nullptr) : QObject(parent) {
        qRegisterMetaType<quota_size_t>("quota_size_t");
        qRegisterMetaType<dbid_t>("dbid_t");
    }

private Q_SLOTS:
    void initTestCase() {}

    void isNotValid();
    void constructor();
    void testMove();
    void nameIdString();
    void id();
    void domainId();
    void username();
    void isImapEnabled();
    void isPopEnabled();
    void isSieveEnabled();
    void isSmtpAuthEnabled();
    void addresses();
    void forwards();
    void quota();
    void usage();
    void usagePercent();
    void created();
    void updated();
    void validUntil();
    void passwordExpires();
    void keepLocal();
    void catchAll();
    void passwordExpired();
    void expired();
    void status();
    void calcStatus();
    void calcStatus_data();
    void datastream();
    void toJson();

    void cleanupTestCase() {}

private:
    Account acc;
    const QDateTime baseDate = QDateTime::currentDateTime();
};

void AccountTest::isNotValid()
{
    QVERIFY(!acc.isValid());
}

void AccountTest::constructor()
{
    acc = Account(TA_INIT_ID, TA_INIT_DOMAINID, QStringLiteral(TA_INIT_USERNAME), true, true, true, true, QStringList(QStringLiteral("test@example.com")),  QStringList(QStringLiteral("test@example2.com")), TA_INIT_QUOTA, TA_INIT_USAGE, baseDate.addYears(-1), baseDate, baseDate.addYears(1), baseDate.addDays(182), TA_INIT_KEEPLOCAL, TA_INIT_CATCHALL, TA_INIT_STATUS);
    QVERIFY(acc.isValid());
}

void AccountTest::testMove()
{
    // Test move constructor
    {
        Account a1(1, 2, QStringLiteral("tester"), true, true, true, true, QStringList(QStringLiteral("test@example.com")),  QStringList(QStringLiteral("test@example2.com")), 123456, 1234, baseDate.addYears(-1), baseDate, baseDate.addYears(1), baseDate.addDays(182), false, false, 0);
        QCOMPARE(a1.username(), QStringLiteral("tester"));
        Account a2(std::move(a1));
        QCOMPARE(a2.username(), QStringLiteral("tester"));
    }

    // Test move assignment
    {
        Account a1(1, 2, QStringLiteral("tester"), true, true, true, true, QStringList(QStringLiteral("test@example.com")),  QStringList(QStringLiteral("test@example2.com")), 123456, 1234, baseDate.addYears(-1), baseDate, baseDate.addYears(1), baseDate.addDays(182), false, false, 0);
        QCOMPARE(a1.username(), QStringLiteral("tester"));
        Account a2(2, 2, QStringLiteral("tester2"), true, true, true, true, QStringList(QStringLiteral("test2@example.com")),  QStringList(QStringLiteral("test2@example2.com")), 123456, 1234, baseDate.addYears(-1), baseDate, baseDate.addYears(1), baseDate.addDays(182), false, false, 0);
        a2 = std::move(a1);
        QCOMPARE(a2.username(), QStringLiteral("tester"));
    }
}

void AccountTest::nameIdString()
{
    QCOMPARE(acc.nameIdString(), QStringLiteral("test (ID: 1)"));
}

void AccountTest::id()
{
    QCOMPARE(acc.id(), TA_INIT_ID);
    QCOMPARE(Account::staticMetaObject.property(Account::staticMetaObject.indexOfProperty("id")).readOnGadget(&acc).value<dbid_t>(), TA_INIT_ID);
}

void AccountTest::domainId()
{
    QCOMPARE(acc.domainId(), TA_INIT_DOMAINID);
    QCOMPARE(Account::staticMetaObject.property(Account::staticMetaObject.indexOfProperty("domainId")).readOnGadget(&acc).value<dbid_t>(), TA_INIT_DOMAINID);
}

void AccountTest::username()
{
    QCOMPARE(acc.username(), QStringLiteral(TA_INIT_USERNAME));
    QCOMPARE(Account::staticMetaObject.property(Account::staticMetaObject.indexOfProperty("username")).readOnGadget(&acc).toString(), TA_INIT_USERNAME);
}

void AccountTest::isImapEnabled()
{
    QCOMPARE(acc.isImapEnabled(), true);
    QCOMPARE(Account::staticMetaObject.property(Account::staticMetaObject.indexOfProperty("imap")).readOnGadget(&acc).toBool(), true);
}

void AccountTest::isPopEnabled()
{
    QCOMPARE(acc.isPopEnabled(), true);
    QCOMPARE(Account::staticMetaObject.property(Account::staticMetaObject.indexOfProperty("pop")).readOnGadget(&acc).toBool(), true);
}

void AccountTest::isSieveEnabled()
{
    QCOMPARE(acc.isSieveEnabled(), true);
    QCOMPARE(Account::staticMetaObject.property(Account::staticMetaObject.indexOfProperty("sieve")).readOnGadget(&acc).toBool(), true);
}

void AccountTest::isSmtpAuthEnabled()
{
    QCOMPARE(acc.isSmtpauthEnabled(), true);
    QCOMPARE(Account::staticMetaObject.property(Account::staticMetaObject.indexOfProperty("smtpauth")).readOnGadget(&acc).toBool(), true);
}

void AccountTest::addresses()
{
    const QStringList expected{QStringLiteral("test@example.com")};
    QCOMPARE(acc.addresses(), expected);
    QCOMPARE(Account::staticMetaObject.property(Account::staticMetaObject.indexOfProperty("addresses")).readOnGadget(&acc).toStringList(), expected);
}

void AccountTest::forwards()
{
    const QStringList expected{QStringLiteral("test@example2.com")};
    QCOMPARE(acc.forwards(), expected);
    QCOMPARE(Account::staticMetaObject.property(Account::staticMetaObject.indexOfProperty("forwards")).readOnGadget(&acc).toStringList(), expected);
}

void AccountTest::quota()
{
    QCOMPARE(acc.quota(), TA_INIT_QUOTA);
    QCOMPARE(Account::staticMetaObject.property(Account::staticMetaObject.indexOfProperty("quota")).readOnGadget(&acc).value<quota_size_t>(), TA_INIT_QUOTA);
}

void AccountTest::usage()
{
    QCOMPARE(acc.usage(), TA_INIT_USAGE);
    QCOMPARE(Account::staticMetaObject.property(Account::staticMetaObject.indexOfProperty("usage")).readOnGadget(&acc).value<quota_size_t>(), TA_INIT_USAGE);
}

void AccountTest::usagePercent()
{
    QCOMPARE(acc.usagePercent(), 10.0f);
    QCOMPARE(Account::staticMetaObject.property(Account::staticMetaObject.indexOfProperty("usagePercent")).readOnGadget(&acc).toFloat(), 10.0f);
}

void AccountTest::created()
{
    const QDateTime expected = baseDate.addYears(-1);
    QCOMPARE(acc.created(), expected);
    QCOMPARE(Account::staticMetaObject.property(Account::staticMetaObject.indexOfProperty("created")).readOnGadget(&acc).toDateTime(), expected);
}

void AccountTest::updated()
{
    QCOMPARE(acc.updated(), baseDate);
    QCOMPARE(Account::staticMetaObject.property(Account::staticMetaObject.indexOfProperty("updated")).readOnGadget(&acc).toDateTime(), baseDate);
}

void AccountTest::validUntil()
{
    const QDateTime expected = baseDate.addYears(1);
    QCOMPARE(acc.validUntil(), expected);
    QCOMPARE(Account::staticMetaObject.property(Account::staticMetaObject.indexOfProperty("validUntil")).readOnGadget(&acc).toDateTime(), expected);
}

void AccountTest::passwordExpires()
{
    const QDateTime expected = baseDate.addDays(182);
    QCOMPARE(acc.passwordExpires(), expected);
    QCOMPARE(Account::staticMetaObject.property(Account::staticMetaObject.indexOfProperty("passwordExpires")).readOnGadget(&acc).toDateTime(), expected);
}

void AccountTest::keepLocal()
{
    QCOMPARE(acc.keepLocal(), TA_INIT_KEEPLOCAL);
    QCOMPARE(Account::staticMetaObject.property(Account::staticMetaObject.indexOfProperty("keepLocal")).readOnGadget(&acc).toBool(), TA_INIT_KEEPLOCAL);
}

void AccountTest::catchAll()
{
    QCOMPARE(acc.catchAll(), TA_INIT_CATCHALL);
    QCOMPARE(Account::staticMetaObject.property(Account::staticMetaObject.indexOfProperty("catchAll")).readOnGadget(&acc).toBool(), TA_INIT_CATCHALL);
}

void AccountTest::passwordExpired()
{
    QCOMPARE(acc.passwordExpired(), false);
    QCOMPARE(Account::staticMetaObject.property(Account::staticMetaObject.indexOfProperty("passwordExpired")).readOnGadget(&acc).toBool(), false);
}

void AccountTest::expired()
{
    QCOMPARE(acc.expired(), false);
    QCOMPARE(Account::staticMetaObject.property(Account::staticMetaObject.indexOfProperty("expired")).readOnGadget(&acc).toBool(), false);
}

void AccountTest::status()
{
    QCOMPARE(acc.status(), TA_INIT_STATUS);
    QCOMPARE(Account::staticMetaObject.property(Account::staticMetaObject.indexOfProperty("passwordExpired")).readOnGadget(&acc).value<quint8>(), TA_INIT_STATUS);
}

void AccountTest::calcStatus()
{
    QFETCH(QDateTime, validUntil);
    QFETCH(QDateTime, pwdExpires);
    QFETCH(int, result);

    QCOMPARE(static_cast<int>(Account::calcStatus(validUntil, pwdExpires)), result);
}

void AccountTest::calcStatus_data()
{
    QTest::addColumn<QDateTime>("validUntil");
    QTest::addColumn<QDateTime>("pwdExpires");
    QTest::addColumn<int>("result");

    const auto current = QDateTime::currentDateTime();

    QTest::newRow("nothing expired") << current.addYears(1) << current.addYears(1) << 0;
    QTest::newRow("password expired") << current.addYears(1) << current.addDays(-2) << 2;
    QTest::newRow("account expired") << current.addDays(-2) << current.addYears(1) << 1;
    QTest::newRow("both expired") << current.addDays(-2) << current.addDays(-2) << 3;
}

void AccountTest::datastream()
{
    QByteArray ba;
    QDataStream out(&ba, QIODevice::WriteOnly);
    out << acc;

    const QByteArray inBa = ba;
    QDataStream in(inBa);
    Account acc2;
    in >> acc2;

    QCOMPARE(acc.id(), acc2.id());
    QCOMPARE(acc.domainId(), acc2.domainId());
    QCOMPARE(acc.username(), acc2.username());
    QCOMPARE(acc.isImapEnabled(), acc2.isImapEnabled());
    QCOMPARE(acc.isPopEnabled(), acc2.isPopEnabled());
    QCOMPARE(acc.isSieveEnabled(), acc2.isSieveEnabled());
    QCOMPARE(acc.isSmtpauthEnabled(), acc2.isSmtpauthEnabled());
    QCOMPARE(acc.addresses(), acc2.addresses());
    QCOMPARE(acc.forwards(), acc2.forwards());
    QCOMPARE(acc.quota(), acc2.quota());
    QCOMPARE(acc.usage(), acc2.usage());
    QCOMPARE(acc.created(), acc2.created());
    QCOMPARE(acc.updated(), acc2.updated());
    QCOMPARE(acc.validUntil(), acc2.validUntil());
    QCOMPARE(acc.passwordExpires(), acc2.passwordExpires());
    QCOMPARE(acc.keepLocal(), acc2.keepLocal());
    QCOMPARE(acc.catchAll(), acc2.catchAll());
    QCOMPARE(acc.status(), acc2.status());
}

void AccountTest::toJson()
{
    Account a(123,
              456,
              QStringLiteral("tester"),
              true,
              true,
              false,
              true,
              QStringList({QStringLiteral("test@example.com"), QStringLiteral("test2@example.com")}),
              QStringList({QStringLiteral("test@example2.com")}),
              123456,
              2345,
              baseDate.addYears(-1),
              baseDate,
              baseDate.addYears(1),
              baseDate.addDays(182),
              true,
              false,
              0);

    QJsonObject o;
    o.insert(QStringLiteral("id"), 123);
    o.insert(QStringLiteral("domainId"), 456);
    o.insert(QStringLiteral("username"), QStringLiteral("tester"));
    o.insert(QStringLiteral("imap"), true);
    o.insert(QStringLiteral("pop"), true);
    o.insert(QStringLiteral("sieve"), false);
    o.insert(QStringLiteral("smtpauth"), true);
    o.insert(QStringLiteral("addresses"), QJsonArray::fromStringList(QStringList({QStringLiteral("test@example.com"), QStringLiteral("test2@example.com")})));
    o.insert(QStringLiteral("forwards"), QJsonArray::fromStringList(QStringList({QStringLiteral("test@example2.com")})));
    o.insert(QStringLiteral("quota"), 123456);
    o.insert(QStringLiteral("usage"), 2345);
    o.insert(QStringLiteral("created"), baseDate.addYears(-1).toString(Qt::ISODate));
    o.insert(QStringLiteral("updated"), baseDate.toString(Qt::ISODate));
    o.insert(QStringLiteral("validUntil"), baseDate.addYears(1).toString(Qt::ISODate));
    o.insert(QStringLiteral("passwordExpires"), baseDate.addDays(182).toString(Qt::ISODate));
    o.insert(QStringLiteral("passwordExpired"), false);
    o.insert(QStringLiteral("keepLocal"), true);
    o.insert(QStringLiteral("catchAll"), false);
    o.insert(QStringLiteral("expired"), false);
    o.insert(QStringLiteral("status"), 0);

    QCOMPARE(a.toJson(), o);
}

QTEST_MAIN(AccountTest)

#include "testaccount.moc"
