#ifndef SKWEBTEST_H
#define SKWEBTEST_H

#include "skapptestobject.h"
#include <QObject>

class SkWebTest : public SkAppTestObject
{
    Q_OBJECT
public:
    explicit SkWebTest(QObject *parent = nullptr);
    ~SkWebTest() override;

protected:
//    void initWebTest(Browser::Type browserType, const QString &host = QStringLiteral("localhost"), int port = 4444);
    void cleanupWebTest();
};

#endif // SKWEBTEST_H
