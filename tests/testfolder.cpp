#include "../src/objects/folder.h"

#include <QTest>
#include <QDataStream>

class FolderTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() {}

    void constructor();
    void datastream();

    void cleanupTestCase() {}
};

void FolderTest::constructor()
{
    Folder f(1, 2, QStringLiteral("Papierkorb"));

    QCOMPARE(f.getId(), 1);
    QCOMPARE(f.getDomainId(), 2);
    QCOMPARE(f.getName(), QStringLiteral("Papierkorb"));
}

void FolderTest::datastream()
{
    Folder f1(123, 456, QStringLiteral("Papierkorb"));

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
}

QTEST_MAIN(FolderTest)

#include "testfolder.moc"
