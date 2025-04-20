#ifndef __MANAGER_H__
#define __MANAGER_H__

#include <stdint.h>
#include <time.h>
#include <pthread.h>

#include "buff.h"
#include "config.h"
#include "errno.h"
#include "route.h"
#include "table.h"
#include "intf.h"
#include "pipe.h"

struct manager_config {
    uint8_t interface_cnt;
};

struct manager {
    struct interface_ctrl_block ifcb;
    struct pipe_ctrl_block pcb;
    struct route_ctrl_block rcb;
    struct msg_table msg_table;
    struct manager_config config;

    pthread_t tx_thread;
    pthread_t rx_thread;
};

#endif // __MANAGER_H__
