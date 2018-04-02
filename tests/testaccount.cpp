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
    void getSetId();
    void getSetDomainId();
    void getSetUsername();
    void getSetIsImapEnabled();
    void getSetIsPopEnabled();
    void getSetIsSieveEnabeld();
    void getSetIsSmtpauthEnabled();
    void getSetAddresses();
    void getSetForwards();
    void getSetQuota();
    void getSetUsage();
    void usagePercent();
    void getSetCreated();
    void getSetUpdated();
    void getSetValidUntil();
    void getSetPasswordExpires();
    void getSetKeepLocal();
    void getSetCatchAll();
    void passwordExpired();
    void expired();
    void getSetStatus();
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
    acc = Account(_INIT_ID, _INIT_DOMAINID, QStringLiteral(_INIT_USERNAME), true, true, true, true, QStringList(), QStringList(), _INIT_QUOTA, _INIT_USAGE, baseDate.addYears(-1), baseDate, baseDate.addYears(1), baseDate.addDays(182), _INIT_KEEPLOCAL, _INIT_CATCHALL, _INIT_STATUS);
    QVERIFY(acc.isValid());
}

void AccountTest::nameIdString()
{
    QCOMPARE(acc.nameIdString(), QStringLiteral("test (ID: 1)"));
}

void AccountTest::getSetId()
{
    QCOMPARE(acc.id(), _INIT_ID);
    acc.setId(_INIT_ID  + 1);
    QCOMPARE(acc.id(), _INIT_ID + 1);
}

void AccountTest::getSetDomainId()
{
    QCOMPARE(acc.domainId(), _INIT_DOMAINID);
    acc.setDomainId(_INIT_DOMAINID + 1);
    QCOMPARE(acc.domainId(), _INIT_DOMAINID + 1);
}

void AccountTest::getSetUsername()
{
    QCOMPARE(acc.username(), QStringLiteral(_INIT_USERNAME));
    const auto newUsername = QStringLiteral("test2");
    acc.setUsername(newUsername);
    QCOMPARE(acc.username(), newUsername);
}

void AccountTest::getSetIsImapEnabled()
{
    QCOMPARE(acc.isImapEnabled(), true);
    acc.setImapEnabled(false);
    QCOMPARE(acc.isImapEnabled(), false);
}

void AccountTest::getSetIsPopEnabled()
{
    QCOMPARE(acc.isPopEnabled(), true);
    acc.setPopEnabled(false);
    QCOMPARE(acc.isPopEnabled(), false);
}

void AccountTest::getSetIsSieveEnabeld()
{
    QCOMPARE(acc.isSieveEnabled(), true);
    acc.setSieveEnabled(false);
    QCOMPARE(acc.isSieveEnabled(), false);
}

void AccountTest::getSetIsSmtpauthEnabled()
{
    QCOMPARE(acc.isSmtpauthEnabled(), true);
    acc.setSmtpauthEnabled(false);
    QCOMPARE(acc.isSmtpauthEnabled(), false);
}

void AccountTest::getSetAddresses()
{
    QCOMPARE(acc.addresses(), QStringList());
    const auto newAddresses = QStringList({QStringLiteral("test@example.com"), QStringLiteral("test2@example.com")});
    acc.setAddresses(newAddresses);
    QCOMPARE(acc.addresses(), newAddresses);
}

void AccountTest::getSetForwards()
{
    QCOMPARE(acc.forwards(), QStringList());
    const auto newForwards = QStringList({QStringLiteral("test@example2.com"), QStringLiteral("test2@example2.com")});
    acc.setForwards(newForwards);
    QCOMPARE(acc.forwards(), newForwards);
}

void AccountTest::getSetQuota()
{
    QCOMPARE(acc.quota(), _INIT_QUOTA);
    const auto newQuota = _INIT_QUOTA * 2;
    acc.setQuota(newQuota);
    QCOMPARE(acc.quota(), newQuota);
}

void AccountTest::getSetUsage()
{
    QCOMPARE(acc.usage(), _INIT_USAGE);
    const auto newUsage = _INIT_USAGE * 2;
    acc.setUsage(newUsage);
    QCOMPARE(acc.usage(), newUsage);
}

void AccountTest::usagePercent()
{
    QCOMPARE(acc.usagePercent(), 10.0);
}

void AccountTest::getSetCreated()
{
    QCOMPARE(acc.created(), baseDate.addYears(-1));
    const auto newCreated = acc.created().addYears(1);
    acc.setCreated(newCreated);
    QCOMPARE(acc.created(), newCreated);
}

void AccountTest::getSetUpdated()
{
    QCOMPARE(acc.updated(), baseDate);
    const auto newUpdated = acc.updated().addYears(1);
    acc.setUpdated(newUpdated);
    QCOMPARE(acc.updated(), newUpdated);
}

void AccountTest::getSetValidUntil()
{
    QCOMPARE(acc.validUntil(), baseDate.addYears(1));
    const auto newValidUntil = acc.validUntil().addYears(1);
    acc.setValidUntil(newValidUntil);
    QCOMPARE(acc.validUntil(), newValidUntil);
}

void AccountTest::getSetPasswordExpires()
{
    QCOMPARE(acc.passwordExpires(), baseDate.addDays(182));
    const auto newPasswordExpires = acc.passwordExpires().addYears(1);
    acc.setPasswordExpires(newPasswordExpires);
    QCOMPARE(acc.passwordExpires(), newPasswordExpires);
}

void AccountTest::getSetKeepLocal()
{
    QCOMPARE(acc.keepLocal(), _INIT_KEEPLOCAL);
    acc.setKeepLocal(!_INIT_KEEPLOCAL);
    QCOMPARE(acc.keepLocal(), !_INIT_KEEPLOCAL);
}

void AccountTest::getSetCatchAll()
{
    QCOMPARE(acc.catchAll(), _INIT_CATCHALL);
    acc.setCatchAll(!_INIT_CATCHALL);
    QCOMPARE(acc.catchAll(), !_INIT_CATCHALL);
}

void AccountTest::passwordExpired()
{
    QCOMPARE(acc.passwordExpired(), false);
    acc.setPasswordExpires(baseDate.addDays(-1));
    QCOMPARE(acc.passwordExpired(), true);
}

void AccountTest::expired()
{
    QCOMPARE(acc.expired(), false);
    acc.setValidUntil(baseDate.addDays(-1));
    QCOMPARE(acc.expired(), true);
}

void AccountTest::getSetStatus()
{
    QCOMPARE(acc.status(), _INIT_STATUS);
    acc.setStatus(Account::calcStatus(acc.validUntil(), acc.passwordExpires()));
    QCOMPARE(acc.status(), 3);
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
    Account a;
    QJsonObject o;

    a.setId(123);
    o.insert(QStringLiteral("id"), 123);

    a.setDomainId(456);
    o.insert(QStringLiteral("domainId"), 456);

    a.setUsername(QStringLiteral("tester"));
    o.insert(QStringLiteral("username"), QStringLiteral("tester"));

    a.setImapEnabled(true);
    o.insert(QStringLiteral("imap"), true);

    a.setPopEnabled(true);
    o.insert(QStringLiteral("pop"), true);

    a.setSmtpauthEnabled(true);
    o.insert(QStringLiteral("smtpauth"), true);

    a.setSieveEnabled(false);
    o.insert(QStringLiteral("sieve"), false);

    a.setAddresses(QStringList({QStringLiteral("test@example.com"), QStringLiteral("test2@example.com")}));
    o.insert(QStringLiteral("addresses"), QJsonArray::fromStringList(QStringList({QStringLiteral("test@example.com"), QStringLiteral("test2@example.com")})));

    a.setForwards(QStringList({QStringLiteral("test@example2.com")}));
    o.insert(QStringLiteral("forwards"), QJsonArray::fromStringList(QStringList({QStringLiteral("test@example2.com")})));

    a.setQuota(123456);
    o.insert(QStringLiteral("quota"), 123456);

    a.setUsage(2345);
    o.insert(QStringLiteral("usage"), 2345);

    a.setCreated(baseDate.addYears(-1));
    o.insert(QStringLiteral("created"), baseDate.addYears(-1).toString(Qt::ISODate));

    a.setUpdated(baseDate);
    o.insert(QStringLiteral("updated"), baseDate.toString(Qt::ISODate));

    a.setValidUntil(baseDate.addYears(1));
    o.insert(QStringLiteral("validUntil"), baseDate.addYears(1).toString(Qt::ISODate));

    a.setPasswordExpires(baseDate.addDays(182));
    o.insert(QStringLiteral("passwordExpires"), baseDate.addDays(182).toString(Qt::ISODate));
    o.insert(QStringLiteral("passwordExpired"), false);

    a.setKeepLocal(true);
    o.insert(QStringLiteral("keepLocal"), true);

    a.setCatchAll(false);
    o.insert(QStringLiteral("catchAll"), false);

    o.insert(QStringLiteral("expired"), false);
    o.insert(QStringLiteral("status"), 0);

    QCOMPARE(a.toJson(), o);
}

QTEST_MAIN(AccountTest)

#include "testaccount.moc"
