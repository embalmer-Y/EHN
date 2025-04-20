#include "stdlib.h"

#include "pipe.h"
#include "buff.h"
#include <stdint.h>

struct pipe *pipe_create(struct pipe_ctrl_block *pcb)
{
    int ret;
    struct pipe *pipe = malloc(sizeof(struct pipe));
    if (pipe == NULL) {
        return NULL;
    }

    pipe->pcb = pcb;
    ret = pipe_set_id(pcb, pipe);
    if (ret != 0) {
        free(pipe);
        return NULL;
    }

    ret = pipe_ctrl_blk_add(pcb, pipe);
    if (ret != 0) {
        free(pipe);
        return NULL;
    }

    return pipe;
}

void pipe_deinit(struct pipe *pipe)
{
    if (pipe == NULL) {
        return;
    }

    free(pipe);
}

int pipe_set_id(struct pipe_ctrl_block *pcb, struct pipe *pipe)
{
    if (pcb == NULL || pipe == NULL) {
        return -1;
    }

    pipe->id = pcb->tail->id + 1;
    if (pipe->id > PIPE_ID_MAX) {
        return -1;
    }
    
    return 0;
}

int pipe_get_id(struct pipe_ctrl_block *pcb, struct pipe *pipe)
{
    if (pcb == NULL || pipe == NULL) {
        return -1;
    }

    return pipe->id;
}

int pipe_add_msg_buff(struct pipe *pipe, struct msg_buff *mb)
{
    uint8_t q_cnt, q_idx;
    int ret;
    if (pipe == NULL || mb == NULL) {
        return -1;
    }

    q_cnt = pipe->rx_queue_cnt;
    q_idx = msg_buff_select_queue(mb, q_cnt);

    ret = msg_queue_enqueue(&pipe->rx_queue[q_idx], mb);
    if (ret != 0) {
        return -1;
    }

    return 0;
}

int pipe_get_msg_buff_by_qid(struct pipe *pipe, struct msg_buff **mb, uint8_t q_idx)
{
    if (pipe == NULL || mb == NULL) {
        return -1;
    }

    if (q_idx >= pipe->rx_queue_cnt) {
        return -1;
    }

    *mb = msg_queue_dequeue(&pipe->rx_queue[q_idx]);
    if (*mb == NULL) {
        return -1;
    }

    return 0;
}

struct pipe *pipe_ctrl_blk_find_pipe(struct pipe_ctrl_block *pcb, uint16_t id)
{
    if (pcb == NULL) {
        return NULL;
    }

    struct pipe *pipe = pcb->head;
    while (pipe != NULL) {
        if (pipe->id == id) {
            return pipe;
        }
        pipe = pipe->next;
    }

    return NULL;
}

struct pipe_ctrl_block *pipe_ctrl_block_init(void)
{
    struct pipe_ctrl_block *pcb = malloc(sizeof(struct pipe_ctrl_block));
    if (pcb == NULL) {
        return NULL;
    }

    pcb->head = NULL;
    pcb->tail = NULL;
    pcb->pipe_cnt = 0;

    return pcb;
}

void pipe_ctrl_block_deinit(struct pipe_ctrl_block *pcb)
{
    struct pipe *pipe;
    if (pcb == NULL) {
        return;
    }

    while (pcb->head != NULL) {
        pipe = pcb->head;
        pcb->head = pipe->next;
        pipe_deinit(pipe);
    }
    free(pcb);

    return;
}

int pipe_ctrl_blk_add(struct pipe_ctrl_block *pcb, struct pipe *pipe)
{
    if (pcb == NULL || pipe == NULL) {
        return -1;
    }

    if (pcb->head == NULL) {
        pcb->head = pipe;
        pcb->tail = pipe;
    } else {
        pcb->tail->next = pipe;
        pipe->prev = pcb->tail;
        pcb->tail = pipe;
    }

    pcb->pipe_cnt++;
    return 0;
}

int pipe_ctrl_blk_remove(struct pipe_ctrl_block *pcb, uint16_t id)
{
    if (pcb == NULL) {
        return -1;
    }

    struct pipe *pipe = pipe_ctrl_blk_find_pipe(pcb, id);
    if (pipe == NULL) {
        return -1;
    }

    if (pipe->prev != NULL) {
        pipe->prev->next = pipe->next;
    } else {
        pcb->head = pipe->next;
    }

    if (pipe->next != NULL) {
        pipe->next->prev = pipe->prev;
    } else {
        pcb->tail = pipe->prev;
    }

    pcb->pipe_cnt--;

    return 0;
}

int pipe_ctrl_blk_remove_all(struct pipe_ctrl_block *pcb)
{
    if (pcb == NULL) {
        return -1;
    }

    struct pipe *pipe = pcb->head;
    while (pipe != NULL) {
        pipe = pipe->next;
        pipe_deinit(pipe);
    }

    pcb->head = NULL;
    pcb->tail = NULL;
    pcb->pipe_cnt = 0;

    return 0;
}
