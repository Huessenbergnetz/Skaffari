#include "../src/cutelee/skaffaricutelee.h"

#include <QTest>

class TestCuteleePlugin : public QObject
{
    Q_OBJECT
public:
    TestCuteleePlugin(QObject *parent = nullptr) : QObject(parent) {}

private Q_SLOTS:
    void initTestCase();

    void cleanupTestCase();
};

void TestCuteleePlugin::initTestCase()
{

}

void TestCuteleePlugin::cleanupTestCase()
{

}

QTEST_MAIN(TestCuteleePlugin)

#include "testcuteleeplugin.moc"
