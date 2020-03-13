#include "../src/objects/domain.h"
#include "../src/objects/simpledomain.h"
#include "../src/objects/simpleadmin.h"
#include "../src/objects/folder.h"
#include <QTest>
#include <QDataStream>
#include <QMetaObject>
#include <QMetaProperty>

class DomainTest : public QObject
{
    Q_OBJECT
public:
    DomainTest(QObject *parent = nullptr) : QObject(parent) {
        qRegisterMetaType<quota_size_t>("quota_size_t");
        qRegisterMetaType<dbid_t>("dbid_t");
        qRegisterMetaType<std::vector<Folder>>("std::vector<Folder>");
    }

private Q_SLOTS:
    void initTestCase() {}

    void isValid();
    void boolOperator();
    void constructor();
    void testMove();
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
    void folders();
    void folder();
    void accounts();
    void created();
    void updated();
    void validUntil();
    void autoconfig();
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
    std::vector<Folder> m_folders;
    Folder m_draftsFolder;
    Folder m_junkFolder;
    Folder m_sentFolder;
    Folder m_trashFolder;
    Folder m_archiveFolder;
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

    m_draftsFolder = Folder(18, 12, QStringLiteral("Drafts"), SkaffariIMAP::Drafts);
    m_junkFolder = Folder(19, 12, QStringLiteral("Spam"), SkaffariIMAP::Junk);
    m_sentFolder = Folder(20, 12, QStringLiteral("Sent Messages"), SkaffariIMAP::Sent);
    m_trashFolder = Folder(21, 12, QStringLiteral("Trash"), SkaffariIMAP::Trash);
    m_archiveFolder = Folder(22, 12, QStringLiteral("Archive"), SkaffariIMAP::Archive);

    m_folders = std::vector<Folder>{
        m_draftsFolder,
        m_junkFolder,
        m_sentFolder,
        m_trashFolder,
        m_archiveFolder
    };

    m_dom = Domain(12, 5, QStringLiteral("example.com"), QStringLiteral("xmp"), QStringLiteral("cyrus"), 1610612736, 1000, 1610612736000, 161061273600, true, false, 100, QDateTime(QDate(2018, 3, 28), QTime(16, 0)), QDateTime(QDate(2018, 3, 28), QTime(17, 0)), QDateTime(QDate(2118, 3, 28), QTime(16, 0)), Domain::UseCustomAutoconfig, SimpleDomain(10, QStringLiteral("example.de")), std::vector<SimpleDomain>(), admins, m_folders);
    QVERIFY(m_dom);
}

void DomainTest::testMove()
{
    std::vector<SimpleAdmin> admins{
        SimpleAdmin(1, QStringLiteral("admin1")),
        SimpleAdmin(2, QStringLiteral("admin2"))
    };

    // Test move constructor
    {
        Domain d1(12, 5, QStringLiteral("example.com"), QStringLiteral("xmp"), QStringLiteral("cyrus"), 1610612736, 1000, 1610612736000, 161061273600, true, false, 100, QDateTime(QDate(2018, 3, 28), QTime(16, 0)), QDateTime(QDate(2018, 3, 28), QTime(17, 0)), QDateTime(QDate(2118, 3, 28), QTime(16, 0)), Domain::UseCustomAutoconfig, SimpleDomain(10, QStringLiteral("example.de")), std::vector<SimpleDomain>(), admins, m_folders);
        QCOMPARE(d1.name(), QStringLiteral("example.com"));
        Domain d2(std::move(d1));
        QCOMPARE(d2.name(), QStringLiteral("example.com"));
    }

    // Test move assignment
    {
        Domain d1(12, 5, QStringLiteral("example.com"), QStringLiteral("xmp"), QStringLiteral("cyrus"), 1610612736, 1000, 1610612736000, 161061273600, true, false, 100, QDateTime(QDate(2018, 3, 28), QTime(16, 0)), QDateTime(QDate(2018, 3, 28), QTime(17, 0)), QDateTime(QDate(2118, 3, 28), QTime(16, 0)), Domain::UseCustomAutoconfig, SimpleDomain(10, QStringLiteral("example.de")), std::vector<SimpleDomain>(), admins, m_folders);
        QCOMPARE(d1.name(), QStringLiteral("example.com"));
        Domain d2(13, 6, QStringLiteral("example.net"), QStringLiteral("xmpn"), QStringLiteral("cyrus"), 1610612736, 1000, 1610612736000, 161061273600, true, false, 100, QDateTime(QDate(2018, 3, 28), QTime(16, 0)), QDateTime(QDate(2018, 3, 28), QTime(17, 0)), QDateTime(QDate(2118, 3, 28), QTime(16, 0)), Domain::UseCustomAutoconfig, SimpleDomain(10, QStringLiteral("example.de")), std::vector<SimpleDomain>(), admins, m_folders);
        d2 = std::move(d1);
        QCOMPARE(d2.name(), QStringLiteral("example.com"));
    }
}

void DomainTest::nameIdString()
{
    QCOMPARE(m_dom.nameIdString(), QStringLiteral("example.com (ID: 12)"));
}

void DomainTest::id()
{
    QCOMPARE(m_dom.id(), static_cast<dbid_t>(12));
    QCOMPARE(Domain::staticMetaObject.property(Domain::staticMetaObject.indexOfProperty("id")).readOnGadget(&m_dom).value<dbid_t>(), static_cast<dbid_t>(12));
}

void DomainTest::name()
{
    QCOMPARE(m_dom.name(), QStringLiteral("example.com"));
    QCOMPARE(Domain::staticMetaObject.property(Domain::staticMetaObject.indexOfProperty("name")).readOnGadget(&m_dom).toString(), QStringLiteral("example.com"));
}

void DomainTest::prefix()
{
    QCOMPARE(m_dom.prefix(), QStringLiteral("xmp"));
    QCOMPARE(Domain::staticMetaObject.property(Domain::staticMetaObject.indexOfProperty("prefix")).readOnGadget(&m_dom).toString(), QStringLiteral("xmp"));
}

void DomainTest::transport()
{
    QCOMPARE(m_dom.transport(), QStringLiteral("cyrus"));
    QCOMPARE(Domain::staticMetaObject.property(Domain::staticMetaObject.indexOfProperty("transport")).readOnGadget(&m_dom).toString(), QStringLiteral("cyrus"));
}

void DomainTest::quota()
{
    QCOMPARE(m_dom.quota(), static_cast<quota_size_t>(1610612736));
    QCOMPARE(Domain::staticMetaObject.property(Domain::staticMetaObject.indexOfProperty("quota")).readOnGadget(&m_dom).value<quota_size_t>(), static_cast<quota_size_t>(1610612736));
}

void DomainTest::maxAccounts()
{
    QCOMPARE(m_dom.maxAccounts(), static_cast<dbid_t>(1000));
    QCOMPARE(Domain::staticMetaObject.property(Domain::staticMetaObject.indexOfProperty("maxAccounts")).readOnGadget(&m_dom).value<dbid_t>(), static_cast<dbid_t>(1000));
}

void DomainTest::domainQuota()
{
    QCOMPARE(m_dom.domainQuota(), static_cast<quota_size_t>(1610612736000));
    QCOMPARE(Domain::staticMetaObject.property(Domain::staticMetaObject.indexOfProperty("domainQuota")).readOnGadget(&m_dom).value<quota_size_t>(), static_cast<quota_size_t>(1610612736000));
}

void DomainTest::domainQuotaUsed()
{
    QCOMPARE(m_dom.domainQuotaUsed(), static_cast<quota_size_t>(161061273600));
    m_dom.setDomainQuotaUsed(80530636800);
    QCOMPARE(m_dom.domainQuotaUsed(), static_cast<quota_size_t>(80530636800));
    m_dom.setDomainQuotaUsed(161061273600);
    QCOMPARE(Domain::staticMetaObject.property(Domain::staticMetaObject.indexOfProperty("domainQuotaUsed")).readOnGadget(&m_dom).value<quota_size_t>(), static_cast<quota_size_t>(161061273600));
}

void DomainTest::isFreeNamesEnabled()
{
    QCOMPARE(m_dom.isFreeNamesEnabled(), true);
    QCOMPARE(Domain::staticMetaObject.property(Domain::staticMetaObject.indexOfProperty("freeNames")).readOnGadget(&m_dom).toBool(), true);
}

void DomainTest::isFreeAddressEnabled()
{
    QCOMPARE(m_dom.isFreeAddressEnabled(), false);
    QCOMPARE(Domain::staticMetaObject.property(Domain::staticMetaObject.indexOfProperty("freeAddress")).readOnGadget(&m_dom).toBool(), false);
}

void DomainTest::folders()
{
    QCOMPARE(m_dom.folders(), m_folders);
    QCOMPARE(Domain::staticMetaObject.property(Domain::staticMetaObject.indexOfProperty("folders")).readOnGadget(&m_dom).value<std::vector<Folder>>(), m_folders);
}

void DomainTest::folder()
{
    QCOMPARE(m_dom.folder(SkaffariIMAP::Drafts), m_draftsFolder);
    QCOMPARE(Domain::staticMetaObject.property(Domain::staticMetaObject.indexOfProperty("draftsFolder")).readOnGadget(&m_dom).value<Folder>(), m_draftsFolder);

    QCOMPARE(m_dom.folder(SkaffariIMAP::Junk), m_junkFolder);
    QCOMPARE(Domain::staticMetaObject.property(Domain::staticMetaObject.indexOfProperty("junkFolder")).readOnGadget(&m_dom).value<Folder>(), m_junkFolder);

    QCOMPARE(m_dom.folder(SkaffariIMAP::Sent), m_sentFolder);
    QCOMPARE(Domain::staticMetaObject.property(Domain::staticMetaObject.indexOfProperty("sentFolder")).readOnGadget(&m_dom).value<Folder>(), m_sentFolder);

    QCOMPARE(m_dom.folder(SkaffariIMAP::Trash), m_trashFolder);
    QCOMPARE(Domain::staticMetaObject.property(Domain::staticMetaObject.indexOfProperty("trashFolder")).readOnGadget(&m_dom).value<Folder>(), m_trashFolder);

    QCOMPARE(m_dom.folder(SkaffariIMAP::Archive), m_archiveFolder);
    QCOMPARE(Domain::staticMetaObject.property(Domain::staticMetaObject.indexOfProperty("archiveFolder")).readOnGadget(&m_dom).value<Folder>(), m_archiveFolder);

    QCOMPARE(m_dom.folder(SkaffariIMAP::SkaffariOtherFolders), Folder());
    QCOMPARE(Domain::staticMetaObject.property(Domain::staticMetaObject.indexOfProperty("otherFolders")).readOnGadget(&m_dom).value<Folder>(), Folder());
}

void DomainTest::accounts()
{
    QCOMPARE(m_dom.accounts(), static_cast<dbid_t>(100));
    QCOMPARE(Domain::staticMetaObject.property(Domain::staticMetaObject.indexOfProperty("accounts")).readOnGadget(&m_dom).value<dbid_t>(), static_cast<dbid_t>(100));
}

void DomainTest::created()
{
    const QDateTime expected = QDateTime(QDate(2018, 3, 28), QTime(16, 0));
    QCOMPARE(m_dom.created(), expected);
    QCOMPARE(Domain::staticMetaObject.property(Domain::staticMetaObject.indexOfProperty("created")).readOnGadget(&m_dom).toDateTime(), expected);
}

void DomainTest::updated()
{
    const QDateTime expected = QDateTime(QDate(2018, 3, 28), QTime(17, 0));
    QCOMPARE(m_dom.updated(), expected);
    QCOMPARE(Domain::staticMetaObject.property(Domain::staticMetaObject.indexOfProperty("updated")).readOnGadget(&m_dom).toDateTime(), expected);
}

void DomainTest::validUntil()
{
    const QDateTime expected = QDateTime(QDate(2118, 3, 28), QTime(16, 0));
    QCOMPARE(m_dom.validUntil(), expected);
    QCOMPARE(Domain::staticMetaObject.property(Domain::staticMetaObject.indexOfProperty("validUntil")).readOnGadget(&m_dom).toDateTime(), expected);
}

void DomainTest::autoconfig()
{
    QCOMPARE(m_dom.autoconfig(), Domain::UseCustomAutoconfig);
    QCOMPARE(Domain::staticMetaObject.property(Domain::staticMetaObject.indexOfProperty("autoconfig")).readOnGadget(&m_dom).value<Domain::AutoconfigStrategy>(), Domain::UseCustomAutoconfig);
}

void DomainTest::aceName()
{
    QCOMPARE(m_dom.aceName(), m_dom.name());
}

void DomainTest::aceId()
{
    QCOMPARE(m_dom.aceId(), static_cast<dbid_t>(5));
}

void DomainTest::isIdn()
{
    QCOMPARE(m_dom.isIdn(), true);
    QCOMPARE(Domain::staticMetaObject.property(Domain::staticMetaObject.indexOfProperty("isIdn")).readOnGadget(&m_dom).toBool(), true);
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
    QCOMPARE(m_dom.domainQuotaUsagePercent(), 10.0f);
    QCOMPARE(Domain::staticMetaObject.property(Domain::staticMetaObject.indexOfProperty("domainQuotaUsagePercent")).readOnGadget(&m_dom).toFloat(), 10.0f);
}

void DomainTest::accountUsagePercent()
{
    QCOMPARE(m_dom.accountUsagePercent(), 10.0f);
    QCOMPARE(Domain::staticMetaObject.property(Domain::staticMetaObject.indexOfProperty("accountUsagePercent")).readOnGadget(&m_dom).toFloat(), 10.0f);
}

void DomainTest::dataStream()
{
    std::vector<SimpleDomain> children;
    children.emplace_back(SimpleDomain(1, QStringLiteral("example.org")));
    children.emplace_back(SimpleDomain(2, QStringLiteral("example.mil")));

    std::vector<SimpleAdmin> admins;
    admins.emplace_back(SimpleAdmin(1, QStringLiteral("admin")));

    std::vector<Folder> folders;
    folders.emplace_back(Folder(1, 12, QStringLiteral("Papierkorb"), SkaffariIMAP::Trash));
    folders.emplace_back(Folder(2, 12, QStringLiteral("Vorlagen"), SkaffariIMAP::SkaffariOtherFolders));
    folders.emplace_back(Folder(3, 12, QStringLiteral("Versandte Nachrichten"), SkaffariIMAP::Sent));

    Domain d1(12, 0, QStringLiteral("example.com"), QStringLiteral("xmp"), QStringLiteral("cyrus"), 1610612736, 1000, 1610612736000, 161061273600, true, false, 100, QDateTime(QDate(2018, 3, 28), QTime(16, 0)), QDateTime(QDate(2018, 3, 28), QTime(17, 0)), QDateTime(QDate(2118, 3, 28), QTime(16, 0)), Domain::AutoconfigDisabled, SimpleDomain(13, QStringLiteral("example.net")), children, admins, folders);


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
    QCOMPARE(d1.autoconfig(), d2.autoconfig());
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
        QCOMPARE(d1.folders().at(i).getSpecialUse(), d2.folders().at(i).getSpecialUse());
    }
}

QTEST_MAIN(DomainTest)

#include "testdomain.moc"
