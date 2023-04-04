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
            radius: 8;
        }

        Column {
            spacing: 5;
            anchors.fill: parent;
            Text {
                text: `${nofreq ? "Set" : "Change"} frequency`;
                color: "#212121";
                font {
                    weight: Font.DemiBold;
                    pointSize: 18;
                }
            }
            Text {
                text: "Please insert the desired frequency: range from 850 to 930 MHz";
                width: parent.width;
                wrapMode: Text.WordWrap;
                bottomPadding: 15;
            }
            Row {
                SpinBox {
                    id: frequency_input
                    from: 850; to: 930;
                    stepSize: 1;
                    value: 850;
                    wheelEnabled: true
                    width: 80
                    background: Rectangle {
                        radius: 3;
                        color: "white";
                        border.width: 0;
                    }
                }
                Text {
                    text: ".125 MHz"
                    leftPadding: 2
                }
            }

            Row {
                topPadding: 15
                spacing: 10;
                width: parent.width
                layoutDirection: Qt.RightToLeft;

                UIButton {
                    text: "Confirm";
                    onClicked: {
                        const v = frequency_input.value - 850;
                        Antenna.setFrequency(v);
                        console.log("set freq");
                        frequency_popup.close();
                    }
                }
                UIButton {
                    visible: !nofreq;
                    text: "Cancel";
                    onClicked: frequency_popup.close();
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
            font.weight: Font.Bold;
            width: parent.width;
            color: "#212121";
        }
        Text {
            text: "Waiting for a serial signal from the Arduino. If you haven't yet, please plug it in."
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
                        weight: Font.Bold;
                    }
                }
                Text {
                    text: `Frequency = ${Antenna.frequency + 850}.125 MHz`;
                    color: "#efefef"
                }
            }
            UIButton {
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
            id: body;
            height: parent.height;
            width: parent.width - 300;

            Text {
                text: "Cool data"
                anchors.centerIn: parent
            }
        }
    }
}


