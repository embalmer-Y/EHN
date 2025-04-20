#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "errno.h"
#include "proto.h"
#include "buff.h"

void msg_data_src_deinit(struct data_src *data)
{
    if (data == NULL) {
        return;
    }
    
    free(data);
}

struct data_src *msg_data_src_init(uint16_t size, void *usr_data)
{
    struct data_src *data;
    int ret;

    if (usr_data != NULL) {
        return (struct data_src *)usr_data;
    }

    if (size < sizeof(struct data_src)) {
        return NULL;
    }

    data = (struct data_src *)malloc(size);
    if (data == NULL) {
        return NULL; 
    }

    memset(data, 0, size);
    ret = proto_header_set_len(&data->header, size);
    if (ret != ERR_SUCCESS) {
        return NULL; 
    }

    return data;
}

int msg_data_src_fill(struct data_src *data, struct proto_block *usr_block)
{
    struct proto_block *block;
    uint16_t blk_size;
    uint8_t *data_p;

    if (data == NULL || usr_block == NULL) {
        return -ERR_INVALID_ARG;
    } 

    if (usr_block->len > data->header.len) {
        return -ERR_OUT_OF_RANGE;
    }

    data_p = (uint8_t *)(data->blocks);
    block = (struct proto_block *)data_p;
    while (block->len > 0) {
        data_p += block->len + sizeof(struct proto_block);
        block = (struct proto_block *)data_p;
    }

    blk_size = block - data->blocks + 4 + usr_block->len;
    if (blk_size > data->header.len) {
        return -ERR_OUT_OF_RANGE;
    }
    
    block->type = usr_block->type;  
    block->len = usr_block->len;
    memcpy(block->data, usr_block->data, usr_block->len);;

    return ERR_SUCCESS;
}

int msg_data_src_expand(struct data_src **data, uint16_t size)
{
    int ret;

    if (data == NULL || !size) {
        return -ERR_INVALID_ARG; 
    }

    *data = (struct data_src *)realloc(*data, (*data)->header.len + size);
    if (*data == NULL) {
        return -ERR_NO_MEM;
    }

    // (*data)->header.len += size;
    ret = proto_header_set_len(&(*data)->header, (*data)->header.len + size);
    if (ret!= ERR_SUCCESS) {
        return ret; 
    }

    return ERR_SUCCESS;
}

int msg_data_src_truncate(struct data_src **data, uint16_t size)
{
    int ret;

    if (data == NULL) {
        return -ERR_INVALID_ARG;
    }

    if (size > (*data)->header.len) {
        return -ERR_OUT_OF_RANGE;
    }

    // (*data)->header.len -= size;
    ret = proto_header_set_len(&(*data)->header, (*data)->header.len - size);
    if (ret!= ERR_SUCCESS) {
        return ret;
    }

    if ((*data)->header.len < sizeof(struct data_src)) {
        return -ERR_OUT_OF_RANGE;
    }

    (*data) = (struct data_src *)realloc(*data, (*data)->header.len);
    if (*data == NULL) {
        return -ERR_NO_MEM; 
    }

    return ERR_SUCCESS;
}

int msg_data_src_get_blk_cnt(struct data_src *data)
{
    struct proto_block *block;
    uint8_t *data_p;
    uint16_t size;
    int cnt;
    
    if (data == NULL) {
        return -ERR_INVALID_ARG; 
    }

    size = data->header.len - sizeof(struct proto_header);
    data_p = (uint8_t *)(data->blocks);
    block = (struct proto_block *)data_p;
    cnt = 0;
    while (size > 0) {
        if (block->len == 0)
            break;
        size -= block->len + sizeof(struct proto_block);
        data_p += block->len + sizeof(struct proto_block);
        block = (struct proto_block *)data_p;
        cnt++;
    }

    return cnt;
}

void msg_data_blk_dump_hex(struct proto_block *block)
{
    if (block == NULL) {
        return;
    }

    for (int i = 0; i < block->len; i++) {
        printf("%02x ", block->data[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        } 
    }

    if (block->len % 16 != 0) {
        printf("\n"); 
    }

    return;
}

void msg_data_dump(struct data_src *data)
{
    struct proto_block *block;
    uint8_t *data_p;
    uint16_t size;
    
    if (data == NULL) {
        return; 
    }

    size = data->header.len - sizeof(struct proto_header);
    proto_header_dump(&data->header);
    data_p = (uint8_t *)(data->blocks);
    block = (struct proto_block *)data_p;
    while (size > 0) {
        printf("block->type: %d\n", block->type);
        printf("block->len: %d\n", block->len);
        printf("block->data: %s\n", (char *)block->data);
        msg_data_blk_dump_hex(block);
        size -= block->len + sizeof(struct proto_block);
        data_p += block->len + sizeof(struct proto_block);
        block = (struct proto_block *)data_p;
    }

    return;
}

struct proto_block *msg_data_blk_get_tail(struct data_src *data)
{
    struct proto_block *block;
    uint8_t *data_p;
    uint16_t size;

    if (data == NULL) {
        return NULL;
    }

    size = sizeof(struct data_src);
    data_p = (uint8_t *)(data->blocks);
    block = (struct proto_block *)data_p;
    while (size < data->header.len) {
        if (block->len == 0)
            return NULL; 
        size += block->len + sizeof(struct proto_block);
        data_p += block->len + sizeof(struct proto_block);
        block = (struct proto_block *)data_p;
    }

    return block;
}

void msg_buff_deinit(struct msg_buff *msg_buff)
{
    if (msg_buff == NULL) {
        return;
    }
    
    if (msg_buff->data != NULL) {
        free(msg_buff->data);
    }

    free(msg_buff);
}


struct msg_buff *msg_buff_init(void)
{
    struct msg_buff *msg_buff = (struct msg_buff *)malloc(sizeof(struct msg_buff));
    if (msg_buff == NULL) {
        return NULL;
    }

    msg_buff->next = NULL;
    msg_buff->prev = NULL;

    msg_buff->data = NULL;

    return msg_buff;
}

int msg_buff_set_id(struct msg_buff *msg_buff, uint32_t id)
{
    if (msg_buff == NULL) {
        return -ERR_INVALID_ARG;
    }

    msg_buff->id = id;

    return ERR_SUCCESS; 
}

int msg_buff_set_blk_cnt(struct msg_buff *msg_buff, uint8_t cnt)
{
    if (msg_buff == NULL) {
        return -ERR_INVALID_ARG;
    }

    msg_buff->blk_cnt = cnt;

    return ERR_SUCCESS; 
}

int msg_buff_set_time(struct msg_buff *msg_buff, time_t time) 
{
    if (msg_buff == NULL) {
        return -ERR_INVALID_ARG;
    }

    msg_buff->timestamp = time;

    return ERR_SUCCESS;
}

int msg_buff_set_time_now(struct msg_buff *msg_buff)
{
    if (msg_buff == NULL) {
        return -ERR_INVALID_ARG;
    }

    msg_buff->timestamp = time(NULL);

    return ERR_SUCCESS; 
}

int msg_buff_bind_data(struct msg_buff *msg_buff, void *data, uint8_t cnt) 
{
    int ret;

    if (msg_buff == NULL || data == NULL) {
        return -ERR_INVALID_ARG;
    }
    
    ret = msg_buff_set_time_now(msg_buff);
    if (ret!= ERR_SUCCESS) {
        return ret;
    }
    ret = msg_buff_set_blk_cnt(msg_buff, cnt);
    if (ret!= ERR_SUCCESS) {
        return ret; 
    }

    msg_buff->data = data;

    return ERR_SUCCESS;
}

int msg_buff_reset_data(struct msg_buff *msg_buff, void *data, uint8_t cnt)
{
    int ret;

    if (msg_buff == NULL) {
        return -ERR_INVALID_ARG;
    }

    ret = msg_buff_set_blk_cnt(msg_buff, cnt);
    if (ret!= ERR_SUCCESS) {
        return ret;
    }

    ret = msg_buff_set_time_now(msg_buff);
    if (ret!= ERR_SUCCESS) {
        return ret;
    }

    msg_buff->data = data;

    return ERR_SUCCESS;
}

void msg_buff_unbind_data(struct msg_buff *msg_buff)
{
    if (msg_buff == NULL) {
        return; 
    }

    msg_buff->id = 0;
    msg_buff->blk_cnt = 0;
    msg_buff->timestamp = 0;
    msg_buff->data = NULL;

    return;
}

uint8_t msg_buff_select_queue(struct msg_buff *msg_buff, uint8_t q_cnt)
{
    struct data_src *data;

    if (msg_buff == NULL) {
        return 0;
    }

    if (q_cnt == 0) {
        return 0;
    }

    data = (struct data_src *)msg_buff->data;
    if (data == NULL) {
        return 0;
    }

    return (data->header.priority > q_cnt) ? q_cnt : data->header.priority;
}

void msg_queue_deinit(struct msg_queue *msg_queue)
{
    if (msg_queue == NULL) {
        return;
    }

    while (msg_queue->head != NULL) {
        msg_buff_deinit(msg_queue->head);
        msg_queue->head = msg_queue->head->next;
    } 
}

struct msg_queue *msg_queue_init(uint8_t id)
{
    struct msg_queue *msg_queue = (struct msg_queue *)malloc(sizeof(struct msg_queue));
    if (msg_queue == NULL) {
        return NULL;
    }

    msg_queue->cfg.qid = id;
    msg_queue->cfg.rx_type = MSG_QUEUE_RX_HALF;
    msg_queue->stats.count = 0;
    msg_queue->stats.bytes = 0;
    msg_queue->head = NULL;
    msg_queue->tail = NULL;

    return msg_queue;
}

int msg_queue_enqueue(struct msg_queue *msg_queue, struct msg_buff *msg_buff)
{
    struct data_src *data;

    if (msg_queue == NULL || msg_buff == NULL) {
        return -ERR_INVALID_ARG;
    }
    
    data = (struct data_src *)msg_buff->data;
    if (data == NULL) {
        return -ERR_EMPTY; 
    }
    if (data->header.len + msg_queue->stats.bytes > MSG_QUEUE_BYTES_MAX
        || msg_queue->stats.count == MSG_QUEUE_CNT_MAX) {
        return -ERR_OUT_OF_RANGE;   
    }

    if (msg_queue->stats.count == 0) {
        msg_queue->head = msg_buff;
        msg_queue->tail = msg_buff;
    } else {
        msg_queue->tail->next = msg_buff;
        msg_buff->prev = msg_queue->tail;
        msg_queue->tail = msg_buff;
    }

    msg_queue->stats.count++;
    // msg_queue->stats.bytes += data->header.len;

    return ERR_SUCCESS;
}

struct msg_buff *msg_queue_dequeue(struct msg_queue *msg_queue)
{
    struct msg_buff *msg_buff;
    if (msg_queue == NULL) {
        return NULL;
    }
    
    if (msg_queue->stats.count == 0) {
        return NULL; 
    }

    msg_buff = msg_queue->head;
    msg_queue->head = msg_queue->head->next;
    msg_queue->stats.count--;
    msg_queue->stats.bytes += ((struct data_src *)msg_buff->data)->header.len;

    return msg_buff;
}

struct msg_buff *msg_queue_peek(struct msg_queue *msg_queue)
{
    if (msg_queue == NULL) {
        return NULL;
    }

    if (msg_queue->stats.count == 0) {
        return NULL;
    }
    
    return msg_queue->head;
}

struct msg_buff *msg_queue_get_tail(struct msg_queue *msg_queue)
{
    if (msg_queue == NULL) {
        return NULL;
    }

    if (msg_queue->stats.count == 0) {
        return NULL;
    }

    return msg_queue->tail;
}

int msg_queue_get_count(struct msg_queue *msg_queue)
{
    if (msg_queue == NULL) {
        return -ERR_INVALID_ARG;
   
    }

    return msg_queue->stats.count; 
}

int msg_queue_get_data_size(struct msg_queue *msg_queue)
{
    if (msg_queue == NULL) {
        return -ERR_INVALID_ARG;
    }

    return msg_queue->stats.bytes; 
}

int msg_queue_get_id(struct msg_queue *msg_queue)
{
    if (msg_queue == NULL) {
        return -ERR_INVALID_ARG;
    }
    
    return msg_queue->cfg.qid;
}

int msg_queue_set_rx_type(struct msg_queue *msg_queue, uint8_t rx_type)
{
    if (msg_queue == NULL) {
        return -ERR_INVALID_ARG;
    }

    if (rx_type > MSG_QUEUE_RX_TYPE_MAX) {
        return -ERR_INVALID_ARG;
    }

    msg_queue->cfg.rx_type = rx_type;

    return ERR_SUCCESS;
}
