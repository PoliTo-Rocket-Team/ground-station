import QtQuick 2.9
import QtCharts 2.15

ChartView {
    property color lineColor: "gray";
    property real initialMin: -1;
    property real initialMax: +1;
    property real minTimeDelta: 10;
    function add(time, value) {
        series.append(time, value);
        if(internal.first) {
            internal.first = false;
            axisTime.min = time;
            axisTime.max = time + minTimeDelta;
            axisY.min = initialMin;
            axisY.max = initialMax;
        }
        else if(time > axisTime.max) axisTime.max = time;
        if(value < axisY.min) axisY.min = value - 0.5;
        else if(value > axisY.max) axisY.max = value + 0.5;
    }
    function clear() {
        series.clear();
        internal.first = true;
    }
    legend.visible: false;

    QtObject {
        id: internal;
        property bool first: true;
    }

    ValuesAxis {
        id: axisTime;
        min: 0;
        max: 1;
    }
    ValuesAxis {
        id: axisY;
        min: 0;
        max: 1;
    }
    LineSeries {
        id: series;
        axisX: axisTime;
        axisY: axisY;
        color: lineColor;
    }
}
