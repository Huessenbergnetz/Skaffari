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
    bool startContainer(const QMap<QString,QString> &config, const QString &name, int mysqlPort, int imapPort, int sievePort) const;
    bool stopContainer(const QString &name) const;
//    bool startMysql();
//    bool createDatabase();

    QString createContainerName() const;

//    const int m_mysqlPort{46000};
//    const int m_imapPort{47000};
//    const int m_sievePort{48000};

private:
//    QTemporaryDir m_mysqlWorkingDir;
//    QTemporaryDir m_mysqlDataDir;
//    QTemporaryDir m_mysqlSocketDir;
//    QTemporaryDir m_mysqlLogDir;
//    QTemporaryFile m_mysqlConfigFile;
//    QProcess m_mysqlProcess;

};

class SkCmdProc : public QProcess
{
    Q_OBJECT
public:
    explicit SkCmdProc(QObject *parent = nullptr);
    ~SkCmdProc();

    bool enterString(const QString &str);
    qint64 enterNumber(int number);
    qint64 enterBool(bool b);

    void setShowOutput(bool show);

    bool waitForOutput();

private:
    const int m_waitForOutputTimeOut{5000};
    bool m_showOutput = false;
};

#endif // SKAPPTESTOBJECT_H
