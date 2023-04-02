import QtQuick 2.9
import QtQuick.Controls 2.15
import Antenna 1.0

Window {
    minimumWidth: 700
    width: 900
    minimumHeight: 350
    height: 500
    visible: true
    title: "PRT - Ground Station"
    color: "#efefef";

    Connections {
        target: Antenna
        function onConnectedChanged(){
            frequency_popup.open();
        }
        function onNewData() {

        }
    }

    property bool nofreq: Antenna.frequency === 255;

    Popup {
        id: frequency_popup
        modal: true;
        closePolicy: nofreq ? Dialog.NoAutoClose : Dialog.CloseOnEscape;
        anchors.centerIn: parent;
        padding: 15;
        width: 350;
        Overlay.modal: Rectangle {
            color: "#b3121212"
        }
        background: Rectangle {
            color: "#efefef";
            radius: 6;
        }

        Column {
            spacing: 10;
            anchors.fill: parent;
            Text {
                text: `${nofreq ? "Set" : "Change"} frequency`;
                color: "#212121";
                font {
                    weight: 700;
                    pointSize: 18;
                }
            }
            Text {
                text: "Please insert the frequency as it is required from the LoRa library. Range is from 0 to 81";
                width: parent.width;
                wrapMode: Text.WordWrap;
                font.pointSize: 12;
            }
            SpinBox {
                id: frequency_input
                stepSize: 1;
                padding: 5;
                font.pointSize: 14;
            }
            Row {
                spacing: 10;
                width: parent.width
                layoutDirection: Qt.RightToLeft;

                Button {
                    text: "Confirm";
                    font.pointSize: 14;
                    onClicked: {
                        const v = frequency_input.value;
                        Antenna.setFrequency(v);
                        console.log("set freq");
                        frequency_popup.close();
                    }
                }
                Button {
                    visible: !nofreq;
                    text: "Cancel";
                    onClicked: frequency_popup.close();
                    font.pointSize: 14;
                }
            }
        }
    }


    Column {
        anchors.centerIn: parent
        visible: !Antenna.connected
        width: 400;

        Text {
            text: "Serial connection"
            font.pointSize: 20;
            font.weight: 700;
            width: parent.width;
            color: "#212121";
        }
        Text {
            text: "Waiting for a serial signal from the Arduino. If you haven't yet, please plug it in."
            font.pointSize: 12;
            wrapMode: Text.WordWrap;
            width: parent.width;
            color: "#212121";
        }
    }

    Grid {
        anchors.fill: parent;
        visible: Antenna.connected;
        rows: 1; columns: 2;

        Rectangle {
            color: "#212121"
            height: parent.height;
            width: 300;

            Column {
                anchors {
                    top: parent.top;
                    left: parent.left;
                    right: parent.right;
                    margins: 20;
                }
                spacing: 30

                Text {
                    text: "Ground Station"
                    color: "#efefef"
                    font {
                        pointSize: 24;
                        weight: 700;
                    }
                }
                Text {
                    text: `Frequency = ${Antenna.frequency}`;
                    color: "#efefef"
                }
            }

            Button {
                anchors {
                    horizontalCenter: parent.horizontalCenter;
                    bottom: parent.bottom;
                    bottomMargin: 20;
                }
                text: "Change frequency";
                onClicked: frequency_popup.open();
            }
        }
        Rectangle {
            height: parent.height;
            width: parent.width - 300;

            Text {
                anchors.centerIn: parent;
                text: "Cool data"
            }
        }
    }
}


