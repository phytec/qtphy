/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2022 PHYTEC Messtechnik GmbH
 */

import QtQuick 2.0
import QtQuick.Controls 2.0
import PhyTheme 1.0

Label {
    property string dateFormat: "ddd MMM d  hh:mm"

    Component.onCompleted: updateDate()

    function updateDate() {
        text = new Date().toLocaleString(Qt.locale(), dateFormat)
    }

    Timer {
        interval: 1000
        running: true
        repeat: true
        onTriggered: parent.updateDate()
    }
}
