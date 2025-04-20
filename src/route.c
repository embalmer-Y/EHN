#include <cstdint>
#include <stdlib.h>
#include <stdio.h>

#include "route.h"

struct route *route_init(void *hw_info)
{
    struct route *route = malloc(sizeof(struct route));
    if (route == NULL) {
        printf("route_init malloc error\n");
        return NULL;
    }

    route->next = NULL;
    route->prev = NULL;
    route->dst_addr = 0;
    route->dst_hw_info = hw_info;
    route->state = ROUTE_STATE_UNKNOWN;

    return route;
}

void route_deinit(struct route *route)
{
    if (route == NULL) {
        printf("route_deinit error\n");
        return;
    }

    free(route);
}

void route_set_state(struct route *route, enum route_state state)
{
    if (route == NULL) {
        printf("route_set_state error\n");
        return;
    }

    route->state = state;
}

enum route_state route_get_state(struct route *route)
{
    if (route == NULL) {
        printf("route_get_state error\n");
        return ROUTE_STATE_UNKNOWN;
    }

    return route->state;
}

int route_set_hw_info(struct route *route, void *hw_info)
{
    if (route == NULL) {
        printf("route_set_hw_info error\n");
        return -1;
    }

    route->dst_hw_info = hw_info;

    return 0;
}

struct route_ctrl_block *route_ctrl_blk_init(void)
{
    struct route_ctrl_block *route_ctrl_blk = malloc(sizeof(struct route_ctrl_block));
    if (route_ctrl_blk == NULL) {
        printf("route_ctrl_blk_init malloc error\n");
        return NULL;
    }

    route_ctrl_blk->head = NULL;
    route_ctrl_blk->tail = NULL;

    return route_ctrl_blk;
}

void route_ctrl_blk_deinit(struct route_ctrl_block *route_ctrl_blk)
{
    if (route_ctrl_blk == NULL) {
        printf("route_ctrl_blk_deinit error\n");
        return;
    }

    struct route *route = route_ctrl_blk->head;
    while (route != NULL) {
        struct route *next = route->next;
        route_deinit(route);
        route = next;
    }

    free(route_ctrl_blk);
}

int route_ctrl_blk_add_route(struct route_ctrl_block *route_ctrl_blk, struct route *route)
{
    if (route_ctrl_blk == NULL) {
        printf("route_ctrl_blk_add_route error\n");
        return -1;
    }

    if (route_ctrl_blk->head == NULL) {
        route_ctrl_blk->head = route;
        route_ctrl_blk->tail = route;
        return 0;
    }

    route_ctrl_blk->tail->next = route;
    route->prev = route_ctrl_blk->tail;
    route_ctrl_blk->tail = route;

    route_ctrl_blk->route_cnt++;

    return 0;
}

struct route *route_ctrl_blk_get_route(struct route_ctrl_block *route_ctrl_blk, uint32_t dst_addr)
{
    if (route_ctrl_blk == NULL) {
        printf("route_ctrl_blk_get_route error\n");
        return NULL;
    }

    struct route *route = route_ctrl_blk->head;
    while (route != NULL) {
        if (route->dst_addr == dst_addr) {
            return route;
        }
        route = route->next;
    }

    return NULL;
}

int route_ctrl_blk_del_route(struct route_ctrl_block *route_ctrl_blk, uint32_t dst_addr)
{
    if (route_ctrl_blk == NULL) {
        printf("route_ctrl_blk_del_route error\n");
        return -1;
    }

    struct route *route = route_ctrl_blk_get_route(route_ctrl_blk, dst_addr);
    if (route == NULL) {
        printf("route_ctrl_blk_del_route error\n");
        return -1;
    }

    if (route_ctrl_blk->head == route) {
        route_ctrl_blk->head = route->next;
    }

    if (route_ctrl_blk->tail == route) {
        route_ctrl_blk->tail = route->prev;
    }

    if (route->prev != NULL) {
        route->prev->next = route->next;
    }

    if (route->next != NULL) {
        route->next->prev = route->prev;
    }

    route_ctrl_blk->route_cnt--;
    route_deinit(route);

    return 0;
}
