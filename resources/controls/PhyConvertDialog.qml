import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0
import PhyTheme 1.0

Rectangle {
    id: convertDialog
    color: PhyTheme.white
    property string file

    Component {
        id: codecConvertDelegate

        Item {
            width: codecSelectorBox.currentText !== "REMOVE" ? ListView.view.width : 0
            height: codecSelectorBox.currentText !== "REMOVE" ? codecSelectorBox.implicitHeight: 0
            visible: codecSelectorBox.currentText !== "REMOVE"

            RowLayout {
                anchors.fill: parent
                spacing: PhyTheme.marginSmall
                Label {
                    text: type + " Stream " + subIndex + ":"
                    Layout.alignment: Qt.AlignLeft
                    Layout.leftMargin: PhyTheme.marginRegular
                    Layout.fillWidth: true
                    Layout.horizontalStretchFactor: 2
                }
                Label {
                    text: modelData
                }
                Label {
                    text: "convert to"
                    horizontalAlignment: Text.AlignHCenter
                    Layout.fillWidth: true
                    Layout.horizontalStretchFactor: 3
                }
                ComboBox {
                    id: resolutionSelector
                    visible: type === "Video"
                    model: ["", "960 x 540", "1280 x 720", "1600 x 900", "1920 x 1080"]
                    onCurrentValueChanged: parent.updateModelValue()
                    Layout.alignment: Qt.AlignRight
                }
                ComboBox {
                    id: codecSelectorBox
                    textRole: "text"
                    valueRole: "value"
                    displayText: multimediaFormats.format(currentValue, prefix)
                    model: [{"text":"-","value":"-"}]
                    Layout.alignment: Qt.AlignRight
                    onCurrentValueChanged: parent.updateModelValue()
                    Component.onCompleted: {
                        loadCodecModel()
                        containerFormatSelectorBox.onCurrentValueChanged.connect(loadCodecModel)
                        containerDataSelectorBox.onCurrentValueChanged.connect(loadCodecModel)
                        codecSelectorView.onActiveVideoStreamsChanged.connect(loadCodecModel)
                        codecSelectorView.onActiveAudioStreamsChanged.connect(loadCodecModel)
                    }

                    function loadCodecModel() {
                        var copyData = {"text": currentText, "value": currentValue}
                        var newModel = multimediaFormats.getEncodeCodecs(false, false, prefix, containerDataSelectorBox.currentValue ?
                            containerFormatSelectorBox.currentValue + ", " + containerDataSelectorBox.currentValue :
                            containerFormatSelectorBox.currentValue)
                            .sort(multimediaFormats.compareEncodeCodecs)
                            .map(codec => ({"text": multimediaFormats.formatWithDeco(codec, true, prefix), "value": codec}))
                        if (newModel.length === 0)
                            newModel = [{"text":"","value":""}]
                        if (currentValue) {
                            if (typeof multimediaGST !== "undefined") {
                                if (type === "Video" && codecSelectorView.activeVideoStreams > 1)
                                    newModel.push({"text":"REMOVE","value":""})
                                else if (type === "Audio" && codecSelectorView.activeAudioStreams > 1)
                                    newModel.push({"text":"REMOVE","value":""})
                            } else {
                                if ((codecSelectorView.activeVideoStreams + codecSelectorView.activeAudioStreams) > 1)
                                    newModel.push({"text":"REMOVE","value":""})
                            }
                        } else {
                            newModel.push({"text":"REMOVE","value":""})
                        }
                        model = newModel
                        var copyIndex = 0
                        if (copyData.text === "REMOVE")
                            copyIndex = find("REMOVE")
                        else if (copyData.value)
                            copyIndex = indexOfValue(copyData.value)
                        currentIndex = copyIndex > 0 ? copyIndex : 0
                    }
                }

                function updateModelValue() {
                    if (modelValue && !codecSelectorBox.currentValue) {
                        if (type === "Video")
                            codecSelectorView.activeVideoStreams -= 1
                        else if (type === "Audio")
                            codecSelectorView.activeAudioStreams -= 1
                    } else if (!modelValue && codecSelectorBox.currentValue) {
                        if (type === "Video")
                            codecSelectorView.activeVideoStreams += 1
                        else if (type === "Audio")
                            codecSelectorView.activeAudioStreams += 1
                    }
                    if (resolutionSelector.visible && codecSelectorBox.currentValue && resolutionSelector.currentText) {
                        var resolution = resolutionSelector.currentText.split("x")
                        modelValue = codecSelectorBox.currentValue + ",width=" + resolution[0].trim() + ",height=" + resolution[1].trim()
                    } else {
                        modelValue = codecSelectorBox.currentValue
                    }
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            spacing: PhyTheme.marginRegular
            Layout.margins: PhyTheme.marginSmall

            Button {
                text: "Cancel"
                flat: true
                onClicked: convertDialogLoader.active = false
            }
            Label {
                Layout.fillWidth: true
                text: file.replace("file://", "")
                elide: Text.ElideLeft
                Layout.leftMargin: PhyTheme.marginRegular
                Layout.rightMargin: PhyTheme.marginRegular
            }
            Button {
                text: "Convert"
                flat: true
                onClicked: {
                    var destinationFile = (fileNameField.text ?? fileNameField.placeholderText) + extensionSelectorBox.currentValue
                    var containerFormat = containerDataSelectorBox.currentValue ?
                            containerFormatSelectorBox.currentValue + ", " + containerDataSelectorBox.currentValue :
                            containerFormatSelectorBox.currentValue
                    var selectedVideoCodecs = []
                    var selectedAudioCodecs = []
                    for (var i = 0; i < codecSelectorView.count; i++) {
                        if (fileCodecsModel.get(i).type === "Video") {
                            selectedVideoCodecs.push(fileCodecsModel.get(i).modelValue)
                        } else if (fileCodecsModel.get(i).type === "Audio") {
                            selectedAudioCodecs.push(fileCodecsModel.get(i).modelValue)
                        }
                    }
                    multimediaFormats.convertFile(file, destinationFile, containerFormat, selectedVideoCodecs, selectedAudioCodecs)
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            spacing: PhyTheme.marginSmall
            Label {
                text: "Destination:"
                Layout.leftMargin: 2 * PhyTheme.marginRegular
                Layout.fillWidth: true
            }
            TextField {
                id: fileNameField
                Layout.fillWidth: true
                text: file.replace(new RegExp(".+\\/([^\\/]+)\\.[^.]+$"), "$1") + "_converted"
                placeholderText: "File Name"
            }
            ComboBox {
                id: extensionSelectorBox
                model: multimediaFormats.getExtensions(containerFormatSelectorBox.currentValue)
                        .map(extension => "." + extension)
                Layout.rightMargin: PhyTheme.marginRegular
                Layout.alignment: Qt.AlignRight
            }
        }
        RowLayout {
            Layout.fillWidth: true
            spacing: PhyTheme.marginSmall
            Label {
                text: "Container Format:"
                Layout.leftMargin: 2 * PhyTheme.marginRegular
                Layout.rightMargin: PhyTheme.marginRegular
            }
            ComboBox {
                id: containerFormatSelectorBox
                textRole: "text"
                valueRole: "value"
                model: multimediaFormats.getContainerFormats()
                        .filter(format => format.startsWith("video/"))
                        .filter(format =>
                            typeof multimediaGST === "undefined" ||
                            (multimediaFormats.getEncodeCodecs(false, false, "video/", format).length > 0 &&
                                multimediaFormats.getEncodeCodecs(false, false, "audio/", format).length > 0))
                        .map(format => format.includes(", ") ? format.slice(0, format.indexOf(", ")) : format)
                        .filter((format, index, formats) => index === formats.indexOf(format) && multimediaFormats.getExtensions(format).length > 0)
                        .map(format => ({"text": multimediaFormats.format(format), "value": format}))
                Layout.rightMargin: PhyTheme.marginRegular
            }
            Label {
                text: "Data:"
                Layout.rightMargin: PhyTheme.marginRegular
            }
            ComboBox {
                id: containerDataSelectorBox
                model: multimediaFormats.getContainerFormats()
                        .filter(format => format === containerFormatSelectorBox.currentValue || format.startsWith(containerFormatSelectorBox.currentValue + ", "))
                        .filter(format =>
                            typeof multimediaGST === "undefined" ||
                            (multimediaFormats.getEncodeCodecs(false, false, "video/", format).length > 0 &&
                                multimediaFormats.getEncodeCodecs(false, false, "audio/", format).length > 0))
                        .map(format => format.includes(", ") ? format.slice(format.indexOf(", ") + 2) : "")
                delegate: ItemDelegate {
                    required property string modelData

                    text: modelData
                    width: containerDataSelectorBox.width
                    contentItem: Text {
                        text: parent.text
                        font: containerDataSelectorBox.font
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideNone
                        wrapMode: Text.WordWrap
                    }
                }
                Layout.rightMargin: PhyTheme.marginRegular
                Layout.fillWidth: true
            }
        }
        ListView {
            id: codecSelectorView
            property int activeVideoStreams
            property int activeAudioStreams
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            model: updateFileCodecModel(file)
            delegate: codecConvertDelegate
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: PhyTheme.marginRegular
            ListModel {
                id: fileCodecsModel
            }
            function updateFileCodecModel(currentFile) {
                fileCodecsModel.clear()
                var codecs = multimediaFormats.getFileVideoCodec(currentFile)
                activeVideoStreams = codecs.length
                for (var i = 0; i < codecs.length; i++) {
                    fileCodecsModel.append({
                        "type": "Video",
                        "prefix": "video/",
                        "subIndex": i,
                        "modelData": multimediaFormats.format(codecs[i], "video/"),
                        "modelValue": "-"
                    });
                }
                codecs = multimediaFormats.getFileAudioCodec(currentFile)
                activeAudioStreams = codecs.length
                for (var i = 0; i < codecs.length; i++) {
                    fileCodecsModel.append({
                        "type": "Audio",
                        "prefix": "audio/",
                        "subIndex": i,
                        "modelData": multimediaFormats.format(codecs[i], "audio/"),
                        "modelValue": "-"
                    });
                }
                return fileCodecsModel
            }
        }
    }

    Rectangle {
        id: convertProgress
        anchors.centerIn: parent
        visible: multimediaFormats.converting
        width: parent.width / 2
        height: parent.height / 2
        color: PhyTheme.white

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: PhyTheme.marginSmall
            Label {
                text: "<b>Converting...</b>"
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            }
            Label {
                id: progressLabel
                text: formatTime(progressBar.value) + " / " + formatTime(progressBar.to)
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                function formatTime(nanoseconds) {
                    nanoseconds /= 1000000
                    if (nanoseconds >= 3600000) {
                        return new Date(nanoseconds).toLocaleTimeString(Qt.locale(), "hh:" + "mm:" + "ss:" + "zzz")
                    } else {
                        return new Date(nanoseconds).toLocaleTimeString(Qt.locale(), "mm:" + "ss:" + "zzz")
                    }
                }
            }
            ProgressBar {
                id: progressBar
                indeterminate: progressBar.value < 0 || progressBar.to < 0
                Layout.fillWidth: true
            }
        }

        Timer {
            interval: 100
            repeat: true
            running: convertProgress.visible
            triggeredOnStart: true
            onTriggered: {
                progressBar.value = multimediaFormats.getConvertPosition()
                progressBar.to = multimediaFormats.getConvertDuration()
            }
        }
    }

    Rectangle {
        id: convertError
        anchors.centerIn: parent
        visible: false
        width: parent.width / 2
        height: parent.height / 2
        color: PhyTheme.white

        Component.onCompleted: multimediaFormats.conversionError.connect(onError)
        Component.onDestruction: multimediaFormats.conversionError.disconnect(onError)

        function onError(message) {
            convertError.visible = true;
            errorLabel.text = message;
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: PhyTheme.marginSmall
            Label {
                text: "<b>An error occurred:</b>"
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            }
            Label {
                id: errorLabel
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: true
            }
            Button {
                id: errorButton
                text: "Ok"
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                onClicked: {
                    convertError.visible = false
                    errorLabel.text = ""
                }
            }
        }
    }
}
