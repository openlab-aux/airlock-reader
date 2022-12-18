#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_spiffs.h>

#include "./nfc.h"
#include "./cardmgr.h"

void card_task(void *params)
{
    // In a loop, read a card when presented.
    char card_id[32];
    esp_err_t err;

    while (true)
    {
        memset(card_id, 0x00, sizeof(card_id));

        err = nfc_get_card_id(card_id);

        if (err == ESP_OK)
        {
            ESP_LOGI("airlock-reader", "successfully read card");
            if (cardmgr_validate(card_id))
            {
                ESP_LOGI("airlock-reader", "access granted");
            }
            else
            {
                ESP_LOGI("airlock-reader", "access denied");
            }
        }

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    // Setup SPIFFS for config
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/config",
        .partition_label = "config",
        .max_files = 5,
        .format_if_mount_failed = false};
    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    ESP_ERROR_CHECK(ret);

    // Setup components
    nfc_setup();
    cardmgr_init();

    TaskHandle_t reader_task_handle = NULL;
    xTaskCreatePinnedToCore(card_task, "smartcard", 8192, NULL, tskIDLE_PRIORITY, &reader_task_handle, 1);
}
