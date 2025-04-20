#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "intf.h"
#include "buff.h"
#include "proto.h"
#include "route.h"

struct interface *intf_init(void)
{
    struct interface *intf = malloc(sizeof(struct interface));
    if (intf == NULL) {
        printf("intf_init malloc error\n");
        return NULL;
    }

    intf->next = NULL;
    intf->prev = NULL;
    intf->config = NULL;
    intf->info = (struct interface_info){0};
    intf->ops = NULL;

    return intf;
}

void intf_deinit(struct interface *intf)
{
    if (intf == NULL) {
        printf("intf_deinit error\n");
        return;
    }

    free(intf);
}

int intf_register(struct interface_ctrl_block *intf_ctrl_blk, struct interface_config *config, 
                  struct interface_ops *ops)
{
    struct interface *intf;
    struct route *route;
    int ret;

    if (intf_ctrl_blk == NULL) {
        printf("intf_register error\n");
        return -1;
    }

    intf = intf_init();
    if (intf == NULL) {
        printf("intf_register error, intf_init() failed");
        return -1;
    }

    if (config != NULL && ops != NULL) {
        intf->config = config;
        intf->ops = ops;
    }

    if (intf_ctrl_blk->if_ctrl_head == NULL) {
        intf_ctrl_blk->if_ctrl_head = intf;
        intf_ctrl_blk->if_ctrl_tail = intf;
        intf->info.intf_id = 0;
    } else {
        intf->info.intf_id = intf_ctrl_blk->if_ctrl_tail->info.intf_id + 1;
        intf_ctrl_blk->if_ctrl_tail->next = intf;
        intf->prev = intf_ctrl_blk->if_ctrl_tail;
        intf_ctrl_blk->if_ctrl_tail = intf;
    }

    intf_ctrl_blk->if_cnt++;
    ret = intf->ops->init(intf);
    if (ret != 0) {
        printf("intf_register error, init() failed");
        return -1;
    }

    intf->rcb = route_ctrl_blk_init();
    if (intf->rcb == NULL) {
        printf("intf_register error, route_ctrl_blk_init() failed");
        return -1;
    }

    route = route_init((void *)config);
    if (route == NULL) {
        printf("intf_register error, route_init() failed");
        return -1;
    }

    ret = route_ctrl_blk_add_route(intf->rcb, route);
    if (ret != 0) {
        printf("intf_register error, route_ctrl_blk_add_route() failed");
        return -1;
    }

    route_set_state(route, ROUTE_STATE_ACTIVE);

    intf->info.status = INTF_STATUS_RUNNING;

    return 0;
}

int intf_unregister(struct interface_ctrl_block *intf_ctrl_blk, uint8_t intf_id)
{
    if (intf_ctrl_blk == NULL) {
        printf("intf_unregister error\n");
        return -1;
    }

    struct interface *intf = intf_ctrl_blk->if_ctrl_head;
    while (intf != NULL) {
        if (intf->info.intf_id == intf_id) {
            break;
        }
        intf = intf->next;
    }

    if (intf == NULL) {
        printf("intf_unregister error, intf_id: %d\n", intf_id);
        return -1;
    }

    intf->ops->deinit(intf);

    if (intf_ctrl_blk->if_ctrl_head == intf) {
        intf_ctrl_blk->if_ctrl_head = intf->next;
    }

    if (intf_ctrl_blk->if_ctrl_tail == intf) {
        intf_ctrl_blk->if_ctrl_tail = intf->prev;
    }

    if (intf->prev != NULL) {
        intf->prev->next = intf->next;
    }

    if (intf->next != NULL) {
        intf->next->prev = intf->prev;
    }

    intf_ctrl_blk->if_cnt--;
    intf_deinit(intf);

    return 0;
}

int intf_xmit(struct interface *intf, struct msg_buff *msg)
{
    struct route *route;
    struct proto_header *header;
    if (intf == NULL) {
        printf("intf_xmit error\n");
        return -1;
    }

    if (intf->ops == NULL) {
        printf("intf_xmit error, ops is NULL\n");
        return -1;
    }

    header = (struct proto_header *)msg->data;

    route = route_ctrl_blk_get_route(intf->rcb, header->dst_id);
    if (route == NULL) {
        printf("intf_xmit error, route_ctrl_blk_get_route() failed");
        return -1;
    }

    return intf->ops->xmit(intf, (uint8_t *)msg->data, route->dst_hw_info);
}

int intf_recv(struct interface *intf, struct msg_buff *msg)
{
    struct proto_header *header;
    struct route *route;
    void *hw_info;
    int ret;

    if (intf == NULL) {
        printf("intf_recv error\n");
        return -1;
    }

    if (intf->ops == NULL) {
        printf("intf_recv error, ops is NULL\n");
        return -1;
    }

    ret = intf->ops->recv(intf, (uint8_t *)msg->data, hw_info);
    if (ret != 0) {
        printf("intf_recv error, recv() failed");
        return -1;
    }

    ret = msg_buff_set_time_now(msg);
    if (ret != 0) {
        printf("intf_recv error, msg_buff_set_time_now() failed");
        return -1;
    }

    header = (struct proto_header *)msg->data;
    route = route_ctrl_blk_get_route(intf->rcb, header->dst_id);
    if (route == NULL) {
        route = route_init(hw_info);
        if (route == NULL) {
            printf("intf_recv error, route_init() failed");
            return -1;
        }

        ret = route_ctrl_blk_add_route(intf->rcb, route);
        if (ret != 0) {
            printf("intf_recv error, route_ctrl_blk_add_route() failed");
            return -1;
        }
    }

    ret = route_set_hw_info(route, hw_info);
    if (ret != 0) {
        printf("intf_recv error, route_set_hw_info() failed");
        return -1;
    }

    route_set_state(route, ROUTE_STATE_ACTIVE);

    return 0;
}

struct interface_ctrl_block *intf_ctrl_blk_init(void)
{
    struct interface_ctrl_block *intf_ctrl_blk = malloc(sizeof(struct interface_ctrl_block));
    if (intf_ctrl_blk == NULL) {
        printf("intf_ctrl_blk_init malloc error\n");
        return NULL;
    }

    intf_ctrl_blk->if_ctrl_head = NULL;
    intf_ctrl_blk->if_ctrl_tail = NULL;
    intf_ctrl_blk->if_cnt = 0;

    return intf_ctrl_blk;
}

void intf_ctrl_blk_deinit(struct interface_ctrl_block *intf_ctrl_blk)
{
    if (intf_ctrl_blk == NULL) {
        printf("intf_ctrl_blk_deinit error\n");
        return;
    }

    struct interface *intf = intf_ctrl_blk->if_ctrl_head;
    while (intf != NULL) {
        struct interface *next = intf->next;
        intf_deinit(intf);
        intf = next;
    }

    free(intf_ctrl_blk);
}

uint8_t intf_ctrl_blk_get_if_cnt(struct interface_ctrl_block *intf_ctrl_blk)
{
    if (intf_ctrl_blk == NULL) {
        printf("intf_ctrl_blk_get_if_cnt error\n");
        return 0;
    }

    return intf_ctrl_blk->if_cnt;
}
