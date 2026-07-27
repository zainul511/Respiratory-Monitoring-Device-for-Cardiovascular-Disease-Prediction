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


## 2. Firmware (ESP-IDF + FreeRTOS)

The firmware is developed using **ESP-IDF** with **FreeRTOS**, providing deterministic task scheduling and reliable acquisition of respiratory motion data from three MPU6050 IMUs at **100 Hz**.

### Method 1 – I²C Multiplexer Architecture

This implementation uses a **PCA9548A I²C multiplexer** to interface with three MPU6050 sensors that all operate at the default **0x68** I²C address. The ESP32 selects the required multiplexer channel before communicating with each sensor.

| Sensor | PCA9548A Channel | I²C Address |
|---------|-----------------:|------------:|
| IMU 1 | 0 | 0x68 |
| IMU 2 | 1 | 0x68 |
| IMU 3 | 2 | 0x68 |

**Features**
- Single ESP32 hardware I²C bus
- PCA9548A resolves address conflicts
- Sequential sensor polling at **100 Hz**
- Data transmitted to the PC via USB Serial as structured packets

---

### Method 2 – Dual I²C Bus Architecture

This implementation eliminates the external multiplexer by utilizing the ESP32's **two independent hardware I²C controllers**. One bus interfaces with a single MPU6050, while the second bus connects to two MPU6050 sensors configured with different addresses (**0x68** and **0x69**) using the AD0 pin.

| ESP32 I²C Bus | Connected Sensors | I²C Address(es) |
|---------------|-------------------|-----------------|
| I²C Bus 0 | IMU 1 | 0x68 |
| I²C Bus 1 | IMU 2, IMU 3 | 0x68, 0x69 |

**Features**
- No external I²C multiplexer required
- Reduced hardware complexity and wiring
- Uses both ESP32 hardware I²C peripherals
- **Utilizes the MPU6050 Digital Motion Processor (DMP) for onboard sensor fusion**
- **Obtains gravity-compensated linear acceleration directly from the DMP, reducing software processing on the ESP32**
- Improved signal quality by removing the gravity component before transmission
- Data acquired at **100 Hz** and transmitted via USB Serial as structured packets
