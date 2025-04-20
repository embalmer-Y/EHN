#ifndef __BUFF_H__
#define __BUFF_H__

#include <stdint.h>
#include <time.h>

#include "errno.h"
#include "config.h"
#include "proto.h"

#define MSG_RXQ_CNT_DEFAULT 2
#define MSG_TXQ_CNT_DEFAULT 2

#define MSG_QUEUE_CNT_MAX 0xFFFF
#define MSG_QUEUE_BYTES_MAX 0xFFFFFFFF

struct data_src {
    struct proto_header header;
    struct proto_block blocks[];
};

struct msg_buff {
    struct msg_buff *next;
    struct msg_buff *prev;

    uint32_t id;
    uint8_t blk_cnt;
    time_t timestamp;

    void *data;
};

enum msg_queue_rx_type {
    MSG_QUEUE_RX_ZERO = 0,
    MSG_QUEUE_RX_HALF = 1,
    MSG_QUEUE_RX_FULL = 2,
    MSG_QUEUE_RX_TYPE_MAX = 3,
};

struct msg_queue_cfg {
    uint8_t qid;

    enum msg_queue_rx_type rx_type;
};

struct msg_queue_stats {
    uint16_t count;
    uint32_t bytes;
};

struct msg_queue {
    struct msg_buff *head;
    struct msg_buff *tail;

    struct msg_queue_cfg cfg;
    struct msg_queue_stats stats;
};

void msg_data_src_deinit(struct data_src *data);
struct data_src *msg_data_src_init(uint16_t size, void *usr_data);
int msg_data_src_fill(struct data_src *data, struct proto_block *usr_block);
int msg_data_src_expand(struct data_src **data, uint16_t size);
int msg_data_src_truncate(struct data_src **data, uint16_t size);
struct proto_block *msg_data_blk_get_tail(struct data_src *data);
int msg_data_src_get_blk_cnt(struct data_src *data);
void msg_data_blk_dump_hex(struct proto_block *block);
void msg_data_dump(struct data_src *data);

void msg_buff_deinit(struct msg_buff *msg_buff);
struct msg_buff *msg_buff_init(void);
int msg_buff_set_id(struct msg_buff *msg_buff, uint32_t id);
int msg_buff_set_blk_cnt(struct msg_buff *msg_buff, uint8_t cnt);
int msg_buff_set_time(struct msg_buff *msg_buff, time_t time);
int msg_buff_set_time_now(struct msg_buff *msg_buff);
int msg_buff_bind_data(struct msg_buff *msg_buff, void *data, uint8_t cnt);
int msg_buff_reset_data(struct msg_buff *msg_buff, void *data, uint8_t cnt);
void msg_buff_unbind_data(struct msg_buff *msg_buff);
uint8_t msg_buff_select_queue(struct msg_buff *msg_buff, uint8_t q_cnt);

void msg_queue_deinit(struct msg_queue *msg_queue);
struct msg_queue *msg_queue_init(uint8_t id);
int msg_queue_enqueue(struct msg_queue *msg_queue, struct msg_buff *msg_buff);
struct msg_buff *msg_queue_dequeue(struct msg_queue *msg_queue);
struct msg_buff *msg_queue_peek(struct msg_queue *msg_queue);
struct msg_buff *msg_queue_get_tail(struct msg_queue *msg_queue);
int msg_queue_get_count(struct msg_queue *msg_queue);
int msg_queue_get_data_size(struct msg_queue *msg_queue);
int msg_queue_get_id(struct msg_queue *msg_queue);
int msg_queue_set_rx_type(struct msg_queue *msg_queue, uint8_t rx_type);
#endif // __BUFF_H__
