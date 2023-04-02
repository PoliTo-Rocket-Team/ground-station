#include "antenna.h"
#include <QTimer>

Antenna::Antenna(QObject *parent)
    : QObject{parent}
{
    QTimer::singleShot(2000, this, &Antenna::__start);
}

void Antenna::__start() {
    m_connected = true;
    emit connectedChanged(true);

    //    float choice = (float) rand() / (float) RAND_MAX;
    //    if(choice < 0.4) {
    //        m_connected = true;
    //        emit connectedChanged(true);
    //    }
    //    else {
    //        m_error = (choice < 0.6 ? 1 : choice < 0.8 ? 2 : 3);
    //        emit errorChange();
    //    }

}

void Antenna::setFrequency(quint8 f) {

}


void Antenna::reset() {

}
