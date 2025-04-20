#ifndef __PIPE_H__
#define __PIPE_H__

#include <stdint.h>
#include <time.h>

#include "config.h"
#include "buff.h"

#define PIPE_ID_MAX 0xFFF

enum pipe_type {
    PIPE_SYS = 0,
    PIPE_USER,
};

struct pipe {
    struct pipe *next;
    struct pipe *prev;

    uint16_t id;
    uint8_t type;
    struct pipe_ctrl_block *pcb;

#if RX_QUEUE_CNT > 0 && RX_QUEUE_CNT <= PROTO_HEADER_PRIO_CNT
    uint8_t rx_queue_cnt = RX_QUEUE_CNT;
    struct msg_queue rx_queue[RX_QUEUE_CNT];
#else
    uint8_t rx_queue_cnt = MSG_RXQ_CNT_DEFAULT;
    struct msg_queue rx_queue[MSG_RXQ_CNT_DEFAULT];
#endif

#if TX_QUEUE_CNT > 0 && TX_QUEUE_CNT <= PROTO_HEADER_PRIO_CNT
    uint8_t tx_queue_cnt = TX_QUEUE_CNT;
    struct msg_queue tx_queue[TX_QUEUE_CNT];
#else
    uint8_t tx_queue_cnt = MSG_TXQ_CNT_DEFAULT;
    struct msg_queue tx_queue[MSG_TXQ_CNT_DEFAULT];
#endif
};

struct pipe_ctrl_block {
    struct pipe *head;
    struct pipe *tail;

    uint8_t pipe_cnt;
};

int pipe_set_id(struct pipe_ctrl_block *pcb, struct pipe *pipe);
int pipe_ctrl_blk_add(struct pipe_ctrl_block *pcb, struct pipe *pipe);
#endif // __PIPE_H__
