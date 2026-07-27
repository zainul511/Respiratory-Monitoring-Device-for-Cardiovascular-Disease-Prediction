#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include <MPU6050.h>

#define PCA9548A_ADDR 0x70
#define MPU6050_ADDR 0x68
#define MPU6050_WHO_AM_I 0x75
#define MPU6050_PWR_MGMT_1 0x6B
#define MPU6050_ACCEL_XOUT_H 0x3B

// ESP-C3 specific pin assignments
#define I2C_MASTER_SCL_IO 2
#define I2C_MASTER_SDA_IO 3
#define I2C_MASTER_FREQ_HZ 100000
#define I2C_MASTER_PORT I2C_NUM_0

static const char *TAG = "MPU6050";

// Queue to hold sensor data for serial output
static QueueHandle_t serial_queue;

typedef struct {
    int16_t accel[3][3]; // [sensor][x,y,z]
    int64_t timestamp_ms;
} sensor_packet_t;

// I2C initialization
void i2c_master_init() {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_PORT, &conf);
    i2c_driver_install(I2C_MASTER_PORT, conf.mode, 0, 0, 0);
}

// PCA9548A channel selection
esp_err_t pca9548a_select_channel(uint8_t channel) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (PCA9548A_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, (1 << channel), true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_PORT, cmd, 100 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

// I2C write
esp_err_t i2c_write_byte(uint8_t channel, uint8_t reg_addr, uint8_t data) {
    if (pca9548a_select_channel(channel) != ESP_OK) return ESP_FAIL;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_PORT, cmd, 100 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

// I2C read
esp_err_t i2c_read_bytes(uint8_t channel, uint8_t reg_addr, uint8_t *data, size_t len) {
    if (pca9548a_select_channel(channel) != ESP_OK) return ESP_FAIL;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_READ, true);
    if (len > 1) {
        i2c_master_read(cmd, data, len - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, data + len - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_PORT, cmd, 100 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

// MPU6050 initialization
esp_err_t mpu6050_init(uint8_t channel) {
    esp_err_t ret = i2c_write_byte(channel, MPU6050_PWR_MGMT_1, 0x00);
    if (ret != ESP_OK) return ret;

    uint8_t who_am_i;
    ret = i2c_read_bytes(channel, MPU6050_WHO_AM_I, &who_am_i, 1);
    if (ret != ESP_OK || who_am_i != 0x68) return ESP_ERR_NOT_FOUND;

    ESP_LOGI(TAG, "MPU6050 on channel %d initialized", channel);
    return ESP_OK;
}

// Read accelerometer
esp_err_t mpu6050_read_accel(uint8_t channel, int16_t *accel_x, int16_t *accel_y, int16_t *accel_z) {
    uint8_t data[6];
    esp_err_t ret = i2c_read_bytes(channel, MPU6050_ACCEL_XOUT_H, data, 6);
    if (ret != ESP_OK) return ret;

    *accel_x = (int16_t)((data[0] << 8) | data[1]);
    *accel_y = (int16_t)((data[2] << 8) | data[3]);
    *accel_z = (int16_t)((data[4] << 8) | data[5]);
    return ESP_OK;
}

// Function to send data via Serial port 
void send_serial_data(sensor_packet_t *packet) {
    printf("{\"sensor1\":{\"x\":%d,\"y\":%d,\"z\":%d},"
           "\"sensor2\":{\"x\":%d,\"y\":%d,\"z\":%d},"
           "\"sensor3\":{\"x\":%d,\"y\":%d,\"z\":%d},"
           "\"timestamp\":%lld}\n",
           packet->accel[0][0], packet->accel[0][1], packet->accel[0][2],
           packet->accel[1][0], packet->accel[1][1], packet->accel[1][2],
           packet->accel[2][0], packet->accel[2][1], packet->accel[2][2],
           packet->timestamp_ms);
}

// Task to send serial data
void serial_send_task(void *pvParameters) {
    sensor_packet_t packet;
    while (true) {
        if (xQueueReceive(serial_queue, &packet, portMAX_DELAY)) {
            send_serial_data(&packet);
        }
    }
}

// Data collection task
void data_collection_task(void *pvParameters) {
    int16_t accel_data[3][3];
    sensor_packet_t packet;

    while (true) {
        for (uint8_t channel = 0; channel < 3; channel++) {
            if (mpu6050_read_accel(channel, &accel_data[channel][0], 
                                            &accel_data[channel][1], 
                                            &accel_data[channel][2]) == ESP_OK) {
                ESP_LOGI(TAG, "Sensor %d - X: %6d, Y: %6d, Z: %6d",
                        channel, accel_data[channel][0], accel_data[channel][1], accel_data[channel][2]);
            }
        }
        memcpy(packet.accel, accel_data, sizeof(accel_data));
        packet.timestamp_ms = esp_timer_get_time() / 1000;

        // Send to serial queue
        xQueueSend(serial_queue, &packet, 0);
        
        vTaskDelay(100 / portTICK_PERIOD_MS); // 10 Hz
    }
}

void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());

    // Initialize Serial port
    ESP_LOGI(TAG, "Serial port ready for Python communication");

    // Initialize I2C
    i2c_master_init();
    ESP_LOGI(TAG, "I2C initialized");

    // Initialize MPU6050
    for (uint8_t channel = 0; channel < 3; channel++) {
        if (mpu6050_init(channel) != ESP_OK) {
            ESP_LOGE(TAG, "MPU6050 on channel %d initialization failed!", channel);
        }
    }

    // Create queue for serial output
    serial_queue = xQueueCreate(10, sizeof(sensor_packet_t));

    // Create tasks
    xTaskCreate(data_collection_task, "data_collection", 4096, NULL, 5, NULL);
    xTaskCreate(serial_send_task, "serial_send", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "System ready. Data collection is always active");
    ESP_LOGI(TAG, "Data will be sent via serial port in JSON format");
}