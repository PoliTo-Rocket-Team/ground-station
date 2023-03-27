#include "antenna.h"
#include <QTimer>
#include <QIODevice>
#include <QSerialPort>
#include<QDebug>
#include <unistd.h>
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
        arduino = new QSerialPort(this);
    arduino = new QSerialPort(this);
    openSerialPort();
    QSerialPort::connect(arduino, SIGNAL(readyRead()), this, SLOT(readData()));
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

void Antenna::openSerialPort()
{

    // TODO: use QSerialPortInfo to get port name
    arduino->setPortName("/dev/ttyACM0");
    arduino->setBaudRate(QSerialPort::Baud9600);
    arduino->setDataBits(QSerialPort::Data8);
    arduino->setParity(QSerialPort::NoParity);
    arduino->setStopBits(QSerialPort::OneStop);
    arduino->setFlowControl(QSerialPort::NoFlowControl);

    if(!arduino->open(QIODevice::ReadWrite)){
        qDebug() << (tr("error %1").arg(arduino->error()));
        return;
    }
}

void Antenna::setConnected() {
    QByteArray data;
    // 0x42 == B
    data.append(0x42);
    // sending ready signal to arduino
    arduino->write(data);
    arduino->waitForBytesWritten();
}

void Antenna::readData()
{
    QByteArray data = arduino->readAll();

    if(data.at(0) == 0x52)
        setConnected();

    qDebug() << "UART:" << data;

}

void Antenna::reset(){
    return;
}
