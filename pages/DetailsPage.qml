import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../components"

Item {
    id: root

    property string currentPdfPath: ""
    property bool isPdfReady: currentPdfPath.length > 0

    Connections {
        target: pdfGenerator
        function onPdfGenerated(filePath) {
            root.currentPdfPath = filePath;
            statusBanner.type = "success";
            statusBanner.message = "PDF generated successfully!\n" + filePath.split("/").pop();
            statusBanner.autoDismiss = false;
        }
        function onGenerationFailed(errorMessage) {
            statusBanner.type = "error";
            statusBanner.message = "Error generating PDF: " + errorMessage;
            statusBanner.autoDismiss = false;
        }
    }

    Connections {
        target: whatsAppShare
        function onShareSuccess() {
            var phone = detailsForm.phoneNumber;
            statusBanner.type = "success";
            statusBanner.message = "🚀 WhatsApp chat opened for " + phone + "!\nPDF document is being automatically attached and sent.";
            statusBanner.autoDismiss = false;
        }
        function onShareError(errorMessage) {
            var phone = detailsForm.phoneNumber;
            statusBanner.type = "error";
            statusBanner.message = "WhatsApp notice: " + errorMessage;
            statusBanner.autoDismiss = false;
        }
    }

    ScrollView {
        id: scrollView
        anchors.fill: parent
        contentWidth: availableWidth
        clip: true

        ScrollBar.vertical.policy: ScrollBar.AsNeeded

        ColumnLayout {
            width: Math.min(parent.width - 32, 600)
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 20

            Item { height: 8 }

            // Card Container
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: cardLayout.implicitHeight + 32
                radius: 12
                color: "#FFFFFF"
                border.color: "#E2E8F0"
                border.width: 1

                // Subtle shadow
                Rectangle {
                    anchors.fill: parent
                    anchors.topMargin: 3
                    radius: parent.radius
                    color: "#000000"
                    opacity: 0.05
                    z: -1
                }

                ColumnLayout {
                    id: cardLayout
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 16

                    // Instruction Banner
                    Rectangle {
                        Layout.fillWidth: true
                        implicitHeight: introRow.implicitHeight + 16
                        radius: 8
                        color: "#F0FDF4"
                        border.color: "#BBF7D0"
                        border.width: 1

                        RowLayout {
                            id: introRow
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 10

                            Text {
                                text: "💡"
                                font.pixelSize: 18
                                Layout.alignment: Qt.AlignVCenter
                            }

                            Text {
                                text: "Fill in your details below, generate a formatted PDF document, and share it directly via WhatsApp."
                                font.pixelSize: 13
                                color: "#166534"
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                            }
                        }
                    }

                    // Status / Error / Success Message
                    StatusMessage {
                        id: statusBanner
                    }

                    // Input Form
                    DetailsForm {
                        id: detailsForm
                    }

                    // Action Buttons Divider
                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: "#E2E8F0"
                        Layout.topMargin: 8
                        Layout.bottomMargin: 8
                    }

                    // 1. Generate PDF Button
                    CustomButton {
                        id: generateBtn
                        text: "Generate PDF"
                        iconText: "📄"
                        variant: "primary"
                        loading: pdfGenerator.isGenerating
                        enabled: !pdfGenerator.isGenerating

                        onClicked: {
                            statusBanner.message = "";
                            if (detailsForm.validateForm()) {
                                pdfGenerator.generatePdf(
                                    detailsForm.name,
                                    detailsForm.phoneNumber,
                                    detailsForm.email,
                                    detailsForm.address,
                                    detailsForm.additionalDetails
                                );
                            } else {
                                statusBanner.type = "error";
                                statusBanner.message = "Please correct the highlighted fields above.";
                            }
                        }
                    }

                    // 2. Share via WhatsApp Button
                    CustomButton {
                        id: shareBtn
                        text: "Share via WhatsApp"
                        iconText: "💬"
                        variant: "whatsapp"
                        enabled: root.isPdfReady && !pdfGenerator.isGenerating && !whatsAppShare.isSharing
                        loading: whatsAppShare.isSharing

                        onClicked: {
                            if (root.currentPdfPath.length > 0) {
                                var msg = "Here is the personal details PDF for " + detailsForm.name;
                                whatsAppShare.sharePdf(root.currentPdfPath, msg, detailsForm.phoneNumber);
                            }
                        }
                    }

                    // 3. Interactive Drag & Drop PDF Card (Drag directly into WhatsApp)
                    Rectangle {
                        Layout.fillWidth: true
                        implicitHeight: 74
                        radius: 10
                        color: dragMouseArea.containsMouse ? "#DCFCE7" : "#F0FDF4"
                        border.color: dragMouseArea.containsMouse ? "#22C55E" : "#86EFAC"
                        border.width: 2
                        visible: root.isPdfReady

                        Behavior on color { ColorAnimation { duration: 150 } }
                        Behavior on border.color { ColorAnimation { duration: 150 } }

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 12

                            Rectangle {
                                width: 44
                                height: 44
                                radius: 8
                                color: "#128C7E"
                                Layout.alignment: Qt.AlignVCenter

                                Text {
                                    anchors.centerIn: parent
                                    text: "📄"
                                    font.pixelSize: 22
                                }
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2

                                Text {
                                    text: "🖱️ Hold & Drag PDF into WhatsApp"
                                    font.pixelSize: 13
                                    font.weight: Font.Bold
                                    color: "#166534"
                                }

                                Text {
                                    text: "Click and drag this box directly into your WhatsApp chat to attach."
                                    font.pixelSize: 11
                                    color: "#15803D"
                                    wrapMode: Text.WordWrap
                                    Layout.fillWidth: true
                                }
                            }

                            Rectangle {
                                width: 70
                                height: 28
                                radius: 14
                                color: "#25D366"
                                Layout.alignment: Qt.AlignVCenter

                                Text {
                                    anchors.centerIn: parent
                                    text: "DRAG ↗"
                                    font.pixelSize: 11
                                    font.bold: true
                                    color: "#FFFFFF"
                                }
                            }
                        }

                        MouseArea {
                            id: dragMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.OpenHandCursor

                            onPressed: {
                                whatsAppShare.startFileDrag(root.currentPdfPath);
                            }
                        }
                    }

                    // Quick Desktop Actions Row (Reveal in Finder / Reset)
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10
                        visible: root.isPdfReady

                        CustomButton {
                            text: "Reveal in Finder"
                            iconText: "📂"
                            variant: "secondary"
                            buttonHeight: 38
                            onClicked: {
                                whatsAppShare.revealInFinder(root.currentPdfPath);
                            }
                        }

                        CustomButton {
                            text: "Reset Form"
                            iconText: "↺"
                            variant: "outline"
                            buttonHeight: 38
                            enabled: !pdfGenerator.isGenerating
                            onClicked: {
                                detailsForm.resetForm();
                                root.currentPdfPath = "";
                                statusBanner.message = "";
                            }
                        }
                    }

                    // Reset Button (when PDF not yet ready)
                    CustomButton {
                        text: "Reset Form"
                        iconText: "↺"
                        variant: "outline"
                        buttonHeight: 40
                        enabled: !pdfGenerator.isGenerating
                        visible: !root.isPdfReady

                        onClicked: {
                            detailsForm.resetForm();
                            root.currentPdfPath = "";
                            statusBanner.message = "";
                        }
                    }
                }
            }

            Item { height: 24 }
        }
    }
}
