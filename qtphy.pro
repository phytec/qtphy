TEMPLATE = app
TARGET = qtphy

QT += qml quick dbus

PKGCONFIG = \
    gstreamer-1.0

SOURCES += \
    src/main.cpp \
    src/device_info.cpp \
    src/multimedia_qmlsink.cpp \
    src/rauc.cpp

HEADERS += \
    src/device_info.hpp \
    src/multimedia_qmlsink.hpp \
    src/rauc.hpp

RESOURCES += \
    resources/resources.qrc

target.path = $$(bindir)
INSTALLS += target
