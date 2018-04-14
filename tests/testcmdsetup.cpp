#include "skapptestobject.h"
#include <QTest>
#include <QProcess>
#include <QDebug>
#include <QTemporaryFile>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

class CmdSetupTest : public SkAppTestObject
{
    Q_OBJECT
public:
    explicit CmdSetupTest(QObject *parent = nullptr) : SkAppTestObject(parent) {}

private Q_SLOTS:
    void initTestCase();

    void doTest();
    void doTest_data();

    void cleanup();
};

void CmdSetupTest::initTestCase()
{
    QVERIFY2(checkRootUser(), "These tests can only be executed as root.");
    QVERIFY(startMysql());
    QVERIFY(createDatabase());
    QVERIFY(startCyrus());
}

void CmdSetupTest::cleanup()
{
    QSqlDatabase::removeDatabase(QStringLiteral("skdb"));
}

void CmdSetupTest::doTest()
{
    QFETCH(QString, dbType);
    QFETCH(QString, dbHost);
    QFETCH(int, admPwAlgo);
    QFETCH(int, admPwIter);
    QFETCH(int, admPwThreshold);
    QFETCH(QString, adminName);
    QFETCH(QString, adminPass);
    QFETCH(int, accPwMethod);
    QFETCH(int, accPwAlgo);
    QFETCH(int, accPwRounds);
    QFETCH(int, accPwThreshold);
    QFETCH(QString, imapPass);

    QTemporaryFile confFile;
    QVERIFY(confFile.open());
    QSettings conf(confFile.fileName(), QSettings::IniFormat);
    QVERIFY(conf.status() == QSettings::NoError);

    QProcess cmd;
    cmd.setProgram(QStringLiteral(SKAFFARI_CMD));
    cmd.setArguments({QStringLiteral("--setup"), QStringLiteral("--ini"), confFile.fileName()});
    cmd.setProcessChannelMode(QProcess::MergedChannels);
    cmd.start();

    QVERIFY(cmd.waitForReadyRead());
    qDebug() << QString::fromUtf8(cmd.readAll());
    cmd.write(dbType.toUtf8());
    cmd.write("\n");

    QVERIFY(cmd.waitForReadyRead());
    qDebug() << QString::fromUtf8(cmd.readAll());
    cmd.write(dbHost.toUtf8());
    cmd.write("\n");

    QVERIFY(cmd.waitForReadyRead());
    qDebug() << QString::fromUtf8(cmd.readAll());
    cmd.write(m_dbName.toUtf8());
    cmd.write("\n");

    QVERIFY(cmd.waitForReadyRead());
    qDebug() << QString::fromUtf8(cmd.readAll());
    cmd.write(m_dbUser.toUtf8());
    cmd.write("\n");

    QVERIFY(cmd.waitForReadyRead());
    qDebug() << QString::fromUtf8(cmd.readAll());
    cmd.write(m_dbPass.toUtf8());
    cmd.write("\n");

    QVERIFY(cmd.waitForReadyRead());
    conf.sync();
    conf.beginGroup(QStringLiteral("Database"));
    QCOMPARE(conf.value(QStringLiteral("type")).toString(), dbType);
    QCOMPARE(conf.value(QStringLiteral("host")).toString(), dbHost);
    QCOMPARE(conf.value(QStringLiteral("name")).toString(), m_dbName);
    QCOMPARE(conf.value(QStringLiteral("user")).toString(), m_dbUser);
    QCOMPARE(conf.value(QStringLiteral("password")).toString(), m_dbPass);
    conf.endGroup();
    {
        QSqlDatabase db = QSqlDatabase::addDatabase(dbType, QStringLiteral("skdb"));
        db.setDatabaseName(m_dbName);
        db.setUserName(m_dbUser);
        db.setPassword(m_dbPass);
        db.setConnectOptions(QStringLiteral("UNIX_SOCKET=%1").arg(dbHost));
        QVERIFY(db.open());
    }
    qDebug() << QString::fromUtf8(cmd.readAll());

    cmd.write(QString::number(admPwAlgo).toUtf8());
    cmd.write("\n");

    QVERIFY(cmd.waitForReadyRead());
    qDebug() << QString::fromUtf8(cmd.readAll());
    cmd.write(QString::number(admPwIter).toUtf8());
    cmd.write("\n");

    QVERIFY(cmd.waitForReadyRead());
    qDebug() << QString::fromUtf8(cmd.readAll());
    cmd.write(QString::number(admPwThreshold).toUtf8());
    cmd.write("\n");

    QVERIFY(cmd.waitForReadyRead());
    qDebug() << QString::fromUtf8(cmd.readAll());
    cmd.write("\n");

    QVERIFY(cmd.waitForReadyRead());
    qDebug() << QString::fromUtf8(cmd.readAll());
    cmd.write(adminName.toUtf8());
    cmd.write("\n");

    QVERIFY(cmd.waitForReadyRead());
    qDebug() << QString::fromUtf8(cmd.readAll());
    cmd.write(adminPass.toUtf8());
    cmd.write("\n");

    QVERIFY(cmd.waitForReadyRead());
    conf.sync();
    conf.beginGroup(QStringLiteral("Admins"));
    QCOMPARE(conf.value(QStringLiteral("pwalgorithm")).toInt(), admPwAlgo);
    QCOMPARE(conf.value(QStringLiteral("pwrounds")).toInt(), admPwIter);
    QCOMPARE(conf.value(QStringLiteral("pwthreshold")).toInt(), admPwThreshold);
    conf.endGroup();
    {
        QSqlQuery q(QSqlDatabase::database(QStringLiteral("skdb")));
        QVERIFY(q.exec(QStringLiteral("SELECT username, password, type FROM adminuser")));
        QCOMPARE(q.size(), 1);
        QVERIFY(q.next());
        QCOMPARE(q.value(0).toString(), adminName);
        QVERIFY(!q.value(1).toString().isEmpty());
        QCOMPARE(q.value(2).toInt(), 255);
    }
    qDebug() << QString::fromUtf8(cmd.readAll());
    cmd.write("y\n");

    QVERIFY(cmd.waitForReadyRead());
    qDebug() << QString::fromUtf8(cmd.readAll());
    cmd.write(QString::number(accPwMethod).toUtf8());
    cmd.write("\n");

    if (accPwMethod == 1 || accPwMethod == 2) {
        QVERIFY(cmd.waitForReadyRead());
        qDebug() << QString::fromUtf8(cmd.readAll());
        cmd.write(QString::number(accPwAlgo).toUtf8());
        cmd.write("\n");
    }

    if ((accPwMethod == 1) && (accPwAlgo == 3 || accPwAlgo == 4 || accPwAlgo == 5)) {
        QVERIFY(cmd.waitForReadyRead());
        qDebug() << QString::fromUtf8(cmd.readAll());
        cmd.write(QString::number(accPwRounds).toUtf8());
        cmd.write("\n");
    }

    QVERIFY(cmd.waitForReadyRead());
    qDebug() << QString::fromUtf8(cmd.readAll());
    cmd.write(QString::number(accPwThreshold).toUtf8());
    cmd.write("\n");

    QVERIFY(cmd.waitForReadyRead());
    qDebug() << QString::fromUtf8(cmd.readAll());
    cmd.write("\n");

    QVERIFY(cmd.waitForReadyRead());
    qDebug() << QString::fromUtf8(cmd.readAll());
    cmd.write(m_imapUser.toUtf8());
    cmd.write("\n");

    QVERIFY(cmd.waitForReadyRead());
    qDebug() << QString::fromUtf8(cmd.readAll());
    cmd.write(imapPass.toUtf8());
    cmd.write("\n");

    QVERIFY(cmd.waitForReadyRead());
    qDebug() << QString::fromUtf8(cmd.readAll());
}

void CmdSetupTest::doTest_data()
{
    QTest::addColumn<QString>("dbType");
    QTest::addColumn<QString>("dbHost");
    QTest::addColumn<int>("admPwAlgo");
    QTest::addColumn<int>("admPwIter");
    QTest::addColumn<int>("admPwThreshold");
    QTest::addColumn<QString>("adminName");
    QTest::addColumn<QString>("adminPass");
    QTest::addColumn<int>("accPwMethod");
    QTest::addColumn<int>("accPwAlgo");
    QTest::addColumn<int>("accPwRounds");
    QTest::addColumn<int>("accPwThreshold");
    QTest::addColumn<QString>("imapPass");

    QTest::newRow("test-00") << QStringLiteral("QMYSQL") << mysqlSocket() << 3 << 10000 << 50 << QStringLiteral("admin") << QStringLiteral("wYezOAT3elS9") << 0 << 0 << 0 << 30 << QStringLiteral("BLeon8WD70d7");
}

QTEST_MAIN(CmdSetupTest)

#include "testcmdsetup.moc"
