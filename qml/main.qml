import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import HeartMonitor 1.0

ApplicationWindow {
    id: mainWindow
    width: 1200
    height: 800
    visible: true
    title: "Wireless Heart Monitor"
    
    property bool darkTheme: true
    
    // Theme colors
    property color backgroundColor: darkTheme ? "#1e1e1e" : "#ffffff"
    property color surfaceColor: darkTheme ? "#2d2d2d" : "#f5f5f5"
    property color primaryColor: "#e74c3c"
    property color accentColor: "#3498db"
    property color textColor: darkTheme ? "#ffffff" : "#000000"
    property color cardColor: darkTheme ? "#3d3d3d" : "#ffffff"
    
    color: backgroundColor
    
    // Connection status bar
    Rectangle {
        id: statusBar
        anchors.top: parent.top
        width: parent.width
        height: 40
        color: hmController.isConnected ? "#27ae60" : "#e74c3c"
        
        Row {
            anchors.centerIn: parent
            spacing: 10
            
            Rectangle {
                width: 12
                height: 12
                radius: 6
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                
                SequentialAnimation on opacity {
                    running: hmController.isConnected
                    loops: Animation.Infinite
                    NumberAnimation { to: 0.2; duration: 1000 }
                    NumberAnimation { to: 1.0; duration: 1000 }
                }
            }
            
            Text {
                text: hmController.connectionStatus
                color: "white"
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
            }
            
            Text {
                text: hmController.isConnected ? 
                      "| HR: " + hmController.currentHeartRate + " BPM" : ""
                color: "white"
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }
    
    // Main content area
    RowLayout {
        anchors.top: statusBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 10
        spacing: 10
        
        // Left panel - Controls and Info
        Rectangle {
            Layout.preferredWidth: 300
            Layout.fillHeight: true
            color: cardColor
            radius: 8
            border.color: darkTheme ? "#555" : "#ddd"
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 15
                
                // Connection Controls
                GroupBox {
                    title: "Connection"
                    Layout.fillWidth: true
                    
                    background: Rectangle {
                        color: "transparent"
                        border.color: darkTheme ? "#555" : "#ccc"
                        radius: 4
                    }
                    
                    label: Text {
                        text: parent.title
                        color: textColor
                        font.bold: true
                    }
                    
                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 10
                        
                        Button {
                            text: hmController.isConnected ? "Disconnect" : "Connect"
                            Layout.fillWidth: true
                            onClicked: {
                                if (hmController.isConnected) {
                                    hmController.stopConnection()
                                } else {
                                    hmController.startConnection()
                                }
                            }
                        }
                        
                        Button {
                            text: hmController.isRecording ? "Stop Recording" : "Start Recording"
                            Layout.fillWidth: true
                            enabled: hmController.isConnected
                            onClicked: {
                                if (hmController.isRecording) {
                                    hmController.stopRecording()
                                } else {
                                    hmController.startRecording()
                                }
                            }
                        }
                    }
                }
                
                // Heart Rate Display
                Rectangle {
                    Layout.fillWidth: true
                    height: 120
                    color: primaryColor
                    radius: 8
                    
                    Column {
                        anchors.centerIn: parent
                        spacing: 5
                        
                        Text {
                            text: "HEART RATE"
                            color: "white"
                            font.pixelSize: 14
                            font.bold: true
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        
                        Text {
                            text: hmController.currentHeartRate
                            color: "white"
                            font.pixelSize: 48
                            font.bold: true
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        
                        Text {
                            text: "BPM"
                            color: "white"
                            font.pixelSize: 16
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                    
                    // Heartbeat animation
                    Rectangle {
                        width: 8
                        height: 8
                        radius: 4
                        color: "white"
                        anchors.top: parent.top
                        anchors.right: parent.right
                        anchors.margins: 10
                        
                        SequentialAnimation on scale {
                            running: hmController.isConnected && hmController.currentHeartRate > 0
                            loops: Animation.Infinite
                            NumberAnimation { to: 1.5; duration: 100 }
                            NumberAnimation { to: 1.0; duration: 400 }
                            PauseAnimation { duration: Math.max(100, 60000/Math.max(1, hmController.currentHeartRate) - 500) }
                        }
                    }
                }
                
                // Alert Panel
                Rectangle {
                    Layout.fillWidth: true
                    height: alertText.visible ? 80 : 0
                    color: hmController.alertLevel >= 3 ? "#e74c3c" : "#f39c12"
                    radius: 8
                    visible: hmController.alertMessage.length > 0
                    
                    Text {
                        id: alertText
                        anchors.centerIn: parent
                        text: hmController.alertMessage
                        color: "white"
                        font.bold: true
                        wrapMode: Text.WordWrap
                        width: parent.width - 20
                        horizontalAlignment: Text.AlignHCenter
                    }
                    
                    SequentialAnimation on opacity {
                        running: parent.visible
                        loops: Animation.Infinite
                        NumberAnimation { to: 0.7; duration: 500 }
                        NumberAnimation { to: 1.0; duration: 500 }
                    }
                }
                
                // Data Export Controls
                GroupBox {
                    title: "Data Management"
                    Layout.fillWidth: true
                    
                    background: Rectangle {
                        color: "transparent"
                        border.color: darkTheme ? "#555" : "#ccc"
                        radius: 4
                    }
                    
                    label: Text {
                        text: parent.title
                        color: textColor
                        font.bold: true
                    }
                    
                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 10
                        
                        Button {
                            text: "Export Data"
                            Layout.fillWidth: true
                            onClicked: exportDialog.open()
                        }
                        
                        Button {
                            text: "Clear History"
                            Layout.fillWidth: true
                            onClicked: clearDialog.open()
                        }
                        
                        Text {
                            text: "Records: " + hmController.ecgDataModel.getReadingCount()
                            color: textColor
                            font.pixelSize: 12
                        }
                    }
                }
                
                Item { Layout.fillHeight: true }
            }
        }
        
        // Center panel - ECG Graph
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: cardColor
            radius: 8
            border.color: darkTheme ? "#555" : "#ddd"
            
            Column {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 10
                
                Text {
                    text: "Real-time ECG"
                    color: textColor
                    font.pixelSize: 18
                    font.bold: true
                }
                
                EcgGraph {
                    id: ecgGraph
                    width: parent.width
                    height: parent.height - 40
                    backgroundColor: cardColor
                    gridColor: darkTheme ? "#555" : "#ddd"
                    signalColor: primaryColor
                    
                    Connections {
                        target: hmController
                        function onNewEcgData(voltage, timestamp) {
                            ecgGraph.addDataPoint(voltage, timestamp)
                        }
                    }
                }
            }
        }
        
        // Right panel - Historical Data
        Rectangle {
            Layout.preferredWidth: 350
            Layout.fillHeight: true
            color: cardColor
            radius: 8
            border.color: darkTheme ? "#555" : "#ddd"
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 15
                
                Text {
                    text: "Historical Data"
                    color: textColor
                    font.pixelSize: 18
                    font.bold: true
                }
                
                // Rhythm Analysis
                Rectangle {
                    Layout.fillWidth: true
                    height: 100
                    color: surfaceColor
                    radius: 6
                    border.color: darkTheme ? "#444" : "#ccc"
                    
                    Column {
                        anchors.centerIn: parent
                        spacing: 8
                        
                        Text {
                            text: "Current Rhythm"
                            color: textColor
                            font.pixelSize: 12
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        
                        Text {
                            text: "Normal Sinus" // This would come from arrhythmia detector
                            color: accentColor
                            font.pixelSize: 16
                            font.bold: true
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        
                        Row {
                            anchors.horizontalCenter: parent.horizontalCenter
                            spacing: 20
                            
                            Column {
                                Text {
                                    text: "RR Avg"
                                    color: textColor
                                    font.pixelSize: 10
                                    anchors.horizontalCenter: parent.horizontalCenter
                                }
                                Text {
                                    text: "850ms"
                                    color: textColor
                                    font.pixelSize: 12
                                    anchors.horizontalCenter: parent.horizontalCenter
                                }
                            }
                            
                            Column {
                                Text {
                                    text: "HRV"
                                    color: textColor
                                    font.pixelSize: 10
                                    anchors.horizontalCenter: parent.horizontalCenter
                                }
                                Text {
                                    text: "45ms"
                                    color: textColor
                                    font.pixelSize: 12
                                    anchors.horizontalCenter: parent.horizontalCenter
                                }
                            }
                        }
                    }
                }
                
                // Historical data list
                Text {
                    text: "Recent Readings"
                    color: textColor
                    font.pixelSize: 14
                    font.bold: true
                }
                
                ListView {
                    id: historyListView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: hmController.ecgDataModel
                    clip: true
                    
                    delegate: Rectangle {
                        width: historyListView.width
                        height: 60
                        color: index % 2 === 0 ? surfaceColor : "transparent"
                        
                        Row {
                            anchors.left: parent.left
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.leftMargin: 10
                            spacing: 15
                            
                            Column {
                                anchors.verticalCenter: parent.verticalCenter
                                
                                Text {
                                    text: formattedTime
                                    color: textColor
                                    font.pixelSize: 12
                                    font.bold: true
                                }
                                
                                Text {
                                    text: "Voltage: " + Number(voltage).toFixed(3) + "V"
                                    color: textColor
                                    font.pixelSize: 10
                                    opacity: 0.7
                                }
                            }
                            
                            Column {
                                anchors.verticalCenter: parent.verticalCenter
                                visible: heartRate > 0
                                
                                Text {
                                    text: heartRate + " BPM"
                                    color: primaryColor
                                    font.pixelSize: 12
                                    font.bold: true
                                }
                                
                                Rectangle {
                                    width: 40
                                    height: 3
                                    color: heartRate < 60 ? "#3498db" : 
                                           heartRate > 100 ? "#e74c3c" : "#27ae60"
                                    radius: 1.5
                                }
                            }
                        }
                    }
                    
                    ScrollBar.vertical: ScrollBar {
                        active: true
                        policy: ScrollBar.AlwaysOn
                    }
                }
            }
        }
    }
    
    // Dialogs
    Dialog {
        id: exportDialog
        title: "Export ECG Data"
        modal: true
        anchors.centerIn: parent
        width: 400
        height: 200
        
        background: Rectangle {
            color: cardColor
            border.color: darkTheme ? "#555" : "#ccc"
            radius: 8
        }
        
        ColumnLayout {
            anchors.fill: parent
            spacing: 20
            
            Text {
                text: "Export historical ECG data to CSV file"
                color: textColor
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            
            TextField {
                id: exportPathField
                placeholderText: "Enter file path (e.g., /path/to/ecg_data.csv)"
                Layout.fillWidth: true
                text: "ecg_data_" + Qt.formatDateTime(new Date(), "yyyyMMdd_hhmmss") + ".csv"
            }
            
            Row {
                Layout.alignment: Qt.AlignRight
                spacing: 10
                
                Button {
                    text: "Cancel"
                    onClicked: exportDialog.close()
                }
                
                Button {
                    text: "Export"
                    onClicked: {
                        hmController.exportData("file://" + exportPathField.text)
                        exportDialog.close()
                    }
                }
            }
        }
    }
    
    Dialog {
        id: clearDialog
        title: "Clear Historical Data"
        modal: true
        anchors.centerIn: parent
        width: 350
        height: 150
        
        background: Rectangle {
            color: cardColor
            border.color: darkTheme ? "#555" : "#ccc"
            radius: 8
        }
        
        ColumnLayout {
            anchors.fill: parent
            spacing: 20
            
            Text {
                text: "Are you sure you want to clear all historical ECG data?"
                color: textColor
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            
            Row {
                Layout.alignment: Qt.AlignRight
                spacing: 10
                
                Button {
                    text: "Cancel"
                    onClicked: clearDialog.close()
                }
                
                Button {
                    text: "Clear All"
                    onClicked: {
                        hmController.clearHistory()
                        clearDialog.close()
                    }
                }
            }
        }
    }
    
    // Toast notifications for export results
    Connections {
        target: hmController
        function onDataExported(success, message) {
            toastMessage.text = message
            toastMessage.color = success ? "#27ae60" : "#e74c3c"
            toastAnimation.start()
        }
    }
    
    Rectangle {
        id: toastMessage
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 50
        width: toastText.width + 40
        height: 50
        color: "#27ae60"
        radius: 8
        opacity: 0
        
        property alias text: toastText.text
        
        Text {
            id: toastText
            anchors.centerIn: parent
            color: "white"
            font.bold: true
        }
        
        SequentialAnimation {
            id: toastAnimation
            NumberAnimation { target: toastMessage; property: "opacity"; to: 1.0; duration: 300 }
            PauseAnimation { duration: 3000 }
            NumberAnimation { target: toastMessage; property: "opacity"; to: 0.0; duration: 300 }
        }
    }
}