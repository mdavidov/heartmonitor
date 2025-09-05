import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQml


Canvas {
    id: ecgCanvas
    
    property color backgroundColor: "#1e1e1e"
    property color gridColor: "#555555"
    property color signalColor: "#e74c3c"
    property real signalWidth: 2.0
    property real gridLineWidth: 0.5
    
    // ECG data properties
    property real timeWindow: 10000 // 10 seconds in milliseconds
    property real voltageRange: 2.0 // -1V to +1V
    property int maxDataPoints: 2500 // 250Hz * 10s
    
    // Internal data storage
    property var ecgData: []
    property var timeData: []
    property real currentTime: 0
    property bool isRunning: false
    
    // Grid properties
    property real majorGridInterval: 1000 // 1 second
    property real minorGridInterval: 200  // 0.2 seconds
    property real voltageGridInterval: 0.5 // 0.5V
    
    onPaint: {
        var ctx = getContext("2d")
        ctx.clearRect(0, 0, width, height)
        
        drawBackground(ctx)
        drawGrid(ctx)
        drawEcgSignal(ctx)
        drawLabels(ctx)
    }
    
    function addDataPoint(voltage, timestamp) {
        ecgData.push(voltage)
        timeData.push(timestamp)
        
        // Keep only data within time window
        var cutoffTime = timestamp - timeWindow
        while (timeData.length > 0 && timeData[0] < cutoffTime) {
            ecgData.shift()
            timeData.shift()
        }
        
        // Limit array size for performance
        if (ecgData.length > maxDataPoints) {
            ecgData.shift()
            timeData.shift()
        }
        
        currentTime = timestamp
        isRunning = true
        
        // Request repaint
        requestPaint()
    }
    
    function clearData() {
        ecgData = []
        timeData = []
        isRunning = false
        requestPaint()
    }
    
    function drawBackground(ctx) {
        ctx.fillStyle = backgroundColor
        ctx.fillRect(0, 0, width, height)
    }
    
    function drawGrid(ctx) {
        if (!isRunning || timeData.length === 0) return
        
        ctx.strokeStyle = gridColor
        ctx.lineWidth = gridLineWidth
        ctx.setLineDash([])
        
        var latestTime = currentTime
        var startTime = latestTime - timeWindow
        
        // Vertical grid lines (time)
        var timeStep = minorGridInterval
        var startTimeGrid = Math.floor(startTime / timeStep) * timeStep
        
        for (var t = startTimeGrid; t <= latestTime + timeStep; t += timeStep) {
            if (t < startTime) continue
            
            var x = timeToX(t, startTime, latestTime)
            if (x >= 0 && x <= width) {
                ctx.beginPath()
                ctx.moveTo(x, 0)
                ctx.lineTo(x, height)
                
                // Major grid lines (every second)
                if (Math.abs(t % majorGridInterval) < timeStep / 2) {
                    ctx.strokeStyle = Qt.lighter(gridColor, 1.5)
                    ctx.lineWidth = gridLineWidth * 2
                } else {
                    ctx.strokeStyle = gridColor
                    ctx.lineWidth = gridLineWidth
                }
                
                ctx.stroke()
            }
        }
        
        // Horizontal grid lines (voltage)
        var voltageStep = voltageGridInterval / 2 // Minor grid
        var minVoltage = -voltageRange / 2
        var maxVoltage = voltageRange / 2
        
        for (var v = minVoltage; v <= maxVoltage; v += voltageStep) {
            var y = voltageToY(v)
            if (y >= 0 && y <= height) {
                ctx.beginPath()
                ctx.moveTo(0, y)
                ctx.lineTo(width, y)
                
                // Major grid lines (every 0.5V)
                if (Math.abs(v % voltageGridInterval) < 0.001) {
                    ctx.strokeStyle = Qt.lighter(gridColor, 1.5)
                    ctx.lineWidth = gridLineWidth * 2
                } else {
                    ctx.strokeStyle = gridColor
                    ctx.lineWidth = gridLineWidth
                }
                
                ctx.stroke()
            }
        }
        
        // Zero line
        var zeroY = voltageToY(0)
        ctx.beginPath()
        ctx.moveTo(0, zeroY)
        ctx.lineTo(width, zeroY)
        ctx.strokeStyle = Qt.lighter(gridColor, 2)
        ctx.lineWidth = gridLineWidth * 1.5
        ctx.stroke()
    }
    
    function drawEcgSignal(ctx) {
        if (ecgData.length < 2) return
        
        var latestTime = currentTime
        var startTime = latestTime - timeWindow
        
        ctx.strokeStyle = signalColor
        ctx.lineWidth = signalWidth
        ctx.setLineDash([])
        ctx.lineCap = "round"
        ctx.lineJoin = "round"
        
        ctx.beginPath()
        
        var firstPoint = true
        for (var i = 0; i < ecgData.length; i++) {
            if (timeData[i] < startTime) continue
            
            var x = timeToX(timeData[i], startTime, latestTime)
            var y = voltageToY(ecgData[i])
            
            if (firstPoint) {
                ctx.moveTo(x, y)
                firstPoint = false
            } else {
                ctx.lineTo(x, y)
            }
        }
        
        ctx.stroke()
        
        // Draw current point indicator
        if (ecgData.length > 0) {
            var lastX = timeToX(timeData[timeData.length - 1], startTime, latestTime)
            var lastY = voltageToY(ecgData[ecgData.length - 1])
            
            ctx.beginPath()
            ctx.arc(lastX, lastY, 3, 0, 2 * Math.PI)
            ctx.fillStyle = signalColor
            ctx.fill()
            
            // Pulsing effect
            ctx.beginPath()
            ctx.arc(lastX, lastY, 6, 0, 2 * Math.PI)
            ctx.strokeStyle = signalColor
            ctx.lineWidth = 1
            ctx.globalAlpha = 0.5 * (1 + Math.sin(Date.now() / 200))
            ctx.stroke()
            ctx.globalAlpha = 1.0
        }
    }
    
    function drawLabels(ctx) {
        ctx.fillStyle = Qt.lighter(gridColor, 2)
        ctx.font = "10px Arial"
        ctx.textAlign = "left"
        ctx.textBaseline = "top"
        
        // Voltage labels
        var minVoltage = -voltageRange / 2
        var maxVoltage = voltageRange / 2
        
        for (var v = minVoltage; v <= maxVoltage; v += voltageGridInterval) {
            var y = voltageToY(v)
            if (Math.abs(v) > 0.001) { // Skip zero
                ctx.fillText(v.toFixed(1) + "V", 5, y - 12)
            }
        }
        
        // Time labels
        if (isRunning && timeData.length > 0) {
            ctx.textAlign = "center"
            ctx.textBaseline = "bottom"
            
            var latestTime = currentTime
            var startTime = latestTime - timeWindow
            
            for (var t = startTime; t <= latestTime; t += majorGridInterval) {
                var x = timeToX(t, startTime, latestTime)
                if (x >= 30 && x <= width - 30) {
                    var timeStr = Math.floor((latestTime - t) / 1000) + "s"
                    ctx.fillText(timeStr, x, height - 5)
                }
            }
        }
        
        // Title and info
        ctx.textAlign = "left"
        ctx.textBaseline = "top"
        ctx.fillStyle = "#ffffff"
        ctx.font = "12px Arial"
        ctx.fillText("ECG Signal", 10, 10)
        
        if (isRunning) {
            ctx.font = "10px Arial"
            ctx.fillText("Sweep: " + (timeWindow/1000) + "s | Range: ±" + (voltageRange/2) + "V", 10, 30)
            ctx.fillText("Samples: " + ecgData.length, 10, 45)
        }
    }
    
    function timeToX(time, startTime, endTime) {
        return width * (time - startTime) / (endTime - startTime)
    }
    
    function voltageToY(voltage) {
        var normalized = (voltage + voltageRange/2) / voltageRange
        return height * (1 - normalized) // Invert Y axis
    }
    
    // Animation for sweep effect when no data
    Timer {
        id: sweepTimer
        interval: 100
        running: !isRunning
        repeat: true
        property real sweepPosition: 0

        onTriggered: {
            sweepPosition += 0.02
            if (sweepPosition > 1.0) {
                sweepPosition = 0
            }
            // Draw sweep line
            var ctx = ecgCanvas.getContext("2d")
            if (ctx) {
                ecgCanvas.requestPaint()
                // Add sweep line
                function addSweepLine() {
                    var ctx2 = ecgCanvas.getContext("2d")
                    ctx2.strokeStyle = Qt.rgba(0.2, 1.0, 0.2, 0.8)
                    ctx2.lineWidth = 1
                    ctx2.beginPath()
                    var x = width * sweepPosition
                    ctx2.moveTo(x, 0)
                    ctx2.lineTo(x, height)
                    ctx2.stroke()
                }
                addSweepLine()
            }
        }
    }
    
    // Mouse interaction for measurements
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        
        property point lastMousePos: Qt.point(0, 0)
        property bool measuring: false
        
        onPressed: {
            measuring = true
            lastMousePos = Qt.point(mouse.x, mouse.y)
        }
        
        onReleased: {
            measuring = false
            ecgCanvas.requestPaint()
        }
        
        onPositionChanged: {
            if (measuring) {
                // Draw measurement line
                var ctx = ecgCanvas.getContext("2d")
                ecgCanvas.requestPaint()
                
                // Add measurement overlay
                Timer.singleShot(1, function() {
                    var ctx2 = ecgCanvas.getContext("2d")
                    ctx2.strokeStyle = "#ffff00"
                    ctx2.lineWidth = 1
                    ctx2.setLineDash([5, 5])
                    
                    // Vertical measurement line
                    ctx2.beginPath()
                    ctx2.moveTo(mouse.x, 0)
                    ctx2.lineTo(mouse.x, height)
                    ctx2.stroke()
                    
                    // Horizontal measurement line
                    ctx2.beginPath()
                    ctx2.moveTo(0, mouse.y)
                    ctx2.lineTo(width, mouse.y)
                    ctx2.stroke()
                    
                    // Measurement text
                    var voltage = (height - mouse.y) / height * voltageRange - voltageRange/2
                    ctx2.fillStyle = "#ffff00"
                    ctx2.font = "10px Arial"
                    ctx2.fillText(voltage.toFixed(3) + "V", mouse.x + 5, mouse.y - 15)
                })
            }
        }
    }
}