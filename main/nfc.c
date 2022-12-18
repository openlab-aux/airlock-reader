#include "./nfc.h"
#include <stdio.h>
#include "esp_log.h"
#include "esp_spiffs.h"
#include "ST25R3911B.h"
#include "iso14443a.h"
#include "desfireaes.h"

st25r3911b_t nfc_device;
iso14443a_ctx_t nfc_isoctx = {
    .device = (void *)&nfc_device,
    .rx_fn = st25r3911b_receive,
    .tx_fn = st25r3911b_transmit_no_crc,
    .tx_fn_crc = st25r3911b_transmit_crc,
    .tx_reqa_fn = st25r3911b_transmit_reqa,
};
df_t nfc_desfire;

uint8_t nfc_desfire_aid[3];
uint8_t nfc_desfire_key[16];

esp_err_t nfc_setup()
{
    // Read Config from SPIFFS
    FILE *fh;

    fh = fopen("/config/aid", "r");
    fread(nfc_desfire_aid, 3, 1, fh);
    fclose(fh);

    fh = fopen("/config/key", "r");
    fread(nfc_desfire_key, 16, 1, fh);
    fclose(fh);

    // Initialize Reader
    st25r3911b_init(&nfc_device);
    if (!st25r3911b_check(&nfc_device))
    {
        ESP_LOGE("nfc", "init failed for nfc module");
        return ESP_FAIL;
    }
    st25r3911b_powerup(&nfc_device);
    st25r3911b_set_mode_nfca(&nfc_device);

    // Initialize DESFireAES
    df_init(&nfc_desfire, (void *)&nfc_isoctx, iso14443a_dx);

    ESP_LOGI("nfc", "nfc module startup sequence completed.");
    return ESP_OK;
}

void nfc_send_hlta()
{
    ESP_LOGI("nfc", "send PICC to sleep");
    uint8_t hlta_frame[2] = {0x50, 0x00};
    st25r3911b_transmit_crc((void *)&nfc_device, 2, hlta_frame);
}

esp_err_t nfc_get_card_id(char *card_id)
{
    uint8_t uid[10];
    uint8_t uidlen = iso14443a_anticollision_loop(&nfc_isoctx, uid);
    if (uidlen > 0)
    {
        ESP_LOGI("nfc", "Found UID");
        ESP_LOG_BUFFER_HEXDUMP("nfc", uid, uidlen, ESP_LOG_INFO);
    }
    else
    {
        return ESP_FAIL;
    }

    iso14443a_request_ats(&nfc_isoctx);

    const char *ret;
    ret = df_select_application(&nfc_desfire, (const unsigned char *)nfc_desfire_aid);
    if (ret != NULL)
    {
        ESP_LOGE("nfc", "error selecting application: %s", ret);
        nfc_send_hlta();
        return ESP_FAIL;
    }

    ret = df_authenticate(&nfc_desfire, 0x01, nfc_desfire_key);
    if (ret != NULL)
    {
        ESP_LOGE("nfc", "error authenticating with picc: %s", ret);
        nfc_send_hlta();
        return ESP_FAIL;
    }

    ret = df_read_data(&nfc_desfire, 0x00, DF_MODE_ENC, 0x00, 32, (unsigned char *)card_id);
    if (ret != NULL)
    {
        ESP_LOGE("nfc", "error reading file: %s", ret);
        nfc_send_hlta();
        return ESP_FAIL;
    }

    ESP_LOG_BUFFER_HEXDUMP("nfc", card_id, 32, ESP_LOG_INFO);

    nfc_send_hlta();
    return ESP_OK;
}