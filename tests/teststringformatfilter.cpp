#include "../grantlee/stringformatfilter.h"
#include <grantlee/filter.h>
#include <grantlee/engine.h>
#include <grantlee/template.h>

#include <QTest>

class StringFormatFilterTest : public QObject
{
    Q_OBJECT
public:
    StringFormatFilterTest(QObject *parent = nullptr) : QObject(parent) {}

private Q_SLOTS:
    void initTestCase();

    void simpleTest();

    void cleanupTestCase();

private:
    void doTest();

    Grantlee::Engine *m_engine = nullptr;
    QSharedPointer<Grantlee::InMemoryTemplateLoader> m_loader;
};

void StringFormatFilterTest::initTestCase()
{
    m_engine = new Grantlee::Engine(this);
    QVERIFY(m_engine);

    m_loader = QSharedPointer<Grantlee::InMemoryTemplateLoader>(new Grantlee::InMemoryTemplateLoader());
}

void StringFormatFilterTest::simpleTest()
{

}

void StringFormatFilterTest::cleanupTestCase()
{
    delete m_engine;
}

QTEST_MAIN(StringFormatFilterTest)

#include "teststringformatfilter.moc"
