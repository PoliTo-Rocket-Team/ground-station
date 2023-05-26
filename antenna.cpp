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
    : QObject{parent},
    buffer()
{
    __timer = new QTimer(this);
    scanTimer = new QTimer(this);
    connect(scanTimer,&QTimer::timeout,this,&Antenna::openSerialPort);
    arduino = new QSerialPort(this);
    QSerialPort::connect(arduino, SIGNAL(readyRead()), this, SLOT(readData()));
    scanTimer->start(1000);
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
            scanTimer->stop();
            break;
        }
    }
}

void Antenna::readData()
{
    QByteArray data = arduino->readAll();
    for (int i = 0; i < data.size(); i++){
        // 0xAA == start/end sequence identifier
        if (data.at(i) == (char)0xAA){
            if(buffer.size() == PACKET_SIZE){
                handleBuffer();
                buffer.clear();
            } else
                // discard incomplete packet
                buffer.clear();
        } else
            buffer.append(data.at(i));
    }
    qDebug() << "UART:" << data;
}

void Antenna::handleBuffer(){
    switch (buffer.at(0)){
    case 'R':
        if(!m_isArduinoConnected){
            // sending ready signal to arduino
            sendToArduino('B');
            emit connectedChanged(m_isArduinoConnected = true);}
        break;
    case 'C':
        if(m_state != State::CONNECTED){
            emit stateChanged(m_state = State::CONNECTED);
            m_startTime = QTime::currentTime();
            __timer->start(1000);
        }
        break;
    case 'E':
        emit errorChange(m_error = buffer.at(1) - '0');
        break;
    case 'D':
        float bar = packFloat(1);
        float temperature = packFloat(5);
        float l_accx = packFloat(9);
        float l_accy = packFloat(13);
        float l_accz = packFloat(17);
        float a_accx = packFloat(21);
        float a_accy = packFloat(25);
        float a_accz = packFloat(29);

        RocketData data{};
        data.barometer = (bar);
        data.temperature = temperature;
        data.acc_lin = QVector3D(l_accx,l_accy,l_accz);
        data.acc_ang = QVector3D(a_accx,a_accy,a_accz);
        emit newData(m_startTime.secsTo(QTime::currentTime()), data);
        break;
    }
}

float Antenna::packFloat(int index){
    char bytes[4];
    float f;
    for (int i = 0; i < 4; i++)
        bytes[i] = buffer.at(index + i);
    memcpy(&f, &bytes, sizeof(f));
    return f;

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
