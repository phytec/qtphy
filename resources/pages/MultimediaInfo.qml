/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2021 PHYTEC Messtechnik GmbH
 */

import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0
import Phytec.DeviceInfo 1.0
import PhyTheme 1.0
import "../controls"

Page {
    id: infoPage
    readonly property var videoCodecs: {
        "hwDecode": multimediaFormats.getDecodeCodecs(true, false, "video/"),
        "hwEncode": multimediaFormats.getEncodeCodecs(true, false, "video/"),
        "swDecode": multimediaFormats.getDecodeCodecs(false, true, "video/"),
        "swEncode": multimediaFormats.getEncodeCodecs(false, true, "video/"),
    }
    readonly property var audioCodecs: {
        "hwDecode": multimediaFormats.getDecodeCodecs(true, false, "audio/"),
        "hwEncode": multimediaFormats.getEncodeCodecs(true, false, "audio/"),
        "swDecode": multimediaFormats.getDecodeCodecs(false, true, "audio/"),
        "swEncode": multimediaFormats.getEncodeCodecs(false, true, "audio/"),
    }

    header: PhyToolBar {
        title: "Multimedia Information"
        buttonBack.onClicked: stack.pop()
        buttonMenu.visible: false
    }

    Flickable {
        id: scrollView
        anchors.fill: parent
        contentWidth: content.width
        contentHeight: content.height

        ColumnLayout {
            id: content
            width: scrollView.width

            GridLayout {
                columns: 2
                columnSpacing: PhyTheme.marginBig
                rowSpacing: PhyTheme.marginSmall
                Layout.margins: PhyTheme.marginRegular
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop | Qt.AlignLeft

                Row {
                    spacing: PhyTheme.marginRegular
                    Label {
                        text: "  " + PhyTheme.iconFont.cpu + "  "
                        font.family: icons.font.family
                        color: PhyTheme.white
                        background: Rectangle { color: PhyTheme.teal2 }
                    }
                    Label { text: "<h3>Hardware</h3>" }
                }
                Label {}
                Label { text: "Video Decode Codecs"; color: PhyTheme.gray3 }
                Label { text: multimediaFormats.formatList(infoPage.videoCodecs["hwDecode"]).join(", "); wrapMode: Text.WordWrap; Layout.fillWidth: true }
                Label { text: "Video Encode Codecs"; color: PhyTheme.gray3 }
                Label { text: multimediaFormats.formatList(infoPage.videoCodecs["hwEncode"]).join(", "); wrapMode: Text.WordWrap; Layout.fillWidth: true }
                Label { text: "Audio Decode Codecs"; color: PhyTheme.gray3 }
                Label { text: multimediaFormats.formatList(infoPage.audioCodecs["hwDecode"], "audio/").join(", "); wrapMode: Text.WordWrap; Layout.fillWidth: true }
                Label { text: "Audio Encode Codecs"; color: PhyTheme.gray3 }
                Label { text: multimediaFormats.formatList(infoPage.audioCodecs["hwEncode"], "audio/").join(", "); wrapMode: Text.WordWrap; Layout.fillWidth: true }
                Row {

                    Layout.topMargin: 2 * PhyTheme.marginBig
                    spacing: PhyTheme.marginRegular
                    Label {
                        text: "  " + PhyTheme.iconFont.code + "  "
                        font.family: icons.font.family
                        color: PhyTheme.white
                        background: Rectangle { color: PhyTheme.teal2 }
                    }
                    Label { text: "<h3>Software</h3>" }
                }
                Label {}
                Label { text: "Video Decode Codecs"; color: PhyTheme.gray3 }
                Label { text: multimediaFormats.formatList(infoPage.videoCodecs["swDecode"]).join(", "); wrapMode: Text.WordWrap; Layout.fillWidth: true }
                Label { text: "Video Encode Codecs"; color: PhyTheme.gray3 }
                Label { text: multimediaFormats.formatList(infoPage.videoCodecs["swEncode"]).join(", "); wrapMode: Text.WordWrap; Layout.fillWidth: true }
                Label { text: "Audio Decode Codecs"; color: PhyTheme.gray3 }
                Label { text: multimediaFormats.formatList(infoPage.audioCodecs["swDecode"], "audio/").join(", "); wrapMode: Text.WordWrap; Layout.fillWidth: true }
                Label { text: "Audio Encode Codecs"; color: PhyTheme.gray3 }
                Label { text: multimediaFormats.formatList(infoPage.audioCodecs["swEncode"], "audio/").join(", "); wrapMode: Text.WordWrap; Layout.fillWidth: true }
            }
        }
    }
}
