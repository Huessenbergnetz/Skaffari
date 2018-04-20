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
//    QVERIFY(stopContainer());
}

void CmdSetupTest::cleanup()
{
    QSqlDatabase::removeDatabase(QStringLiteral(QSQLDBNAME));
}

void CmdSetupTest::doTest()
{
    QFETCH(QString, dbType);
    QFETCH(int, dbPort);
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
    QFETCH(int, imapPort);
    QFETCH(int, sievePort);
    QFETCH(QString, imapUser);
    QFETCH(QString, imapPass);
    QFETCH(int, imapEnc);
    QFETCH(QString, imapPeer);
    QFETCH(int, imapAuthMech);
    QFETCH(bool, unixHierarchySep);
    QFETCH(bool, domainAsPrefix);
    QFETCH(bool, fqun);
    QFETCH(int, createMailboxes);

    qDebug() << "AdmPwAlgo:" << admPwAlgo << "AuthMech:" << imapAuthMech << "UnixHierarchySep:" << unixHierarchySep << "DomainAsPrefix:" << domainAsPrefix << "FQUN:" << fqun << "CreateMailBoxes:" << createMailboxes;

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
    switch (imapAuthMech) {
    case 0:
        config.insert(QStringLiteral("SASL_MECH_LIST"), QStringLiteral("login plain"));
        break;
    case 1:
        config.insert(QStringLiteral("SASL_MECH_LIST"), QStringLiteral("login"));
        break;
    case 2:
        config.insert(QStringLiteral("SASL_MECH_LIST"), QStringLiteral("plain"));
        break;
    case 3:
        config.insert(QStringLiteral("SASL_MECH_LIST"), QStringLiteral("cram-md5"));
        break;
    default:
        QFAIL("Invalid imap authentication mechanism.");
        break;
    }
    const QString containerName = createContainerName();
    QVERIFY(startContainer(config, containerName, dbPort, imapPort, sievePort));

    QTemporaryFile confFile;
    QVERIFY(confFile.open());
    QSettings conf(confFile.fileName(), QSettings::IniFormat);
    QVERIFY(conf.status() == QSettings::NoError);

    SkCmdProc cmd;
    cmd.setShowOutput(false);
    if (imapAuthMech > 1) {
        cmd.setTimeOut(10000);
    }
    cmd.setArguments({QStringLiteral("--setup"), QStringLiteral("--ini"), confFile.fileName()});
    cmd.start();

    QVERIFY(cmd.waitForOutput());
    QVERIFY(cmd.enterString(dbType));

    QVERIFY(cmd.waitForOutput());
    QCOMPARE(cmd.write("127.0.0.1\n"), Q_INT64_C(10));

    QVERIFY(cmd.waitForOutput());
    QVERIFY(cmd.enterNumber(dbPort));

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
    QVERIFY(cmd.enterNumber(imapPort));

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
    QVERIFY(cmd.enterNumber(imapAuthMech));

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
    QCOMPARE(conf.value(QStringLiteral("port")).toInt(), dbPort);
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
    QCOMPARE(conf.value(QStringLiteral("port")).toInt(), imapPort);
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
    QCOMPARE(conf.value(QStringLiteral("createmailbox")).toInt(), createMailboxes);
    conf.endGroup();

    {
        QSqlDatabase db = QSqlDatabase::addDatabase(dbType, QStringLiteral(QSQLDBNAME));
        db.setDatabaseName(dbName);
        db.setUserName(dbUser);
        db.setPassword(dbPass);
        db.setHostName(QStringLiteral("127.0.0.1"));
        db.setPort(dbPort);
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

    QVERIFY(stopContainer(containerName));
}

void CmdSetupTest::doTest_data()
{
    QTest::addColumn<QString>("dbType");
    QTest::addColumn<int>("dbPort");
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
    QTest::addColumn<int>("imapPort");
    QTest::addColumn<int>("sievePort");
    QTest::addColumn<QString>("imapUser");
    QTest::addColumn<QString>("imapPass");
    QTest::addColumn<int>("imapEnc");
    QTest::addColumn<QString>("imapPeer");
    QTest::addColumn<int>("imapAuthMech");
    QTest::addColumn<bool>("unixHierarchySep");
    QTest::addColumn<bool>("domainAsPrefix");
    QTest::addColumn<bool>("fqun");
    QTest::addColumn<int>("createMailboxes");

    int testNo = 0;
    int port = 44819;

    for (int _admPwAlgo : {3,4,5,6,7,8,9,10}) {
        QTest::newRow(QStringLiteral("test-%1").arg(++testNo, 3, 10, QLatin1Char('0')).toUtf8().constData())
                << QStringLiteral("QMYSQL")             // db type
                << port++                               // db port
                << QStringLiteral("maildb")             // db name
                << QStringLiteral("skaffari")           // db user
                << QStringLiteral("LaN4TEsaLk2d")       // db pass
                << _admPwAlgo                           // admin password algo
                << 10000                                // admin password iterations
                << 50                                   // admin password quality threshold
                << QStringLiteral("admin")              // admin user name
                << QStringLiteral("wYezOAT3elS9")       // admin user password
                << 0                                    // account password method (0: unencrypted plain text)
                << 0                                    // account password algo (not needed for plain text)
                << 0                                    // account password iterations (not needed for plain text)
                << 30                                   // account password threshold
                << port++                               // imap port
                << port++                               // sieve port
                << QStringLiteral("cyrus")              // imap admin user
                << QStringLiteral("BLeon8WD70d7")       // imap admin password
                << 0                                    // imap transport encryption (0: unsecured)
                << QString()                            // imap ssl peer name (not used because connection is not encrypted)
                << 0                                    // imap authentication mechanism
                << false                                // use unix hierarchy separator
                << false                                // use domain as prefix (not available if unix hierarchy seperator is disabled)
                << false                                // use fully qualified use names (not available if unix hierarchy seperator or domain as prefix is disabled)
                << 0;                                   // create mailboxes
    }

    for (int _createMailboxes : {1,2,3}) {
        QTest::newRow(QStringLiteral("test-%1").arg(++testNo, 3, 10, QLatin1Char('0')).toUtf8().constData())
                << QStringLiteral("QMYSQL")             // db type
                << port++                               // db port
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
                << port++                               // imap port
                << port++                               // sieve port
                << QStringLiteral("cyrus")              // imap admin user
                << QStringLiteral("BLeon8WD70d7")       // imap admin password
                << 0                                    // imap transport encryption (0: unsecured)
                << QString()                            // imap ssl peer name (not used because connection is not encrypted)
                << 0                                    // imap authentication mechanism
                << false                                // use unix hierarchy separator
                << false                                // use domain as prefix (not available if unix hierarchy seperator is disabled)
                << false                                // use fully qualified use names (not available if unix hierarchy seperator or domain as prefix is disabled)
                << _createMailboxes;                    // create mailboxes
    }

    for (bool _domainAsPrefix : {false,true}) {
        for (bool _fqun : {false,true}) {
            QTest::newRow(QStringLiteral("test-%1").arg(++testNo, 3, 10, QLatin1Char('0')).toUtf8().constData())
                    << QStringLiteral("QMYSQL")             // db type
                    << port++                               // db port
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
                    << port++                               // imap port
                    << port++                               // sieve port
                    << QStringLiteral("cyrus")              // imap admin user
                    << QStringLiteral("BLeon8WD70d7")       // imap admin password
                    << 0                                    // imap transport encryption (0: unsecured)
                    << QString()                            // imap ssl peer name (not used because connection is not encrypted)
                    << 0                                    // imap authentication mechanism
                    << true                                 // use unix hierarchy separator
                    << _domainAsPrefix                      // use domain as prefix (not available if unix hierarchy seperator is disabled)
                    << _fqun                                // use fully qualified use names (not available if unix hierarchy seperator or domain as prefix is disabled)
                    << 0;                                   // create mailboxes
        }
    }

    for (int _authMech : {0,1,2,3}) {
        QTest::newRow(QStringLiteral("test-%1").arg(++testNo, 3, 10, QLatin1Char('0')).toUtf8().constData())
                << QStringLiteral("QMYSQL")             // db type
                << port++                               // db port
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
                << port++                               // imap port
                << port++                               // sieve port
                << QStringLiteral("cyrus")              // imap admin user
                << QStringLiteral("BLeon8WD70d7")       // imap admin password
                << 0                                    // imap transport encryption (0: unsecured)
                << QString()                            // imap ssl peer name (not used because connection is not encrypted)
                << _authMech                            // imap authentication mechanism
                << false                                // use unix hierarchy separator
                << false                                // use domain as prefix (not available if unix hierarchy seperator is disabled)
                << false                                // use fully qualified use names (not available if unix hierarchy seperator or domain as prefix is disabled)
                << 0;                                   // create mailboxes
    }

//    QTest::newRow(QStringLiteral("test-%1").arg(++testNo, 3, 10, QLatin1Char('0')).toUtf8().constData())
//            << QStringLiteral("QMYSQL")             // db type
//            << port++                               // db port
//            << QStringLiteral("maildb")             // db name
//            << QStringLiteral("skaffari")           // db user
//            << QStringLiteral("LaN4TEsaLk2d")       // db pass
//            << 3                                    // admin password algo
//            << 10000                                // admin password iterations
//            << 50                                   // admin password quality threshold
//            << QStringLiteral("admin")              // admin user name
//            << QStringLiteral("wYezOAT3elS9")       // admin user password
//            << 0                                    // account password method (0: unencrypted plain text)
//            << 0                                    // account password algo (not needed for plain text)
//            << 0                                    // account password iterations (not needed for plain text)
//            << 30                                   // account password threshold
//            << port++                               // imap port
//            << port++                               // sieve port
//            << QStringLiteral("cyrus")              // imap admin user
//            << QStringLiteral("BLeon8WD70d7")       // imap admin password
//            << 0                                    // imap transport encryption (0: unsecured)
//            << QString()                            // imap ssl peer name (not used because connection is not encrypted)
//            << 2                                    // imap authentication mechanism
//            << false                                // use unix hierarchy separator
//            << false                                // use domain as prefix (not available if unix hierarchy seperator is disabled)
//            << false                                // use fully qualified use names (not available if unix hierarchy seperator or domain as prefix is disabled)
//            << 0;                                   // create mailboxes
}

QTEST_MAIN(CmdSetupTest)

#include "testcmdsetup.moc"
