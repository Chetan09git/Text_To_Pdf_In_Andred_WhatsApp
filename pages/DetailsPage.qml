import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../components"

Item {
    id: root

    property string currentPdfPath: ""
    property bool isPdfReady: currentPdfPath.length > 0
    property bool isAndroid: Qt.platform.os === "android"
    property bool pendingShareOnGenerate: false

    Connections {
        target: pdfGenerator
        function onPdfGenerated(filePath) {
            root.currentPdfPath = filePath;
            statusBanner.type = "success";
            statusBanner.message = "PDF generated successfully!\n" + filePath.split("/").pop();
            statusBanner.autoDismiss = false;

            if (root.pendingShareOnGenerate) {
                root.pendingShareOnGenerate = false;
                var msg = root.buildInformationMessage();
                whatsAppShare.sharePdf(filePath, msg, detailsForm.phoneNumber);
            }
        }
        function onGenerationFailed(errorMessage) {
            root.pendingShareOnGenerate = false;
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
            if (root.isAndroid) {
                statusBanner.message = "🚀 WhatsApp opened! Sharing PDF with " + phone;
            } else {
                statusBanner.message = "🚀 WhatsApp opened! Message sent to " + phone;
            }
            statusBanner.autoDismiss = false;
        }
        function onShareError(errorMessage) {
            statusBanner.type = "error";
            statusBanner.message = "WhatsApp notice: " + errorMessage;
            statusBanner.autoDismiss = false;
        }
    }

    function buildInformationMessage() {
        var msg = "📋 *Personal Details*\n\n";
        msg += "👤 *Name:* " + detailsForm.name + "\n";
        msg += "📞 *Phone:* " + detailsForm.phoneNumber + "\n";
        if (detailsForm.address.trim().length > 0) {
            msg += "📍 *Address:* " + detailsForm.address.trim() + "\n";
        }
        if (detailsForm.additionalDetails.trim().length > 0) {
            msg += "📝 *Additional Details:* " + detailsForm.additionalDetails.trim() + "\n";
        }
        return msg;
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
                                    detailsForm.address,
                                    detailsForm.additionalDetails
                                );
                            } else {
                                statusBanner.type = "error";
                                statusBanner.message = "Please correct the highlighted fields above.";
                            }
                        }
                    }

                    // 2. Unified WhatsApp Button (Sends PDF on Android, Sends Message on Desktop)
                    CustomButton {
                        id: shareBtn
                        text: root.isAndroid ? "Share PDF via WhatsApp" : "Share via WhatsApp"
                        iconText: root.isAndroid ? "📄💬" : "💬"
                        variant: "whatsapp"
                        enabled: !pdfGenerator.isGenerating && !whatsAppShare.isSharing
                        loading: whatsAppShare.isSharing || (root.pendingShareOnGenerate && pdfGenerator.isGenerating)

                        onClicked: {
                            statusBanner.message = "";
                            if (!detailsForm.validateForm()) {
                                statusBanner.type = "error";
                                statusBanner.message = "Please correct the highlighted fields above.";
                                return;
                            }

                            var msg = root.buildInformationMessage();

                            if (root.isAndroid) {
                                // On Android: Send PDF (auto-generate first if needed)
                                if (root.isPdfReady) {
                                    whatsAppShare.sharePdf(root.currentPdfPath, msg, detailsForm.phoneNumber);
                                } else {
                                    root.pendingShareOnGenerate = true;
                                    pdfGenerator.generatePdf(
                                        detailsForm.name,
                                        detailsForm.phoneNumber,
                                        detailsForm.address,
                                        detailsForm.additionalDetails
                                    );
                                }
                            } else {
                                // On Desktop: Send information message directly to WhatsApp contact
                                whatsAppShare.sendTextMessage(detailsForm.phoneNumber, msg);
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
