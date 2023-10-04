import QtQuick 2.9
import QtCharts 2.15

ChartView {
    id: cv;
    property real min: -1;
    property real max: +1;
    property bool adaptive: false;
    property real spacing: 0.5;
    property int number: 1;

    property real minTimeDelta: 10;

    function add(time, a, b) {
        series_1.append(time, a);
        series_2.append(time, b);

        let _min = a, _max = b;
        if(a > b) {
            _min = b;
            _max = a;
        }

        if(internal.first) {
            internal.first = false;
            timeAxis.min = time;
            timeAxis.max = time + minTimeDelta;
            internal.min = _min - spacing;
            internal.max = _max + spacing;
        }
        else if(time > timeAxis.max) timeAxis.max = time;
        if(_min < internal.min) internal.min = _min - spacing;
        else if(_max > internal.max) internal.max = _max + spacing;
    }
    function clear() {
        series_1.clear();
        series_2.clear();
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
        id: series_1
        axisX: timeAxis;
        axisY: axisY;
        color: "#ee682b";
    }
    LineSeries {
        id: series_2
        axisX: timeAxis;
        axisY: axisY;
        color: "#121212";
    }
}
