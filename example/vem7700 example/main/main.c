/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#include "sdkconfig.h"
#include <stdio.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "soft_i2c_master.h"
#include "veml7700.h"

/* GPIO Pins for I2C */
#define I2C_SCL_PIN             4  // GPIO pin for SCL
#define I2C_SDA_PIN             5  // GPIO pin for SDA

soft_i2c_master_bus_t i2c_bus = NULL;
const char* EXAMPLE_TAG = "soft_i2c_master";

void i2c_bus_init(soft_i2c_master_bus_t bus){
    /* Configure software I2C with pins and frequency */
    soft_i2c_master_config_t i2c_config = {
        .scl_pin = I2C_SCL_PIN,
        .sda_pin = I2C_SDA_PIN,
        .freq = SOFT_I2C_100KHZ          
        };

    /* Initialize the software I2C bus */
    esp_err_t ret = soft_i2c_master_new(&i2c_config, &i2c_bus);
}

void i2c_scan_devices(soft_i2c_master_bus_t bus) {
    ESP_LOGI(TAG, "Scanning for I2C devices...");

    uint8_t dummy_data = 0x00; // Dummy data to write

    for (uint8_t address = 1; address < 127; ++address) {
        esp_err_t ret = soft_i2c_master_write(bus, address, &dummy_data, 1); // Write dummy data
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Found I2C device at address 0x%02X", address);
        }
    }

    ESP_LOGI(TAG, "I2C scan complete.");
}

void app_main(void)
{
    //setup an I2C bus
    i2c_bus_init(i2c_bus);

    // Initialize the VEML7700 sensor
    esp_err_t ret = ESP_OK;
    ret = veml7700_init(i2c_bus);
    
    //A value to store lux readings in
    float lux = 0;
    while(1){

        //get device ID information
        ret = veml7700_read_device_id(i2c_bus, &lux);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Error reading als light");
        }

        //get light sensor data from ALS sensor
        ret = veml7700_read_als(i2c_bus, &lux);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Error reading als light");
        }

        //get light sensor data from white light sensor
        ret = veml7700_read_white(i2c_bus, &lux);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Error reading als light");
        }

        vTaskDelay(pdMS_TO_TICKS(4000)); // Wait 1 second between readings

    }
}
