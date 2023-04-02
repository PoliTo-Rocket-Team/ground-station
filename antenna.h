#ifndef ANTENNA_H
#define ANTENNA_H

#include <QObject>

struct RocketData {
    float barometer;
    float lat, lng;
    float acc_lin[3];
    float acc_ang[3];
};

class Antenna : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)
    Q_PROPERTY(quint8 error READ getError NOTIFY errorChange)
    Q_PROPERTY(quint8 frequency READ getFrequency WRITE setFrequency NOTIFY frequencyChanged)

private:
    /*
     * Whether the app is connected to the rocket.
     *
     * It is false at startup and during frequency change. Becomes true upon receiving [C]
     */
    bool m_connected = false;
    /*
     * Error code
     *
     * - 0: no error
     * - 1: no IMU
     * - 2: no barometer
     * - 3: no GPS
     * - 4: could not change frequency
     */
    quint8 m_error = 0;
    /*
     * Number between 0 and 81 representing the frequency used by the LoRa library
     *
     * Value of 255 stands for unitliazed frequency (it must be provided by the user at startup)
     */
    quint8 m_frequency = 255;

    void __start();

public:
    explicit Antenna(QObject *parent = nullptr);

    bool isConnected() const { return m_connected; };
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

signals:
    void connectedChanged(bool);
    void frequencyChanged();
    void errorChange();
    void newData(RocketData data);
};

#endif // ANTENNA_H
