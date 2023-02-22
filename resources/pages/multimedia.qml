/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2021 PHYTEC Messtechnik GmbH
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import PhyTheme
import "../controls"

Page {
    header: PhyToolBar {
        id: header
        title: "Multimedia"
        buttonBack.onClicked: {
            stack.pop()
            video.pause()
        }
        buttonMenu {
            text: PhyTheme.iconFont.folderOpen + " Open"
            onClicked: fileDialog.visible = true
            visible: true
        }
    }
    background: Rectangle {
        color: PhyTheme.black
    }
    footer: ToolBar {
        ColumnLayout {
            anchors.fill: parent

            RowLayout {
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter

                ToolButton {
                    text: PhyTheme.iconFont.play
                    onClicked: video.play()
                }
                ToolButton {
                    text: PhyTheme.iconFont.pause
                    onClicked: video.pause()
                }
                ToolButton {
                    text: PhyTheme.iconFont.skipBack
                    onClicked: video.seek(video.position - 5000)
                }
                ToolButton {
                    text: PhyTheme.iconFont.skipForward
                    onClicked: video.seek(video.position + 5000)
                }
                Slider {
                    Layout.fillWidth: true
                    from: 0
                    to: video.duration
                    value: video.position
                    onMoved: video.seek(value)
                }
            }
        }
    }

    Video {
        id: video
        anchors.fill: parent
        source: fileDialog.selectedFile

        MouseArea {
            anchors.fill: parent
            onClicked: video.playbackState != MediaPlayer.PlayingState ?
                       video.play() : video.pause()
        }
        Label {
            text: PhyTheme.iconFont.pause
            anchors.centerIn: parent
            color: PhyTheme.white
            scale: 4
            visible: video.playbackState === MediaPlayer.PausedState
        }
        Label {
            text: !fileDialog.selectedFile ? "No video opened. Select a file first!" :
                                             fileDialog.selectedFile.toString().replace("file://", "")
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            color: PhyTheme.white
        }
    }

    PhyFileDialog {
        id: fileDialog
        nameFilters: ["*.webm", "*.mp4"]
    }
}
