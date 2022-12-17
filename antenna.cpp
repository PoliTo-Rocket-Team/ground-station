#include "antenna.h"
#include <QTimer>

Antenna::Antenna(QObject *parent)
    : QObject{parent}
{
    _connected = false;
}

void Antenna::setConnected() {
    _connected = true;
    emit connectedChanged();
}

void Antenna::connect() {
    if(_connected) return;
    QTimer::singleShot(2000, this, &Antenna::setConnected);
}

bool Antenna::isConnected() const {
    return _connected;
}
