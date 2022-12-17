#ifndef ANTENNA_H
#define ANTENNA_H

#include <QObject>

class Antenna : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)

private:
    bool _connected;
    void setConnected();

public:
    explicit Antenna(QObject *parent = nullptr);
    Q_INVOKABLE void connect();
    bool isConnected() const;

signals:
    void connectedChanged();
//    void frequencyChanged();

};

#endif // ANTENNA_H
