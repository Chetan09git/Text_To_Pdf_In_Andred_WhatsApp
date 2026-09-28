import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ColumnLayout {
    id: root

    property string label: ""
    property string text: ""
    property string placeholderText: ""
    property int inputMethodHints: Qt.ImhNone
    property int maximumLength: 32767
    property bool required: false
    property bool isMultiline: false
    property string errorMessage: ""
    property bool hasError: false
    property string iconText: ""
    property var validator: null
    property bool readOnly: false

    signal textChangedByUser(string newText)
    signal accepted()

    spacing: 6
    Layout.fillWidth: true

    // Synchronize external text property
    onTextChanged: {
        if (isMultiline) {
            if (multiArea.text !== root.text) multiArea.text = root.text
        } else {
            if (singleInput.text !== root.text) singleInput.text = root.text
        }
    }

    // Field Label
    RowLayout {
        spacing: 4
        visible: root.label.length > 0
        Layout.fillWidth: true

        Text {
            text: root.label
            font.pixelSize: 14
            font.weight: Font.DemiBold
            color: root.hasError ? "#E53935" : "#37474F"
        }

        Text {
            text: "*"
            font.pixelSize: 14
            font.weight: Font.Bold
            color: "#E53935"
            visible: root.required
        }
    }

    // Input Box Container
    Rectangle {
        id: boxRect
        Layout.fillWidth: true
        height: root.isMultiline ? 100 : 48
        radius: 8
        color: (root.isMultiline ? multiArea.activeFocus : singleInput.activeFocus) ? "#FFFFFF" : "#F8FAFC"
        border.color: root.hasError ? "#E53935" : ((root.isMultiline ? multiArea.activeFocus : singleInput.activeFocus) ? "#128C7E" : "#CFD8DC")
        border.width: (root.isMultiline ? multiArea.activeFocus : singleInput.activeFocus) || root.hasError ? 2 : 1

        Behavior on border.color {
            ColorAnimation { duration: 150 }
        }
        Behavior on color {
            ColorAnimation { duration: 150 }
        }

        RowLayout {
            anchors.fill: parent
            anchors.margins: 8
            spacing: 8

            // Optional Icon
            Text {
                text: root.iconText
                font.pixelSize: 16
                color: (root.isMultiline ? multiArea.activeFocus : singleInput.activeFocus) ? "#128C7E" : "#90A4AE"
                visible: root.iconText.length > 0
                Layout.alignment: root.isMultiline ? Qt.AlignTop : Qt.AlignVCenter
            }

            // Single line Input
            TextInput {
                id: singleInput
                visible: !root.isMultiline
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                font.pixelSize: 15
                color: "#263238"
                verticalAlignment: TextInput.AlignVCenter
                inputMethodHints: root.inputMethodHints
                maximumLength: root.maximumLength
                validator: root.validator
                readOnly: root.readOnly
                selectByMouse: true
                selectionColor: "#80CBC4"

                Text {
                    text: root.placeholderText
                    color: "#90A4AE"
                    font.pixelSize: 15
                    anchors.fill: parent
                    verticalAlignment: Text.AlignVCenter
                    visible: !singleInput.text && !singleInput.inputMethodComposing
                }

                onTextEdited: {
                    root.text = singleInput.text
                    root.textChangedByUser(singleInput.text)
                }
                onAccepted: {
                    root.accepted()
                }
            }

            // Multiline Area
            Flickable {
                id: flickableArea
                visible: root.isMultiline
                Layout.fillWidth: true
                Layout.fillHeight: true
                contentWidth: width
                contentHeight: multiArea.implicitHeight
                clip: true

                TextArea.flickable: TextArea {
                    id: multiArea
                    font.pixelSize: 15
                    color: "#263238"
                    wrapMode: TextArea.Wrap
                    inputMethodHints: root.inputMethodHints
                    readOnly: root.readOnly
                    selectByMouse: true
                    selectionColor: "#80CBC4"
                    background: null
                    padding: 0

                    Text {
                        text: root.placeholderText
                        color: "#90A4AE"
                        font.pixelSize: 15
                        anchors.top: parent.top
                        anchors.left: parent.left
                        visible: !multiArea.text && !multiArea.inputMethodComposing
                    }

                    onTextChanged: {
                        root.text = multiArea.text
                        root.textChangedByUser(multiArea.text)
                    }
                }
            }

            // Clear Button (only for single line)
            Rectangle {
                width: 24
                height: 24
                radius: 12
                color: clearMouseArea.containsMouse ? "#ECEFF1" : "transparent"
                visible: !root.isMultiline && singleInput.text.length > 0 && !root.readOnly
                Layout.alignment: Qt.AlignVCenter

                Text {
                    anchors.centerIn: parent
                    text: "✕"
                    font.pixelSize: 12
                    color: "#78909C"
                }

                MouseArea {
                    id: clearMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        singleInput.text = ""
                        root.text = ""
                        root.textChangedByUser("")
                    }
                }
            }
        }
    }

    // Error Message
    Text {
        text: root.errorMessage
        font.pixelSize: 12
        color: "#E53935"
        visible: root.hasError && root.errorMessage.length > 0
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
    }
}
