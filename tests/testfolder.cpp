#include "../src/objects/folder.h"

#include <QTest>
#include <QDataStream>
#include <QMetaObject>
#include <QMetaProperty>

class FolderTest : public QObject
{
    Q_OBJECT
public:
    FolderTest(QObject *parent = nullptr) : QObject(parent) {
        qRegisterMetaType<dbid_t>("dbid_t");
    }

private Q_SLOTS:
    void initTestCase() {}

    void constructor();
    void testMove();
    void datastream();

    void cleanupTestCase() {}
};

void FolderTest::constructor()
{
    Folder f(1, 2, QStringLiteral("Papierkorb"), SkaffariIMAP::Trash);

    QCOMPARE(f.getId(), static_cast<dbid_t>(1));
    QCOMPARE(Folder::staticMetaObject.property(Folder::staticMetaObject.indexOfProperty("id")).readOnGadget(&f).value<dbid_t>(), static_cast<dbid_t>(1));
    QCOMPARE(f.getDomainId(), static_cast<dbid_t>(2));
    QCOMPARE(Folder::staticMetaObject.property(Folder::staticMetaObject.indexOfProperty("domainId")).readOnGadget(&f).value<dbid_t>(), static_cast<dbid_t>(2));
    QCOMPARE(f.getName(), QStringLiteral("Papierkorb"));
    QCOMPARE(Folder::staticMetaObject.property(Folder::staticMetaObject.indexOfProperty("name")).readOnGadget(&f).toString(), QStringLiteral("Papierkorb"));
    QCOMPARE(f.getSpecialUse(), SkaffariIMAP::Trash);
}

void FolderTest::testMove()
{
    // Test move constructor
    {
        Folder f1(1, 2, QStringLiteral("Papierkorb"), SkaffariIMAP::Trash);
        QCOMPARE(f1.getName(), QStringLiteral("Papierkorb"));
        Folder f2(std::move(f1));
        QCOMPARE(f2.getName(), QStringLiteral("Papierkorb"));
    }

    // Test move assignment
    {
        Folder f1(1, 2, QStringLiteral("Papierkorb"), SkaffariIMAP::Trash);
        QCOMPARE(f1.getName(), QStringLiteral("Papierkorb"));
        Folder f2(2, 2, QStringLiteral("Archiv"), SkaffariIMAP::Archive);
        f2 = std::move(f1);
        QCOMPARE(f2.getName(), QStringLiteral("Papierkorb"));
    }
}

void FolderTest::datastream()
{
    Folder f1(123, 456, QStringLiteral("Papierkorb"), SkaffariIMAP::Trash);

    QByteArray outBa;
    QDataStream out(&outBa, QIODevice::WriteOnly);
    out << f1;

    const QByteArray inBa = outBa;
    QDataStream in(inBa);
    Folder f2;
    in >> f2;

    QCOMPARE(f1.getId(), f2.getId());
    QCOMPARE(f1.getDomainId(), f2.getDomainId());
    QCOMPARE(f1.getName(), f2.getName());
    QCOMPARE(f1.getSpecialUse(), f2.getSpecialUse());
}

QTEST_MAIN(FolderTest)

#include "testfolder.moc"
