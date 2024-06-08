#include "imap.h"

#include <Cutelyst/Context>

using namespace Skaffari;

Imap::Imap(Cutelyst::Context *c, QObject *parent)
    : QSslSocket{parent}
    , m_c{c}
{

}

#include "moc_imap.cpp"
