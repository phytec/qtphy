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
import Phytec.CameraDemo.Enums 1.0

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
                visible: (cameraDemo.status == EnumNamespace.WRONG_OVERLAYS) ? true : false

                text: "Unconfigured camera found!"
                informativeText: "Seems like a camera is connected but the wrong devicetree overlays have been selected.\n" +
                "The detectCamera script recommends the following devicetree overlays:\n\n" + cameraDemo.recommendedOverlays + "\n\n" +
                "Do you want to load these overlays and reboot?"
                buttons: MessageDialog.Ok | MessageDialog.Cancel
                onAccepted: cameraDemo.reloadOverlays()
            }
            MessageDialog {
                id: errorDialog2
                visible: (cameraDemo.status == EnumNamespace.NO_CAM) ? true : false

                text: "No Camera Found!"
                informativeText: "No camera found on the CSI interfaces of the board!"
                buttons: MessageDialog.Ok
            }
            MessageDialog {
                id: errorDialog3
                visible: (cameraDemo.status == EnumNamespace.ISP_UNAVAILABLE) ? true : false

                text: "ISP overlay not loaded!"
                informativeText: "Your hardware has an ISP but you did not load the ISP overlay for all connected cameras.\n" +
                "The detectCamera script recommends the following devicetree overlays:\n\n" + cameraDemo.recommendedOverlays + "\n\n" +
                "Do you want to load these overlays and reboot?"
                buttons: MessageDialog.Ok | MessageDialog.Cancel
                onAccepted: cameraDemo.reloadOverlays()
            }
            MessageDialog {
                id: errorDialog4
                visible: (cameraDemo.status == EnumNamespace.ISI_UNAVAILABLE) ? true : false

                text: "ISI overlay not loaded!"
                informativeText: "Your hardware has an ISI but you did not load the ISI overlay for all connected cameras.\n" +
                "The detectCamera script recommends the following devicetree overlays:\n\n" + cameraDemo.recommendedOverlays + "\n\n" +
                "Do you want to load these overlays and reboot?"
                buttons: MessageDialog.Ok | MessageDialog.Cancel
                onAccepted: cameraDemo.reloadOverlays()
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
                    visible: (cameraDemo.hostHardware.hasISP && cameraDemo.hostHardware.hasISI)
                    enabled: (cameraDemo.ispAvailable)
                    text: "ISP"
                    Layout.alignment: Qt.AlignVCenter
                }
                Switch {
                    id: videoSourceSwitch
                    visible: (cameraDemo.hostHardware.hasISP && cameraDemo.hostHardware.hasISI)
                    enabled:(cameraDemo.isiAvailable && cameraDemo.ispAvailable)
                    checked: cameraDemo.videoSrc
                    onCheckedChanged: {
                        cameraDemo.setVideoSource(checked)
                    }
                    Layout.alignment: Qt.AlignVCenter
                }
                Label {
                    visible: (cameraDemo.hostHardware.hasISP && cameraDemo.hostHardware.hasISI)
                    enabled: (cameraDemo.isiAvailable)
                    text: "ISI"
                    Layout.alignment: Qt.AlignVCenter
                    Layout.rightMargin: 10
                }

                Label {
                    visible: (cameraDemo.hostHardware.hasDualCam)
                    text: "CSI1"
                    Layout.alignment: Qt.AlignVCenter
                }
                Switch {
                    id: interfaceSwitch
                    visible: (cameraDemo.hostHardware.hasDualCam)
                    enabled: (cameraDemo.hostHardware.dualCamAvailable)
                    checked: (cameraDemo.interface == 2) ? 1 : 0

                    onCheckedChanged: {
                        cameraDemo.setInterface(checked)
                    }
                    Layout.alignment: Qt.AlignVCenter
                }
                Label {
                    visible: (cameraDemo.hostHardware.hasDualCam)
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
                checked: (cameraDemo.autoExposure);
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
                to: 10000 // limit exposure time to 10ms
                // step = 1
                onMoved: {
                    cameraDemo.setExposure(exposureSlider.value)
                }
            }
            // ISP Controls
            Label {
                visible: cameraDemo.hostHardware.hasISP
                Layout.topMargin: PhyTheme.marginSmall
                Layout.fillHeight: true
                text: "ISP Controls: "
            }
            // Auto Exposure (ISP)
            CheckBox {
                id: aecCheckbox
                visible: cameraDemo.hostHardware.hasISP
                enabled: (!cameraDemo.videoSrc && cameraDemo.ispAvailable)
                Layout.fillHeight: true
                text: "ISP Auto Exposure"
                checked: !cameraDemo.videoSrc
                onClicked: {
                    cameraDemo.setAec(aecCheckbox.checked)
                }
            }

            // Auto White Balance
            CheckBox {
                id: awbCheckbox
                visible: cameraDemo.hostHardware.hasISP
                enabled: (!cameraDemo.videoSrc && cameraDemo.ispAvailable)
                Layout.fillHeight: true
                text: "Auto White Balance"
                checked: !cameraDemo.videoSrc
                onClicked: {
                    cameraDemo.setAwb(awbCheckbox.checked)
                }
            }
            // Lens Shading Correction
            CheckBox {
                id: lscCheckbox
                visible: cameraDemo.hostHardware.hasISP
                enabled: (!cameraDemo.videoSrc && cameraDemo.ispAvailable)
                Layout.fillHeight: true
                text: "Lens Shading Correction"
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
