#include <stdlib.h>

#include "manager.h"

struct manager *manager_init(void)
{
    struct manager *manager = malloc(sizeof(struct manager));
    if (manager == NULL) {
        return NULL;
    }

    return manager;
}

void manager_deinit(struct manager *manager)
{
    if (manager == NULL) {
        return;
    }

    free(manager);
}

void manager_settings(struct manager *manager, struct manager_config *config)
{
    if (manager == NULL || config == NULL) {
        return;
    }

    manager->config.interface_cnt = config->interface_cnt;
}

void manager_rx(struct manager *manager)
{
    if (manager == NULL) {
        return;
    }
    while (1) {
        // if
    }
}