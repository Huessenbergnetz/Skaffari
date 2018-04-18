#include "skapptestobject.h"
#include <QTest>
#include <QProcess>
#include <QDebug>
#include <QTemporaryFile>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#define QSQLDBNAME "skdb"

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
    void cleanupTestCase();
};

void CmdSetupTest::initTestCase()
{
    QMap<QString,QString> config{
        {QStringLiteral("SKAFFARI_DB_NAME"), m_dbName},
        {QStringLiteral("SKAFFARI_DB_USER"), m_dbUser},
        {QStringLiteral("SKAFFARI_DB_PASS"), m_dbPass},
        {QStringLiteral("IMAP_ADMINS"), m_imapUser},
        {QStringLiteral("UNIXHIERARCHYSEP"), QStringLiteral("no")},
        {QStringLiteral("VIRTDOMAINS"), QStringLiteral("no")},
        {QStringLiteral("SASL_PWCHECK_METHOD"), QStringLiteral("auxprop")},
        {QStringLiteral("SASL_AUXPROP_PLUGIN"), QStringLiteral("sql")},
        {QStringLiteral("SASL_SQL_ENGINE"), QStringLiteral("mysql")},
        {QStringLiteral("SASL_SQL_HOSTNAMES"), QStringLiteral("127.0.0.1:3306")}

    };
    QVERIFY(startContainer(config));
}

void CmdSetupTest::cleanupTestCase()
{
    QVERIFY(stopContainer());
}

void CmdSetupTest::cleanup()
{
    QSqlDatabase::removeDatabase(QStringLiteral(QSQLDBNAME));
}

void CmdSetupTest::doTest()
{
    QFETCH(QString, dbType);
    QFETCH(QString, dbHost);
    QFETCH(int, dbPort);
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
    QFETCH(QString, imapHost);
    QFETCH(int, imapPort);


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
    cmd.write(QString::number(dbPort).toUtf8());
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
    qDebug() << QString::fromUtf8(cmd.readAll());
    conf.sync();
    conf.beginGroup(QStringLiteral("Database"));
    QCOMPARE(conf.value(QStringLiteral("type")).toString(), dbType);
    QCOMPARE(conf.value(QStringLiteral("host")).toString(), dbHost);
    QCOMPARE(conf.value(QStringLiteral("port")).toInt(), dbPort);
    QCOMPARE(conf.value(QStringLiteral("name")).toString(), m_dbName);
    QCOMPARE(conf.value(QStringLiteral("user")).toString(), m_dbUser);
    QCOMPARE(conf.value(QStringLiteral("password")).toString(), m_dbPass);
    conf.endGroup();
    {
        QSqlDatabase db = QSqlDatabase::addDatabase(dbType, QStringLiteral(QSQLDBNAME));
        db.setDatabaseName(m_dbName);
        db.setUserName(m_dbUser);
        db.setPassword(m_dbPass);
        db.setHostName(dbHost);
        db.setPort(dbPort);
        QVERIFY(db.open());
    }

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
        QSqlQuery q(QSqlDatabase::database(QStringLiteral(QSQLDBNAME)));
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

    QVERIFY(cmd.waitForReadyRead());
    qDebug() << QString::fromUtf8(cmd.readAll());
    cmd.write(imapHost.toUtf8());
    cmd.write("\n");

    QVERIFY(cmd.waitForReadyRead());
    qDebug() << QString::fromUtf8(cmd.readAll());
    cmd.write(QString::number(imapPort).toUtf8());
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
    cmd.write(QString::number(0).toUtf8());
    cmd.write("\n");

    QVERIFY(cmd.waitForReadyRead());
    qDebug() << QString::fromUtf8(cmd.readAll());
    cmd.write(QString::number(0).toUtf8());
    cmd.write("\n");

    QVERIFY(cmd.waitForReadyRead());
    qDebug() << QString::fromUtf8(cmd.readAll());

    QSqlDatabase::removeDatabase(QStringLiteral(QSQLDBNAME));
}

void CmdSetupTest::doTest_data()
{
    QTest::addColumn<QString>("dbType");
    QTest::addColumn<QString>("dbHost");
    QTest::addColumn<int>("dbPort");
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
    QTest::addColumn<QString>("imapHost");
    QTest::addColumn<int>("imapPort");

    QTest::newRow("test-00") << QStringLiteral("QMYSQL") << QStringLiteral("127.0.0.1") << m_mysqlPort << 3 << 10000 << 50 << QStringLiteral("admin") << QStringLiteral("wYezOAT3elS9") << 0 << 0 << 0 << 30 << QStringLiteral("BLeon8WD70d7") << QStringLiteral("127.0.0.1") << m_imapPort;
}

QTEST_MAIN(CmdSetupTest)

#include "testcmdsetup.moc"
