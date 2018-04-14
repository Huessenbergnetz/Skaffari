#include "skapptestobject.h"
#include <QTest>
#include <QTextStream>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QFileInfo>
#include <QStringList>

#define MYSQL_UG "mysql:mysql"
#define CYRUS_UG "cyrus:mail"

SkAppTestObject::SkAppTestObject(QObject *parent) : QObject(parent)
{
    bool autoRemove = true;
    m_cyrusConfDir.setAutoRemove(autoRemove);
    m_cyrusDataDir.setAutoRemove(autoRemove);
    m_cyrusSieveDir.setAutoRemove(autoRemove);
    m_cyrusPidDir.setAutoRemove(autoRemove);
    m_cyrusAnnotaionsFile.setAutoRemove(autoRemove);
    m_cyrusConfFile.setAutoRemove(autoRemove);
    m_cyrusImapFile.setAutoRemove(autoRemove);
}

bool SkAppTestObject::checkRootUser() const
{
    return QDir::home().dirName() == QLatin1String("root");
}

bool SkAppTestObject::startMysql()
{
    if (!m_mysqlWorkingDir.isValid()) {
        qCritical() << "Can not create mysql working directory.";
        return false;
    }
    chown(m_mysqlWorkingDir.path(), QStringLiteral(MYSQL_UG));

    if (!m_mysqlDataDir.isValid()) {
        qCritical() << "Can not create mysql data directory.";
        return false;
    }
    chown(m_mysqlDataDir.path(), QStringLiteral(MYSQL_UG), 750);

    if (!m_mysqlSocketDir.isValid()) {
        qCritical() << "Can not create mysql socket directory.";
        return false;
    }
    chown(m_mysqlSocketDir.path(), QStringLiteral(MYSQL_UG));

    if (!m_mysqlLogDir.isValid()) {
        qCritical() << "Can not create mysql log directory.";
        return false;
    }
    chown(m_mysqlLogDir.path(), QStringLiteral(MYSQL_UG), 750);

    if (!m_mysqlConfigFile.open()) {
        qCritical() << "Can not open mysql config file.";
        return false;
    }

    {
        QTextStream mysqlConfOut(&m_mysqlConfigFile);
        mysqlConfOut.setCodec("UTF-8");
        mysqlConfOut << "[mysqld]" << endl;
        mysqlConfOut << "bind-address=127.0.0.1" << endl;
        mysqlConfOut << "log-error=" << m_mysqlLogDir.filePath(QStringLiteral("mysql.log")) << endl;
        mysqlConfOut << "innodb_file_format=Barracuda" << endl;
        mysqlConfOut << "innodb_file_per_table=ON" << endl;
        mysqlConfOut << "datadir=" << m_mysqlDataDir.path() << endl;
        mysqlConfOut << "port=" << m_mysqlPort << endl;
        mysqlConfOut << "socket=" << m_mysqlSocketDir.filePath(QStringLiteral("mysql.sock")) << endl;
        mysqlConfOut << "server-id=1" << endl;
        mysqlConfOut.flush();
    }

    chown(m_mysqlConfigFile.fileName(), QStringLiteral("root:mysql"), 640);

    const QString mysqlDefaultsArg = QLatin1String("--defaults-file=") + m_mysqlConfigFile.fileName();
    const QStringList mysqlInstallArgs{QStringLiteral("--force"), QStringLiteral("--skip-auth-anonymous-user"), QStringLiteral("--auth-root-authentication-method=normal"), mysqlDefaultsArg, QStringLiteral("--user=mysql")};

    if (QProcess::execute(QStringLiteral("/usr/bin/mysql_install_db"), mysqlInstallArgs) != 0) {
        qCritical() << "Can not install initial mysql databases.";
        return false;
    }

    QFileInfo socketFi(m_mysqlSocketDir.filePath(QStringLiteral("mysql.sock")));

    m_mysqlProcess.setArguments({mysqlDefaultsArg, QStringLiteral("--user=mysql")});
    m_mysqlProcess.setProgram(QStringLiteral("/usr/sbin/mysqld"));
    m_mysqlProcess.setWorkingDirectory(m_mysqlWorkingDir.path());
    m_mysqlProcess.start();

    auto cur = QDateTime::currentDateTime();
    while (!socketFi.exists() && (cur.msecsTo(QDateTime::currentDateTime()) < 5000)) {

    }

    if (!socketFi.exists()) {
        qCritical() << "Failed to start mysql server within 5 seconds.";
        qDebug() << "MySQL startup params:" << m_mysqlProcess.arguments().join(QChar(QChar::Space));
        return false;
    }

    return true;
}

bool SkAppTestObject::createDatabase()
{
    {
        QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QMYSQL"), QStringLiteral("skaffaridb"));
        db.setDatabaseName(QStringLiteral("mysql"));
        db.setUserName(QStringLiteral("root"));
        db.setConnectOptions(QStringLiteral("UNIX_SOCKET=%1").arg(m_mysqlSocketDir.filePath(QStringLiteral("mysql.sock"))));
        if (Q_UNLIKELY(!db.open())) {
            qCritical() << "Failed to open database connection:" << db.lastError().text();
            return false;
        }

        QSqlQuery q(db);

        if (!q.exec(QStringLiteral("CREATE DATABASE %1").arg(m_dbName))) {
            qCritical() << "Failed to create database:" << q.lastError().text();
            return false;
        }

        if (!q.exec(QStringLiteral("CREATE USER '%1'@'localhost' IDENTIFIED BY '%2'").arg(m_dbUser, m_dbPass))) {
            qCritical() << "Failed to create database user:" << q.lastError().text();
            return false;
        }

        if (!q.exec(QStringLiteral("GRANT ALL ON %1.* TO '%2'@'localhost'").arg(m_dbName, m_dbUser))) {
            qCritical() << "Failed to grant access to db user:" << q.lastError().text();
            return false;
        }

        db.close();
    }

    QSqlDatabase::removeDatabase(QStringLiteral("skaffaridb"));

    return true;
}

bool SkAppTestObject::startCyrus(const QHash<QString,QString> &config)
{
    // like /var/lib/imap
    if (!m_cyrusConfDir.isValid()) {
        qCritical() << "Can not create cyrus config dir.";
        return false;
    }
    chown(m_cyrusConfDir.path(), QStringLiteral(CYRUS_UG), 750);

    // like /var/spool/imap
    if (!m_cyrusDataDir.isValid()) {
        qCritical() << "Can not create cyrus data dir.";
        return false;
    }
    chown(m_cyrusDataDir.path(), QStringLiteral(CYRUS_UG), 750);

    // like /var/lib/sieve
    if (!m_cyrusSieveDir.isValid()) {
        qCritical() << "Can not create cyrus sieve dir.";
        return false;
    }
    chown(m_cyrusSieveDir.path(), QStringLiteral(CYRUS_UG), 750);

    if (!m_cyrusPidDir.isValid()) {
        qCritical() << "Can not create cyrus pid dir.";
        return false;
    }
    chown(m_cyrusPidDir.path(), QStringLiteral(CYRUS_UG));

    const QStringList chownArgsList{QStringLiteral(CYRUS_UG), m_cyrusConfDir.path(), m_cyrusDataDir.path(), m_cyrusSieveDir.path(), m_cyrusPidDir.path()};
    if (Q_UNLIKELY(QProcess::execute(QStringLiteral("chown"), chownArgsList) != 0)) {
        qCritical() << "Can not chown imap directories to imap user.";
        return false;
    }

    if (Q_UNLIKELY(!m_cyrusAnnotaionsFile.open())) {
        qCritical() << "Can not open cyrus annotations file.";
        return false;
    }
    chown(m_cyrusAnnotaionsFile.fileName(), QStringLiteral("root:root"), 644);

    {
        QTextStream annotationsOut(&m_cyrusAnnotaionsFile);
        annotationsOut << "/vendor/kolab/activesync,mailbox,string,backend,value.priv,r" << endl;
        annotationsOut << "/vendor/kolab/color,mailbox,string,backend,value.shared value.priv,a" << endl;
        annotationsOut << "/vendor/kolab/displayname,mailbox,string,backend,value.shared value.priv,a" << endl;
        annotationsOut << "/vendor/kolab/folder-test,mailbox,string,backend,value.shared value.priv,a" << endl;
        annotationsOut << "/vendor/kolab/folder-type,mailbox,string,backend,value.shared value.priv,a" << endl;
        annotationsOut << "/vendor/kolab/incidences-for,mailbox,string,backend,value.shared value.priv,a" << endl;
        annotationsOut << "/vendor/kolab/pxfb-readable-for,mailbox,string,backend,value.shared value.priv,a" << endl;
        annotationsOut << "/vendor/kolab/uniqueid,mailbox,string,backend,value.shared value.priv,a" << endl;
        annotationsOut << "/vendor/kolab/h-share-attr-desc,mailbox,string,backend,value.shared value.priv,a" << endl;
        annotationsOut << "/vendor/horde/share-params,mailbox,string,backend,value.shared value.priv,a" << endl;
        annotationsOut << "/vendor/x-toltec/test,mailbox,string,backend,value.shared value.priv,a" << endl;
        annotationsOut.flush();
    }

    if (Q_UNLIKELY(!m_cyrusConfFile.open())) {
        qCritical() << "Failed to open cyrus config file.";
        return false;
    }
    chown(m_cyrusConfFile.fileName(), QStringLiteral("root:root"), 644);

    if (Q_UNLIKELY(!m_cyrusImapFile.open())) {
        qCritical() << "Failed to open cyrus imap file.";
        return false;
    }
    chown(m_cyrusImapFile.fileName(), QStringLiteral("root:root"), 644);

    {
        QTextStream confFileOut(&m_cyrusConfFile);
        confFileOut << "START {" << endl;
        confFileOut << "    recover             cmd=\"ctl_cyrusdb -r\"" << endl;
//        confFileOut << "    idled               cmd=\"idled\"" << endl;
        confFileOut << "}" << endl;

        confFileOut << "SERVICES {" << endl;
        confFileOut << "    imap                cmd=\"imapd\" listen=\"" << m_imapPort << "\" prefork=0" << endl;
//        confFileOut << "    pop3                cmd=\"pop3d\" listen=\"" << m_pop3Port << "\" prefork=0" << endl;
        confFileOut << "    sieve               cmd=\"timsieved\" listen=\"" << m_sievePort << "\" prefork=0" << endl;
        confFileOut << "    lmtpunix            cmd=\"lmtpd\" listen=\"" << m_cyrusConfDir.filePath(QStringLiteral("socket/lmtp")) << "\" proto=\"udp\" prefork=1" << endl;
        confFileOut << "}" << endl;

        confFileOut << "EVENTS {" << endl;
        confFileOut << "    checkpoint          cmd=\"ctl_cyrusdb -c\" period=30" << endl;
        confFileOut << "    checkpoint          cmd=\"ctl_cyrusdb -c\" period=30" << endl;
        confFileOut << "    deleteprune         cmd=\"cyr_expire -E 4 -D 69\" at=0430" << endl;
        confFileOut << "    expungeprune        cmd=\"cyr_expire -E 4 -X 69\" at=0445" << endl;
        confFileOut << "    tlsprune            cmd=\"tls_prune\" at=0400" << endl;
        confFileOut << "    squatter            cmd=\"squatter -s -i\" period=480" << endl;
        confFileOut << "}" << endl;
        confFileOut.flush();
    }

    {
        QHash<QString, QString> ic;
        ic.insert(QStringLiteral("admins"), m_imapUser);
        ic.insert(QStringLiteral("unixhierarchysep"), QStringLiteral("0"));
        ic.insert(QStringLiteral("virtdomains"), QStringLiteral("0"));

        auto confIt = config.constBegin();
        while (confIt != config.constEnd()) {
            ic.insert(confIt.key(), confIt.value());
            ++confIt;
        }

        ic.insert(QStringLiteral("configdirectory"), m_cyrusConfDir.path());
        ic.insert(QStringLiteral("partition-default"), m_cyrusDataDir.path());
        ic.insert(QStringLiteral("sievedir"), m_cyrusSieveDir.path());
        ic.insert(QStringLiteral("annotation_definitions"), m_cyrusAnnotaionsFile.fileName());
        ic.insert(QStringLiteral("allowanonymouslogin"), QStringLiteral("no"));
        ic.insert(QStringLiteral("allowplaintext"), QStringLiteral("1"));
        ic.insert(QStringLiteral("tls_required"), QStringLiteral("0"));
        ic.insert(QStringLiteral("expunge_mode"), QStringLiteral("delayed"));
        ic.insert(QStringLiteral("deletedprefix"), QStringLiteral("DELETED"));
        ic.insert(QStringLiteral("delete_mode"), QStringLiteral("delayed"));
        ic.insert(QStringLiteral("allowapop"), QStringLiteral("0"));

        QTextStream imapOut(&m_cyrusImapFile);
        auto icIt = ic.constBegin();
        while (icIt != ic.constEnd()) {
            imapOut << icIt.key() << ": " << icIt.value() << endl;
            ++icIt;
        }
        imapOut.flush();
    }

    const QStringList sudoMkImapArgs{QStringLiteral("-u"), QStringLiteral("cyrus"), QStringLiteral("/usr/lib/cyrus/tools/mkimap"), m_cyrusImapFile.fileName()};
    if (Q_UNLIKELY(QProcess::execute(QStringLiteral("sudo"), sudoMkImapArgs) != 0)) {
        qCritical() << "Failed to make imap directory structure.";
        return false;
    }

    const QStringList cyrusArgs{QStringLiteral("-C"), m_cyrusImapFile.fileName(), QStringLiteral("-M"), m_cyrusConfFile.fileName(), QStringLiteral("-p"), m_cyrusPidDir.filePath(QStringLiteral("master.pid")), QStringLiteral("-D")};

    QFileInfo lmtpSocketFi(m_cyrusConfDir.filePath(QStringLiteral("socket/lmtp")));

    m_cyrusProcesss.setProgram(QStringLiteral("/usr/lib/cyrus/bin/master"));
    m_cyrusProcesss.setArguments(cyrusArgs);
    m_cyrusProcesss.start();

    auto cur = QDateTime::currentDateTime();
    while (!lmtpSocketFi.exists() && (cur.msecsTo(QDateTime::currentDateTime()) < 5000)) {

    }

    if (!lmtpSocketFi.exists()) {
        qCritical() << "Failed to start cyrus imap server within 10 seconds.";
        qDebug() << "Cyrus start parameters:" << cyrusArgs.join(QChar(QChar::Space));
        qDebug() << "Cyrus exit status:" << m_cyrusProcesss.exitCode();
        return false;
    }

    return true;
}

void SkAppTestObject::chown(const QString &dir, const QString &ug, int mode)
{
    const QStringList chownArgs{ug, dir};
    QProcess::execute(QStringLiteral("chown"), chownArgs);
    const QStringList chmodArgs{QString::number(mode), dir};
    QProcess::execute(QStringLiteral("chmod"), chmodArgs);
}

QString SkAppTestObject::mysqlSocket() const
{
    return m_mysqlSocketDir.filePath(QStringLiteral("mysql.sock"));
}
