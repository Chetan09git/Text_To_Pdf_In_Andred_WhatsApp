import QtQuick 2.15
import QtQuick.Controls 2.15
import "pages"

ApplicationWindow {
    id: window
    width: 640
    height: 800
    minimumWidth: 360
    minimumHeight: 500
    visible: true
    title: qsTr("Text to PDF & WhatsApp Share")
    color: "#F8FAFC"

    header: ToolBar {
        background: Rectangle {
            color: "#075E54"
        }
        Row {
            anchors.centerIn: parent
            spacing: 8
            Text {
                text: "📄"
                font.pixelSize: 20
                anchors.verticalCenter: parent.verticalCenter
            }
            Text {
                text: qsTr("Text to PDF & WhatsApp")
                color: "#FFFFFF"
                font.bold: true
                font.pixelSize: 18
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    DetailsPage {
        anchors.fill: parent
    }
}
