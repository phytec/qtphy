/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2021 PHYTEC Messtechnik GmbH
 */

import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0
import PhyTheme 1.0
import QtMultimedia
import Phytec.CameraDemo 1.0
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

            Image {
                id: streamImage
                // anchors.fill: parent // TBD: change
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
            Row {
                Button {
                    id: startButton
                    text: "(Re)-Open"
                    onClicked: {
                        camDemoMain.openCamera()
                    }
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
                    text: "TBD !!!"
                }
            }

            // Lens
            Label {
                text: "Lens: TBD"
                enabled: false
            }
            ComboBox {
                id: lensComboBox
                model: ["DEFAULT", "Lens 1", "Lens 2", "Lens 3"]
                enabled: false
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
                // checkState: cameraDemo.autoExposure
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
                enabled: !autoExposureCheckbox.checked
                from: 0
                value: camDemoMain.exposure
                to: 1000
                // to: 65535 // TBD: this is the original maximum but it is way too high
                // maybe orient on auto_exposure_max
                // step = 1
                onMoved: {
                    camDemoMain.setExposure(exposureSlider.value)
                }
            }

            // Dewarping (TBD)
            CheckBox {
                id: dewarpingCheckbox
                text: "Dewarping"
                enabled: false
            }

            // Auto Gain (TBD)
            CheckBox {
                id: autoGainCheckbox
                text: "Auto Gain (analog)"
                enabled: false
            }

            // Black Level Correction (TBD)
            CheckBox {
                id: blackLevelCorrectionCheckbox
                text: "Black Level Correction"
                enabled: false
            }
        }
    }
}
