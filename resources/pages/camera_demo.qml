/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2021 PHYTEC Messtechnik GmbH
 */

import QtQuick 2.0
import QtQuick.Dialogs
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.0
import PhyTheme 1.0
import QtMultimedia
// import Phytec.CameraDemo 1.0
import "../controls"

// import org.freedesktop.gstreamer.Qt6GLVideoItem 1.0
// import org.freedesktop.gstreamer.Qt6GLVideoItem

Page {
    header: PhyToolBar {
        title: "Camera Demo"
        buttonBack.onClicked: stack.pop()
        buttonMenu.visible: false
    }

    RowLayout {
        id: content
        spacing: PhyTheme.marginBig
        anchors.fill: parent

        // anchors.centerIn: parent

        ColumnLayout {
            Layout.topMargin: PhyTheme.marginBig
            Layout.leftMargin: PhyTheme.marginBig
            Layout.bottomMargin: PhyTheme.marginBig
            Layout.fillHeight: true
            Layout.fillWidth: false
            Layout.preferredWidth: parent.width * 0.7

            MessageDialog {
                id: errorDialog1
                visible: (camDemoMain.errorDialog == 0) ? true : false

                text: "Cannot open Camera!"
                informativeText: "Seems like a camera is connected but the wrong devicetree overlays have been selected.\n" +
                "The detectCamera script recommends the following devicetree overlays:\n\n" + camDemoMain.recommendedOverlays + "\n\n" +
                "Do you want to load these overlays and reboot?"
                buttons: MessageDialog.Ok | MessageDialog.Cancel
                onAccepted: camDemoMain.reloadOverlays()
            }

            MessageDialog {
                id: errorDialog3
                visible: (camDemoMain.errorDialog == 1) ? true : false

                text: "No Camera Found!"
                informativeText: "No camera found on the CSI interfaces of the board!"
                buttons: MessageDialog.Ok
                // onAccepted: 
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
            Layout.fillHeight: true
            Layout.fillWidth: false
            Layout.preferredWidth: parent.width * 0.25
            // spacing: PhyTheme.marginBig

            // Open Camera Button
            RowLayout {
                Button {
                    id: startButton
                    text: "(Re)-Open"
                    onClicked: {
                        camDemoMain.openCamera()
                        // notFoundDialog1.open()
                    }
                    Layout.rightMargin: 10
                    Layout.alignment: Qt.AlignVCenter
                }
                Label {
                    text: "ISP"
                    Layout.alignment: Qt.AlignVCenter
                }
                Switch {
                    id: videoSourceSwitch
                    checked: false

                    onCheckedChanged: {
                        camDemoMain.setVideoSource(checked)                        
                    }
                    Layout.alignment: Qt.AlignVCenter
                }
                Label {
                    text: "ISI"
                    Layout.alignment: Qt.AlignVCenter
                }
            }

            // Camera and Sensor Name
            Row {
                Label {
                    text: "Connected Camera: "
                }
                Label {
                    id: cameraNameLabel
                    text: camDemoMain.cameraName
                }
            }

            // Interface
            Row {
                Label {
                    text: "Interface: "
                }
                Label {
                    id: interfaceLabel
                    text: camDemoMain.interface
                }
            }

            // Resolution
            Row {
                Label {
                    text: "Resolution: "
                }
                Label {
                    id: resolutionLabel
                    text: camDemoMain.framesize
                }
            }

            // Color Format
            Row {
                Label {
                    text: "Format: "
                }
                Label {
                    id: formatLabel
                    text: camDemoMain.format
                }
            }

            // Video SRC (ISP / ISI)
            Row {
                Label {
                    text: "Video Source: "
                }
                Label {
                    id: videoSrcLabel
                    text: camDemoMain.videoSrc
                }
            }

            // Horizontal Flip
            CheckBox {
                id: flipHorizontalCheckbox
                text: "Flip Horizontal"
                checked: camDemoMain.flipHorizontal
                onClicked: {
                    camDemoMain.setFlipHorizontal(flipHorizontalCheckbox.checked)
                }
            }

            // Vertical Flip
            CheckBox {
                id: flipVerticalCheckbox
                text: "Flip Vertical"
                checked: camDemoMain.flipVertical
                onClicked: {
                    camDemoMain.setFlipVertical(flipVerticalCheckbox.checked)
                }
            }

            // Auto Exposure
            CheckBox {
                id: autoExposureCheckbox
                text: "Auto Exposure"
                // TBD: separate cameraname and sensor
                enabled: (camDemoMain.videoSrc == "ISP" || camDemoMain.cameraName == "VM017 (ar0521)") ? 0 : 1; // disable if ISP is used or vm017 is connected
                checked: camDemoMain.autoExposure
                onClicked: {
                    camDemoMain.setAutoExposure(autoExposureCheckbox.checked)
                }
            }

            // Exposure Slider
            Label {
                text: "Exposure"
            }
            Slider {
                id: exposureSlider
                enabled: !(autoExposureCheckbox.checked || aecCheckbox.checked)
                from: 0
                value: camDemoMain.exposure
                to: 1500
                // to: 65535 // TBD: this is the original maximum but it is way too high
                // maybe orient on auto_exposure_max
                // step = 1
                onMoved: {
                    camDemoMain.setExposure(exposureSlider.value)
                }
            }

            // // Lens
            // Label {
            //     text: "Lens:"
            //     enabled: (camDemoMain.videoSrc=="ISP")  ? true : false
            // }
            // ComboBox {
            //     id: lensComboBox
            //     model: ["DEFAULT", "Lens 1", "Lens 2", "Lens 3"]
            //     enabled: (camDemoMain.videoSrc=="ISP")  ? true : false
            // }

            // // Dewarping (TBD)
            // CheckBox {
            //     id: dweCheckbox
            //     text: "Dewarping"
            //     enabled: (camDemoMain.videoSrc=="ISP")  ? true : false
            //     checked: camDemoMain.dwe
            //     onClicked: {
            //         camDemoMain.setDwe(dweCheckbox.checked)
            //     }
            // }

            Label {
                text: "ISP Controls: "
            }

            // Auto Exposure (ISP)
            CheckBox {
                id: aecCheckbox
                text: "ISP Auto Exposure"
                enabled: (camDemoMain.videoSrc=="ISP")  ? true : false
                checked: (camDemoMain.videoSrc == "ISP") ? true : false
                onClicked: {
                    camDemoMain.setAec(aecCheckbox.checked)
                }
            }

            // Auto White Balance (TBD)
            CheckBox {
                id: awbCheckbox
                text: "Auto White Balance"
                enabled: (camDemoMain.videoSrc=="ISP")  ? true : false
                checked: (camDemoMain.videoSrc == "ISP") ? true : false
                onClicked: {
                    camDemoMain.setAwb(awbCheckbox.checked)
                }
            }

            // Lens Shading Correction (TBD)
            CheckBox {
                id: lscCheckbox
                text: "Lens Shading Correction"
                enabled: (camDemoMain.videoSrc=="ISP")  ? true : false
                checked: (camDemoMain.videoSrc == "ISP") ? true : false
                onClicked: {
                    camDemoMain.setLsc(lscCheckbox.checked)
                }
            }
            

            // // Auto Gain (TBD)
            // CheckBox {
            //     id: autoGainCheckbox
            //     text: "Auto Gain (analog)"
            //     enabled: (camDemoMain.videoSrc=="ISP")  ? true : false
            // }

            // // Black Level Correction (TBD)
            // CheckBox {
            //     id: blackLevelCorrectionCheckbox
            //     text: "Black Level Correction"
            //     enabled: (camDemoMain.videoSrc=="ISP")  ? true : false
            // }
        }
    }
}
