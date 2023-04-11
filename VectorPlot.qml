import QtQuick 2.9
import QtCharts 2.15

ChartView {
    property real minHalfYWidth: 4;
    property real minTimeDelta: 10;
    function add(time, vec) {
        x_series.append(time, vec.x);
        y_series.append(time, vec.y);
        z_series.append(time, vec.z);
        if(internal.first) {
            internal.first = false;
            axisTime.min = time;
            axisTime.max = time + minTimeDelta;
            axisY.min = -minHalfYWidth;
            axisY.max = +minHalfYWidth;
        }
        else if(time > axisTime.max) axisTime.max = time;
        const min = Math.min(vec.x,vec.y,vec.z);
        if(min < axisY.min) axisY.min = min - 0.5;
        const max = Math.max(vec.x,vec.y,vec.z);
        if(max > axisY.max) axisY.max = max + 0.5;
    }
    function clear() {
        x_series.clear();
        y_series.clear();
        z_series.clear();
        internal.first = true;
    }

    antialiasing: true;

    QtObject {
        id: internal;
        property bool first: true;
    }

    ValuesAxis {
        id: axisTime;
        min: 0;
        max: minTimeDelta;
    }
    ValuesAxis {
        id: axisY;
        min: 0;
        max: 1;
    }

    LineSeries {
        id: x_series;
        name: "x";
        axisX: axisTime;
        axisY: axisY;
        color: "#bf212f"
    }
    LineSeries {
        id: y_series;
        name: "y";
        axisX: axisTime;
        axisY: axisY;
        color: "#27b376";
    }
    LineSeries {
        id: z_series;
        name: "z";
        axisX: axisTime;
        axisY: axisY;
        color: "#264b96"
    }
}
