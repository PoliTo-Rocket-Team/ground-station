import QtQuick 2.9

Column {
    property string title: "";
    property string description: "";
    spacing: 2;
    Text {
        text: title;
        font.pointSize: 20;
        font.weight: Font.Bold;
        width: parent.width;
        color: "#212121";
    }
    Text {
        text: description;
        wrapMode: Text.WordWrap;
        width: parent.width;
        color: "#212121";
    }
}
