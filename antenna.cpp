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

void Antenna::setFrequency(quint8 f) {
    emit frequencyChanged(m_frequency = f);
    emit stateChanged(m_state = State::POLLING);
    sendToArduino('F');
    sendToArduino(f);
    QTimer::singleShot(5000, this, [this](){
        if(m_state == State::POLLING)
            emit stateChanged(m_state = State::DISCONNECTED);
    });
}

void Antenna::openSerialPort()
{
    const auto serialPortInfos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &portInfo : serialPortInfos) {
        if(portInfo.manufacturer().toLower().contains("arduino") || portInfo.description().toLower().contains("arduino")){
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

void Antenna::readData()
{
    QByteArray data = arduino->readAll();
    switch (data.at(0)){
    case 'R':
        // sending ready signal to arduino
        sendToArduino('B');
        emit connectedChanged(m_isArduinoConnected = true);
        break;
    case 'C':
        emit stateChanged(m_state = State::CONNECTED);
        m_startTime = QTime::currentTime();
        __timer->start(1000);
        break;
    }
    qDebug() << "UART:" << data;

}

int Antenna::sendToArduino(quint8 data){
    QByteArray payload;
    payload.append(data);
    int bytesWritten = arduino->write(payload);
    arduino->waitForBytesWritten();
    return bytesWritten;
}

void Antenna::reset(){
    return;
}
