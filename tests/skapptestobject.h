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
    ~SkAppTestObject();

protected:
    bool startContainer(const QMap<QString,QString> &config) const;
    bool stopContainer() const;
//    bool startMysql();
//    bool createDatabase();

    const int m_mysqlPort{45678};
    const int m_imapPort{46000};
    const int m_sievePort{46400};

    const QString m_dbName{"skaffaridb"};
    const QString m_dbUser{"skaffari"};
    const QString m_dbPass{"w9cf7j1QxzgK"};

    const QString m_imapUser{"cyrus"};

private:
//    QTemporaryDir m_mysqlWorkingDir;
//    QTemporaryDir m_mysqlDataDir;
//    QTemporaryDir m_mysqlSocketDir;
//    QTemporaryDir m_mysqlLogDir;
//    QTemporaryFile m_mysqlConfigFile;
//    QProcess m_mysqlProcess;

    QString m_containerName;
};

#endif // SKAPPTESTOBJECT_H
