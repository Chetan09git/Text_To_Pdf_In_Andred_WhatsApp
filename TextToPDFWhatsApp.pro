QT += quick quickcontrols2 gui

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

SOURCES += \
    main.cpp \
    services/PdfGenerator.cpp \
    services/WhatsAppShare.cpp

HEADERS += \
    services/PdfGenerator.h \
    services/WhatsAppShare.h \
    services/MacNativeShare.h

macx {
    OBJECTIVE_SOURCES += services/MacNativeShare.mm
    LIBS += -framework AppKit -framework Cocoa
}

RESOURCES += qml.qrc

INCLUDEPATH += $$PWD $$PWD/services

# Android specific settings
android {
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
    OTHER_FILES += \
        android/AndroidManifest.xml \
        android/res/xml/file_paths.xml
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
