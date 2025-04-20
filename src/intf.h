#ifndef __INTF_H__
#define __INTF_H__

#include <stdint.h>
#include "pthread.h"

#include "config.h"
#include "proto.h"
#include "buff.h"
#include "route.h"

enum hw_type {
    HW_TYPE_UNKNOWN = 0,
    HW_TYPE_NET,
    HW_TYPE_BT,
    HW_TYPE_USB,
    HW_TYPE_IIC,
    HW_TYPE_SPI,
    HW_TYPE_CAN,
    HW_TYPE_UART,
};

enum intf_status {
    INTF_STATUS_UNKNOWN = 0,
    INTF_STATUS_INIT,
    INTF_STATUS_DEINIT,
    INTF_STATUS_RUNNING,
};

struct interface_ops {
    int (*init)(struct interface *intf);
    void (*deinit)(struct interface *intf);
    int (*xmit)(struct interface *intf, uint8_t *pkt, void *arg);
    int (*recv)(struct interface *intf, uint8_t *pkt, void *arg); // must not blocking.
    int (*ioctl)(struct interface *intf, uint8_t cmd, void *arg);
    // int (*rx_handler)(struct msg_buff *msg);
};

struct interface_config {
    char intf_name[100];
    enum hw_type hw_type;

    uint16_t budget;

    /* pthread cond */
    pthread_cond_t cond;
    pthread_mutex_t lock;
    // ...
};

struct interface_info {
    uint8_t intf_id;
    uint32_t rx_bytes;
    uint32_t tx_bytes;
    enum intf_status status;
    pthread_t thread;
    // ...
};

struct interface {
    struct interface *next;
    struct interface *prev;

    struct interface_info info;
    struct interface_config *config;
    struct interface_ops *ops;
    struct route_ctrl_block *rcb;
};

struct interface_ctrl_block {
    struct interface *if_ctrl_head;
    struct interface *if_ctrl_tail;

    uint8_t if_cnt;
};

#endif // __INTF_H__
