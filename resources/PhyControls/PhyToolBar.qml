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
    property alias buttonBack: buttonBack
    property alias buttonMenu: buttonMenu

    RowLayout {
        anchors.fill: parent

        ToolButton {
            id: buttonBack
            text: PhyTheme.iconFont.arrowLeft
            flat: true
            leftPadding: PhyTheme.marginRegular
            rightPadding: PhyTheme.marginRegular
        }
        Label {
            text: title
            elide: Label.ElideRight
            horizontalAlignment: Qt.AlignHCenter
            verticalAlignment: Qt.AlignVCenter
            Layout.fillWidth: true
        }
        ToolButton {
            id: buttonMenu
            text: PhyTheme.iconFont.list
            flat: true
            visible: false
            leftPadding: PhyTheme.marginRegular
            rightPadding: PhyTheme.marginRegular
        }
    }
}
