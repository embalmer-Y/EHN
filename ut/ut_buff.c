#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "../src/proto.h"
#include "../src/errno.h"
#include "../src/buff.h"

#include "ut_common.h"

int msg_data_src_case(void) {
    struct data_src *data;
    struct proto_block *block;
    int ret;

    /* msg_data_src_init start */
    data = msg_data_src_init(1024 + sizeof(struct proto_header), NULL);
    if (data == NULL) {
        return -1; 
    }

    block = proto_block_init(1, 1024 - sizeof(struct proto_block));
    if (block == NULL) {
        return -1;
    }
    
    ret = proto_block_reset_data(&block, (uint8_t *)"hello world", 1024 - sizeof(struct proto_block));
    if (ut_common_compile_ret(ret, ERR_SUCCESS)) {
        printf("proto_block_set_data ERR_SUCCESS failed\n");
        return -2;
    }
    /* msg_data_src_init start */

    /* msg_data_src_fill start */
    ret = msg_data_src_fill(data, block);
    if (ut_common_compile_ret(ret, ERR_SUCCESS)) {
        printf("msg_data_src_fill ERR_SUCCESS failed\n");
        return -2; 
    }

    if (ut_common_compile_uint16(data->header.len, 1024 + sizeof(struct proto_header))) {
        printf("msg_data_src_fill len failed\n");
        return -2;
    }

    if (ut_common_compile_uint8(data->blocks[0].type, 1)) {
        printf("msg_data_src_fill type failed\n");
        return -2; 
    }

    if (ut_common_compile_uint16(data->blocks[0].len, 1024 - sizeof(struct proto_block))) {
        printf("msg_data_src_fill block len failed\n");
        return -2; 
    }

    if (ut_common_compile_uint8(data->blocks[0].data[0], 'h')) {
        printf("msg_data_src_fill data failed\n");
        return -2; 
    }

    ret = msg_data_src_fill(data, block);
    if (ut_common_compile_ret(ret, -ERR_OUT_OF_RANGE)) {
        printf("msg_data_src_fill -ERR_OUT_OF_RANGE failed\n");
        return -2;
    }
    msg_data_dump(data);
    /* msg_data_src_fill end */

    /* msg_data_src_expand start */
    ret = msg_data_src_expand(&data, 1024);
    if (ut_common_compile_ret(ret, ERR_SUCCESS)) {
        printf("msg_data_src_expand ERR_SUCCESS failed\n");
        return -3; 
    }

    if (ut_common_compile_uint16(data->header.len, 2048 + sizeof(struct proto_header))) {
        printf("msg_data_src_expand len failed\n");
        return -3; 
    }

    ret = msg_data_src_fill(data, block);
    if (ut_common_compile_ret(ret, ERR_SUCCESS)) {
        printf("msg_data_src_fill ERR_SUCCESS failed\n");
        return -3; 
    }
    /* msg_data_src_expand end */

    /* msg_data_src_truncate start */
    ret = msg_data_src_truncate(&data, 1024);
    if (ut_common_compile_ret(ret, ERR_SUCCESS)) {
        printf("msg_data_src_truncate ERR_SUCCESS failed\n");
        return -4;
    }

    if (ut_common_compile_uint16(data->header.len, 1024 + sizeof(struct proto_header))) {
        printf("msg_data_src_truncate len failed\n");
        return -4;
    }
    msg_data_dump(data);
    /* msg_data_src_truncate end */

    /* msg_data_blk_get_tail start */
    struct proto_block *blk_tail;
    blk_tail = msg_data_blk_get_tail(data);
    if (ut_common_compile_uint16(blk_tail->len, 1024 - sizeof(struct proto_block))) {
        printf("msg_data_blk_get_tail len failed\n");
        return -5;
    }

    if (ut_common_compile_uint8(blk_tail->data[0], 'h')) {
        printf("msg_data_blk_get_tail data failed\n");
        return -5; 
    }
    /* msg_data_blk_get_tail end */

    /* msg_data_src_deinit start */
    proto_block_deinit(block);
    msg_data_src_deinit(data);
    /* msg_data_src_deinit end */
    return 0;
}

int msg_buff_case(void) {
    struct msg_buff *buff;
    struct data_src *data;
    struct data_src *data2;
    struct proto_block *block;
    struct proto_block *block2;
    time_t now;
    int ret;

    /* msg_data_src_init start */
    data = msg_data_src_init(1024 + sizeof(struct proto_header), NULL);
    if (data == NULL) {
        return -1; 
    }

    block = proto_block_init(1, 1024 - sizeof(struct proto_block));
    if (block == NULL) {
        return -1;
    }
    
    ret = proto_block_reset_data(&block, (uint8_t *)"hello world", 1024 - sizeof(struct proto_block));
    if (ut_common_compile_ret(ret, ERR_SUCCESS)) {
        printf("proto_block_set_data ERR_SUCCESS failed\n");
        return -1;
    }

    ret = msg_data_src_fill(data, block);
    if (ut_common_compile_ret(ret, ERR_SUCCESS)) {
        printf("msg_data_src_fill ERR_SUCCESS failed\n");
        return -1;
    }
    /* msg_data_src_init start */

    /* msg_buff_init start */
    buff = msg_buff_init();
    if (buff == NULL) {
        return -1; 
    }
    /* msg_buff_init end */

    /* msg_buff_set_id start */
    ret = msg_buff_set_id(buff, 1);
    if (ut_common_compile_ret(ret, ERR_SUCCESS)) {
        printf("msg_buff_set_id ERR_SUCCESS failed\n");
        return -2; 
    }

    if (ut_common_compile_uint32(buff->id, 1)) {
        printf("msg_buff_set_id id failed\n");
        return -2;
    }
    /* msg_buff_set_id end */

    /* msg_buff_set_blk_cnt start */
    ret = msg_buff_set_blk_cnt(buff, 1);
    if (ut_common_compile_ret(ret, ERR_SUCCESS)) {
        printf("msg_buff_set_blk_cnt ERR_SUCCESS failed\n");
        return -3;
    }

    if (ut_common_compile_uint8(buff->blk_cnt, 1)) {
        printf("msg_buff_set_blk_cnt blk_cnt failed\n");
        return -3;
    }
    /* msg_buff_set_blk_cnt end */

    /* msg_buff_set_time start */
    time (&now);
    ret = msg_buff_set_time(buff, now);
    if (ut_common_compile_ret(ret, ERR_SUCCESS)) {
        printf("msg_buff_set_time ERR_SUCCESS failed\n");
        return -4;
    }

    if (ut_common_compile_uint32(buff->timestamp, now)) {
        printf("msg_buff_set_time time failed\n");
        return -4;
    }
    /* msg_buff_set_time end */

    /* msg_buff_bind_data start*/
    ret = msg_buff_bind_data(buff, (void *)data, 1);
    if (ut_common_compile_ret(ret, ERR_SUCCESS)) {
        printf("msg_buff_bind_data ERR_SUCCESS failed\n");
        return -5; 
    }
    
    data2 = (struct data_src *)buff->data;
    if (ut_common_compile_uint16(data2->header.len, 1024 + sizeof(struct proto_header))) {
        printf("msg_buff_bind_data data failed\n");
        return -5;
    }
    /* msg_buff_bind_data end */
    data2 = msg_data_src_init(512 + sizeof(struct proto_header), NULL);
    if (data == NULL) {
        return -1; 
    }

    block2 = proto_block_init(2, 512 - sizeof(struct proto_block));
    if (block2 == NULL) {
        return -1;
    }
    
    ret = proto_block_reset_data(&block2, (uint8_t *)"wtf", 512 - sizeof(struct proto_block));
    if (ut_common_compile_ret(ret, ERR_SUCCESS)) {
        printf("proto_block_set_data ERR_SUCCESS failed\n");
        return -1;
    }

    ret = msg_data_src_fill(data2, block2);
    if (ut_common_compile_ret(ret, ERR_SUCCESS)) {
        printf("msg_data_src_fill ERR_SUCCESS failed\n");
        return -1;
    }

    ret = msg_buff_reset_data(buff, (void *)data2, 1);
    if (ut_common_compile_ret(ret, ERR_SUCCESS)) {
        printf("msg_buff_reset_data ERR_SUCCESS failed\n");
        return -6;
    }

    if (ut_common_compile_uint16(data2->header.len, 512 + sizeof(struct proto_header))) {
        printf("msg_buff_reset_data data failed\n");
        return -6;
    }
    /* msg_buff_reset_data start */

    /* msg_buff_unbind_data start */
    msg_buff_unbind_data(buff);
    if (buff->data != NULL) {
        printf("msg_buff_unbind_data failed\n");
        return -7; 
    }

    if (ut_common_compile_uint16(data2->header.len, 512 + sizeof(struct proto_header))) {
        printf("msg_buff_unbind_data data failed\n");
        return -7;
    }
    /* msg_buff_unbind_data end */

    /* msg_buff_deinit start */
    msg_buff_deinit(buff);
    /* msg_buff_deinit end */

    proto_block_deinit(block);
    msg_data_src_deinit(data);
    proto_block_deinit(block2);
    msg_data_src_deinit(data2);

    return 0;
}

int msg_queue_case(void)
{
    int ret;
    struct msg_queue *queue;
    struct msg_buff *buff;
    struct msg_buff *buff2;
    struct data_src *data;
    struct proto_block *block;
    time_t now;

    /* Init flow */
    queue = msg_queue_init(1);
    if (queue == NULL) {
        return -1;
    }

    data = msg_data_src_init(1024 + sizeof(struct proto_header), NULL);
    if (data == NULL) {
        return -1; 
    }

    block = proto_block_init(1, 1024 - sizeof(struct proto_block));
    if (block == NULL) {
        return -1;
    }

    ret = proto_block_reset_data(&block, (uint8_t *)"hello world", 1024 - sizeof(struct proto_block));
    if (ut_common_compile_ret(ret, ERR_SUCCESS)) {
        printf("proto_block_set_data ERR_SUCCESS failed\n");
        return -1;
    }

    ret = msg_data_src_fill(data, block);
    if (ut_common_compile_ret(ret, ERR_SUCCESS)) {
        printf("msg_data_src_fill ERR_SUCCESS failed\n");
        return -1;
    }

    buff = msg_buff_init();
    if (buff == NULL) {
        return -1; 
    }

    /* msg_buff_set_id start */
    ret = msg_buff_set_id(buff, 1);
    if (ut_common_compile_ret(ret, ERR_SUCCESS)) {
        printf("msg_buff_set_id ERR_SUCCESS failed\n");
        return -2; 
    }

    if (ut_common_compile_uint32(buff->id, 1)) {
        printf("msg_buff_set_id id failed\n");
        return -2;
    }
    /* msg_buff_set_id end */

    /* msg_buff_set_blk_cnt start */
    ret = msg_buff_set_blk_cnt(buff, 1);
    if (ut_common_compile_ret(ret, ERR_SUCCESS)) {
        printf("msg_buff_set_blk_cnt ERR_SUCCESS failed\n");
        return -3;
    }

    if (ut_common_compile_uint8(buff->blk_cnt, 1)) {
        printf("msg_buff_set_blk_cnt blk_cnt failed\n");
        return -3;
    }
    /* msg_buff_set_blk_cnt end */

    /* msg_buff_set_time start */
    time (&now);
    ret = msg_buff_set_time(buff, now);
    if (ut_common_compile_ret(ret, ERR_SUCCESS)) {
        printf("msg_buff_set_time ERR_SUCCESS failed\n");
        return -4;
    }

    if (ut_common_compile_uint32(buff->timestamp, now)) {
        printf("msg_buff_set_time time failed\n");
        return -4;
    }
    /* msg_buff_set_time end */

    /* msg_buff_bind_data start*/
    ret = msg_buff_bind_data(buff, (void *)data, 1);
    if (ut_common_compile_ret(ret, ERR_SUCCESS)) {
        printf("msg_buff_bind_data ERR_SUCCESS failed\n");
        return -5; 
    }

    ret = msg_queue_enqueue(queue, buff);
    if (ut_common_compile_ret(ret, ERR_SUCCESS)) {
        printf("msg_queue_enqueue ERR_SUCCESS failed\n");
        return -6;
    }

    buff2 = msg_queue_peek(queue);
    if (ut_common_compile_uint32(buff2->id, 1)) {
        printf("msg_queue_peek id failed\n");
        return -7;
    }

    buff2 = msg_queue_get_tail(queue);
    if (ut_common_compile_uint32(buff2->id, 1)) {
        printf("msg_queue_get_tail id failed\n");
        return -7;
    }

    buff2 = msg_queue_dequeue(queue);
    if (ut_common_compile_ret(ret, ERR_SUCCESS)) {
        printf("msg_queue_dequeue ERR_SUCCESS failed\n");
        return -7;
    }

    if (ut_common_compile_uint32(buff2->id, 1)) {
        printf("msg_queue_dequeue id failed\n");
        return -7;
    }

    if (ut_common_compile_uint32(msg_queue_get_count(queue), 0)) {
        printf("msg_queue_get_count count failed\n");
        return -7;
    }

    if (ut_common_compile_uint32(msg_queue_get_data_size(queue), data->header.len)) {
        printf("msg_queue_get_data_size size failed\n");
        return -7;
    }

    if (ut_common_compile_uint32(msg_queue_get_id(queue), 1)) {
        printf("msg_queue_get_id id failed\n");
        return -7;
    }

    ret = msg_queue_set_rx_type(queue, 1);
    if (ut_common_compile_ret(ret, ERR_SUCCESS)) {
        printf("msg_queue_set_rx_type ERR_SUCCESS failed\n");
        return -7;
    }

    if (ut_common_compile_uint8(queue->cfg.rx_type, 1)) {
        printf("msg_queue_get_rx_type rx_type failed\n");
        return -7;
    }

    /* msg_buff_deinit start */
    msg_buff_deinit(buff);
    /* msg_buff_deinit end */

    proto_block_deinit(block);
    // msg_data_src_deinit(data);
    msg_queue_deinit(queue);

    return 0;
}

int main(void) {
    int ret;
    ret = msg_data_src_case();
    if (ret != 0) {
        printf("buff_data_src_case failed\n");
        return -1;
    }

    ret = msg_buff_case();
    if (ret != 0) {
        printf("buff_msg_buff_case failed\n");
        return -2; 
    }

    ret = msg_queue_case();
    if (ret != 0) {
        printf("buff_msg_queue_case failed\n");
        return -3; 
    }

    printf("buff_data_src_case passed\n");
    return 0;
}
