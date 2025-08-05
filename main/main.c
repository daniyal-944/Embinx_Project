/*****************************************************************************
 *                                                                           *
 *  Copyright 2018 Simon M. Werner                                           *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *      http://www.apache.org/licenses/LICENSE-2.0                           *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"
#include "esp_task_wdt.h"

#include "driver/i2c.h"

#include "ahrs.h"
#include "mpu9250.h"
#include "calibrate.h"
#include "common.h"

#define I2C_MASTER_NUM         I2C_NUM_0
#define I2C_MASTER_SCL_IO      22    // Change according to your wiring
#define I2C_MASTER_SDA_IO      21    // Change according to your wiring
#define I2C_MASTER_FREQ_HZ     400000
#define MPU9250_ADDR           0x68  // AD0 low = 0x68, AD0 high = 0x69
#define WHO_AM_I_REG           0x75

static const char *TAG = "WHO_AM_I_TEST";

esp_err_t i2c_master_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
    return i2c_driver_install(I2C_MASTER_NUM, conf.mode,
                               0, 0, 0);

  return ESP_OK;
}

esp_err_t mpu9250_read_whoami(uint8_t *whoami)
{
    uint8_t reg = WHO_AM_I_REG;

    // Write register address
    esp_err_t ret = i2c_master_write_to_device(I2C_MASTER_NUM,
                                               MPU9250_ADDR,
                                               &reg, 1,
                                               pdMS_TO_TICKS(1000));
    if (ret != ESP_OK) return ret;

    // Read one byte from that register
    return i2c_master_read_from_device(I2C_MASTER_NUM,
                                       MPU9250_ADDR,
                                       whoami, 1,
                                       pdMS_TO_TICKS(1000));
}

void app_main(void)
{
    ESP_ERROR_CHECK(i2c_master_init());

    uint8_t id;
    esp_err_t ret = mpu9250_read_whoami(&id);

    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "WHO_AM_I = 0x%02X", id);
        if (id == 0x71) {
            ESP_LOGI(TAG, "MPU9250 detected successfully!");
        } else {
            ESP_LOGW(TAG, "Unexpected WHO_AM_I value");
        }
    }
    else
    {
        ESP_LOGE(TAG, "Failed to read WHO_AM_I register");
    }

    // Stop here
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

