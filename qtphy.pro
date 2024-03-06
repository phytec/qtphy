TEMPLATE = app
TARGET = qtphy

QT += qml quick dbus widgets multimedia

INCLUDEPATH += /home/hahn/imx8/imx8mp_PD23.1.0_1/build/tmp/work/cortexa53-crypto-phytec-linux/qtphy/git-r0/recipe-sysroot/usr/include/gstreamer-1.0
INCLUDEPATH += /home/hahn/imx8/imx8mp_PD23.1.0_1/build/tmp/work/cortexa53-crypto-phytec-linux/qtphy/git-r0/recipe-sysroot/usr/include/glib-2.0
INCLUDEPATH += /home/hahn/imx8/imx8mp_PD23.1.0_1/build/tmp/work/cortexa53-crypto-phytec-linux/qtphy/git-r0/recipe-sysroot/usr/lib/glib-2.0/include

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
