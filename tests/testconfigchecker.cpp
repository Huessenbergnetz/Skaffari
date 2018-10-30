#include "../cmd/configchecker.h"
#include "../common/config.h"
#include <QTest>
#include <QSettings>
#include <QTemporaryFile>

class ConfigCheckerTest : public QObject
{
    Q_OBJECT
public:
    explicit ConfigCheckerTest(QObject *parent = nullptr) : QObject(parent) {}

private Q_SLOTS:
    void initTestCase();
    void init();

    void allValid();
    void dbTypeMissing();
    void dbTypeEmpty();
    void dbTypeInvalid();
    void dbHostMissing();
    void dbHostEmpty();
    void dbHostInvalidString();
    void dbHostInvalidIPv4();
    void dbHostInvalidIPv6();
    void dbPortMissing();
    void dbPortEmpty();
    void dbPortInvalid();
    void dbNameMissing();
    void dbNameEmpty();
    void dbUserMissing();
    void dbUserEmpty();
    void adminPwAlgoTooLow();
    void adminPwAlgoTooHigh();
    void adminPwAlgoEmpty();
    void adminPwAlgoMissing();
    void adminPwRoundsTooLow();
    void adminPwRoundsEmpty();
    void adminPwRoundsMissing();
#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
    void adminPwThresholdTooLow();
    void adminPwThresholdTooHigh();
    void adminPwThresholdEmpty();
    void adminPwThresholdMissing();
#else
    void adminPwMinlengthTooHigh();
    void adminPwMinlengthEmpty();
    void adminPwMinlengthMissing();
#endif
    void accPwMethodTooLow();
    void accPwMethodTooHigh();
    void accPwMethodEmpty();
    void accPwMethodMissing();
    void accPwCryptAlgoTooLow();
    void accPwCryptAlgoTooHigh();
    void accPwCryptAlogMissing();
    void accPwCryptAlgoEmpty();
    void accPwMysqlAlgoTooLow();
    void accPwMysqlAlgoTooHigh();
    void accPwMysqlAlgoMissing();
    void accPwMysqlAlgoEmpty();
    void accPwCryptBcryptRoundsTooLow();
    void accPwCryptBcryptRoundsTooHigh();
    void accPwCryptBcryptRoundsMissing();
    void accPwCryptBcryptRoundsEmpty();
    void accPwCryptSha256RoundsTooLow();
    void accPwCryptSha256RoundsTooHigh();
    void accPwCryptSha256RoundsMissing();
    void accPwCryptSha256RoundsEmpty();
    void accPwCryptSha512RoundsTooLow();
    void accPwCryptSha512RoundsTooHigh();
    void accPwCryptSha512RoundsMissing();
    void accPwCryptSha512RoundsEmpty();
#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
    void accPwThresholdTooLow();
    void accPwThresholdTooHigh();
    void accPwThresholdEmpty();
    void accPwThresholdMissing();
#else
    void accPwMinlengthTooHigh();
    void accPwMinlengthEmpty();
    void accPwMinlengthMissing();
#endif
    void imapHostMissing();
    void imapHostEmpty();
    void imapHostInvalidString();
    void imapHostInvalidIPv4();
    void imapHostInvalidIPv6();
    void imapPortMissing();
    void imapPortEmpty();
    void imapPortInvalid();
    void imapProtocolTooLow();
    void imapProtocolTooHigh();
    void imapProtocolMissing();
    void imapProtocolEmpty();
    void imapEncryptionTooLow();
    void imapEncryptionTooHigh();
    void imapEncryptionMissing();
    void imapEncryptionEmpty();
    void imapAuthmechTooLow();
    void imapAuthmechTooHigh();
    void imapAuthmechMissing();
    void imapAuthmechEmpty();
    void imapUserMissing();
    void imapUserEmpty();
    void imapPasswordMissing();
    void imapPasswordEmpty();
    void imapCreatemailboxTooLow();
    void imapCreatemailboxTooHigh();
    void imapCreatemailboxMissing();
    void imapCreatemailboxEmpty();

    void cleanupTestCase() {}
    void cleanup();

private:
    QTemporaryFile m_confFile;
    void setConfValue(const QString &key, const QVariant &value);
    void removeConfKey(const QString &key);
};

void ConfigCheckerTest::initTestCase()
{
    QVERIFY(m_confFile.open());
}

void ConfigCheckerTest::init()
{
    QSettings s(m_confFile.fileName(), QSettings::IniFormat);

    s.beginGroup(QStringLiteral("Database"));
    s.setValue(QStringLiteral("type"), QStringLiteral("QMYSQL"));
    s.setValue(QStringLiteral("host"), QStringLiteral("localhost"));
    s.setValue(QStringLiteral("port"), 3306);
    s.setValue(QStringLiteral("name"), QStringLiteral("skaffaridb"));
    s.setValue(QStringLiteral("user"), QStringLiteral("skaffari"));
    s.setValue(QStringLiteral("password"), QStringLiteral("EeA3bT6yNzYS"));
    s.endGroup();

    s.beginGroup(QStringLiteral("Admins"));
    s.setValue(QStringLiteral("pwalgorithm"), 6);
    s.setValue(QStringLiteral("pwrounds"), 140000);
#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
    s.setValue(QStringLiteral("pwthreshold"), 60);
#else
    s.setValue(QStringLiteral("pwminlength"), 10);
#endif
    s.endGroup();

    s.beginGroup(QStringLiteral("Accounts"));
    s.setValue(QStringLiteral("pwmethod"), 1);
    s.setValue(QStringLiteral("pwalgorithm"), 5);
    s.setValue(QStringLiteral("pwrounds"), 12);
#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
    s.setValue(QStringLiteral("pwthreshold"), 30);
#else
    s.setValue(QStringLiteral("pwminlength"), 8);
#endif
    s.endGroup();

    s.beginGroup(QStringLiteral("IMAP"));
    s.setValue(QStringLiteral("host"), QStringLiteral("localhost"));
    s.setValue(QStringLiteral("port"), 143);
    s.setValue(QStringLiteral("protocol"), 2);
    s.setValue(QStringLiteral("encryption"), 1);
    s.setValue(QStringLiteral("authmech"), 1);
    s.setValue(QStringLiteral("user"), QStringLiteral("cyrus"));
    s.setValue(QStringLiteral("password"), QStringLiteral("NSuo8hWskQNU"));
    s.setValue(QStringLiteral("createmailbox"), 3);
    s.endGroup();

    s.sync();
}

void ConfigCheckerTest::cleanup()
{
    QSettings s(m_confFile.fileName(), QSettings::IniFormat);
    s.clear();
    s.sync();
}

void ConfigCheckerTest::setConfValue(const QString &key, const QVariant &value)
{
    QSettings s(m_confFile.fileName(), QSettings::IniFormat);
    s.setValue(key, value);
    s.sync();
}

void ConfigCheckerTest::removeConfKey(const QString &key)
{
    QSettings s(m_confFile.fileName(), QSettings::IniFormat);
    s.remove(key);
    s.sync();
}

void ConfigCheckerTest::allValid()
{
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 0);
}

void ConfigCheckerTest::dbTypeMissing()
{
    removeConfKey(QStringLiteral("Database/type"));
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 0);
}

void ConfigCheckerTest::dbTypeEmpty()
{
    setConfValue(QStringLiteral("Database/type"), QString());
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::dbTypeInvalid()
{
    setConfValue(QStringLiteral("Database/type"), QStringLiteral("MYOWNDBTYPE"));
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::dbHostMissing()
{
    removeConfKey(QStringLiteral("Database/host"));
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 0);
}

void ConfigCheckerTest::dbHostEmpty()
{
    setConfValue(QStringLiteral("Database/host"), QString());
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::dbHostInvalidString()
{
    setConfValue(QStringLiteral("Database/host"), QStringLiteral("foo bar"));
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::dbHostInvalidIPv4()
{
    setConfValue(QStringLiteral("Database/host"), QStringLiteral("256.256.256.256"));
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::dbHostInvalidIPv6()
{
    setConfValue(QStringLiteral("Database/host"), QStringLiteral("200g:0db8:85a3:08d3:1319:8a2e:0370:7344"));
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::dbPortMissing()
{
    removeConfKey(QStringLiteral("Database/port"));
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 0);
}

void ConfigCheckerTest::dbPortEmpty()
{
    setConfValue(QStringLiteral("Database/port"), QString());
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::dbPortInvalid()
{
    setConfValue(QStringLiteral("Database/port"), 123456);
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::dbNameMissing()
{
    removeConfKey(QStringLiteral("Database/name"));
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::dbNameEmpty()
{
    setConfValue(QStringLiteral("Database/name"), QString());
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::dbUserMissing()
{
    removeConfKey(QStringLiteral("Database/user"));
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::dbUserEmpty()
{
    setConfValue(QStringLiteral("Database/user"), QString());
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::adminPwAlgoTooLow()
{
    setConfValue(QStringLiteral("Admins/pwalgorithm"), SK_MIN_ADM_PWALGORITHM - 1);
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::adminPwAlgoTooHigh()
{
    setConfValue(QStringLiteral("Admins/pwalgorithm"), SK_MAX_ADM_PWALGORITHM + 1);
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::adminPwAlgoEmpty()
{
    setConfValue(QStringLiteral("Admins/pwalgorithm"), QString());
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::adminPwAlgoMissing()
{
    removeConfKey(QStringLiteral("Admins/pwalgorithm"));
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 0);
}

void ConfigCheckerTest::adminPwRoundsTooLow()
{
    setConfValue(QStringLiteral("Admins/pwrounds"), 999);
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::adminPwRoundsEmpty()
{
    setConfValue(QStringLiteral("Admins/pwrounds"), QString());
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::adminPwRoundsMissing()
{
    removeConfKey(QStringLiteral("Admins/pwrounds"));
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 0);
}

#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
void ConfigCheckerTest::adminPwThresholdTooLow()
{
    setConfValue(QStringLiteral("Admins/pwthreshold"), -1);
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::adminPwThresholdTooHigh()
{
    setConfValue(QStringLiteral("Admins/pwthreshold"), 101);
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::adminPwThresholdEmpty()
{
    setConfValue(QStringLiteral("Admins/pwthreshold"), QString());
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 0);
}

void ConfigCheckerTest::adminPwThresholdMissing()
{
    removeConfKey(QStringLiteral("Admins/pwthreshold"));
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 0);
}
#else
void ConfigCheckerTest::adminPwMinlengthTooHigh()
{
    setConfValue(QStringLiteral("Admins/pwminlength"), 256);
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::adminPwMinlengthEmpty()
{
    setConfValue(QStringLiteral("Admins/pwminlength"), QString());
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 0);
}

void ConfigCheckerTest::adminPwMinlengthMissin()
{
    removeConfKey(QStringLiteral("Admins/pwminlength"));
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 0);
}
#endif

void ConfigCheckerTest::accPwMethodTooLow()
{
    setConfValue(QStringLiteral("Accounts/pwmethod"), -1);
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::accPwMethodTooHigh()
{
    setConfValue(QStringLiteral("Accounts/pwmethod"), SK_MAX_ACC_PWMETHOD + 1);
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::accPwMethodMissing()
{
    removeConfKey(QStringLiteral("Accounts/pwmethod"));
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 0);
}

void ConfigCheckerTest::accPwMethodEmpty()
{
    setConfValue(QStringLiteral("Accounts/pwmethod"), QString());
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 0);
}

void ConfigCheckerTest::accPwCryptAlgoTooLow()
{
    setConfValue(QStringLiteral("Accounts/pwmethod"), 1);
    setConfValue(QStringLiteral("Accounts/pwalgorithm"), -1);
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::accPwCryptAlgoTooHigh()
{
    setConfValue(QStringLiteral("Accounts/pwmethod"), 1);
    setConfValue(QStringLiteral("Accounts/pwalgorithm"), 32);
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::accPwCryptAlogMissing()
{
    setConfValue(QStringLiteral("Accounts/pwmethod"), 1);
    removeConfKey(QStringLiteral("Accounts/pwalgorithm"));
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 0);
}

void ConfigCheckerTest::accPwCryptAlgoEmpty()
{
    setConfValue(QStringLiteral("Accounts/pwmethod"), 1);
    setConfValue(QStringLiteral("Accounts/pwalgorithm"), QString());
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 0);
}

void ConfigCheckerTest::accPwMysqlAlgoTooLow()
{
    setConfValue(QStringLiteral("Accounts/pwmethod"), 2);
    setConfValue(QStringLiteral("Accounts/pwalgorithm"), 5);
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::accPwMysqlAlgoTooHigh()
{
    setConfValue(QStringLiteral("Accounts/pwmethod"), 2);
    setConfValue(QStringLiteral("Accounts/pwalgorithm"), 255);
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::accPwMysqlAlgoMissing()
{
    setConfValue(QStringLiteral("Accounts/pwmethod"), 2);
    removeConfKey(QStringLiteral("Accounts/pwalgorithm"));
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 0);
}

void ConfigCheckerTest::accPwMysqlAlgoEmpty()
{
    setConfValue(QStringLiteral("Accounts/pwmethod"), 2);
    setConfValue(QStringLiteral("Accounts/pwalgorithm"), QString());
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 0);
}

void ConfigCheckerTest::accPwCryptBcryptRoundsTooLow()
{
    setConfValue(QStringLiteral("Accounts/pwmethod"), 1);
    setConfValue(QStringLiteral("Accounts/pwalgorithm"), 5);
    setConfValue(QStringLiteral("Accounts/pwrounds"), 1);
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::accPwCryptBcryptRoundsTooHigh()
{
    setConfValue(QStringLiteral("Accounts/pwmethod"), 1);
    setConfValue(QStringLiteral("Accounts/pwalgorithm"), 5);
    setConfValue(QStringLiteral("Accounts/pwrounds"), 1000);
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::accPwCryptBcryptRoundsMissing()
{
    setConfValue(QStringLiteral("Accounts/pwmethod"), 1);
    setConfValue(QStringLiteral("Accounts/pwalgorithm"), 5);
    removeConfKey(QStringLiteral("Accounts/pwrounds"));
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::accPwCryptBcryptRoundsEmpty()
{
    setConfValue(QStringLiteral("Accounts/pwmethod"), 1);
    setConfValue(QStringLiteral("Accounts/pwalgorithm"), 5);
    setConfValue(QStringLiteral("Accounts/pwrounds"), QString());
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::accPwCryptSha256RoundsTooLow()
{
    setConfValue(QStringLiteral("Accounts/pwmethod"), 1);
    setConfValue(QStringLiteral("Accounts/pwalgorithm"), 3);
    setConfValue(QStringLiteral("Accounts/pwrounds"), 4);
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::accPwCryptSha256RoundsTooHigh()
{
    setConfValue(QStringLiteral("Accounts/pwmethod"), 1);
    setConfValue(QStringLiteral("Accounts/pwalgorithm"), 3);
    setConfValue(QStringLiteral("Accounts/pwrounds"), 999999999 + 1);
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::accPwCryptSha256RoundsMissing()
{
    setConfValue(QStringLiteral("Accounts/pwmethod"), 1);
    setConfValue(QStringLiteral("Accounts/pwalgorithm"), 3);
    removeConfKey(QStringLiteral("Accounts/pwrounds"));
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 0);
}

void ConfigCheckerTest::accPwCryptSha256RoundsEmpty()
{
    setConfValue(QStringLiteral("Accounts/pwmethod"), 1);
    setConfValue(QStringLiteral("Accounts/pwalgorithm"), 3);
    setConfValue(QStringLiteral("Accounts/pwrounds"), QString());
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::accPwCryptSha512RoundsTooLow()
{
    setConfValue(QStringLiteral("Accounts/pwmethod"), 1);
    setConfValue(QStringLiteral("Accounts/pwalgorithm"), 4);
    setConfValue(QStringLiteral("Accounts/pwrounds"), 4);
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::accPwCryptSha512RoundsTooHigh()
{
    setConfValue(QStringLiteral("Accounts/pwmethod"), 1);
    setConfValue(QStringLiteral("Accounts/pwalgorithm"), 4);
    setConfValue(QStringLiteral("Accounts/pwrounds"), 999999999 + 1);
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::accPwCryptSha512RoundsMissing()
{
    setConfValue(QStringLiteral("Accounts/pwmethod"), 1);
    setConfValue(QStringLiteral("Accounts/pwalgorithm"), 4);
    removeConfKey(QStringLiteral("Accounts/pwrounds"));
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 0);
}

void ConfigCheckerTest::accPwCryptSha512RoundsEmpty()
{
    setConfValue(QStringLiteral("Accounts/pwmethod"), 1);
    setConfValue(QStringLiteral("Accounts/pwalgorithm"), 4);
    setConfValue(QStringLiteral("Accounts/pwrounds"), QString());
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
void ConfigCheckerTest::accPwThresholdTooLow()
{
    setConfValue(QStringLiteral("Accounts/pwthreshold"), -1);
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::accPwThresholdTooHigh()
{
    setConfValue(QStringLiteral("Accounts/pwthreshold"), 101);
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::accPwThresholdEmpty()
{
    setConfValue(QStringLiteral("Accounts/pwthreshold"), QString());
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 0);
}

void ConfigCheckerTest::accPwThresholdMissing()
{
    removeConfKey(QStringLiteral("Accounts/pwthreshold"));
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 0);
}
#else
void ConfigCheckerTest::accPwMinlengthTooHigh()
{
    setConfValue(QStringLiteral("Accounts/pwminlength"), 256);
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::accPwMinlengthEmpty()
{
    setConfValue(QStringLiteral("Accounts/pwminlength"), QString());
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 0);
}

void ConfigCheckerTest::accPwMinlengthMissin()
{
    removeConfKey(QStringLiteral("Accounts/pwminlength"));
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 0);
}
#endif

void ConfigCheckerTest::imapHostMissing()
{
    removeConfKey(QStringLiteral("IMAP/host"));
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 0);
}

void ConfigCheckerTest::imapHostEmpty()
{
    setConfValue(QStringLiteral("IMAP/host"), QString());
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::imapHostInvalidString()
{
    setConfValue(QStringLiteral("IMAP/host"), QStringLiteral("foo bar"));
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::imapHostInvalidIPv4()
{
    setConfValue(QStringLiteral("IMAP/host"), QStringLiteral("256.256.256.256"));
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::imapHostInvalidIPv6()
{
    setConfValue(QStringLiteral("IMAP/host"), QStringLiteral("200g:0db8:85a3:08d3:1319:8a2e:0370:7344"));
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::imapPortMissing()
{
    removeConfKey(QStringLiteral("IMAP/port"));
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 0);
}

void ConfigCheckerTest::imapPortEmpty()
{
    setConfValue(QStringLiteral("IMAP/port"), QString());
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::imapPortInvalid()
{
    setConfValue(QStringLiteral("IMAP/port"), 123456);
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::imapProtocolTooLow()
{
    setConfValue(QStringLiteral("IMAP/protocol"), -1);
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::imapProtocolTooHigh()
{
    setConfValue(QStringLiteral("IMAP/protocol"), SK_MAX_IMAP_PROTOCOL + 1);
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::imapProtocolMissing()
{
    removeConfKey(QStringLiteral("IMAP/protocol"));
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 0);
}

void ConfigCheckerTest::imapProtocolEmpty()
{
    setConfValue(QStringLiteral("IMAP/protocol"), QString());
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 0);
}

void ConfigCheckerTest::imapEncryptionTooLow()
{
    setConfValue(QStringLiteral("IMAP/protocol"), -1);
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::imapEncryptionTooHigh()
{
    setConfValue(QStringLiteral("IMAP/protocol"), SK_MAX_IMAP_ENCRYPTION + 1);
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::imapEncryptionMissing()
{
    removeConfKey(QStringLiteral("IMAP/protocol"));
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 0);
}

void ConfigCheckerTest::imapEncryptionEmpty()
{
    setConfValue(QStringLiteral("IMAP/protocol"), QString());
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 0);
}

void ConfigCheckerTest::imapAuthmechTooLow()
{
    setConfValue(QStringLiteral("IMAP/authmech"), -1);
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::imapAuthmechTooHigh()
{
    setConfValue(QStringLiteral("IMAP/authmech"), SK_MAX_IMAP_AUTHMECH + 1);
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::imapAuthmechMissing()
{
    removeConfKey(QStringLiteral("IMAP/authmech"));
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 0);
}

void ConfigCheckerTest::imapAuthmechEmpty()
{
    setConfValue(QStringLiteral("IMAP/authmech"), QString());
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 0);
}

void ConfigCheckerTest::imapUserMissing()
{
    removeConfKey(QStringLiteral("IMAP/user"));
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::imapUserEmpty()
{
    setConfValue(QStringLiteral("IMAP/user"), QString());
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::imapPasswordMissing()
{
    removeConfKey(QStringLiteral("IMAP/password"));
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::imapPasswordEmpty()
{
    setConfValue(QStringLiteral("IMAP/password"), QString());
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::imapCreatemailboxTooLow()
{
    setConfValue(QStringLiteral("IMAP/createmailbox"), -1);
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::imapCreatemailboxTooHigh()
{
    setConfValue(QStringLiteral("IMAP/createmailbox"), SK_MAX_IMAP_CREATEMAILBOX + 1);
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 2);
}

void ConfigCheckerTest::imapCreatemailboxMissing()
{
    removeConfKey(QStringLiteral("IMAP/createmailbox"));
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 0);
}

void ConfigCheckerTest::imapCreatemailboxEmpty()
{
    setConfValue(QStringLiteral("IMAP/createmailbox"), QString());
    ConfigChecker cc(m_confFile.fileName());
    QCOMPARE(cc.exec(), 0);
}

QTEST_MAIN(ConfigCheckerTest)

#include "testconfigchecker.moc"
