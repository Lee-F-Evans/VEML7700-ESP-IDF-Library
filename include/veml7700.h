#include "soft_i2c_master.h"

/*
Documentation:
https://www.vishay.com/docs/84286/veml7700.pdf
https://www.vishay.com/docs/84323/designingveml7700.pdf
*/

/* I2C device address for VEML7700 */
#define VEML7700_ADDRESS 0x10

#define TAG "VEML7700"

/* Command Register addresses*/
#define VEML7700_ALS_CONF   0x00 // ALS Configuration Register
#define VEML7700_ALS_WH     0x01 // High threshold window setting 
#define VEML7700_ALS_WL     0x02 // Low threshold window setting
#define VEML7700_PS         0x03 // Power saving
#define VEML7700_ALS        0x04 // ALS Data Register
#define VEML7700_WHITE      0x05 // WHITE Light Data Register
#define VEML7700_ALS_INT    0x06 // Device ID register
#define VEML7700_ID         0x07 // Device ID register

/*Conversion factor determined from application notes, page 5. 
 https://www.vishay.com/docs/84323/designingveml7700.pdf

From the provided table on page 5 of AP
with integration time = 25ms, gain = 1/8, our conversion factor = 2.1504Lux/count.*/
#define DEFAULT_LUX_CONV_FACTOR 2.1504

esp_err_t veml7700_init(soft_i2c_master_bus_t bus);
esp_err_t veml7700_read_als(soft_i2c_master_bus_t bus, float* lux);
esp_err_t veml7700_read_white(soft_i2c_master_bus_t bus, float* lux);
esp_err_t veml7700_read_device_id(soft_i2c_master_bus_t bus, uint16_t* device_id);
void test_veml7700_register_write(soft_i2c_master_bus_t bus);
