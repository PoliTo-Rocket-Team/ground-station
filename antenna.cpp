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
    buffer(),
    packet()
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
        if(
            portInfo.manufacturer().toLower().contains("arduino") ||
            portInfo.description().toLower().contains("arduino")
        ){
            arduino->setPortName(portInfo.systemLocation());
            arduino->setBaudRate(QSerialPort::Baud9600);
            arduino->setDataBits(QSerialPort::Data8);
            arduino->setParity(QSerialPort::NoParity);
            arduino->setStopBits(QSerialPort::OneStop);
            arduino->setFlowControl(QSerialPort::NoFlowControl);

            if(!arduino->open(QIODevice::ReadWrite)){
                qDebug() << "Error in opening port " << portInfo.portName() << ", code " << arduino->error();
                return;
            }
            scanTimer->stop();
            emit connectedChanged(m_isArduinoConnected = true);
            break;
        }
    }
}

void Antenna::readData()
{
    QByteArray data = arduino->readAll();
    buffer.append(data);
    readBuffer();

    qDebug() << "UART:" << data;
}

void Antenna::readBuffer() {
    bool readingPacket = false;
    for (int i = 0; i < buffer.size(); i++){
        // 0xAA == start sequence identifier
        if (buffer.at(i) == (char)'\xAA'){
            readingPacket = true;
            continue;
        }
        // 0xBB == end sequence identifier
        else if(buffer.at(i) == (char)'\xBB'){
            if(packet.size() == PACKET_SIZE){
                handlePacket();
                packet.clear();
                // shifts the buffer left
                buffer = buffer.last(buffer.size() - i);
            } else
                // discard incomplete packet
                packet.clear();
            readingPacket = false;
            continue;
        } else if(readingPacket)
            packet.append(buffer.at(i));
    }
}

void Antenna::handlePacket(){
    switch (packet.at(0)){
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
        emit errorChange(m_error = packet.at(1) - '0');
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
        qDebug() << "bar:" << bar;
        qDebug() << "temp:" << temperature;
        qDebug() << "acc linx:" <<  l_accx;
        qDebug() << "acc liny:" << l_accy;
        qDebug() << "acc linz:" << l_accz;
        qDebug() << "acc angx:" << a_accx;
        qDebug() << "acc angy:" << a_accy;
        qDebug() << "acc angz:" << a_accz;

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
        bytes[i] = packet.at(index + i);
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
