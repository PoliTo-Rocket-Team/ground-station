import QtQuick 2.9
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Antenna 1.0

Window {
    minimumWidth: 900
    width: 1200
    minimumHeight: 600
    height: 900
    visible: true
    title: "PRT - Ground Station"
    color: "#efefef";

    Connections {
        target: Antenna
        function onStateChanged(s){
            if(s === Antenna.OFFLINE) frequency_popup.open();
        }
        function onNewData(time, data) {;
            acc_lin.add(time, data.acc_lin);
            acc_ang.add(time, data.acc_ang);
            pressure.add(time, data.pressure1, data.pressure2)
            temperature.add(time, data.temperature1, data.temperature2);
        }
        function onFrequencyChanged() {
            acc_lin.clear();
            acc_ang.clear();
            pressure.clear();
            temperature.clear();
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
                text: "Please insert the desired frequency: range goes from 850 to 930 MHz";
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
                    text: "MHz"
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

    Message {
        anchors.centerIn: parent
        visible: Antenna.state === Antenna.SCANNING
        width: 400;
        title: "Scanning for Arduino";
        description: "Waiting for a serial signal from the Arduino. If you haven't yet, please plug it in."
    }

    Message {
        anchors.centerIn: parent
        visible: Antenna.state === Antenna.OPENING_SERIAL
        width: 560;
        title: "Opening serial port";
        description: `Trying to connect to the board "${Antenna.boardName}" on port ${Antenna.portName}. The connection won't be successful unless the ground-station arduino code has been uploaded to the board. Should this message persist, close the application, open and close the serial monitor of the Arduino IDE, and finally re-open the application.`
    }

    Grid {
        anchors.fill: parent;
        visible: Antenna.state === Antenna.OFFLINE || Antenna.state === Antenna.ONLINE || Antenna.state === Antenna.POLLING;
        rows: 1; columns: 2;

        Rectangle {
            id: sidebar;
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
                spacing: 40

                Text {
                    text: "Ground Station"
                    color: "#efefef"
                    font {
                        pointSize: 24;
                        weight: Font.Bold;
                    }
                }
                Column {
                    spacing: 5;
                    Text {
                        text: "Board"
                        color: "#efefef"
                        font {
                            pointSize: 14;
                            weight: Font.Bold;
                        }
                    }
                    Text {
                        text: "Port: " + Antenna.portName;
                        color: "#efefef";
                    }
                    Text {
                        text: "Name: " + Antenna.boardName;
                        color: "#efefef";
                    }
                }

                Column {
                    spacing: 5;
                    Text {
                        text: "Frequency"
                        color: "#efefef"
                        font {
                            pointSize: 14;
                            weight: Font.Bold;
                        }
                    }
                    Text {
                        text: nofreq ? "Not set" : `${Antenna.frequency + 850}MHz`;
                        color: "#efefef"
                    }
                    Item { width: 1; height: 3; }
                    UIButton {
                        text: "Change";
                        onClicked: frequency_popup.open();
                    }
                }
//                Column {
//                    spacing: 5;
//                    Text {
//                        text: "Parachute"
//                        color: "#efefef"
//                        font {
//                            pointSize: 14;
//                            weight: Font.Bold;
//                        }
//                    }
//                    Text {
//                        text: "Target height = 4000m";
//                        color: "#efefef";
//                    }
//                    Text {
//                        text: "No parachute deployed";
//                        color: "#efefef";
//                    }
//                }
                Column {
                    spacing: 5;
                    Text {
                        text: "Graphs"
                        color: "#efefef"
                        font {
                            pointSize: 14;
                            weight: Font.Bold;
                        }
                    }
                    UISwitch {
                        id: adaptive;
                        text: "Adaptive";
                    }

                    Item { width: 1; height: 3; }
                    UIButton {
                        text: "Reset axis";
                        onClicked: {
                            acc_lin.clear();
                            acc_ang.clear();
                            barometer.clear();
                        }
                    }
                }
            }
            Image {
                id: logo
                source: "qrc:///imgs/logo"
                width: 100;
                height: width * 289/600;
                smooth: true;
                sourceSize {
                    width: 600;
                    height: 289
                }
                anchors {
                    bottom: parent.bottom;
                    bottomMargin: 20;
                    horizontalCenter: parent.horizontalCenter;
                }
            }
        }
        Rectangle {
            id: body;
            height: parent.height;
            width: parent.width - 300;

            Message {
                width: 350
                anchors.centerIn: parent
                visible: Antenna.state === Antenna.POLLING;
                title: "Polling";
                description: `Waiting for a signal from the rocket at frequency ${Antenna.frequency+850} MHz`;
            }

            Message {
                width: 350
                anchors.centerIn: parent
                visible: Antenna.state === Antenna.OFFLINE;
                title: "Offline";
                description: nofreq ? "No frequency was selected" : "No signal was received from the rocket";
            }



            Message {
                width: 350
                anchors.centerIn: parent
                visible: Antenna.state === Antenna.CONNECTED && Antenna.error !== 0;
                title: `${getFaultingComponent(Antenna.error)} not working`;
                description: `Error #${Antenna.error}`

                function getFaultingComponent(code) {
                    switch(code) {
                    case 1: return "IMU";
                    case 2: return "Barometer";
                    case 3: return "GPS";
                    default: return "Something"
                    }
                }
            }

            GridLayout {
                visible: Antenna.state === Antenna.CONNECTED && Antenna.error === 0;
                rows: 2;
                columns: 2;
                anchors {
                    fill: parent;
                    margins: 30;
                }
                VectorPlot {
                    id: acc_lin;
                    title: "Linear acceleration (m/s^2)"
                    minTimeDelta: 60;
                    Layout.fillWidth: true;
                    Layout.fillHeight: true;
                }
                VectorPlot {
                    id: acc_ang;
                    title: "Angular velocity (rad/s)"
                    minTimeDelta: 60;
                    Layout.fillWidth: true;
                    Layout.fillHeight: true;
                }
                DualPlot {
                    id: pressure;
                    title: "Pressure (hPa)";
                    min: 500;
                    max: 1100;
                    minTimeDelta: 60;
                    spacing: 10;
                    Layout.fillWidth: true;
                    Layout.fillHeight: true;
                    adaptive: adaptive.checked;
                    number: 2;
                }
                DualPlot {
                    id: temperature;
                    title: "Temperature (°C)";
                    min: 20;
                    max: 60;
                    minTimeDelta: 60;
                    spacing: 2;
                    Layout.fillWidth: true;
                    Layout.fillHeight: true;
                    adaptive: adaptive.checked;
                    number: 2;
                }
            }
        }
    }
}


