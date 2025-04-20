#ifndef __ROUTE_H__
#define __ROUTE_H__

#include <stdint.h>

#include "config.h"
#include "proto.h"
#include "buff.h"

enum route_state {
    ROUTE_STATE_UNKNOWN = 0,
    ROUTE_STATE_UP,
    ROUTE_STATE_ACTIVE,
    ROUTE_STATE_DOWN, 
};

struct route {
    struct route *next;
    struct route *prev;

    uint32_t dst_addr;
    enum route_state state;
    void *dst_hw_info;
};

struct route_ctrl_block {
    struct route *head;
    struct route *tail;

    uint32_t route_cnt;
};

struct route *route_init(void *hw_info);
void route_deinit(struct route *route);
void route_set_state(struct route *route, enum route_state state);
enum route_state route_get_state(struct route *route);
int route_set_hw_info(struct route *route, void *hw_info);

struct route_ctrl_block *route_ctrl_blk_init(void);
void route_ctrl_blk_deinit(struct route_ctrl_block *route_ctrl_blk);
int route_ctrl_blk_add_route(struct route_ctrl_block *route_ctrl_blk, struct route *route);
struct route *route_ctrl_blk_get_route(struct route_ctrl_block *route_ctrl_blk, uint32_t dst_addr);
int route_ctrl_blk_del_route(struct route_ctrl_block *route_ctrl_blk, uint32_t dst_addr);

#endif // __ROUTE_H__
