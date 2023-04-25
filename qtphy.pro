TEMPLATE = app
TARGET = qtphy

QT += qml quick dbus widgets multimedia


QT_CONFIG -= no-pkg-config
CONFIG += link_pkgconfig debug

DEFINES += GST_USE_UNSTABLE_API

INCLUDEPATH += ./lib
INCLUDEPATH += ./usr/lib

LIBS += $(shell pkg-config opencv --libs)
LIBS += -L/usr/lib -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_shape -lopencv_videoio -lopencv_imgproc

LD_LIBRARY_PATH = /usr/bin

SOURCES += \
    src/main.cpp \
    src/device_info.cpp \
    src/rauc.cpp \
    src/camera_demo.cpp

HEADERS += \
    src/device_info.hpp \
    src/rauc.hpp \
    src/camera_demo.hpp

RESOURCES += \
    resources/resources.qrc

target.path = $$(bindir)
INSTALLS += target
