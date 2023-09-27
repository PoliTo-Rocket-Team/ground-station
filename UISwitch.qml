import QtQuick 2.9
import QtQuick.Controls 2.15

Item {
    id: container
    property color color: "#efefef";
    property real size: 26;
    property real space: 2;
    property string text: "";
    property alias checked: control.checked;

    height: control.height;
    width: control.width;

    Switch {
        id: control;
        text: container.text;
        indicator: Rectangle {
            implicitWidth: 48
            implicitHeight: container.size;
            x: 0;
            y: control.height / 2 - height / 2;
            radius: container.size / 2;
            color: "#eaeaea";
            border.color: control.checked ? "#FA8650" : "#bababa"

            Rectangle {
                id: bullet;
                property real dim: container.size - 2*container.space;
                y: container.space;
                x: container.space;
                width: dim;
                height: dim;
                radius: dim / 2;
                color: control.checked ? "#FA8650" : "#bababa"
        //            border.color: control.checked ? "#FA8650" : "#999999"
            }
        }
        contentItem: Text {
            text: container.text
            opacity: enabled ? 1.0 : 0.3
            color: container.color;
            verticalAlignment: Text.AlignVCenter
            leftPadding: control.indicator.width + control.spacing
        }

        states: State {
            name: "checked";
            when: control.checked;
            PropertyChanges {
                target: bullet;
                x: control.indicator.width - bullet.width - container.space;
            }
        }
        transitions: Transition {
            from: "*"; to: "*";
            NumberAnimation  {

                property: x;
                easing.type: Easing.InOutQuad;
                duration: 200;
            }
        }
    }
}
