/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2021 PHYTEC Messtechnik GmbH
 */

import QtQuick 2.6
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0
import QtQuick.Window 2.2
import PhyTheme 1.0

ApplicationWindow {
    visible: true
    //visibility: Window.FullScreen
    width: 1280
    height: 800

    property int itemAngle: 55
    property int itemSize: 0.4 * width

    function showPage() {
        var page = pageModel.get(pathView.currentIndex)

        if (page.page) {
            pageLoader.source = page.page
            stack.push(pageLoader)
        }
    }

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

    Rectangle {
        id: mainView

        Component {
            id: pageDelegate

            Rectangle {
                id: itemRectangle
                color: PhyTheme.black
                z: PathView.onPath ? PathView.z : 0
                opacity: PathView.onPath ? PathView.pageOpacity : 0
                width: 0.3 * parent.width
                height: Math.min(0.36 * parent.width, 0.9 * parent.height)

                RowLayout {
                    spacing: 0
                    anchors.fill: parent

                    ColumnLayout {
                        spacing: PhyTheme.marginBig
                        Layout.margins: PhyTheme.marginBig

                        Label {
                            text: icon
                            color: PhyTheme.teal1
                            font.pointSize: Math.max(0.05 * parent.width, 48)
                            Layout.alignment: Qt.AlignHCenter
                        }
                        Label {
                            text: name
                            elide: Text.ElideRight
                            color: PhyTheme.white
                            font.pointSize: Math.max(0.015 * parent.width, 24)
                            Layout.alignment: Qt.AlignHCenter
                            Layout.fillWidth: true
                        }
                        Label {
                            text: description
                            elide: Text.ElideRight
                            wrapMode: Text.WordWrap
                            color: PhyTheme.gray1
                            font.weight: Font.Light
                            font.pointSize: Math.max(0.01 * parent.width, 20)
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                        }
                    }
                }
            }
        }

        PathView {
            id: pathView
            anchors.fill: parent
            model: pageModel
            delegate: pageDelegate
            pathItemCount: 6 // must be even for proper item placement
            snapMode: PathView.SnapToItem
            preferredHighlightBegin: 0.5
            preferredHighlightEnd: 0.5

            Image {
                anchors.fill: parent
                source: "qrc:///images/background.jpg"
                fillMode: Image.PreserveAspectCrop
                asynchronous: true
            }

            MouseArea {
                id: mouseAreaShowPage
                x: 0.35 * parent.width
                y: (parent.height - Math.min(0.36 * parent.width, 0.9 * parent.height)) / 2
                width: 0.3 * parent.width
                height: Math.min(0.36 * parent.width, 0.9 * parent.height)
                onClicked: if (!parent.moving) showPage()
            }
            MouseArea {
                x: 0.0325 * parent.width
                y: mouseAreaShowPage.y
                width: mouseAreaShowPage.width
                height: mouseAreaShowPage.height
                onClicked: pathView.currentIndex = pathView.currentIndex - 1
            }
            MouseArea {
                x: 0.6675 * parent.width
                y: mouseAreaShowPage.y
                width: mouseAreaShowPage.width
                height: mouseAreaShowPage.height
                onClicked: pathView.currentIndex = pathView.currentIndex + 1
            }

            path: Path {
                startX: 0
                startY: 0.5 * height

                PathAttribute { name: "pageOpacity"; value: 0 }
                PathAttribute { name: "z"; value: 0 }

                PathLine { x: 0.1 * width; y: 0.5 * height }
                PathPercent { value: 0.29 }
                PathAttribute { name: "pageOpacity"; value: 0.8 }
                PathAttribute { name: "z"; value: 10 }

                PathLine { x: 0.5 * width; y: 0.5 * height }
                PathPercent { value: 0.5 }
                PathAttribute { name: "pageOpacity"; value: 1 }
                PathAttribute { name: "z"; value: 20 }

                PathLine { x: 0.9 * width; y: 0.5 * height }
                PathPercent { value: 0.71 }
                PathAttribute { name: "pageOpacity"; value: 0.8 }
                PathAttribute { name: "z"; value: 10 }

                PathLine { x: width; y: 0.5 * height }
                PathAttribute { name: "pageOpacity"; value: 0 }
                PathAttribute { name: "z"; value: 0 }
            }
        }
    }

    Loader {
        id: pageLoader
        visible: false
    }
}
