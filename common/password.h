#ifndef PASSWORD_H
#define PASSWORD_H

#include <QString>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(SK_PASSWORD)

class Password
{
public:
    enum Type : quint8 {
        PlainText       = 0,
        Crypt           = 1,
        MySQL           = 2,
        MD5             = 3,
        SHA1            = 4
    };

    enum Method : quint8 {
        Default         = 0,
        CryptDES        = 1,
        CryptMD5        = 2,
        CryptSHA256     = 3,
        CryptSHA512     = 4,
        CryptBcrypt     = 5,
        MySQLNew        = 6,
        MySQLOld        = 7
    };

    explicit Password(const QString &pw);

    QByteArray encrypt(Type type, Method method = Default, quint32 rounds = 0);
    bool check(const QByteArray &savedPw);

private:
    QString m_password;

    static QByteArray requestSalt(quint16 length, const QByteArray allowedChars = QByteArray());
};

#endif // PASSWORD_H
