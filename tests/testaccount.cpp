#include "../src/objects/account.h"

#include <QTest>
#include <QDataStream>
#include <QJsonObject>
#include <QJsonArray>

#define _INIT_ID 1
#define _INIT_DOMAINID 1
#define _INIT_USERNAME "test"
#define _INIT_QUOTA 102400
#define _INIT_USAGE 10240
#define _INIT_KEEPLOCAL true
#define _INIT_CATCHALL false
#define _INIT_STATUS 0

class AccountTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() {}

    void isValid();
    void constructor();
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

void AccountTest::isValid()
{
    QVERIFY(!acc.isValid());
}

void AccountTest::constructor()
{
    acc = Account(_INIT_ID, _INIT_DOMAINID, QStringLiteral(_INIT_USERNAME), true, true, true, true, QStringList(QStringLiteral("test@example.com")),  QStringList(QStringLiteral("test@example2.com")), _INIT_QUOTA, _INIT_USAGE, baseDate.addYears(-1), baseDate, baseDate.addYears(1), baseDate.addDays(182), _INIT_KEEPLOCAL, _INIT_CATCHALL, _INIT_STATUS);
    QVERIFY(acc.isValid());
}

void AccountTest::nameIdString()
{
    QCOMPARE(acc.nameIdString(), QStringLiteral("test (ID: 1)"));
}

void AccountTest::id()
{
    QCOMPARE(acc.id(), _INIT_ID);
}

void AccountTest::domainId()
{
    QCOMPARE(acc.domainId(), _INIT_DOMAINID);
}

void AccountTest::username()
{
    QCOMPARE(acc.username(), QStringLiteral(_INIT_USERNAME));
}

void AccountTest::isImapEnabled()
{
    QCOMPARE(acc.isImapEnabled(), true);
}

void AccountTest::isPopEnabled()
{
    QCOMPARE(acc.isPopEnabled(), true);
}

void AccountTest::isSieveEnabled()
{
    QCOMPARE(acc.isSieveEnabled(), true);
}

void AccountTest::isSmtpAuthEnabled()
{
    QCOMPARE(acc.isSmtpauthEnabled(), true);
}

void AccountTest::addresses()
{
    QCOMPARE(acc.addresses(), QStringList(QStringLiteral("test@example.com")));
}

void AccountTest::forwards()
{
    QCOMPARE(acc.forwards(), QStringList(QStringLiteral("test@example2.com")));
}

void AccountTest::quota()
{
    QCOMPARE(acc.quota(), _INIT_QUOTA);
}

void AccountTest::usage()
{
    QCOMPARE(acc.usage(), _INIT_USAGE);
}

void AccountTest::usagePercent()
{
    QCOMPARE(acc.usagePercent(), 10.0);
}

void AccountTest::created()
{
    QCOMPARE(acc.created(), baseDate.addYears(-1));
}

void AccountTest::updated()
{
    QCOMPARE(acc.updated(), baseDate);
}

void AccountTest::validUntil()
{
    QCOMPARE(acc.validUntil(), baseDate.addYears(1));
}

void AccountTest::passwordExpires()
{
    QCOMPARE(acc.passwordExpires(), baseDate.addDays(182));
}

void AccountTest::keepLocal()
{
    QCOMPARE(acc.keepLocal(), _INIT_KEEPLOCAL);
}

void AccountTest::catchAll()
{
    QCOMPARE(acc.catchAll(), _INIT_CATCHALL);
}

void AccountTest::passwordExpired()
{
    QCOMPARE(acc.passwordExpired(), false);
}

void AccountTest::expired()
{
    QCOMPARE(acc.expired(), false);
}

void AccountTest::status()
{
    QCOMPARE(acc.status(), _INIT_STATUS);
}

void AccountTest::calcStatus()
{
    QFETCH(QDateTime, validUntil);
    QFETCH(QDateTime, pwdExpires);
    QFETCH(int, result);

    QCOMPARE(Account::calcStatus(validUntil, pwdExpires), result);
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
