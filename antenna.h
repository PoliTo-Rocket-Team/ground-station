#ifndef ANTENNA_H
#define ANTENNA_H

#include <QObject>
#include <QTimer>
#include <QTime>
#include <QVector3D>
#include <QSerialPort>

struct RocketData {
    Q_GADGET
    Q_PROPERTY(float barometer MEMBER barometer);
    // Q_PROPERTY(float latitude MEMBER lat);
    // Q_PROPERTY(float longitude MEMBER lng);
    Q_PROPERTY(QVector3D acc_lin MEMBER acc_lin);
    Q_PROPERTY(QVector3D acc_ang MEMBER acc_ang);
public:
    float barometer;
    // float lat, lng;
    QVector3D acc_lin, acc_ang;
    RocketData(std::byte* raw);
    RocketData() = default;
};
Q_DECLARE_METATYPE(RocketData);

class Antenna : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool isArduinoConnected READ isConnected NOTIFY connectedChanged)
    Q_PROPERTY(quint8 error READ getError NOTIFY errorChange)
    Q_PROPERTY(quint8 frequency READ getFrequency WRITE setFrequency NOTIFY frequencyChanged)
    Q_PROPERTY(State state READ getState NOTIFY stateChanged)

public:
    explicit Antenna(QObject *parent = nullptr);

    enum State { POLLING, DISCONNECTED, CONNECTED };
    Q_ENUM(State)

    bool isConnected() const { return m_isArduinoConnected; };
    State getState() const { return m_state; };
    quint8 getError() const { return m_error; };
    quint8 getFrequency() const { return m_frequency; };


    /*
     * Used to set the frequency
     *
     * While m_frequency == 255 (app startup), it changes simply the listening frequency of the message [C] from the rocket.
     *
     * Otherwise, it sends a [F] message to the rocket and set connected = false. The outcome can be then [E4] or connected with the new frequency.
     *
     */
    Q_INVOKABLE void setFrequency(quint8 f);
    /*
     * Resets the antenna as if it was the app startup
     */
    Q_INVOKABLE void reset();

private:
    /*
     * Whether the app is connected to the rocket.
     *
     * It is false at startup and during frequency change. Becomes true upon receiving [C]
     */
    bool m_isArduinoConnected = false;

    State m_state = State::DISCONNECTED;
    /*
     * Error code
     *
     * - 0: no error
     * - 1: no IMU
     * - 2: no barometer
     * - 3: no GPS
     */
    quint8 m_error = 0;
    /*
     * Number between 0 and 81 representing the frequency used by the LoRa library
     *
     * Value of 255 stands for unitliazed frequency (it must be provided by the user at startup)
     */
    quint8 m_frequency = 255;

    QTime m_startTime;
    QSerialPort *arduino;
    void openSerialPort();
    int sendToArduino(quint8 data);
    QByteArray buffer;
    void handleBuffer();

    void __start();
    quint8 __f;
    QTimer *__timer;
    QTimer *scanTimer;
    float __lastBaro = 1013;
    void __randomData();

private slots:
    void readData();

signals:
    void connectedChanged(bool);
    void stateChanged(Antenna::State state);
    void frequencyChanged(quint8 f);
    void errorChange(quint8 e);
    void newData(int ms, RocketData data);
    void rollback();
};

#endif // ANTENNA_H
