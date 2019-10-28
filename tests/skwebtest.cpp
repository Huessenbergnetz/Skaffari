#include "skwebtest.h"
#include <QTest>

SkWebTest::SkWebTest(QObject *parent) : SkAppTestObject(parent)
{

}

SkWebTest::~SkWebTest()
{

}

//void SkWebTest::initWebTest(Browser::Type browserType, const QString &host, int port)
//{
////    auto proxy = new Proxy(Proxy::DIRECT);
//    auto caps = new DesiredCapabilities(browserType);
////    caps->setProxy(proxy);

//}

void SkWebTest::cleanupWebTest()
{

}
