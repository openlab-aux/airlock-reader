#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "esp_err.h"
#include "esp_log.h"

#include "./cardmgr.h"

cardmgr_node_t *cardmgr_cards;

esp_err_t cardmgr_reset_cardids()
{
    // Reset cardmgr nodes
    cardmgr_cards = (struct cardmgr_node *)NULL;

    return ESP_OK;
}

esp_err_t cardmgr_read_cardids(char *path)
{
    cardmgr_reset_cardids();

    int ncards = 0;
    char buf[34];
    int fd = open(path, NULL, "r");
    if (fd < 0)
    {
        ESP_LOGE("cardmgr", "error opening file %s: %d", path, errno);
        return ESP_ERR_NOT_FOUND;
    }

    cardmgr_node_t *old = cardmgr_cards;

    for (;;)
    {
        size_t rbytes = read(fd, buf, 33);
        ESP_LOGD("cardmgr", "rbytes=%d", rbytes);
        if (rbytes == 33)
        {
            cardmgr_node_t *new = malloc(sizeof(cardmgr_node_t));
            new->next = (cardmgr_node_t *)NULL;
            memcpy(new->card_id, buf, 32);

            if (old == (cardmgr_node_t *)NULL)
            {
                cardmgr_cards = new;
            }
            else
            {
                old->next = new;
            }

            ncards += 1;
            old = new;
        }
        else
        {
            break;
        }
    }

    ESP_LOGI("cardmgr", "read %d cards from %s", ncards, path);

    close(fd);
    return ESP_OK;
}

bool cardmgr_validate(char *card_id)
{
    ESP_LOG_BUFFER_HEX_LEVEL("cardmgr_user", card_id, 32, ESP_LOG_INFO);
    cardmgr_node_t *next = cardmgr_cards;
    while (next != (cardmgr_node_t *)NULL)
    {
        ESP_LOG_BUFFER_HEX_LEVEL("cardmgr_valid", next->card_id, 32, ESP_LOG_INFO);

        if (!memcmp((void *)next->card_id, (void *)card_id, 32))
        {
            return true;
        }

        next = next->next;
    }

    return false;
}

esp_err_t cardmgr_init()
{
    cardmgr_read_cardids("/config/cid");
    return ESP_OK;
}