QT += quick quickcontrols2 gui

CONFIG += c++17

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
    QMAKE_LIBS_OPENGL = -framework OpenGL
    CONFIG += sdk_no_version_check
}

RESOURCES += qml.qrc

INCLUDEPATH += $$PWD $$PWD/services

android {
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
    OTHER_FILES += \
        android/AndroidManifest.xml \
        android/res/xml/file_paths.xml
}
