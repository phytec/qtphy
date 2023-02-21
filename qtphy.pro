TEMPLATE = app
TARGET = qtphy

QT += qml quick dbus

SOURCES += \
    src/main.cpp \
    src/device_info.cpp \
    src/rauc.cpp

HEADERS += \
    src/device_info.hpp \
    src/rauc.hpp

RESOURCES += \
    resources/resources.qrc

target.path = $$(bindir)
INSTALLS += target
