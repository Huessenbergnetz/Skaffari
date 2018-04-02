#include "../src/objects/domain.h"
#include "../src/objects/simpledomain.h"
#include "../src/objects/simpleadmin.h"
#include "../src/objects/folder.h"
#include <QTest>
#include <QDataStream>

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
    void dataStream();

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

void DomainTest::dataStream()
{
    Domain d1(12, QStringLiteral("example.com"), QStringLiteral("xmp"), QStringLiteral("cyrus"), 1610612736, 1000, 1610612736000, 161061273600, true, false, 100, QDateTime(QDate(2018, 3, 28), QTime(16, 0)), QDateTime(QDate(2018, 3, 28), QTime(17, 0)), QDateTime(QDate(2118, 3, 28), QTime(16, 0)));
    d1.setParent(SimpleDomain(13, QStringLiteral("example.net")));

    std::vector<SimpleDomain> children;
    children.emplace_back(SimpleDomain(1, QStringLiteral("example.org")));
    children.emplace_back(SimpleDomain(2, QStringLiteral("example.mil")));
    d1.setChildren(children);

    std::vector<SimpleAdmin> admins;
    admins.emplace_back(SimpleAdmin(1, QStringLiteral("admin")));
    d1.setAdmins(admins);

    std::vector<Folder> folders;
    folders.emplace_back(Folder(1, 12, QStringLiteral("Papierkorb")));
    folders.emplace_back(Folder(2, 12, QStringLiteral("Vorlagen")));
    folders.emplace_back(Folder(3, 12, QStringLiteral("Versandte Nachrichten")));
    d1.setFolders(folders);

    QVERIFY(d1.isValid());

    QByteArray outBa;
    QDataStream out(&outBa, QIODevice::WriteOnly);
    out << d1;

    const QByteArray inBa = outBa;
    QDataStream in(inBa);
    Domain d2;
    in >> d2;

    QVERIFY(d2.isValid());
    QCOMPARE(d1.id(), d2.id());
    QCOMPARE(d1.aceId(), d2.aceId());
    QCOMPARE(d1.name(), d2.name());
    QCOMPARE(d1.prefix(), d2.prefix());
    QCOMPARE(d1.transport(), d2.transport());
    QCOMPARE(d1.quota(), d2.quota());
    QCOMPARE(d1.domainQuota(), d2.domainQuota());
    QCOMPARE(d1.domainQuotaUsed(), d2.domainQuotaUsed());
    QCOMPARE(d1.parent().id(), d2.parent().id());
    QCOMPARE(d1.parent().name(), d2.parent().name());
    QCOMPARE(d1.created(), d2.created());
    QCOMPARE(d1.updated(), d2.updated());
    QCOMPARE(d1.validUntil(), d2.validUntil());
    QCOMPARE(d1.maxAccounts(), d2.maxAccounts());
    QCOMPARE(d1.accounts(), d2.accounts());
    QCOMPARE(d1.isFreeAddressEnabled(), d2.isFreeAddressEnabled());
    QCOMPARE(d1.isFreeNamesEnabled(), d2.isFreeNamesEnabled());

    for (std::vector<SimpleDomain>::size_type i = 0; i < children.size(); ++i) {
        QCOMPARE(d1.children().at(i).id(), d2.children().at(i).id());
        QCOMPARE(d1.children().at(i).name(), d2.children().at(i).name());
    }

    for (std::vector<SimpleAdmin>::size_type i = 0; i < admins.size(); ++i) {
        QCOMPARE(d1.admins().at(i).id(), d2.admins().at(i).id());
        QCOMPARE(d1.admins().at(i).name(), d2.admins().at(i).name());
    }

    for (std::vector<Folder>::size_type i = 0; i < folders.size(); ++i) {
        QCOMPARE(d1.folders().at(i).getId(), d2.folders().at(i).getId());
        QCOMPARE(d1.folders().at(i).getDomainId(), d2.folders().at(i).getDomainId());
        QCOMPARE(d1.folders().at(i).getName(), d2.folders().at(i).getName());
    }
}

QTEST_MAIN(DomainTest)

#include "testdomain.moc"
