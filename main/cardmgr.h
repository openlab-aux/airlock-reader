#ifndef CARDMGR_H
#define CARDMGR_H

#include "esp_err.h"

typedef struct cardmgr_node
{
    char card_id[32];
    struct cardmgr_node *next;

} cardmgr_node_t;

esp_err_t cardmgr_init();
bool cardmgr_validate(char *);

#endif // CARDMGR_H