#include "antenna.h"

#include <QRandomGenerator>

QVector3D vectorFromBytes(std::byte* raw) {
    float x, y, z;
    std::memcpy(&x, raw, 4);
    std::memcpy(&y, raw+4, 4);
    std::memcpy(&z, raw+8, 4);
    return QVector3D(x,y,z);
}

RocketData::RocketData(std::byte* raw)
    : acc_lin(vectorFromBytes(raw+12)),
      acc_ang(vectorFromBytes(raw+24))
{
    std::memcpy(&barometer, raw, 4);
    std::memcpy(&barometer, raw+4, 4);
    std::memcpy(&barometer, raw+8, 4);
}

Antenna::Antenna(QObject *parent)
    : QObject{parent}
{
    QTimer::singleShot(3000, this, &Antenna::__start);
    __timer = new QTimer(this);
    connect(__timer, &QTimer::timeout, this, &Antenna::__randomData);
}

void Antenna::__start() {
    emit connectedChanged(m_isArduinoConnected = true);

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

float getRndAcc() {
    return (QRandomGenerator::global()->generateDouble() * 6.0) - 3.0;
}

void Antenna::__randomData() {
    RocketData data{};
    data.barometer = (__lastBaro += QRandomGenerator::global()->generateDouble());
    data.lat = 45;
    data.lng = 12;
    data.acc_lin = QVector3D(getRndAcc(),getRndAcc(),getRndAcc());
    data.acc_ang = QVector3D(getRndAcc(),getRndAcc(),getRndAcc());
    emit newData(m_startTime.secsTo(QTime::currentTime()), data);
}

void Antenna::__setF() {
    const float choice = (float) rand() / (float) RAND_MAX;
    if(choice < 0.5) {
        emit stateChanged(m_state = State::DISCONNECTED);
    }
    else {
        emit stateChanged(m_state = State::CONNECTED);
        m_startTime = QTime::currentTime();
        __timer->start(1000);
    }
}

void Antenna::setFrequency(quint8 f) {
    emit frequencyChanged(m_frequency = f);
    emit stateChanged(m_state = State::POLLING);
    QTimer::singleShot(1000, this, &Antenna::__setF);
}


void Antenna::reset() {

}
