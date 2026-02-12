/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2021 PHYTEC Messtechnik GmbH
 */

import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0
import PhyTheme 1.0

ToolBar {
    property string title: ""
    property string subTitle: ""
    property alias buttonBack: buttonBack
    property alias buttonMenu: buttonMenu
    property alias infoMenu: infoMenu

    RowLayout {
        anchors.fill: parent

        RowLayout {
            id: toolBarLeft
            Layout.fillWidth: false
            Layout.minimumWidth: toolBarRight.width
            ToolButton {
                id: buttonBack
                text: PhyTheme.iconFont.arrowLeft
                font.family: icons.font.family
                flat: true
                leftPadding: PhyTheme.marginBig
                rightPadding: PhyTheme.marginBig
                topPadding: PhyTheme.marginRegular
                bottomPadding: PhyTheme.marginRegular
            }
        }
        ColumnLayout {
            Layout.alignment: Qt.AlignCenter
            Label {
                text: "<b>" + title + "</b>"
                elide: Text.ElideRight
                Layout.alignment: Qt.AlignHCenter
            }
            Label {
                text: subTitle
                color: PhyTheme.gray4
                visible: text !== ""
                elide: Text.ElideLeft
                scale: 0.8
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
            }
        }
        RowLayout {
            id: toolBarRight
            Layout.fillWidth: false
            Layout.minimumWidth: toolBarLeft.width
            ToolButton {
                id: infoMenu
                Layout.fillWidth: false
                text: PhyTheme.iconFont.info
                font.family: icons.font.family
                flat: true
                visible: false
                leftPadding: PhyTheme.marginBig
                rightPadding: PhyTheme.marginBig
                topPadding: PhyTheme.marginRegular
                bottomPadding: PhyTheme.marginRegular
            }
            ToolButton {
                id: buttonMenu
                Layout.fillWidth: false
                text: PhyTheme.iconFont.list
                font.family: icons.font.family
                flat: true
                visible: false
                leftPadding: PhyTheme.marginBig
                rightPadding: PhyTheme.marginBig
                topPadding: PhyTheme.marginRegular
                bottomPadding: PhyTheme.marginRegular
            }
        }
    }
}
