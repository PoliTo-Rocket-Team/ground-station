#ifndef ANTENNA_H
#define ANTENNA_H

#include <QObject>
#include <QTimer>
#include <QTime>
#include <QVector3D>
#include <QSerialPort>
#include <fstream>


struct RocketData {
    Q_GADGET
    Q_PROPERTY(float pressure1 MEMBER pressure1);
    Q_PROPERTY(float pressure2 MEMBER pressure2);
    Q_PROPERTY(float temperature1 MEMBER temperature1);
    Q_PROPERTY(float temperature2 MEMBER temperature2);
    // Q_PROPERTY(float latitude MEMBER lat);
    // Q_PROPERTY(float longitude MEMBER lng);
    Q_PROPERTY(QVector3D acc_lin MEMBER acc_lin);
    Q_PROPERTY(QVector3D acc_ang MEMBER acc_ang);
public:
    float pressure1, pressure2;
    float temperature1, temperature2;
    // float lat, lng;
    QVector3D acc_lin, acc_ang;
    RocketData(std::byte* raw);
    RocketData() = default;
};
Q_DECLARE_METATYPE(RocketData);

class Antenna : public QObject
{
    Q_OBJECT

    Q_PROPERTY(quint8 error READ getError NOTIFY errorChange)
    Q_PROPERTY(quint8 channel READ getChannel NOTIFY channelChanged)
    Q_PROPERTY(State state READ getState NOTIFY stateChanged)
    Q_PROPERTY(QString portName READ getPortName NOTIFY portNameChanged)
    Q_PROPERTY(QString boardName READ getBoardName NOTIFY boardNameChanged)

public:
    explicit Antenna(QObject *parent = nullptr);

    enum State { SCANNING, OPENING_SERIAL, OFFLINE, POLLING, ONLINE, MEASURING };
    Q_ENUM(State)

    State getState() const { return m_state; };
    QString getPortName() const { return portName; };
    QString getBoardName() const { return boardName; };
    quint8 getError() const { return m_error; };
    quint8 getChannel() const { return m_channel; };


    /*
     * Used to set the frequency
     *
     * While m_frequency == 255 (app startup), it changes simply the listening frequency of the message [C] from the rocket.
     *
     * Otherwise, it sends a [F] message to the rocket and set connected = false. The outcome can be then [E4] or connected with the new frequency.
     *
     */
    Q_INVOKABLE void setChannel(quint8 c, bool ch_l);
    /*
     * Resets the antenna as if it was the app startup
     */
    Q_INVOKABLE void reset();

    Q_INVOKABLE bool openOutputFile(QString path);

    Q_INVOKABLE void closeOutputFile();

    Q_INVOKABLE void startMeasuring();

private:
    State m_state = State::ONLINE;
    QString portName;
    QString boardName;

    std::ofstream output_file;
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
    quint8 m_channel = 255;
    quint8 old_channel = 255;
    void confirmOnline();

    QTime m_startTime;
    QSerialPort *arduino;
    void openSerialPort();
    QTimer *scanTimer;
    int sendToArduino(quint8 data);


    // payload-related members
    QByteArray payload;
    qsizetype remaining_bytes = 0;
    char current_code = 0x00;
    void goForward(QByteArray& a, qsizetype from);
    void startPayloadAt(QByteArray& a, qsizetype from);
    void handlePayload();
    float packFloat(qsizetype from);

private slots:
    void readData();

signals:
    void stateChanged(Antenna::State state);
    void portNameChanged(QString name);
    void boardNameChanged(QString name);
    void channelChanged(quint8 f);
    void errorChange(quint8 e);
    void newData(int ms, RocketData data);
    void rollback();
};

#endif // ANTENNA_H
