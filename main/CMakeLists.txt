idf_component_register(SRCS "cardmgr.c" "main.c" "nfc.c" "cardmgr.c"
                    INCLUDE_DIRS "."
                    REQUIRES "freertos" "spiffs" "esp-iso14443a" "esp-st25r3911b" "esp-desfireaes")

spiffs_create_partition_image(config spiffs FLASH_IN_PROJECT)