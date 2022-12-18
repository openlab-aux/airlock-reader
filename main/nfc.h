#ifndef NFC_H
#define NFC_H

#include "esp_err.h"

esp_err_t nfc_setup();
esp_err_t nfc_get_card_id(char *);

#endif // NFC_H