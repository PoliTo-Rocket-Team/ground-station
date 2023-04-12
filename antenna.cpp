#include "antenna.h"
#include <QTimer>
#include <QIODevice>
#include <QSerialPort>
#include <QSerialPortInfo>
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
    __start();
    __timer = new QTimer(this);
    connect(__timer, &QTimer::timeout, this, &Antenna::__randomData);
}

void Antenna::__start() {
    arduino = new QSerialPort(this);
    openSerialPort();
    QSerialPort::connect(arduino, SIGNAL(readyRead()), this, SLOT(readData()));
}

float getRndAcc() {
    return (QRandomGenerator::global()->generateDouble() * 6.0) - 3.0;
}

void Antenna::__randomData() {
    RocketData data{};
    data.barometer = (__lastBaro -= QRandomGenerator::global()->generateDouble());
    // data.lat = 45;
    // data.lng = 12;
    data.acc_lin = QVector3D(getRndAcc(),getRndAcc(),getRndAcc());
    data.acc_ang = QVector3D(getRndAcc(),getRndAcc(),getRndAcc());
    emit newData(m_startTime.secsTo(QTime::currentTime()), data);
}

void Antenna::__setF() {
    const float choice = (float) rand() / (float) RAND_MAX;
    if(choice < 0) {
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
    const auto serialPortInfos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &portInfo : serialPortInfos) {
        if(portInfo.manufacturer().toLower().contains("arduino")){
            arduino->setPortName(portInfo.systemLocation());
            arduino->setBaudRate(QSerialPort::Baud9600);
            arduino->setDataBits(QSerialPort::Data8);
            arduino->setParity(QSerialPort::NoParity);
            arduino->setStopBits(QSerialPort::OneStop);
            arduino->setFlowControl(QSerialPort::NoFlowControl);

            if(!arduino->open(QIODevice::ReadWrite)){
                qDebug() << (tr("error %1").arg(arduino->error()));
                return;
            }
            break;
        }
    }
}

void Antenna::setConnected() {
    QByteArray data;
    // 0x42 == B
    data.append(0x42);
    // sending ready signal to arduino
    arduino->write(data);
    arduino->waitForBytesWritten();
    emit connectedChanged(m_isArduinoConnected = true);
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
