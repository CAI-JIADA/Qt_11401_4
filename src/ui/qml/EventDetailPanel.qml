import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

Rectangle {
    id: root
    color: "#fafafa"

    property var event: null

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        // 關閉按鈕
        RowLayout {
            Layout.fillWidth: true

            Item { Layout.fillWidth: true }

            ToolButton {
                Layout.preferredWidth: 32
                Layout.preferredHeight: 32

                Label {
                    anchors.centerIn: parent
                    text: "✕"
                    font.pixelSize: 14
                }

                onClicked: {
                    event = null
                }
            }
        }

        // 事件標題
        Label {
            Layout.fillWidth: true
            text: event ? event.title : ""
            font.pixelSize: 20
            font.weight: Font.Medium
            wrapMode: Text.WordWrap
        }

        // 時間資訊
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: "white"
            radius: 8

            RowLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 12

                // 日期圖示
                Rectangle {
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40
                    radius: 8
                    color: "#e3f2fd"

                    Label {
                        anchors.centerIn: parent
                        text: "📅"
                        font.pixelSize: 18
                    }
                }

                // 日期時間
                Column {
                    Layout.fillWidth: true
                    spacing: 4

                    Label {
                        text: event ? formatDate(event.startTime) : ""
                        font.pixelSize: 14
                        font.weight: Font.Medium
                    }

                    Label {
                        text: event ? formatTimeRange(event.startTime, event.endTime) : ""
                        font.pixelSize: 12
                        color: "#666666"
                    }
                }
            }
        }

        // 地點資訊 (如果有)
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 50
            color: "white"
            radius: 8
            visible: event && event.location && event.location.length > 0

            RowLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 12

                Label {
                    text: "📍"
                    font.pixelSize: 16
                }

                Label {
                    Layout.fillWidth: true
                    text: event ? event.location : ""
                    font.pixelSize: 13
                    elide: Text.ElideRight
                }
            }
        }

        // 描述
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: descriptionText.implicitHeight + 24
            color: "white"
            radius: 8
            visible: event && event.description && event.description.length > 0

            Label {
                id: descriptionText
                anchors.fill: parent
                anchors.margins: 12
                text: event ? event.description : ""
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                color: "#333333"
            }
        }

        // 參與者
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: attendeesColumn.implicitHeight + 24
            color: "white"
            radius: 8
            visible: event && event.attendees && event.attendees.length > 0

            Column {
                id: attendeesColumn
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                Label {
                    text: qsTr("參與者")
                    font.pixelSize: 12
                    font.weight: Font.Medium
                    color: "#666666"
                }

                Repeater {
                    model: event ? event.attendees : []

                    RowLayout {
                        width: parent.width
                        spacing: 8

                        Rectangle {
                            Layout.preferredWidth: 24
                            Layout.preferredHeight: 24
                            radius: 12
                            color: "#e0e0e0"

                            Label {
                                anchors.centerIn: parent
                                text: modelData.charAt(0).toUpperCase()
                                font.pixelSize: 12
                            }
                        }

                        Label {
                            Layout.fillWidth: true
                            text: modelData
                            font.pixelSize: 13
                            elide: Text.ElideRight
                        }
                    }
                }
            }
        }

        // 來源平台
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            color: "white"
            radius: 8

            RowLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                Label {
                    text: qsTr("來源:")
                    font.pixelSize: 12
                    color: "#666666"
                }

                Rectangle {
                    Layout.preferredWidth: 20
                    Layout.preferredHeight: 20
                    radius: 4
                    color: getPlatformColor(event ? event.platform : "")

                    Label {
                        anchors.centerIn: parent
                        text: getPlatformIcon(event ? event.platform : "")
                        font.pixelSize: 10
                        color: "white"
                    }
                }

                Label {
                    text: event ? getPlatformName(event.platform) : ""
                    font.pixelSize: 12
                }
            }
        }

        Item { Layout.fillHeight: true }

        // 操作按鈕
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Button {
                Layout.fillWidth: true
                text: qsTr("編輯")
                flat: true
            }

            Button {
                Layout.fillWidth: true
                text: qsTr("刪除")
                flat: true
                Material.foreground: "#f44336"
            }
        }
    }

    // 輔助函數
    function formatDate(dateTime) {
        if (!dateTime) return ""
        var options = { year: 'numeric', month: 'long', day: 'numeric', weekday: 'long' }
        return dateTime.toLocaleDateString(Qt.locale(), options)
    }

    function formatTimeRange(start, end) {
        if (!start) return ""
        var startStr = start.toLocaleTimeString(Qt.locale(), { hour: '2-digit', minute: '2-digit' })
        if (!end) return startStr
        var endStr = end.toLocaleTimeString(Qt.locale(), { hour: '2-digit', minute: '2-digit' })
        return startStr + " - " + endStr
    }

    function getPlatformIcon(platform) {
        switch (platform) {
            case "Google": return "G"
            case "Apple": return "A"
            case "Outlook": return "O"
            default: return "?"
        }
    }

    function getPlatformName(platform) {
        switch (platform) {
            case "Google": return "Google Calendar"
            case "Apple": return "Apple Calendar"
            case "Outlook": return "Microsoft Outlook"
            default: return qsTr("未知")
        }
    }

    function getPlatformColor(platform) {
        switch (platform) {
            case "Google": return "#4285F4"
            case "Apple": return "#FF9500"
            case "Outlook": return "#0078D4"
            default: return "#9e9e9e"
        }
    }
}
