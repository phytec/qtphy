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
        
        // MediaPlayer {
        //     id: player
        //     // source: "file:////home/root/sampleVideo.mp4"
        //     source: "gst-pipeline: v4l2src device=/dev/video0 ! video/x-raw,format=GRAY8,depth=8,width=1280,height=800 ! videoconvert n-threads=4 ! queue ! autovideosink sync=false"
        //     videoOutput: videoOutput
        // }



        MediaDevices {
            id: mediaDevices
        }
        
        CaptureSession {
            camera: Camera {
                cameraDevice: mediaDevices.videoInputs[0]
                // cameraDevice: mediaDevices.defaultVideoInput
            }
            videoOutput: videoOutput
        }

        VideoOutput {
            id: videoOutput
            // anchors.fill: parent
        }

        ColumnLayout {
            Layout.topMargin: PhyTheme.marginWBig
            Layout.leftMargin: PhyTheme.marginBig
            Layout.bottomMargin: PhyTheme.marginBig
            Layout.fillWidth: true

            TextField {
                Layout.fillWidth: true
                placeholderText: mediaDevices.videoInputs[0].description
            }
            TextField {
                Layout.fillWidth: true
                placeholderText: "Hello"
            }
        }
    }

}
