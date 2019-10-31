#include "../src/objects/autoconfigserver.h"

#include <QTest>
#include <QDataStream>

class AutoconfigServerTest : public QObject
{
    Q_OBJECT
public:
    AutoconfigServerTest(QObject *parent = nullptr) : QObject(parent) {}

private Q_SLOTS:
    void initTestCase() {}

    void constructor();
    void datastream();
    void equalOperator();
    void unequalOperator();
    void vectorDatastream();

    void cleanupTestCase() {}
};

void AutoconfigServerTest::constructor()
{
    AutoconfigServer s(1, 2, AutoconfigServer::Smtp, QStringLiteral("smtp.exmaple.com"), 465, AutoconfigServer::Ssl, AutoconfigServer::Cleartext, 10);

    QCOMPARE(s.id(), static_cast<dbid_t>(1));
    QCOMPARE(s.domainId(), static_cast<dbid_t>(2));
    QCOMPARE(s.type(), AutoconfigServer::Smtp);
    QCOMPARE(s.hostname(), QStringLiteral("smtp.exmaple.com"));
    QCOMPARE(s.port(), static_cast<quint16>(465));
    QCOMPARE(s.socketType(), AutoconfigServer::Ssl);
    QCOMPARE(s.authentication(), AutoconfigServer::Cleartext);
    QCOMPARE(s.sorting(), static_cast<qint8>(10));
}

void AutoconfigServerTest::datastream()
{
    AutoconfigServer s1(1, 2, AutoconfigServer::Smtp, QStringLiteral("smtp.exmaple.com"), 465, AutoconfigServer::Ssl, AutoconfigServer::Cleartext, 10);

    QByteArray outBa;
    QDataStream out(&outBa, QIODevice::WriteOnly);
    out << s1;

    const QByteArray inBa = outBa;
    QDataStream in(inBa);
    AutoconfigServer s2;
    in >> s2;

    QCOMPARE(s1.id(), s2.id());
    QCOMPARE(s1.domainId(), s2.domainId());
    QCOMPARE(s1.type(), s2.type());
    QCOMPARE(s1.hostname(), s2.hostname());
    QCOMPARE(s1.port(), s2.port());
    QCOMPARE(s1.socketType(), s2.socketType());
    QCOMPARE(s1.authentication(), s2.authentication());
    QCOMPARE(s1.sorting(), s2.sorting());
}

void AutoconfigServerTest::equalOperator()
{
    AutoconfigServer s1(1, 2, AutoconfigServer::Smtp, QStringLiteral("smtp.exmaple.com"), 465, AutoconfigServer::Ssl, AutoconfigServer::Cleartext, 10);
    AutoconfigServer s2(1, 2, AutoconfigServer::Smtp, QStringLiteral("smtp.exmaple.com"), 465, AutoconfigServer::Ssl, AutoconfigServer::Cleartext, 10);

    QVERIFY(s1 == s2);
}

void AutoconfigServerTest::unequalOperator()
{
    AutoconfigServer s1(1, 2, AutoconfigServer::Smtp, QStringLiteral("smtp.exmaple.com"), 465, AutoconfigServer::Ssl, AutoconfigServer::Cleartext, 10);
    AutoconfigServer s2(2, 4, AutoconfigServer::Imap, QStringLiteral("imap.exmaple.com"), 993, AutoconfigServer::Ssl, AutoconfigServer::Cleartext, 20);

    QVERIFY(s1 != s2);
}

void AutoconfigServerTest::vectorDatastream()
{
    std::vector<AutoconfigServer> list1({
                                            AutoconfigServer(1, 2, AutoconfigServer::Smtp, QStringLiteral("smtp.exmaple.com"), 465, AutoconfigServer::Ssl, AutoconfigServer::Cleartext, 10),
                                            AutoconfigServer(2, 4, AutoconfigServer::Imap, QStringLiteral("imap.exmaple.com"), 993, AutoconfigServer::Ssl, AutoconfigServer::Cleartext, 20)
                                        });

    QByteArray outBa;
    QDataStream out(&outBa, QIODevice::WriteOnly);
    out << list1;

    const QByteArray inBa = outBa;
    QDataStream in(inBa);
    std::vector<AutoconfigServer> list2;
    in >> list2;

    for (std::vector<AutoconfigServer>::size_type i = 0; i < list1.size(); ++i) {
        QVERIFY(list1.at(i) == list2.at(i));
    }
}

QTEST_MAIN(AutoconfigServerTest)

#include "testautoconfigserver.moc"
