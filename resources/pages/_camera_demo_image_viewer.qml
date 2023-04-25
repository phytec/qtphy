/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2021 PHYTEC Messtechnik GmbH
 */

import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0
import PhyTheme 1.0
import QtMultimedia
import "../controls"

Page {
    header: PhyToolBar {
        title: "Camera Demo"
        buttonBack.onClicked: stack.pop()
        buttonMenu.visible: false
    }

    RowLayout {
        id: content
        spacing: PhyTheme.marginBig
        
        // MediaPlayer {
        //     id: player
        //     // source: "file:////home/root/sampleVideo.mp4"
        //     source: "gst-pipeline: v4l2src device=/dev/video0 ! video/x-raw,format=GRAY8,depth=8,width=1280,height=800 ! videoconvert n-threads=4 ! queue ! autovideosink sync=false"
        //     videoOutput: videoOutput
        // }

        // MediaDevices {
        //     id: mediaDevices
        // }
        // CaptureSession {
        //     camera: Camera {
        //         cameraDevice: mediaDevices.defaultVideoInput
        //     }
        //     videoOutput: videoOutput
        // }

        // VideoOutput {
        //     id: videoOutput
        //     // anchors.fill: parent
        // }

        Item {
        anchors.fill: parent

        GstGLQt6VideoItem {
            id: video
            objectName: "videoItem"
            anchors.centerIn: parent
            width: parent.width
            height: parent.height
        }

        Rectangle {
            color: Qt.rgba(1, 1, 1, 0.7)
            border.width: 1
            border.color: "white"
            anchors.bottom: video.bottom
            anchors.bottomMargin: 15
            anchors.horizontalCenter: parent.horizontalCenter
            width : parent.width - 30
            height: parent.height - 30
            radius: 8

            MouseArea {
                id: mousearea
                anchors.fill: parent
                hoverEnabled: true
                onEntered: {
                    parent.opacity = 1.0
                    hidetimer.start()
                }
            }

            Timer {
                id: hidetimer
                interval: 5000
                onTriggered: {
                    parent.opacity = 0.0
                    stop()
                }
            }
        }
    }


        ColumnLayout {
            Layout.topMargin: PhyTheme.marginWBig
            Layout.leftMargin: PhyTheme.marginBig
            Layout.bottomMargin: PhyTheme.marginBig
            Layout.fillWidth: true

            TextField {
                Layout.fillWidth: true
                placeholderText: "Regular text fid"
            }
            TextField {
                Layout.fillWidth: true
                placeholderText: "Regular text field"
            }
        }
    }

}
