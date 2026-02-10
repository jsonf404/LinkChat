#include "global.h"

QString gate_url_prefix = "";

QString sha256(const QString &input)
{
    QByteArray data = input.toUtf8();
    QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
    return hash.toHex();
}
