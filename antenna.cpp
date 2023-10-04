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
    buffer()
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
    QTimer::singleShot(5000, this, [this](){
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

void Antenna::readData()
{
    QByteArray data = arduino->readAll();
    // if(notAllJunk(data)) qDebug() << "UART:" << data;

    if(buffer.size()) buffer.append(data);
    else {
        qsizetype p_start = data.indexOf((char)'\xAA');
        if(p_start == -1) return;
        buffer = data.last(data.size() - 1 - p_start);
    }
    readBuffer();
}

void Antenna::readBuffer() {
    qsizetype p_end = buffer.indexOf((char)'\xBB', searchEndFrom);
    if(p_end == -1) searchEndFrom = buffer.size();
    else {
        searchEndFrom = 0;
        QByteArray packet = buffer.first(p_end);
        handlePacket(packet);

        qsizetype p_start = buffer.indexOf((char)'\xAA', p_end);
        if(p_start == -1) buffer.clear();
        else {
            buffer.remove(0, p_start+1); // remove also \xBB
            readBuffer();
        }
    }
}

float packFloat(QByteArray & a, qsizetype from) {
    char f[4];
    for (int i = 0; i < 4; i++) f[i] = a.at(from + i);
    return *((float*) &f);
}

void Antenna::handlePacket(QByteArray packet){
    static auto last = QDateTime::currentMSecsSinceEpoch();
    static qint64 now;
    now = QDateTime::currentMSecsSinceEpoch();
    qDebug() << now - last << " Serial message: " << packet;
    last = now;
    switch (packet.at(0)){
    case 'R':
        // sending ready signal to arduino
        qDebug() << "Send [B]";
//        sendToArduino('B');
        arduino->write("B", 1);
        // arduino->waitForBytesWritten();
        if(!m_isArduinoConnected) {
            emit connectedChanged(m_isArduinoConnected = true);
            emit stateChanged(m_state = State::CONNECTED);
            emit frequencyChanged(m_frequency = 23);
        }
        break;
    case 'C':
        if(m_state != State::CONNECTED){
            emit stateChanged(m_state = State::CONNECTED);
            m_startTime = QTime::currentTime();
        }
        break;
    case 'E':
        emit errorChange(m_error = packet.at(1) - '0');
        break;
    case 'D':
        if(packet.size() != PACKET_SIZE) {
            qDebug() << "Data packet has wrong diemsions";
            break;
        }
        float bar1 = packFloat(packet, 1);
        float bar2 = packFloat(packet, 5);
        float temperature1 = packFloat(packet, 9);
        float temperature2 = packFloat(packet, 13);
        float l_accx = packFloat(packet, 17);
        float l_accy = packFloat(packet, 21);
        float l_accz = packFloat(packet, 25);
        float a_accx = packFloat(packet, 29);
        float a_accy = packFloat(packet, 33);
        float a_accz = packFloat(packet, 37);
        qDebug() << "bar1:" << bar1;
        qDebug() << "bar2:" << bar2;
        qDebug() << "temp1:" << temperature1;
        qDebug() << "temp2:" << temperature2;
        qDebug() << "acc linx:" <<  l_accx;
        qDebug() << "acc liny:" << l_accy;
        qDebug() << "acc linz:" << l_accz;
        qDebug() << "acc angx:" << a_accx;
        qDebug() << "acc angy:" << a_accy;
        qDebug() << "acc angz:" << a_accz;

        RocketData data{};
        data.pressure1 = (bar1);
        data.pressure2 = (bar2);
        data.temperature1 = temperature1;
        data.temperature2 = temperature2;
        data.acc_lin = QVector3D(l_accx,l_accy,l_accz);
        data.acc_ang = QVector3D(a_accx,a_accy,a_accz);
        emit newData(m_startTime.secsTo(QTime::currentTime()), data);
        break;
    }
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
