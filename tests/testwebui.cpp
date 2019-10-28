#include "skwebtest.h"
#include <QTest>

class WebUiTest : public SkWebTest
{
    Q_OBJECT
public:
    explicit WebUiTest(QObject *parent = nullptr) : SkWebTest(parent) {}

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void checkit();
};

void WebUiTest::initTestCase()
{

}

void WebUiTest::cleanupTestCase()
{
    cleanupWebTest();
}

void WebUiTest::checkit()
{

}

QTEST_MAIN(WebUiTest)

#include "testwebui.moc"
