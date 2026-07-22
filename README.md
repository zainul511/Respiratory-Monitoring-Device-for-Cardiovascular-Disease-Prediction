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
| PCA9548A Multiplexer | 1 | Used in V1 implementation to manage I²C address conflicts. |
| Elastic Belt | 1 | 1.5-inch wide stretchable belt with a side-release buckle. |
| JST Connectors | 3 | Detachable connectors for sensor connections. |
| Heat Shrink Tube | – | 1.2 cm diameter tubing for wire insulation and cable management. |
