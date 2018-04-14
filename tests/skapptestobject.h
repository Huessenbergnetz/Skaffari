#ifndef SKAPPTESTOBJECT_H
#define SKAPPTESTOBJECT_H

#include <QObject>
#include <QProcess>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QHash>

class SkAppTestObject : public QObject
{
    Q_OBJECT
public:
    explicit SkAppTestObject(QObject *parent = nullptr);

protected:
    bool checkRootUser() const;
    bool startMysql();
    bool createDatabase();
    bool startCyrus(const QHash<QString, QString> &config = QHash<QString, QString>());

    const int m_mysqlPort{45678};
    const int m_imapPort{46000};
    const int m_imapsPort{46100};
    const int m_pop3Port{46200};
    const int m_pop3sPort{46300};
    const int m_sievePort{46400};

    const QString m_dbName{"skaffaridb"};
    const QString m_dbUser{"skaffari"};
    const QString m_dbPass{"schalke04"};

    const QString m_imapUser{"cyrus"};

    QString mysqlSocket() const;

private:
    QTemporaryDir m_mysqlWorkingDir;
    QTemporaryDir m_mysqlDataDir;
    QTemporaryDir m_mysqlSocketDir;
    QTemporaryDir m_mysqlLogDir;
    QTemporaryFile m_mysqlConfigFile;
    QProcess m_mysqlProcess;

    QTemporaryDir m_cyrusConfDir;
    QTemporaryDir m_cyrusDataDir;
    QTemporaryDir m_cyrusSieveDir;
    QTemporaryDir m_cyrusPidDir;
    QTemporaryFile m_cyrusAnnotaionsFile;
    QTemporaryFile m_cyrusConfFile;
    QTemporaryFile m_cyrusImapFile;
    QProcess m_cyrusProcesss;

    void chown(const QString &dir, const QString &ug, int mode = 777);
};

#endif // SKAPPTESTOBJECT_H
