import QtQuick 2.9
import QtQuick.Controls 2.15
//import UIButton.qml
import Antenna 1.0

Window {
    minimumWidth: 480
    width: 640
    minimumHeight: 270
    height: 480
    visible: true
    title: "Hello World"

    Text {
        anchors.centerIn: parent
        text: Antenna.connected ? "Connected" : "Disconnected"
        font.pointSize: 18;
    }

    UIButton {
        text: "Connect"
        font.pointSize: 18
        onClicked: Antenna.connect()
        visible: !Antenna.connected
        anchors {
            bottom: parent.bottom;
            left: parent.left
            bottomMargin: 40
            leftMargin: 40
        }
    }
}
