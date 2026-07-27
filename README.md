# Respiratory-Monitoring-Device-for-Cardiovascular-Disease-Prediction
Wearable respiratory monitor using ESP32 &amp; 3x MPU6050s. Extracts  breathing patterns via Incremental PCA &amp; DMP signal fusion. Includes ESP-IDF firmware (Dual I2C), Code for Python dashboard (PyQt5), and 3D mechanical files for real-time cardiovascular analysis.

##  System Architecture

The device measures chest expansion and contraction using three strategically placed IMUs on a wearable belt. The system is designed to provide  respiratory pattern data through sensor fusion and advanced software processing.

### Key Features
* **Multi-Sensor Fusion:** Utilizes three **MPU6050** accelerometers to capture chest wall movement.
* **Real-Time Visualization:** A  **Python dashboard (PyQt5)** plots breathing waveforms and give visual analysis of respiratory patterns.
* **Advanced Signal Processing:**
    * **Incremental PCA (Principal Component Analysis):** Extracts the dominant breathing motion vector from the 3-axis acceleration data.
    * **EMA (Exponential Moving Average):** Applies smoothing filters to eliminate high-frequency noise and jitter.
* **Wearable Design:** A stretchable belt with  a custom 3D-printed enclosure

## Components Used

| Component | Quantity | Description |
|-----------|:--------:|-------------|
| ESP32 Development Board | 1 | Main microcontroller (originally ESP32-C3). |
| MPU6050 Sensors | 3 | 6-axis accelerometer/gyroscope modules. |
| Elastic Belt | 1 | 1.5-inch wide stretchable belt with a side-release buckle. |
| JST Connectors | 3 | Detachable connectors for sensor connections. |
| Heat Shrink Tube | – | 1.2 cm diameter tubing for wire insulation and cable management. |
| 3.7 V Li-Po Battery | 1 | Rechargeable lithium-polymer battery for portable power. |
| Boost Converter | 1 | Steps up the battery voltage to the required system voltage. |
| 3.3 V Voltage Regulator | 1 | Provides a stable 3.3 V supply for the ESP32 and sensors. |
| Slide Switch | 1 | Main power ON/OFF switch for the device. |

## 3D Printed Parts

The following custom 3D-printed parts were designed for the wearable system:

- **Enclosure_Box.stl** – Houses the ESP32-C3 and connectors. Includes cutouts for the USB-C port, JST connectors, and the power switch/push button.
- **Bend_on_Belt.stl** – A belt-mounted cable guide that routes sensor wires securely and reduces mechanical stress during movement.

## Technical Implementation

### 1. Sensor Placement & Assembly

- **Three MPU6050 IMUs** are mounted on the elastic belt for respiratory motion sensing.
- **One IMU** is positioned centrally below the chest.
- **Two IMUs** are positioned symmetrically on the left and right sides of the central sensor.
- The sensors are connected using **12 signal lines**:
  - VCC × 3
  - GND × 3
  - SDA × 3
  - SCL × 3
- All wiring is bundled into a **stable cable harness** using heat-shrink tubing to improve mechanical strength, reduce cable movement, and enhance reliability during wearable operation.
