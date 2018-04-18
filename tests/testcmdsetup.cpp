#include "skapptestobject.h"
#include <QTest>
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
    void doTest();
    void doTest_data();

    void cleanup();
    void cleanupTestCase();
};

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
    QFETCH(QString, dbName);
    QFETCH(QString, dbUser);
    QFETCH(QString, dbPass);
    QFETCH(int, admPwAlgo);
    QFETCH(int, admPwIter);
    QFETCH(int, admPwThreshold);
    QFETCH(QString, adminName);
    QFETCH(QString, adminPass);
    QFETCH(int, accPwMethod);
    QFETCH(int, accPwAlgo);
    QFETCH(int, accPwRounds);
    QFETCH(int, accPwThreshold);
    QFETCH(QString, imapUser);
    QFETCH(QString, imapPass);
    QFETCH(int, imapEnc);
    QFETCH(QString, imapPeer);
    QFETCH(bool, unixHierarchySep);
    QFETCH(bool, domainAsPrefix);
    QFETCH(bool, fqun);
    QFETCH(int, createMailboxes);

    QMap<QString,QString> config{
        {QStringLiteral("SKAFFARI_DB_NAME"), dbName},
        {QStringLiteral("SKAFFARI_DB_USER"), dbUser},
        {QStringLiteral("SKAFFARI_DB_PASS"), dbPass},
        {QStringLiteral("IMAP_ADMINS"), imapUser},
        {QStringLiteral("UNIXHIERARCHYSEP"), unixHierarchySep ? QStringLiteral("yes") : QStringLiteral("no")},
        {QStringLiteral("VIRTDOMAINS"), domainAsPrefix ? QStringLiteral("yes") : QStringLiteral("no")},
        {QStringLiteral("SASL_PWCHECK_METHOD"), QStringLiteral("auxprop")},
        {QStringLiteral("SASL_AUXPROP_PLUGIN"), QStringLiteral("sql")},
        {QStringLiteral("SASL_SQL_HOSTNAMES"), QStringLiteral("127.0.0.1:3306")}

    };
    if (dbType == QLatin1String("QMYSQL")) {
        config.insert(QStringLiteral("SASL_SQL_ENGINE"), QStringLiteral("mysql"));
    } else {
        QFAIL("Invalid SQL database type.");
    }
    QVERIFY(startContainer(config));

    QTemporaryFile confFile;
    QVERIFY(confFile.open());
    QSettings conf(confFile.fileName(), QSettings::IniFormat);
    QVERIFY(conf.status() == QSettings::NoError);

    SkCmdProc cmd;
    cmd.setArguments({QStringLiteral("--setup"), QStringLiteral("--ini"), confFile.fileName()});
    cmd.start();

    QVERIFY(cmd.waitForOutput());
    QVERIFY(cmd.enterString(dbType));

    QVERIFY(cmd.waitForOutput());
    QCOMPARE(cmd.write("127.0.0.1\n"), Q_INT64_C(10));

    QVERIFY(cmd.waitForOutput());
    QVERIFY(cmd.enterNumber(m_mysqlPort));

    QVERIFY(cmd.waitForOutput());
    QVERIFY(cmd.enterString(dbName));

    QVERIFY(cmd.waitForOutput());
    QVERIFY(cmd.enterString(dbUser));

    QVERIFY(cmd.waitForOutput());
    QVERIFY(cmd.enterString(dbPass));

    QVERIFY(cmd.waitForOutput());
    QVERIFY(cmd.enterNumber(admPwAlgo));

    QVERIFY(cmd.waitForOutput());
    QVERIFY(cmd.enterNumber(admPwIter));

    QVERIFY(cmd.waitForOutput());
    QVERIFY(cmd.enterNumber(admPwThreshold));

    QVERIFY(cmd.waitForOutput());
    QVERIFY(cmd.write("\n"));

    QVERIFY(cmd.waitForOutput());
    QVERIFY(cmd.enterString(adminName));

    QVERIFY(cmd.waitForOutput());
    QVERIFY(cmd.enterString(adminPass));

    QVERIFY(cmd.waitForOutput());
    QVERIFY(cmd.enterBool(true));

    QVERIFY(cmd.waitForOutput());
    QVERIFY(cmd.enterNumber(accPwMethod));

    if (accPwMethod == 1 || accPwMethod == 2) {
        QVERIFY(cmd.waitForOutput());
        QVERIFY(cmd.enterNumber(accPwAlgo));
    }

    if ((accPwMethod == 1) && (accPwAlgo == 3 || accPwAlgo == 4 || accPwAlgo == 5)) {
        QVERIFY(cmd.waitForOutput());
        QVERIFY(cmd.enterNumber(accPwRounds));
    }

    QVERIFY(cmd.waitForOutput());
    QVERIFY(cmd.enterNumber(accPwThreshold));

    QVERIFY(cmd.waitForOutput());
    QCOMPARE(cmd.write("\n"), Q_INT64_C(1)); // do not use a settings file for pwquality

    QVERIFY(cmd.waitForOutput());
    QVERIFY(cmd.enterString(imapUser));

    QVERIFY(cmd.waitForOutput());
    QVERIFY(cmd.enterString(imapPass));

    QVERIFY(cmd.waitForOutput());

    QVERIFY(cmd.waitForOutput());
    QVERIFY(cmd.write("127.0.0.1\n"));

    QVERIFY(cmd.waitForOutput());
    QVERIFY(cmd.enterNumber(m_imapPort));

    QVERIFY(cmd.waitForOutput());
    QVERIFY(cmd.enterString(imapUser));

    QVERIFY(cmd.waitForOutput());
    QVERIFY(cmd.enterString(imapPass));

    QVERIFY(cmd.waitForOutput());
    QVERIFY(cmd.enterNumber(0));

    QVERIFY(cmd.waitForOutput());
    QVERIFY(cmd.enterNumber(imapEnc));

    if (imapEnc > 0) {
        QVERIFY(cmd.waitForOutput());
        QVERIFY(cmd.enterString(imapPeer));
    }

    QVERIFY(cmd.waitForOutput());
    QVERIFY(cmd.enterBool(true));

    QVERIFY(cmd.waitForOutput());
    QVERIFY(cmd.enterBool(unixHierarchySep));

    if (unixHierarchySep) {
        QVERIFY(cmd.waitForOutput());
        QVERIFY(cmd.enterBool(domainAsPrefix));
    }

    if (unixHierarchySep && domainAsPrefix) {
        QVERIFY(cmd.waitForOutput());
        QVERIFY(cmd.enterBool(fqun));
    }

    QVERIFY(cmd.waitForOutput());
    QVERIFY(cmd.enterNumber(createMailboxes));

    QVERIFY(cmd.waitForOutput());

    QTRY_COMPARE(cmd.state(), QProcess::NotRunning);
    QCOMPARE(cmd.exitStatus(), QProcess::NormalExit);
    QCOMPARE(cmd.exitCode(), 0);

    conf.sync();
    conf.beginGroup(QStringLiteral("Database"));
    QCOMPARE(conf.value(QStringLiteral("type")).toString(), dbType);
    QCOMPARE(conf.value(QStringLiteral("host")).toString(), QStringLiteral("127.0.0.1"));
    QCOMPARE(conf.value(QStringLiteral("port")).toInt(), m_mysqlPort);
    QCOMPARE(conf.value(QStringLiteral("name")).toString(), dbName);
    QCOMPARE(conf.value(QStringLiteral("user")).toString(), dbUser);
    QCOMPARE(conf.value(QStringLiteral("password")).toString(), dbPass);
    conf.endGroup();

    conf.beginGroup(QStringLiteral("Admins"));
    QCOMPARE(conf.value(QStringLiteral("pwalgorithm")).toInt(), admPwAlgo);
    QCOMPARE(conf.value(QStringLiteral("pwrounds")).toInt(), admPwIter);
    QCOMPARE(conf.value(QStringLiteral("pwthreshold")).toInt(), admPwThreshold);
    conf.endGroup();

    conf.beginGroup(QStringLiteral("Accounts"));
    QCOMPARE(conf.value(QStringLiteral("pwmethod")).toInt(), accPwMethod);
    if (accPwMethod == 1 || accPwMethod == 2) {
        QCOMPARE(conf.value(QStringLiteral("pwalgorithm")).toInt(), accPwAlgo);
    }
    if ((accPwMethod == 1) && (accPwAlgo == 3 || accPwAlgo == 4 || accPwAlgo == 5)) {
        QCOMPARE(conf.value(QStringLiteral("pwrounds")).toInt(), accPwRounds);
    }
    QCOMPARE(conf.value(QStringLiteral("pwthreshold")).toInt(), accPwThreshold);
    conf.endGroup();

    conf.beginGroup(QStringLiteral("IMAP"));
    QCOMPARE(conf.value(QStringLiteral("host")).toString(), QStringLiteral("127.0.0.1"));
    QCOMPARE(conf.value(QStringLiteral("port")).toInt(), m_imapPort);
    QCOMPARE(conf.value(QStringLiteral("protocol")).toInt(), 0);
    QCOMPARE(conf.value(QStringLiteral("encryption")).toInt(), imapEnc);
    QCOMPARE(conf.value(QStringLiteral("user")).toString(), imapUser);
    QCOMPARE(conf.value(QStringLiteral("unixhierarchysep")).toBool(), unixHierarchySep);
    if (unixHierarchySep) {
        QCOMPARE(conf.value(QStringLiteral("domainasprefix")).toBool(), domainAsPrefix);
    }
    if (unixHierarchySep && domainAsPrefix) {
        QCOMPARE(conf.value(QStringLiteral("fqun")).toBool(), fqun);
    }
    QCOMPARE(conf.value(QStringLiteral("createmailbox")).toBool(), createMailboxes);
    conf.endGroup();

    {
        QSqlDatabase db = QSqlDatabase::addDatabase(dbType, QStringLiteral(QSQLDBNAME));
        db.setDatabaseName(dbName);
        db.setUserName(dbUser);
        db.setPassword(dbPass);
        db.setHostName(QStringLiteral("127.0.0.1"));
        db.setPort(m_mysqlPort);
        QVERIFY(db.open());

        QSqlQuery q(QSqlDatabase::database(QStringLiteral(QSQLDBNAME)));
        QVERIFY(q.exec(QStringLiteral("SELECT username, password, type FROM adminuser")));
        QCOMPARE(q.size(), 1);
        QVERIFY(q.next());
        QCOMPARE(q.value(0).toString(), adminName);
        QVERIFY(!q.value(1).toString().isEmpty());
        QCOMPARE(q.value(2).toInt(), 255);

        QVERIFY(q.exec(QStringLiteral("SELECT domain_id, username, password FROM accountuser")));
        QCOMPARE(q.size(), 1);
        QVERIFY(q.next());
        QCOMPARE(q.value(0).toInt(), 0);
        QCOMPARE(q.value(1).toString(), imapUser);
        if (accPwMethod == 0) {
            QCOMPARE(q.value(2).toString(), imapPass);
        } else {
            QVERIFY(!q.value(2).toString().isEmpty());
            QVERIFY(q.value(2).toString() != imapPass);
        }
    }

    QSqlDatabase::removeDatabase(QStringLiteral(QSQLDBNAME));

    QVERIFY(stopContainer());
}

void CmdSetupTest::doTest_data()
{
    QTest::addColumn<QString>("dbType");
    QTest::addColumn<QString>("dbName");
    QTest::addColumn<QString>("dbUser");
    QTest::addColumn<QString>("dbPass");
    QTest::addColumn<int>("admPwAlgo");
    QTest::addColumn<int>("admPwIter");
    QTest::addColumn<int>("admPwThreshold");
    QTest::addColumn<QString>("adminName");
    QTest::addColumn<QString>("adminPass");
    QTest::addColumn<int>("accPwMethod");
    QTest::addColumn<int>("accPwAlgo");
    QTest::addColumn<int>("accPwRounds");
    QTest::addColumn<int>("accPwThreshold");
    QTest::addColumn<QString>("imapUser");
    QTest::addColumn<QString>("imapPass");
    QTest::addColumn<int>("imapEnc");
    QTest::addColumn<QString>("imapPeer");
    QTest::addColumn<bool>("unixHierarchySep");
    QTest::addColumn<bool>("domainAsPrefix");
    QTest::addColumn<bool>("fqun");
    QTest::addColumn<int>("createMailboxes");

    QTest::newRow("test-00")
            << QStringLiteral("QMYSQL")             // db type
            << QStringLiteral("maildb")             // db name
            << QStringLiteral("skaffari")           // db user
            << QStringLiteral("LaN4TEsaLk2d")       // db pass
            << 3                                    // admin password algo
            << 10000                                // admin password iterations
            << 50                                   // admin password quality threshold
            << QStringLiteral("admin")              // admin user name
            << QStringLiteral("wYezOAT3elS9")       // admin user password
            << 0                                    // account password method (0: unencrypted plain text)
            << 0                                    // account password algo (not needed for plain text)
            << 0                                    // account password iterations (not needed for plain text)
            << 30                                   // account password threshold
            << QStringLiteral("cyrus")              // imap admin user
            << QStringLiteral("BLeon8WD70d7")       // imap admin password
            << 0                                    // imap transport encryption (0: unsecured)
            << QString()                            // imap ssl peer name (not used because connection is not encrypted)
            << false                                // use unix hierarchy separator
            << false                                // use domain as prefix (not available if unix hierarchy seperator is disabled)
            << false                                // use fully qualified use names (not available if unix hierarchy seperator or domain as prefix is disabled)
            << 0                                    // create mailboxes
;
}

QTEST_MAIN(CmdSetupTest)

#include "testcmdsetup.moc"
