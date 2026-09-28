import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root

    property string message: ""
    property string type: "info" // "success", "error", "info", "warning"
    property bool autoDismiss: false
    property int dismissDuration: 5000

    signal dismissed()

    visible: message.length > 0
    opacity: visible ? 1 : 0
    Layout.fillWidth: true
    implicitHeight: mainLayout.implicitHeight + 24
    radius: 8

    readonly property color bgColor: {
        if (type === "success") return "#E8F5E9"
        if (type === "error") return "#FFEBEE"
        if (type === "warning") return "#FFF8E1"
        return "#E1F5FE"
    }

    readonly property color borderColor: {
        if (type === "success") return "#4CAF50"
        if (type === "error") return "#EF5350"
        if (type === "warning") return "#FFA000"
        return "#03A9F4"
    }

    readonly property color contentColor: {
        if (type === "success") return "#2E7D32"
        if (type === "error") return "#C62828"
        if (type === "warning") return "#F57F17"
        return "#0277BD"
    }

    readonly property string iconSymbol: {
        if (type === "success") return "✓"
        if (type === "error") return "⚠"
        if (type === "warning") return "!"
        return "ℹ"
    }

    color: bgColor
    border.color: borderColor
    border.width: 1

    Behavior on opacity {
        NumberAnimation { duration: 200 }
    }

    Timer {
        id: dismissTimer
        interval: root.dismissDuration
        running: root.visible && root.autoDismiss
        onTriggered: {
            root.message = ""
            root.dismissed()
        }
    }

    RowLayout {
        id: mainLayout
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        // Status Badge / Icon
        Rectangle {
            width: 24
            height: 24
            radius: 12
            color: root.borderColor
            Layout.alignment: Qt.AlignTop

            Text {
                anchors.centerIn: parent
                text: root.iconSymbol
                font.pixelSize: 13
                font.weight: Font.Bold
                color: "#FFFFFF"
            }
        }

        // Message text
        Text {
            text: root.message
            font.pixelSize: 14
            color: root.contentColor
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
        }

        // Dismiss cross button
        Rectangle {
            width: 24
            height: 24
            radius: 12
            color: closeMouseArea.containsMouse ? "#B0BEC5" : "transparent"
            Layout.alignment: Qt.AlignTop

            Text {
                anchors.centerIn: parent
                text: "✕"
                font.pixelSize: 11
                color: root.contentColor
            }

            MouseArea {
                id: closeMouseArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    root.message = ""
                    root.dismissed()
                }
            }
        }
    }
}
