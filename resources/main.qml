/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2021 PHYTEC Messtechnik GmbH
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import Qt5Compat.GraphicalEffects
import PhyTheme
import PhyControls

ApplicationWindow {
    visible: true
    //visibility: Window.FullScreen
    width: 1280
    height: 800

    property int itemAngle: 55
    property int itemSize: 0.4 * width

    FontLoader {
        source: "qrc:///fonts/phosphor.woff2"
    }

    ListModel {
        id: pageModel

        ListElement {
            icon: "\uf1dd"
            name: "Image Viewer"
            description: "View images in a gallery"
            page: "qrc:///pages/image_viewer.qml"
        }
        ListElement {
            icon: "\uf172"
            name: "Multimedia"
            description: "Play movies with hardware acceleration"
            page: "qrc:///pages/multimedia.qml"
        }
        ListElement {
            icon: "\uf14b"
            name: "RAUC – Update Client"
            description: "View the RAUC status and update your device with new software"
            page: "qrc:///pages/rauc.qml"
        }
        ListElement {
            icon: "\uf1be"
            name: "Multitouch"
            description: "Move multiple objects at once by touching them with your fingers"
            page: "qrc:///pages/multitouch.qml"
        }
        ListElement {
            icon: "\uf119"
            name: "Device Information"
            description: "Get information about the device's hardware and software components"
            page: "qrc:///pages/device_info.qml"
        }
        ListElement {
            icon: "\uf19a"
            name: "Widget Factory"
            description: "Try out different widgets and controls of Qt"
            page: "qrc:///pages/widget_factory.qml"
        }
        ListElement {
            icon: "\uf1e0"
            name: "About PHYTEC"
            description: "General information about PHYTEC"
            page: "qrc:///pages/about.qml"
        }
    }

    StackView {
        id: stack
        initialItem: mainView
        anchors.fill: parent

        popEnter: Transition {
            YAnimator {
                from: 0
                to: 0
                duration: 250
                easing.type: Easing.OutCubic
            }
        }
        popExit: Transition {
            YAnimator {
                from: 0
                to: stack.height
                duration: 250
                easing.type: Easing.OutCubic
            }
        }
        pushEnter: Transition {
            YAnimator {
                from: stack.height
                to: 0
                duration: 250
                easing.type: Easing.OutCubic
            }
        }
        pushExit: Transition {
            YAnimator {
                from: 0
                to: 0
                duration: 250
                easing.type: Easing.OutCubic
            }
        }
    }

    Image {
        id: mainView
        source: "qrc:///images/background.jpg"
        fillMode: Image.PreserveAspectCrop
        asynchronous: true
        z: 1

        Rectangle  {
            color: PhyTheme.background
            opacity: 0.96
            z: 2
            anchors.fill: parent
        }
        ColumnLayout {
            anchors.fill: parent
            spacing: 0
            z: 3

            Rectangle {
                height: 42
                color: "black"
                Layout.fillWidth: true

                RowLayout {
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16
                    anchors.topMargin: 2
                    anchors.bottomMargin: 2
                    anchors.fill: parent
                    spacing: 6

                    PhyDateTime {
                        color: PhyTheme.white
                        font.pointSize: 18
                        Layout.margins: 0
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                    Label {
                        text: PhyTheme.iconFont.wifiSlash
                        color: PhyTheme.white
                        font.pointSize: 20
                    }
                    Label {
                        text: PhyTheme.iconFont.speakerX
                        color: PhyTheme.white
                        font.pointSize: 20
                    }
                    Label {
                        text: PhyTheme.iconFont.batteryMedium
                        color: PhyTheme.white
                        font.pointSize: 20
                    }
                }
            }

            Component {
                id: pageDelegate

                ColumnLayout {
                    spacing: 0
                    width: 0.45 * listView.width
                    height: listView.height - 2 * PhyTheme.marginBig

                    Rectangle {
                        width: PhyTheme.radiusBig * 4
                        height: width
                        radius: width / 2
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: PhyTheme.marginBig
                        color: PhyTheme.baseSelected

                        Label {
                            text: icon
                            color: PhyTheme.labelHighlight
                            font.pointSize: 48
                            anchors.centerIn: parent
                        }
                    }
                    Rectangle {
                        color: PhyTheme.white
                        radius: PhyTheme.radiusRegular
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.margins: PhyTheme.marginBig
                        layer.enabled: true
                        layer.effect: DropShadow {
                            transparentBorder: true
                            horizontalOffset: 0
                            verticalOffset: 8
                            radius: 32
                            samples: radius * 2 + 1
                            color: "#60000000"
                        }

                        RowLayout {
                            spacing: 0
                            anchors.fill: parent

                            ColumnLayout {
                                spacing: PhyTheme.marginBig
                                Layout.margins: PhyTheme.marginBig

                                Label {
                                    text: name
                                    elide: Text.ElideRight
                                    color: PhyTheme.labelHighlight
                                    font.pointSize: 24
                                    font.weight: Font.DemiBold
                                    Layout.alignment: Qt.AlignVCenter
                                    Layout.fillWidth: true
                                }
                                Label {
                                    text: description
                                    elide: Text.ElideRight
                                    wrapMode: Text.WordWrap
                                    color: PhyTheme.label
                                    font.pointSize: 20
                                    Layout.fillHeight: true
                                    Layout.fillWidth: true
                                }
                            }
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                console.log(page)
                                pageLoader.source = page
                                stack.push(pageLoader)
                            }
                        }
                    }
                }
            }

            ListView {
                id: listView
                model: pageModel
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.leftMargin: 4 * PhyTheme.marginBig
                displayMarginBeginning: width * 2
                displayMarginEnd: width * 2
                delegate: pageDelegate
                snapMode: ListView.SnapToItem
                orientation: ListView.Horizontal
            }
        }
    }

    Loader {
        id: pageLoader
        visible: false
    }
}
