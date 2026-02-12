TEMPLATE = app
TARGET = qtphy

QT += qml quick dbus

CONFIG += link_pkgconfig
PKGCONFIG += gstreamer-1.0 gstreamer-pbutils-1.0

SOURCES += \
    src/main.cpp \
    src/device_info.cpp \
    src/rauc.cpp \
    src/multimedia_formats.cpp

HEADERS += \
    src/device_info.hpp \
    src/rauc.hpp \
    src/multimedia_formats.hpp

qmlsink {
    SOURCES += src/multimedia_qmlsink.cpp
    HEADERS += src/multimedia_qmlsink.hpp
}

RESOURCES += \
    resources/resources.qrc

target.path = $$(bindir)
INSTALLS += target
