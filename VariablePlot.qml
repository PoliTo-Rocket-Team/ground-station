import QtQuick 2.9
import QtCharts 2.15

ChartView {
    id: cv;
    property real min: -1;
    property real max: +1;
    property bool adaptive: false;
    property real spacing: 0.5;

    property color lineColor: "gray";
    property real minTimeDelta: 10;

    function add(time, value) {
        series.append(time, value);
        if(internal.first) {
            internal.first = false;
            timeAxis.min = time;
            timeAxis.max = time + minTimeDelta;
            internal.min = value - spacing;
            internal.max = value + spacing;
        }
        else if(time > timeAxis.max) timeAxis.max = time;
        if(value < internal.min) internal.min = value - spacing;
        else if(value > internal.max) internal.max = value + spacing;
    }
    function clear() {
        series.clear();
        internal.first = true;
    }

    legend.visible: false;

    QtObject {
        id: internal;
        property bool first: true;
        property real min: -1;
        property real max: +1;
    }

    ValuesAxis {
        id: timeAxis;
        min: 0;
        max: 1;
    }
    ValuesAxis {
        id: axisY;
        min: adaptive ? internal.min : cv.min;
        max: adaptive ? internal.max : cv.max;
    }
    LineSeries {
        id: series;
        axisX: timeAxis;
        axisY: axisY;
        color: lineColor;
    }
}
