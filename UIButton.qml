import QtQuick 2.9
import QtQuick.Controls 2.0

Button {
    id: btn
    text: ""
    flat: true;


    contentItem: Text {
        id: textual
        text: btn.text
        font.pointSize: 12;
        font.weight: Font.DemiBold;
        opacity: enabled ? 1.0 : 0.3
        color: btn.hovered ? "#efefef" : "#FA8650";
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        topPadding: 3;
        bottomPadding: 3;
        leftPadding: 8;
        rightPadding: 8;
    }

    background: Rectangle {
        opacity: enabled ? 1 : 0.3
        color: btn.hovered ? "#FA8650" : "transparent";
        radius: 4
        border {
            color: "#FA8650";
            width: 2;
        }
    }
}
