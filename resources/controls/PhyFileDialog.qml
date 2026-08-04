/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2021 PHYTEC Messtechnik GmbH
 */

import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0
import Qt.labs.folderlistmodel 2.15
import PhyTheme 1.0

Rectangle {
    id: dialog
    width: parent.width - 2 * PhyTheme.marginBig
    height: parent.height - 2 * PhyTheme.marginBig
    anchors.centerIn: parent
    visible: false
    color: PhyTheme.white
    property string selectedFile: ""
    property string currentFile: "file:///"
    property alias nameFilters: folderListModel.nameFilters

    FolderListModel {
        id: folderListModel
        showDotAndDotDot: true
        showDirsFirst: true
    }

    Component {
        id: fileDelegate

        Item {
            id: fileItem
            property int discoverResult
            width: listView.width
            height: labelFileName.implicitHeight

            Component.onCompleted: {
                if (folderListModel.isFolder(index) || !fileUrl) {
                    discoverResult = 0
                } else {
                    discoverResult = multimediaFormats.getFileDiscoverResult(fileUrl)
                    if (typeof multimediaGST !== "undefined" && discoverResult === 0) {
                        if (!multimediaFormats.getFileVideoCodec(fileUrl).length || !multimediaFormats.getFileAudioCodec(fileUrl).length)
                            discoverResult = 5
                    }
                }
                enabled = discoverResult === 0
            }

            RowLayout {
                anchors.fill: parent
                spacing: PhyTheme.marginSmall

                Label {
                    text: folderListModel.isFolder(index) ? PhyTheme.iconFont.folder : PhyTheme.iconFont.file
                    font.family: icons.font.family
                    Layout.leftMargin: PhyTheme.marginSmall
                }
                Label {
                    id: labelFileName
                    text: fileName
                    elide: Text.ElideMiddle
                    Layout.fillWidth: true
                }
                Label {
                    Component.onCompleted: {
                        if (folderListModel.isFolder(index) || !fileUrl)
                            return
                        if (fileItem.discoverResult === 0)
                            text = multimediaFormats.formatList(multimediaFormats.getFileVideoCodec(fileUrl)).join(", ")
                        else if (fileItem.discoverResult === 2)
                            text = "unknown"
                        else if (fileItem.discoverResult === 5)
                            text = "unsupported"
                        else
                            text = "failed"
                    }
                    Layout.rightMargin: PhyTheme.marginSmall
                }
                Label {
                    text: fileSize + " B"
                    Layout.rightMargin: PhyTheme.marginSmall
                    leftPadding: labelHeaderSize.leftPadding - contentWidth + labelHeaderSize.contentWidth
                    onContentWidthChanged: () => {
                        labelHeaderSize.leftPadding = Math.max(labelHeaderSize.leftPadding, contentWidth - labelHeaderSize.contentWidth)
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    listView.currentIndex = index

                    if (folderListModel.isFolder(index)) {
                        if (fileName === ".") {
                            return
                        } else if (fileName === "..") {
                            if (folderListModel.folder == "file:///") {
                                return
                            }
                            dialog.currentFile = folderListModel.parentFolder
                        } else {
                            if (folderListModel.folder == "file:///") {
                                dialog.currentFile = "file:///" + fileName
                            } else {
                                dialog.currentFile = folderListModel.folder + "/" + fileName
                            }
                        }
                        folderListModel.folder = dialog.currentFile
                        listView.currentIndex = -1
                    } else {
                        dialog.currentFile = fileUrl
                    }
                }
            }
        }
    }

    ColumnLayout {
        visible: !convertDialogLoader.active
        anchors.fill: dialog
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            spacing: PhyTheme.marginRegular
            Layout.margins: PhyTheme.marginSmall

            Button {
                text: "Cancel"
                flat: true
                onClicked: dialog.visible = false
            }
            Label {
                Layout.fillWidth: true
                text: folderListModel.folder.toString().replace("file://", "")
                elide: Text.ElideLeft
                Layout.leftMargin: PhyTheme.marginRegular
                Layout.rightMargin: PhyTheme.marginRegular
            }
            Button {
                text: "Convert"
                flat: true
                onClicked: {
                    if (listView.currentIndex === -1)
                        return
                    convertDialogLoader.active = true
                }
            }
            Button {
                text: "Open"
                flat: true
                onClicked: {
                    if (listView.currentIndex === -1)
                        return
                    dialog.selectedFile = dialog.currentFile
                    dialog.visible = false
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            color: PhyTheme.gray2
            implicitHeight: labelHeaderName.implicitHeight

            RowLayout {
                anchors.fill: parent

                Label {
                    id: labelHeaderName
                    text: "Name"
                    Layout.fillWidth: true
                    Layout.leftMargin: PhyTheme.marginSmall
                }
                Label {
                    text: "Video Codec"
                    Layout.rightMargin: PhyTheme.marginSmall
                }
                Label {
                    id: labelHeaderSize
                    text: "Size"
                    Layout.rightMargin: PhyTheme.marginSmall
                }
            }
        }

        ListView {
            id: listView
            Layout.fillHeight: true
            Layout.fillWidth: true
            clip: true
            currentIndex: -1
            boundsBehavior: Flickable.StopAtBounds
            model: folderListModel
            delegate: fileDelegate
            highlight: Rectangle {
                color: PhyTheme.gray1
            }
        }
    }

    Loader {
        id: convertDialogLoader
        active: false
        source: "PhyConvertDialog.qml"
        anchors.fill: parent
        onLoaded: item.file = dialog.currentFile
    }
}
