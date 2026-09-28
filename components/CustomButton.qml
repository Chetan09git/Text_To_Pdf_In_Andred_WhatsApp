import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root

    property string text: "Button"
    property string iconText: ""
    property string variant: "primary" // "primary", "whatsapp", "secondary", "outline"
    property bool enabled: true
    property bool loading: false
    property int buttonHeight: 50

    signal clicked()

    Layout.fillWidth: true
    Layout.preferredHeight: root.buttonHeight
    height: root.buttonHeight
    radius: 10

    // Theme color resolution
    readonly property color baseColor: {
        if (!root.enabled) return "#CFD8DC"
        if (root.variant === "whatsapp") return "#25D366"
        if (root.variant === "secondary") return "#ECEFF1"
        if (root.variant === "outline") return "transparent"
        return "#128C7E" // primary
    }

    readonly property color textAndIconColor: {
        if (!root.enabled) return "#90A4AE"
        if (root.variant === "secondary") return "#37474F"
        if (root.variant === "outline") return "#128C7E"
        return "#FFFFFF"
    }

    color: mouseArea.pressed && root.enabled ? Qt.darker(baseColor, 1.12) : (mouseArea.containsMouse && root.enabled ? Qt.lighter(baseColor, 1.08) : baseColor)
    border.color: root.variant === "outline" ? (root.enabled ? "#128C7E" : "#CFD8DC") : "transparent"
    border.width: root.variant === "outline" ? 1.5 : 0

    Behavior on color {
        ColorAnimation { duration: 120 }
    }

    scale: mouseArea.pressed && root.enabled ? 0.98 : 1.0
    Behavior on scale {
        NumberAnimation { duration: 100; easing.type: Easing.OutQuad }
    }

    // Shadow / elevation effect
    Rectangle {
        anchors.fill: parent
        anchors.topMargin: 2
        radius: parent.radius
        color: "#000000"
        opacity: root.enabled && root.variant !== "outline" && !mouseArea.pressed ? 0.08 : 0
        z: -1
    }

    RowLayout {
        anchors.centerIn: parent
        spacing: 10

        // Loading spinner animation
        Item {
            width: 20
            height: 20
            visible: root.loading
            Layout.alignment: Qt.AlignVCenter

            Rectangle {
                id: spinnerDot
                width: 18
                height: 18
                radius: 9
                color: "transparent"
                border.color: root.textAndIconColor
                border.width: 2.5

                Rectangle {
                    width: 6
                    height: 6
                    radius: 3
                    color: root.textAndIconColor
                    anchors.top: parent.top
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                RotationAnimation on rotation {
                    from: 0
                    to: 360
                    duration: 900
                    loops: Animation.Infinite
                    running: root.loading
                }
            }
        }

        // Icon
        Text {
            text: root.iconText
            font.pixelSize: 18
            color: root.textAndIconColor
            visible: !root.loading && root.iconText.length > 0
            Layout.alignment: Qt.AlignVCenter
        }

        // Text
        Text {
            text: root.loading ? "Processing..." : root.text
            font.pixelSize: 16
            font.weight: Font.Bold
            color: root.textAndIconColor
            Layout.alignment: Qt.AlignVCenter
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        enabled: root.enabled && !root.loading
        hoverEnabled: true
        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
        onClicked: root.clicked()
    }
}
