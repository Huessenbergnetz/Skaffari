#ifndef SKAFFARI_IMAP_H
#define SKAFFARI_IMAP_H

#include <QSslSocket>

namespace Cutelyst {
class Context;
}

namespace Skaffari {

class Imap : public QSslSocket
{
    Q_OBJECT
public:
    explicit Imap(Cutelyst::Context *, QObject *parent = nullptr);

    ~Imap() override = default;

private:
    Cutelyst::Context *m_c{nullptr};
};

}

#endif // SKAFFARI_IMAP_H
