import QtQuick 2.9
import QtQuick.Controls 2.0

Button {
    id: controlBt
    text: ""

    contentItem: Text {
        id: textual
        text: controlBt.text
        font: controlBt.font
        opacity: enabled ? 1.0 : 0.3
        color: controlBt.down ? "#17a81a" : "#21be2b"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        implicitWidth: textual.width + 30
        implicitHeight: textual.height + 15
        opacity: enabled ? 1 : 0.3
        border.color: controlBt.down ? "#17a81a" : "#21be2b"
        border.width: 1
        radius: 2
    }
}
