#include "math.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"

#include "soft_i2c_master.h"
#include "veml7700.h"

/*
Currently only convienient to use with max input range(0-140k Lux) 
but can be configured with some bit math and using the veml7700_write_conf function. 
TODO:   - add different gain options and a table for easy config changes between them
        - add interrupt functionality
        - add power saving functionality

To use:
1. setup an I2C bus using ESP-IDF provided soft_i2c_master files
2. Call veml7700_init for default config setup
3. Use veml7700_read_als() or veml7700_read_white() for getting lux
*/


/* DEFAULT CONFIG
15:13   = 000   (reserved)
12:11   = 10    ALS_GAIN (ALS gain, 00 = 1, 01 = 2, 10 = 1/8, 11 = 1/4)
10      = 0     (reserved)
9:6     = 1100  (ALS_IT, integration time) 25ms
5:4     = 00    (ALS_PERS, persistence protect number)
3:2     = 00    (reserved)
1       = 0     (ALS_INT_EN, interrupt enable, 0 = off)
0       = 0     (ALS_SD, shutdown. 0 = on, 1 = off) 
*/
uint8_t default_config_data[] = {
    VEML7700_ALS_CONF,       // Register address
    0x00,                      // LSB of configuration (default settings) 100ms delay for reading
    0x13                       // MSB of configuration
};

float lux_conversion_factor = DEFAULT_LUX_CONV_FACTOR;

/*
Details:    Function to initialize and configure the VEML7700. 
            Defaults to lowest resolution and highest reading range (0-140k Lux) 

Parameters: soft_i2c_master_bus_t instance
Returns: esp_err_t
*/
esp_err_t veml7700_init(soft_i2c_master_bus_t bus) {
    esp_err_t ret = ESP_OK;

    ret = soft_i2c_master_write(bus, VEML7700_ADDRESS, &default_config_data, 3);
    ESP_RETURN_ON_ERROR(ret, TAG, "Failed to write configuration to VEML7700");
    ESP_LOGI(TAG, "VEML7700 initialized successfully");
    return ret;
}

/*
Details:    Function to initialize and configure the VEML7700. 
            Defaults to lowest resolution and highest reading range (0-140k Lux) 

Parameters: soft_i2c_master_bus_t instance
Returns: esp_err_t
*/
esp_err_t veml7700_write_conf(soft_i2c_master_bus_t bus) {
    esp_err_t ret = ESP_OK;
    /* Register Description
    15:13   = (reserved)
    12:11   = ALS_GAIN (ALS gain, 00 = 1, 01 = 2, 10 = 1/8, 11 = 1/4)
    10      = (reserved)
    9:6     = (ALS_IT, integration time)
    5:4     = (ALS_PERS, persistence protect number)
    3:2     = (reserved)
    1       = (ALS_INT_EN, interrupt enable, 0 = off)
    0       = (ALS_SD, shutdown. 0 = on, 1 = off) 
    */
    uint8_t config_data[] = {
        VEML7700_ALS_CONF,       // Register address
        0x00,                      // LSB of configuration (default settings) 100ms delay for reading
        0x13                       // MSB of configuration
    };

    ret = soft_i2c_master_write(bus, VEML7700_ADDRESS, &config_data, 3);
    ESP_RETURN_ON_ERROR(ret, TAG, "Failed to write configuration to VEML7700");
    ESP_LOGI(TAG, "VEML7700 initialized successfully");
    return ret;
}

/*
Details: This register responds well to only ~500-600nm wavelengths. See datasheet for details
You must pass a pointer to a float for the new lux value to be written to. 

Parameters: soft_i2c_master_bus_t instance, float* lux 
Returns: esp_err_t
*/
esp_err_t veml7700_read_als(soft_i2c_master_bus_t bus, float* lux) {
    esp_err_t ret = ESP_OK;
    uint8_t als_register = VEML7700_ALS;  // Register to read
    uint8_t als_data[2] = {0};

    // send device address with write bit, send command register you want
    // send device address with read bit, Read 2 bytes of data from the register
    ret = soft_i2c_master_write_read(bus, VEML7700_ADDRESS, &als_register, 1, &als_data, 2);
    // ret = soft_i2c_master_read(bus, VEML7700_ADDRESS, als_data, 2);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read ALS data: %s", esp_err_to_name(ret));
        return ret;
    }

    // data is recieved in the opposite order, so re organize LSByte and MSByte 
    uint16_t raw_data = (als_data[1] << 8) | als_data[0];

    // Optional print raw data in hex
    ESP_LOGI(TAG, "Raw ALS Data: 0x%04X", raw_data);

    // Convert raw data to lux. read data sheet for more info.
    *lux = raw_data * lux_conversion_factor;
    ESP_LOGI(TAG, "ALS Light before: %.2f lux", *lux);

    // if lux exceeds 1000, apply the correction formula mentioned in application notes page 5.
    if (*lux >1000){
        double lux_in = *lux;
        float lux_out = 0;

        lux_out = (((6.0135e-13 * lux_in - 9.3924e-9) * lux_in + 8.1488e-5) * lux_in + 1.0023) * lux_in;

        // Values are not reliable above 140k Lux, so print a constant 140,000
        if(lux_out > 140000){
            *lux = 140000;
        } else {
            *lux = lux_out;
        }
    }

    ESP_LOGI(TAG, "ALS Light after: %.2f lux", *lux);

    return ESP_OK;
}

/*
Details: This register has a higher response to a wider range of wavelengths. See datasheet for details.
You must pass a pointer to a float for the new lux value to be written to. 

Parameters: soft_i2c_master_bus_t instance, float* lux 
Returns: esp_err_t
*/
esp_err_t veml7700_read_white(soft_i2c_master_bus_t bus, float* lux) {
    esp_err_t ret = ESP_OK;
    uint8_t als_register = VEML7700_WHITE;  // Register to read
    uint8_t als_data[2] = {0};

    // send device address with write bit, send command register you want
    // send device address with read bit, Read 2 bytes of data from the register
    ret = soft_i2c_master_write_read(bus, VEML7700_ADDRESS, &als_register, 1, &als_data, 2);
    // ret = soft_i2c_master_read(bus, VEML7700_ADDRESS, als_data, 2);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read ALS data: %s", esp_err_to_name(ret));
        return ret;
    }

    // data is recieved in the opposite order, so re organize LSByte and MSByte 
    uint16_t raw_data = (als_data[1] << 8) | als_data[0];

    // Optional print raw data in hex
    ESP_LOGI(TAG, "Raw White Data: 0x%04X", raw_data);

    // Convert raw data to lux. read data sheet for more info.
    *lux = raw_data * lux_conversion_factor;
    ESP_LOGI(TAG, "White Light: %.2f lux", *lux);

    return ESP_OK;
}

/*
Details: This function returns device ID for the VEML7700 chip.
Use this for debug or tracking your sensors.

Parameters: soft_i2c_master_bus_t instance, uint16_t* device_id 
Returns: esp_err_t
*/
esp_err_t veml7700_read_device_id(soft_i2c_master_bus_t bus, uint16_t* device_id) {
    esp_err_t ret = ESP_OK;
    uint8_t id_register = VEML7700_ID;
    uint8_t id_data[2] = {0};

    /* DEFAULT CONFIG
    15:8   = Slave address option code. For slave address option 0x20: 11000100 = 0xC4, For slave address option 0x90: 11010100 = 0xD4
    7:0    = Device ID code    
    */

    // send device address with write bit, send command register you want
    // send device address with read bit, Read 2 bytes of data from the register
    ret = soft_i2c_master_write_read(bus, VEML7700_ADDRESS, &id_register, 1, &id_data, 2);
    // ret = soft_i2c_master_read(bus, VEML7700_ADDRESS, als_data, 2);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read ID: %s", esp_err_to_name(ret));
        return ret;
    }

    // Combine MSB and LSB into a 16-bit value
    *device_id = (id_data[1] << 8) | id_data[0];
    ESP_LOGI(TAG, "VEML7700 Device ID: 0x%04X", *device_id);

    return ESP_OK;
}