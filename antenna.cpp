#include "antenna.h"
#include <QTimer>
#include <QIODevice>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <unistd.h>
#include <QRandomGenerator>
#include <QDateTime>

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
    std::memcpy(&pressure1, raw, 4);
    std::memcpy(&pressure2, raw+4, 4);
    std::memcpy(&temperature1, raw+8, 4);
    std::memcpy(&temperature2, raw+12, 4);
}



Antenna::Antenna(QObject *parent)
    : QObject{parent},
    payload(sizeof(RocketData), 0x00)
{
    scanTimer = new QTimer(this);
    connect(scanTimer,&QTimer::timeout,this,&Antenna::openSerialPort);
    arduino = new QSerialPort(this);
    QSerialPort::connect(arduino, SIGNAL(readyRead()), this, SLOT(readData()));
    scanTimer->start(1000);
}

void Antenna::setFrequency(quint8 f) {
    emit frequencyChanged(m_frequency = f);
    emit stateChanged(m_state = State::POLLING);
    char msg[2] = { 'F', (char)f };
    arduino->write(msg,2);
    QTimer::singleShot(20000, this, [this](){
        if(m_state == State::POLLING)
            emit stateChanged(m_state = State::DISCONNECTED);
    });
}

void Antenna::openSerialPort()
{
    qDebug() << "Scanning for Arduino";
    const auto serialPortInfos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &portInfo : serialPortInfos) {
        qDebug() << " -" << portInfo.manufacturer() << ": " << portInfo.description();
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
            break;
        }
    }
}

bool notAllJunk(QByteArray a) {
    for (int i = 0; i < a.size(); i++) if(a.at(i) != (char)'\x00') return true;
    return false;
}

qsizetype getPayloadLengthOf(char c) {
    switch(c) {
    case 'D': return sizeof(RocketData);
    case 'E': return 1;
    case 'C':
    case 'R':
        return 0;
    default:
        return -1;
    }
}

void Antenna::readData()
{
    QByteArray data = arduino->readAll();
    // qDebug() << "UART: received" << data;
    // if(notAllJunk(data)) qDebug() << "UART:" << data;

    if(remaining_bytes) {
        if(data.size() < remaining_bytes) {
            payload.append(data);
            remaining_bytes -= data.size();
        }
        else {
            payload.append(data.first(remaining_bytes));
            handlePayload();
            goForward(data, remaining_bytes);
        }
    }
    else if(current_code == 0x05) startPayloadAt(data,0);
    else goForward(data, 0);
}

void Antenna::goForward(QByteArray& a, qsizetype from) {
    if(from >= a.size()) {
        current_code = 0x00;
        remaining_bytes = 0;
        return;
    }
    auto start = a.indexOf('~',from)+1;
    if(start == 0) {
        current_code = 0x00;
        remaining_bytes = 0;
    }
    else if(start == a.size()) {
        current_code = 0x05;
        remaining_bytes = 0;
    }
    else startPayloadAt(a, start);
}

void Antenna::startPayloadAt(QByteArray& a, qsizetype from) {
    current_code = a.at(from);
    remaining_bytes = getPayloadLengthOf(current_code);
    if(remaining_bytes == -1) goForward(a, from);
    else if(remaining_bytes == 0) {
        handlePayload();
        goForward(a, from+1);
    }
    else if(from + remaining_bytes <= a.size() - 1) {
        payload = a.sliced(from+1, remaining_bytes);
        handlePayload();
        goForward(a,from+remaining_bytes+1);
    }
    else {
        const auto num = a.size() - 1 - from;
        payload = a.last(num);
        remaining_bytes -= num;
    }
}

void Antenna::handlePayload() {
    static auto last = QDateTime::currentMSecsSinceEpoch();
    static qint64 now;
    now = QDateTime::currentMSecsSinceEpoch();
    qDebug() << now - last << current_code << " -> " << payload;
    last = now;
    switch (current_code){
    case 'R': {
        // sending ready signal to arduino
        qDebug() << "Send [B]";
        arduino->write("BBBBB", 5);
        if(!m_isArduinoConnected) {
            emit connectedChanged(m_isArduinoConnected = true);
            emit stateChanged(m_state = State::CONNECTED);
            emit frequencyChanged(m_frequency = 23);
        }
        break;
    }
    case 'C':
        if(m_state != State::CONNECTED){
            emit stateChanged(m_state = State::CONNECTED);
            m_startTime = QTime::currentTime();
        }
        break;
    case 'E':
        emit errorChange(m_error = payload.at(0) - '0');
        break;
    case 'D': {
        if(m_state != State::CONNECTED){
            emit stateChanged(m_state = State::CONNECTED);
        }
        RocketData data{};
        data.pressure1 = packFloat(0);
        data.pressure2 = packFloat(4);
        data.temperature1 = packFloat(8);
        data.temperature2 = packFloat(12);
        data.acc_lin = QVector3D(packFloat(16), packFloat(20), packFloat(24));
        data.acc_ang = QVector3D(packFloat(28), packFloat(32), packFloat(36));
        emit newData(m_startTime.secsTo(QTime::currentTime()), data);
        break;
    }
    }
}

float Antenna::packFloat(qsizetype from) {
    char f[4];
    for (int i = 0; i < 4; i++) f[i] = payload.at(from + i);
    return *((float*) &f);
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
