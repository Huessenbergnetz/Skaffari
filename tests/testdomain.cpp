#include "../src/objects/domain.h"
#include "../src/objects/simpledomain.h"
#include "../src/objects/simpleadmin.h"
#include "../src/objects/folder.h"
#include <QTest>
#include <QDataStream>

class DomainTest : public QObject
{
    Q_OBJECT
public:
    DomainTest(QObject *parent = nullptr) : QObject(parent) {}

private Q_SLOTS:
    void initTestCase() {}

    void isValid();
    void boolOperator();
    void constructor();
    void nameIdString();
    void id();
    void name();
    void prefix();
    void transport();
    void quota();
    void maxAccounts();
    void domainQuota();
    void domainQuotaUsed();
    void isFreeNamesEnabled();
    void isFreeAddressEnabled();
    void accounts();
    void created();
    void updated();
    void validUntil();
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
    std::vector<SimpleAdmin> admins{
        SimpleAdmin(1, QStringLiteral("admin1")),
        SimpleAdmin(2, QStringLiteral("admin2"))
    };

    std::vector<Folder> folders{
        Folder(20, 12, QStringLiteral("Trash")),
        Folder(21, 21, QStringLiteral("Spam"))
    };

    m_dom = Domain(12, 5, QStringLiteral("example.com"), QStringLiteral("xmp"), QStringLiteral("cyrus"), 1610612736, 1000, 1610612736000, 161061273600, true, false, 100, QDateTime(QDate(2018, 3, 28), QTime(16, 0)), QDateTime(QDate(2018, 3, 28), QTime(17, 0)), QDateTime(QDate(2118, 3, 28), QTime(16, 0)), SimpleDomain(10, QStringLiteral("example.de")), std::vector<SimpleDomain>(), admins, folders);
    QVERIFY(m_dom);
}

void DomainTest::nameIdString()
{
    QCOMPARE(m_dom.nameIdString(), QStringLiteral("example.com (ID: 12)"));
}

void DomainTest::id()
{
    QCOMPARE(m_dom.id(), 12);
}

void DomainTest::name()
{
    QCOMPARE(m_dom.name(), QStringLiteral("example.com"));
}

void DomainTest::prefix()
{
    QCOMPARE(m_dom.prefix(), QStringLiteral("xmp"));
}

void DomainTest::transport()
{
    QCOMPARE(m_dom.transport(), QStringLiteral("cyrus"));
}

void DomainTest::quota()
{
    QCOMPARE(m_dom.quota(), 1610612736);
}

void DomainTest::maxAccounts()
{
    QCOMPARE(m_dom.maxAccounts(), 1000);
}

void DomainTest::domainQuota()
{
    QCOMPARE(m_dom.domainQuota(), 1610612736000);
}

void DomainTest::domainQuotaUsed()
{
    QCOMPARE(m_dom.domainQuotaUsed(), 161061273600);
    m_dom.setDomainQuotaUsed(80530636800);
    QCOMPARE(m_dom.domainQuotaUsed(), 80530636800);
    m_dom.setDomainQuotaUsed(161061273600);
}

void DomainTest::isFreeNamesEnabled()
{
    QCOMPARE(m_dom.isFreeNamesEnabled(), true);
}

void DomainTest::isFreeAddressEnabled()
{
    QCOMPARE(m_dom.isFreeAddressEnabled(), false);
}

void DomainTest::accounts()
{
    QCOMPARE(m_dom.accounts(), 100);
}

void DomainTest::created()
{
    QCOMPARE(m_dom.created(), QDateTime(QDate(2018, 3, 28), QTime(16, 0)));
}

void DomainTest::updated()
{
    QCOMPARE(m_dom.updated(), QDateTime(QDate(2018, 3, 28), QTime(17, 0)));
}

void DomainTest::validUntil()
{
    QCOMPARE(m_dom.validUntil(), QDateTime(QDate(2118, 3, 28), QTime(16, 0)));
}

void DomainTest::aceName()
{
    QCOMPARE(m_dom.aceName(), m_dom.name());
}

void DomainTest::aceId()
{
    QCOMPARE(m_dom.aceId(), 5);
}

void DomainTest::isIdn()
{
    QCOMPARE(m_dom.isIdn(), true);
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
    std::vector<SimpleDomain> children;
    children.emplace_back(SimpleDomain(1, QStringLiteral("example.org")));
    children.emplace_back(SimpleDomain(2, QStringLiteral("example.mil")));

    std::vector<SimpleAdmin> admins;
    admins.emplace_back(SimpleAdmin(1, QStringLiteral("admin")));

    std::vector<Folder> folders;
    folders.emplace_back(Folder(1, 12, QStringLiteral("Papierkorb")));
    folders.emplace_back(Folder(2, 12, QStringLiteral("Vorlagen")));
    folders.emplace_back(Folder(3, 12, QStringLiteral("Versandte Nachrichten")));

    Domain d1(12, 0, QStringLiteral("example.com"), QStringLiteral("xmp"), QStringLiteral("cyrus"), 1610612736, 1000, 1610612736000, 161061273600, true, false, 100, QDateTime(QDate(2018, 3, 28), QTime(16, 0)), QDateTime(QDate(2018, 3, 28), QTime(17, 0)), QDateTime(QDate(2118, 3, 28), QTime(16, 0)), SimpleDomain(13, QStringLiteral("example.net")), children, admins, folders);


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
