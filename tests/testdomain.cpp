#include "../src/objects/domain.h"
#include "../src/objects/simpledomain.h"
#include <QTest>

class DomainTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() {}

    void isValid();
    void boolOperator();
    void constructor();
    void nameIdString();
    void getSetId();
    void getSetName();
    void getSetPrefix();
    void getSetTransport();
    void getSetQuota();
    void getSetMaxAccounts();
    void getSetDomainQuota();
    void getSetDomainQuotaUsed();
    void getSetFreeNamesEnabled();
    void getSetFreeAddressEnabled();
    void getSetAccounts();
    void getSetCreated();
    void getSetUpdated();
    void getSetValidUntil();
    void aceName();
    void aceId();
    void isIdn();
    void toSimple();
    void domainQuotaUsagePercent();
    void accountUsagePercent();

    void cleanupTestCase() {}

private:
    Domain m_dom;
};

void DomainTest::isValid()
{
    QVERIFY(!m_dom.isValid());
}

void DomainTest::boolOperator()
{

    QVERIFY(!m_dom);
}

void DomainTest::constructor()
{
    m_dom = Domain(12, QStringLiteral("example.com"), QStringLiteral("xmp"), QStringLiteral("cyrus"), 1610612736, 1000, 1610612736000, 161061273600, true, false, 100, QDateTime(QDate(2018, 3, 28), QTime(16, 0)), QDateTime(QDate(2018, 3, 28), QTime(17, 0)), QDateTime(QDate(2118, 3, 28), QTime(16, 0)));
    QVERIFY(m_dom);
}

void DomainTest::nameIdString()
{
    QCOMPARE(m_dom.nameIdString(), QStringLiteral("example.com (ID: 12)"));
}

void DomainTest::getSetId()
{
    QCOMPARE(m_dom.id(), 12);
    const auto newId = 24;
    m_dom.setId(newId);
    QCOMPARE(m_dom.id(), newId);
}

void DomainTest::getSetName()
{
    QCOMPARE(m_dom.name(), QStringLiteral("example.com"));
    const auto newName = QStringLiteral("beispiel.de");
    m_dom.setName(newName);
    QCOMPARE(m_dom.name(), QStringLiteral("beispiel.de"));
}

void DomainTest::getSetPrefix()
{
    QCOMPARE(m_dom.prefix(), QStringLiteral("xmp"));
    const auto newPrefix = QStringLiteral("bsp");
    m_dom.setPrefix(newPrefix);
    QCOMPARE(m_dom.prefix(), newPrefix);
}

void DomainTest::getSetTransport()
{
    QCOMPARE(m_dom.transport(), QStringLiteral("cyrus"));
    const auto newTransport = QStringLiteral("lmtp");
    m_dom.setTransport(newTransport);
    QCOMPARE(m_dom.transport(), newTransport);
}

void DomainTest::getSetQuota()
{
    QCOMPARE(m_dom.quota(), 1610612736);
    const auto newQuota = 805306368;
    m_dom.setQuota(newQuota);
    QCOMPARE(m_dom.quota(), newQuota);
}

void DomainTest::getSetMaxAccounts()
{
    QCOMPARE(m_dom.maxAccounts(), 1000);
    const auto newMaxAccounts = 500;
    m_dom.setMaxAccounts(newMaxAccounts);
    QCOMPARE(m_dom.maxAccounts(), newMaxAccounts);
}

void DomainTest::getSetDomainQuota()
{
    QCOMPARE(m_dom.domainQuota(), 1610612736000);
    const auto newDomainQuota = 805306368000;
    m_dom.setDomainQuota(newDomainQuota);
    QCOMPARE(m_dom.domainQuota(), newDomainQuota);
}

void DomainTest::getSetDomainQuotaUsed()
{
    QCOMPARE(m_dom.domainQuotaUsed(), 161061273600);
    const auto newDomainQuotaUsed = 80530636800;
    m_dom.setDomainQuotaUsed(newDomainQuotaUsed);
    QCOMPARE(m_dom.domainQuotaUsed(), newDomainQuotaUsed);
}

void DomainTest::getSetFreeNamesEnabled()
{
    QCOMPARE(m_dom.isFreeNamesEnabled(), true);
    m_dom.setFreeNamesEnabled(false);
    QCOMPARE(m_dom.isFreeNamesEnabled(), false);
}

void DomainTest::getSetFreeAddressEnabled()
{
    QCOMPARE(m_dom.isFreeAddressEnabled(), false);
    m_dom.setFreeAddressEnabled(true);
    QCOMPARE(m_dom.isFreeAddressEnabled(), true);
}

void DomainTest::getSetAccounts()
{
    QCOMPARE(m_dom.accounts(), 100);
    const auto newAccounts = 50;
    m_dom.setAccounts(newAccounts);
    QCOMPARE(m_dom.accounts(), newAccounts);
}

void DomainTest::getSetCreated()
{
    QCOMPARE(m_dom.created(), QDateTime(QDate(2018, 3, 28), QTime(16, 0)));
    const auto newCreated = QDateTime(QDate(2019, 3, 28), QTime(16, 0));
    m_dom.setCreated(newCreated);
    QCOMPARE(m_dom.created(), newCreated);
}

void DomainTest::getSetUpdated()
{
    QCOMPARE(m_dom.updated(), QDateTime(QDate(2018, 3, 28), QTime(17, 0)));
    const auto newUpdated = QDateTime(QDate(2019, 3, 28), QTime(17, 0));
    m_dom.setUpdated(newUpdated);
    QCOMPARE(m_dom.updated(), newUpdated);
}

void DomainTest::getSetValidUntil()
{
    QCOMPARE(m_dom.validUntil(), QDateTime(QDate(2118, 3, 28), QTime(16, 0)));
    const auto newValidUntil = QDateTime(QDate(2119, 3, 28), QTime(16, 0));
    m_dom.setValidUntil(newValidUntil);
    QCOMPARE(m_dom.validUntil(), newValidUntil);
}

void DomainTest::aceName()
{
    QCOMPARE(m_dom.aceName(), m_dom.name());
}

void DomainTest::aceId()
{
    QCOMPARE(m_dom.aceId(), 0);
}

void DomainTest::isIdn()
{
    QCOMPARE(m_dom.isIdn(), false);
}

void DomainTest::toSimple()
{
    const SimpleDomain sd = m_dom.toSimple();
    QCOMPARE(sd.isValid(), true);
    QCOMPARE(sd.id(), m_dom.id());
    QCOMPARE(sd.name(), m_dom.name());
}

void DomainTest::domainQuotaUsagePercent()
{
    QCOMPARE(m_dom.domainQuotaUsagePercent(), 10.0);
}

void DomainTest::accountUsagePercent()
{
    QCOMPARE(m_dom.accountUsagePercent(), 10.0);
}

QTEST_MAIN(DomainTest)

#include "testdomain.moc"
