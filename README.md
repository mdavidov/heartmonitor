# Heart Monitor

## **Key Components**

### **1. C++ Backend Classes:**

- **HeartMonitorController**: Main application logic, database management, heart rate calculation
- **EcgDataModel**: QAbstractListModel for historical data display in QML ListView
- **BluetoothManager**: Device discovery and data acquisition (with built-in simulation for testing)
- **ArrhythmiaDetector**: Real-time rhythm analysis and abnormality detection

### **2. QML User Interface:**

- **main.qml**: Modern responsive UI with real-time monitoring dashboard
- **EcgGraph.qml**: Custom Canvas-based ECG waveform visualization with grid and measurements

### **3. Key Features Implemented:**

**Real-time ECG Visualization:**

- QML Canvas with 250Hz sampling rate display
- Interactive measurement tools
- Automatic time window scrolling
- Grid overlay with voltage/time scales

**Bluetooth/Device Integration:**

- Device scanning and pairing
- Built-in ECG simulation for testing without hardware
- Robust data parsing and error handling
- Connection status monitoring

**Arrhythmia Detection:**

- R-peak detection algorithm
- RR interval analysis
- Heart rate variability calculation  
- Classification of common arrhythmias (bradycardia, tachycardia, AFib)

**Data Management:**

- SQLite database for historical storage
- QAbstractListModel integration for ListView
- CSV export functionality
- Automatic data cleanup and memory management

**Professional UI Design:**

- Dark theme with medical color scheme
- Real-time heart rate display with pulse animation
- Alert system with severity-based styling
- Responsive layout for different screen sizes

## **Important Medical Disclaimer:**

This is a **demonstration/educational application** and should not be used for actual medical diagnosis or treatment. Real medical devices require:

- FDA/CE certification
- Clinical validation
- Professional calibration
- Proper safety protocols

## **Build Instructions:**

1. Install Qt 6 with Bluetooth, SerialPort, and Multimedia modules
2. Use the provided CMakeLists.txt
3. The app includes simulation mode for testing without actual ECG hardware
4. Place QML files in a `qml/` directory

The application demonstrates professional-grade architecture for medical device software while maintaining clear boundaries about its educational purpose.
