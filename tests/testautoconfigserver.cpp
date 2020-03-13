#include "../src/objects/autoconfigserver.h"

#include <QTest>
#include <QDataStream>
#include <QMetaObject>
#include <QMetaProperty>

class AutoconfigServerTest : public QObject
{
    Q_OBJECT
public:
    AutoconfigServerTest(QObject *parent = nullptr) : QObject(parent) {
        qRegisterMetaType<dbid_t>("dbid_t");
    }

private Q_SLOTS:
    void initTestCase() {}

    void constructor();
    void testMove();
    void datastream();
    void equalOperator();
    void unequalOperator();
    void vectorDatastream();

    void cleanupTestCase() {}
};

void AutoconfigServerTest::constructor()
{
    AutoconfigServer s(1, 2, AutoconfigServer::Smtp, QStringLiteral("smtp.example.com"), 465, AutoconfigServer::Ssl, AutoconfigServer::Cleartext, 10);

    QCOMPARE(s.id(), static_cast<dbid_t>(1));
    QCOMPARE(AutoconfigServer::staticMetaObject.property(AutoconfigServer::staticMetaObject.indexOfProperty("id")).readOnGadget(&s).value<dbid_t>(), static_cast<dbid_t>(1));

    QCOMPARE(s.domainId(), static_cast<dbid_t>(2));
    QCOMPARE(AutoconfigServer::staticMetaObject.property(AutoconfigServer::staticMetaObject.indexOfProperty("domainId")).readOnGadget(&s).value<dbid_t>(), static_cast<dbid_t>(2));

    QCOMPARE(s.type(), AutoconfigServer::Smtp);
    QCOMPARE(AutoconfigServer::staticMetaObject.property(AutoconfigServer::staticMetaObject.indexOfProperty("type")).readOnGadget(&s).value<AutoconfigServer::Type>(), AutoconfigServer::Smtp);

    QCOMPARE(s.hostname(), QStringLiteral("smtp.example.com"));
    QCOMPARE(AutoconfigServer::staticMetaObject.property(AutoconfigServer::staticMetaObject.indexOfProperty("hostname")).readOnGadget(&s).toString(), QStringLiteral("smtp.example.com"));

    QCOMPARE(s.port(), static_cast<quint16>(465));
    QCOMPARE(AutoconfigServer::staticMetaObject.property(AutoconfigServer::staticMetaObject.indexOfProperty("port")).readOnGadget(&s).value<quint16>(), static_cast<quint16>(465));

    QCOMPARE(s.socketType(), AutoconfigServer::Ssl);
    QCOMPARE(AutoconfigServer::staticMetaObject.property(AutoconfigServer::staticMetaObject.indexOfProperty("socketType")).readOnGadget(&s).value<AutoconfigServer::SocketType>(), AutoconfigServer::Ssl);

    QCOMPARE(s.authentication(), AutoconfigServer::Cleartext);
    QCOMPARE(AutoconfigServer::staticMetaObject.property(AutoconfigServer::staticMetaObject.indexOfProperty("authentication")).readOnGadget(&s).value<AutoconfigServer::Authentication>(), AutoconfigServer::Cleartext);

    QCOMPARE(s.sorting(), static_cast<qint8>(10));
    QCOMPARE(AutoconfigServer::staticMetaObject.property(AutoconfigServer::staticMetaObject.indexOfProperty("sorting")).readOnGadget(&s).value<qint8>(), static_cast<qint8>(10));
}

void AutoconfigServerTest::testMove()
{
    // Test move constructor
    {
        AutoconfigServer s1(1, 2, AutoconfigServer::Smtp, QStringLiteral("smtp.example.com"), 465, AutoconfigServer::Ssl, AutoconfigServer::Cleartext, 10);
        QCOMPARE(s1.hostname(), QStringLiteral("smtp.example.com"));
        AutoconfigServer s2(std::move(s1));
        QCOMPARE(s2.hostname(), QStringLiteral("smtp.example.com"));
    }

    // Test move assignment
    {
        AutoconfigServer s1(1, 2, AutoconfigServer::Smtp, QStringLiteral("smtp.example.com"), 465, AutoconfigServer::Ssl, AutoconfigServer::Cleartext, 10);
        QCOMPARE(s1.hostname(), QStringLiteral("smtp.example.com"));
        AutoconfigServer s2(2, 2, AutoconfigServer::Smtp, QStringLiteral("smtp2.example.com"), 465, AutoconfigServer::Ssl, AutoconfigServer::Cleartext, 11);
        s2 = std::move(s1);
        QCOMPARE(s2.hostname(), QStringLiteral("smtp.example.com"));
    }
}

void AutoconfigServerTest::datastream()
{
    AutoconfigServer s1(1, 2, AutoconfigServer::Smtp, QStringLiteral("smtp.example.com"), 465, AutoconfigServer::Ssl, AutoconfigServer::Cleartext, 10);

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
    AutoconfigServer s1(1, 2, AutoconfigServer::Smtp, QStringLiteral("smtp.example.com"), 465, AutoconfigServer::Ssl, AutoconfigServer::Cleartext, 10);
    AutoconfigServer s2(1, 2, AutoconfigServer::Smtp, QStringLiteral("smtp.example.com"), 465, AutoconfigServer::Ssl, AutoconfigServer::Cleartext, 10);

    QVERIFY(s1 == s2);
}

void AutoconfigServerTest::unequalOperator()
{
    AutoconfigServer s1(1, 2, AutoconfigServer::Smtp, QStringLiteral("smtp.example.com"), 465, AutoconfigServer::Ssl, AutoconfigServer::Cleartext, 10);
    AutoconfigServer s2(2, 4, AutoconfigServer::Imap, QStringLiteral("imap.example.com"), 993, AutoconfigServer::Ssl, AutoconfigServer::Cleartext, 20);

    QVERIFY(s1 != s2);
}

void AutoconfigServerTest::vectorDatastream()
{
    std::vector<AutoconfigServer> list1({
                                            AutoconfigServer(1, 2, AutoconfigServer::Smtp, QStringLiteral("smtp.example.com"), 465, AutoconfigServer::Ssl, AutoconfigServer::Cleartext, 10),
                                            AutoconfigServer(2, 4, AutoconfigServer::Imap, QStringLiteral("imap.example.com"), 993, AutoconfigServer::Ssl, AutoconfigServer::Cleartext, 20)
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
