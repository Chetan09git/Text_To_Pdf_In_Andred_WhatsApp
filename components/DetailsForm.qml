import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ColumnLayout {
    id: root

    property alias name: nameField.text
    property alias phoneNumber: phoneField.fullPhoneNumber
    property alias address: addressField.text
    property alias additionalDetails: detailsField.text

    spacing: 16
    Layout.fillWidth: true

    function validateForm() {
        var isValid = true;

        // Validate Name
        if (nameField.text.trim().length === 0) {
            nameField.hasError = true;
            nameField.errorMessage = "Please enter your full name.";
            isValid = false;
        } else if (nameField.text.trim().length < 2) {
            nameField.hasError = true;
            nameField.errorMessage = "Name must be at least 2 characters long.";
            isValid = false;
        } else {
            nameField.hasError = false;
            nameField.errorMessage = "";
        }

        // Validate Phone Number
        var phoneDigits = phoneField.number.replace(/[^0-9]/g, "");
        if (phoneDigits.length === 0) {
            phoneField.hasError = true;
            phoneField.errorMessage = "Please enter your phone number.";
            isValid = false;
        } else if (phoneDigits.length < 7 || phoneDigits.length > 15) {
            phoneField.hasError = true;
            phoneField.errorMessage = "Please enter a valid phone number (7-15 digits).";
            isValid = false;
        } else {
            phoneField.hasError = false;
            phoneField.errorMessage = "";
        }

        return isValid;
    }

    function resetForm() {
        nameField.text = "";
        nameField.hasError = false;
        nameField.errorMessage = "";

        phoneField.number = "";
        phoneField.hasError = false;
        phoneField.errorMessage = "";

        addressField.text = "";
        addressField.hasError = false;
        addressField.errorMessage = "";

        detailsField.text = "";
        detailsField.hasError = false;
        detailsField.errorMessage = "";
    }

    // 1. Full Name Field
    CustomTextField {
        id: nameField
        label: "Full Name"
        placeholderText: "e.g. Chetan Kumar"
        iconText: "👤"
        required: true
        maximumLength: 80
        onTextChangedByUser: {
            if (hasError && newText.trim().length >= 2) {
                hasError = false;
                errorMessage = "";
            }
        }
    }

    // 2. Phone Number Field
    PhoneNumberField {
        id: phoneField
        label: "Phone Number"
        countryCode: "+91"
        placeholderText: "98765 43210"
        required: true
        onPhoneNumberChanged: {
            if (hasError && number.replace(/[^0-9]/g, "").length >= 7) {
                hasError = false;
                errorMessage = "";
            }
        }
    }

    // 3. Address Field
    CustomTextField {
        id: addressField
        label: "Address"
        placeholderText: "e.g. #123, 4th Main, Bangalore, Karnataka"
        iconText: "📍"
        required: false
        isMultiline: true
        maximumLength: 250
    }

    // 5. Additional Details Field
    CustomTextField {
        id: detailsField
        label: "Additional Details"
        placeholderText: "Enter any notes, special requirements, or comments..."
        iconText: "📝"
        required: false
        isMultiline: true
        maximumLength: 500
    }
}
