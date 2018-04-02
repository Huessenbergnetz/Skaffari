#include "../src/objects/folder.h"

#include <QTest>

class FolderTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() {}

    void doTest();

    void cleanupTestCase() {}
};

void FolderTest::doTest()
{
    Folder f(1, 2, QStringLiteral("Papierkorb"));

    QCOMPARE(f.getId(), 1);
    QCOMPARE(f.getDomainId(), 2);
    QCOMPARE(f.getName(), QStringLiteral("Papierkorb"));
}

QTEST_MAIN(FolderTest)

#include "testfolder.moc"
