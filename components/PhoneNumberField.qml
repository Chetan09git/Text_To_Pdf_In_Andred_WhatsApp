import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ColumnLayout {
    id: root

    property string label: "Phone Number"
    property string countryCode: "+91"
    property string number: ""
    readonly property string fullPhoneNumber: countryCode.length > 0 ? (countryCode + " " + number).trim() : number.trim()
    property string placeholderText: "98765 43210"
    property bool required: true
    property string errorMessage: ""
    property bool hasError: false

    signal phoneNumberChanged(string fullNumber)

    spacing: 6
    Layout.fillWidth: true

    onNumberChanged: {
        if (singleInput.text !== root.number) {
            singleInput.text = root.number
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
        height: 48
        radius: 8
        color: singleInput.activeFocus ? "#FFFFFF" : "#F8FAFC"
        border.color: root.hasError ? "#E53935" : (singleInput.activeFocus ? "#128C7E" : "#CFD8DC")
        border.width: singleInput.activeFocus || root.hasError ? 2 : 1

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

            // Phone Icon
            Text {
                text: "📞"
                font.pixelSize: 15
                Layout.alignment: Qt.AlignVCenter
            }

            // Country Code Prefix Box
            Rectangle {
                id: countryCodeBox
                width: 52
                height: 32
                radius: 6
                color: "#ECEFF1"
                Layout.alignment: Qt.AlignVCenter

                TextInput {
                    id: countryCodeInput
                    anchors.centerIn: parent
                    text: root.countryCode
                    font.pixelSize: 13
                    font.weight: Font.Bold
                    color: "#37474F"
                    maximumLength: 5
                    inputMethodHints: Qt.ImhDialableCharactersOnly
                    onTextEdited: {
                        root.countryCode = text
                        root.phoneNumberChanged(root.fullPhoneNumber)
                    }
                }
            }

            // Vertical separator
            Rectangle {
                width: 1
                height: 24
                color: "#CFD8DC"
                Layout.alignment: Qt.AlignVCenter
            }

            // Number Input Field
            TextInput {
                id: singleInput
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                font.pixelSize: 15
                color: "#263238"
                verticalAlignment: TextInput.AlignVCenter
                inputMethodHints: Qt.ImhDigitsOnly | Qt.ImhDialableCharactersOnly
                maximumLength: 15
                selectByMouse: true
                selectionColor: "#80CBC4"

                validator: RegularExpressionValidator {
                    regularExpression: /^[0-9\s\-()]{0,15}$/
                }

                Text {
                    text: root.placeholderText
                    color: "#90A4AE"
                    font.pixelSize: 15
                    anchors.fill: parent
                    verticalAlignment: Text.AlignVCenter
                    visible: !singleInput.text && !singleInput.inputMethodComposing
                }

                onTextEdited: {
                    root.number = singleInput.text
                    root.phoneNumberChanged(root.fullPhoneNumber)
                }
            }

            // Clear Button
            Rectangle {
                width: 24
                height: 24
                radius: 12
                color: clearMouseArea.containsMouse ? "#ECEFF1" : "transparent"
                visible: singleInput.text.length > 0
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
                        root.number = ""
                        root.phoneNumberChanged(root.fullPhoneNumber)
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
