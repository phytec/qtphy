/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2021 PHYTEC Messtechnik GmbH
 */

import QtQuick 2.0
import QtQuick.Dialogs
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.0
import Phytec.CameraDemo 1.0
import PhyTheme 1.0
import QtMultimedia
import "../controls"

Page {
    header: PhyToolBar {
        title: "Camera Demo"
        buttonBack.onClicked: stack.pop()
        buttonMenu.visible: false
    }
    CameraDemo {
        id: cameraDemo
    }
    // Connect cameraDemo and cameraFrameProvider
    Connections {
        target: cameraDemo
        function onNewImage(image) {
            cameraFrameProvider.updateImage(image)
        }
    }
    // Component.onCompleted: {
    //     cameraDemo.openCamera()
    // }

    RowLayout {
        id: content
        spacing: PhyTheme.marginBig
        anchors.fill: parent

        ColumnLayout {
            Layout.topMargin: PhyTheme.marginBig
            Layout.leftMargin: PhyTheme.marginBig
            Layout.bottomMargin: PhyTheme.marginBig
            Layout.fillWidth: true
            Layout.fillHeight: true

            MessageDialog {
                id: errorDialog1
                visible: (cameraDemo.status == 3) ? true : false

                text: "Unconfigured camera found!"
                informativeText: "Seems like a camera is connected but the wrong devicetree overlays have been selected.\n" +
                "The detectCamera script recommends the following devicetree overlays:\n\n" + cameraDemo.recommendedOverlays + "\n\n" +
                "Do you want to load these overlays and reboot?"
                buttons: MessageDialog.Ok | MessageDialog.Cancel
                onAccepted: cameraDemo.reloadOverlays()
            }

            MessageDialog {
                id: errorDialog2
                visible: (cameraDemo.status == 2) ? true : false

                text: "No Camera Found!"
                informativeText: "No camera found on the CSI interfaces of the board!"
                buttons: MessageDialog.Ok
            }
            Image {
                id: streamImage
                Layout.fillHeight: true
                Layout.fillWidth: true
                fillMode: Image.PreserveAspectFit

                property bool counter: false
                source: "image://myCam/image"
                asynchronous: false
                cache: false

                function reloadImage() {
                    counter = !counter
                    source = "image://myCam/image?id=" + counter
                }
            }
            Connections {
                target: cameraFrameProvider

                function onImageChanged() {
                    streamImage.reloadImage()
                }
            }
        }
        ColumnLayout {
            Layout.topMargin: PhyTheme.marginBig
            Layout.rightMargin: PhyTheme.marginBig
            Layout.bottomMargin: PhyTheme.marginBig
            Layout.fillWidth: true
            Layout.fillHeight: true
            // Open Camera Button
            Button {
                id: startButton
                Layout.fillHeight: true
                text: "Open Camera"
                onClicked: {
                    cameraDemo.openCamera()
                }
                Layout.alignment: Qt.AlignVCenter
            }
            RowLayout {
                Layout.fillHeight: true

                Label {
                    text: "ISP"
                    Layout.alignment: Qt.AlignVCenter
                }
                Switch {
                    id: videoSourceSwitch
                    checked: cameraDemo.videoSrc

                    onCheckedChanged: {
                        cameraDemo.setVideoSource(checked)
                    }
                    Layout.alignment: Qt.AlignVCenter
                }
                Label {
                    text: "ISI"
                    Layout.alignment: Qt.AlignVCenter
                    Layout.rightMargin: 10
                }

                Label {
                    text: "CSI1"
                    Layout.alignment: Qt.AlignVCenter
                }
                Switch {
                    id: interfaceSwitch
                    checked: (cameraDemo.interface == 2) ? 1 : 0// TBD enum
                    enabled: (cameraDemo.status == 1) ? 1 : 0 // TBD enum

                    onCheckedChanged: {
                        cameraDemo.setInterface(checked)
                    }
                    Layout.alignment: Qt.AlignVCenter
                }
                Label {
                    text: "CSI2"
                    Layout.alignment: Qt.AlignVCenter
                }
            }
            // Camera and Sensor Name
            Row {
                Layout.fillHeight: true

                Label {
                    text: "Connected Camera: "
                }
                Label {
                    id: cameraNameLabel
                    text: cameraDemo.cameraName
                }
            }
            // Interface
            Row {
                Layout.fillHeight: true

                Label {
                    text: "Interface: "
                }
                Label {
                    id: interfaceLabel
                    text: cameraDemo.interfaceString
                }
            }
            // Resolution
            Row {
                Layout.fillHeight: true

                Label {
                    text: "Resolution: "
                }
                Label {
                    id: resolutionLabel
                    text: cameraDemo.framesize
                }
            }
            // Sensor Controls
            Label {
                Layout.topMargin: PhyTheme.marginSmall
                Layout.fillHeight: true
                text: "Sensor Controls: "
            }
            // Horizontal Flip
            CheckBox {
                Layout.fillHeight: true

                id: flipHorizontalCheckbox
                text: "Flip Horizontal"
                checked: cameraDemo.flipHorizontal
                onClicked: {
                    cameraDemo.setFlipHorizontal(flipHorizontalCheckbox.checked)
                }
            }
            // Vertical Flip
            CheckBox {
                id: flipVerticalCheckbox
                Layout.fillHeight: true
                text: "Flip Vertical"
                checked: cameraDemo.flipVertical
                onClicked: {
                    cameraDemo.setFlipVertical(flipVerticalCheckbox.checked)
                }
            }
            // Auto Exposure
            CheckBox {
                id: autoExposureCheckbox
                Layout.fillHeight: true
                text: "Auto Exposure"
                enabled: (cameraDemo.videoSrc && cameraDemo.hasAutoExposure) // Enable only on ISI
                checked: (cameraDemo.videoSrc && cameraDemo.hasAutoExposure && cameraDemo.autoExposure); // Can only be checked on ISI
                onClicked: {
                    cameraDemo.setAutoExposure(autoExposureCheckbox.checked)
                }
            }
            // Exposure Slider
            Label {
                Layout.fillHeight: true
                text: "Exposure"
            }
            Slider {
                id: exposureSlider
                Layout.fillHeight: true
                // disable when isp or sensor Auto Exposure is enabled
                enabled: !(cameraDemo.autoExposure || aecCheckbox.checked) // TBD: .checked statement
                from: 0
                value: cameraDemo.exposure
                to: 3000
                // to: 65535 // TBD: this is the original maximum but it is way too high
                // maybe orient on auto_exposure_max
                // step = 1
                onMoved: {
                    cameraDemo.setExposure(exposureSlider.value)
                }
            }
            // ISP Controls
            Label {
                Layout.topMargin: PhyTheme.marginSmall
                Layout.fillHeight: true
                text: "ISP Controls: "
            }
            // Auto Exposure (ISP)
            CheckBox {
                id: aecCheckbox
                Layout.fillHeight: true
                text: "ISP Auto Exposure"
                enabled: !cameraDemo.videoSrc
                checked: !cameraDemo.videoSrc
                onClicked: {
                    cameraDemo.setAec(aecCheckbox.checked)
                }
            }

            // Auto White Balance
            CheckBox {
                id: awbCheckbox
                Layout.fillHeight: true
                text: "Auto White Balance"
                enabled: !cameraDemo.videoSrc
                checked: !cameraDemo.videoSrc
                onClicked: {
                    cameraDemo.setAwb(awbCheckbox.checked)
                }
            }
            // Lens Shading Correction
            CheckBox {
                id: lscCheckbox
                Layout.fillHeight: true
                text: "Lens Shading Correction"
                enabled: !cameraDemo.videoSrc
                checked: !cameraDemo.videoSrc
                onClicked: {
                    cameraDemo.setLsc(lscCheckbox.checked)
                }
            }
            Item {
                Layout.fillHeight: true
            }
        }
    }
}
