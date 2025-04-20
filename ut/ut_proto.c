#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "../src/proto.h"
#include "../src/errno.h"

#include "ut_common.h"

int proto_header_case(void)
{
    struct proto_header *header;
    int ret;

    header = proto_header_init();
    if (header == NULL) {
        return -1;
    }

    /* proto_header_set_hop_limit */
    ret = proto_header_set_hop_limit(header, 1);
    if (ut_common_compile_ret(ret, ERR_SUCCESS)) {
        printf("proto_header_set_hop_limit ERR_SUCCESS failed\n");
        return -2; 
    }

    ret = proto_header_set_hop_limit(header, PROTO_HEADER_HOP_LIMIT_MAX + 1);
    if (ut_common_compile_ret(ret, -ERR_INVALID_ARG)) {
        printf("proto_header_set_hop_limit -ERR_INVALID_ARG failed\n");
        return -2;
    }

    if (ut_common_compile_uint8(header->hop_limit, 1)) {
        printf("proto_header_set_hop_limit val failed\n");
        return -2;
    }

    /* proto_header_set_cfg */
    ret = proto_header_set_cfg(header, 1);
    if (ut_common_compile_ret(ret, ERR_SUCCESS)) {
        printf("proto_header_set_cfg ERR_SUCCESS failed\n");
        return -2;
    }

    ret = proto_header_set_cfg(header, PROTO_HEADER_CFG_MAX + 1);
    if (ut_common_compile_ret(ret, -ERR_INVALID_ARG)) {
        printf("proto_header_set_cfg -ERR_INVALID_ARG failed\n");
        return -2;
    }

    if (ut_common_compile_uint8(header->cfg_hdr, 1)) {
        printf("proto_header_set_cfg val failed\n");
        return -2;
    }

    /* proto_header_set_priority */
    ret = proto_header_set_priority(header, 1);
    if (ut_common_compile_ret(ret, ERR_SUCCESS)) {
        printf("proto_header_set_priority ERR_SUCCESS failed\n");
        return -2;
    }

    ret = proto_header_set_priority(header, PROTO_HEADER_PRIORITY_MAX + 1);
    if (ut_common_compile_ret(ret, -ERR_INVALID_ARG)) {
        printf("proto_header_set_priority -ERR_INVALID_ARG failed\n");
        return -2;
    }

    if (ut_common_compile_uint8(header->priority, 1)) {
        printf("proto_header_set_priority val failed\n");
        return -2; 
    }

    /* proto_header_set_heart_rate */
    ret = proto_header_set_heart_rate(header, 1);
    if (ut_common_compile_ret(ret, ERR_SUCCESS)) {
        printf("proto_header_set_heart_rate ERR_SUCCESS failed\n");
        return -2;
    }

    if (ut_common_compile_uint16(header->heart_rate, 1)) {
        printf("proto_header_set_heart_rate val failed\n");
        return -2;
    }

    /* proto_header_set_len */
    ret = proto_header_set_len(header, PROTO_HEADER_SIZE + 1);
    if (ut_common_compile_ret(ret, ERR_SUCCESS)) {
        printf("proto_header_set_len ERR_SUCCESS failed\n");
        return -2; 
    }

    ret = proto_header_set_len(header, PROTO_HEADER_SIZE - 1);
    if (ut_common_compile_ret(ret, -ERR_INVALID_ARG)) {
        printf("proto_header_set_len -ERR_INVALID_ARG failed\n");
        return -2;
    }

    if (ut_common_compile_uint16(header->len, PROTO_HEADER_SIZE + 1)) {
        printf("proto_header_set_len val failed\n");
        return -2; 
    }

    /* proto_header_set_src_id */
    ret = proto_header_set_src_id(header, 1);
    if (ut_common_compile_ret(ret, ERR_SUCCESS)) {
        printf("proto_header_set_src_id ERR_SUCCESS failed\n");
        return -2;
    }

    if (ut_common_compile_uint32(header->src_id, 1)) {
        printf("proto_header_set_src_id val failed\n");
        return -2;
    }

    /* proto_header_set_dst_id */
    ret = proto_header_set_dst_id(header, 1);
    if (ut_common_compile_ret(ret, ERR_SUCCESS)) {
        printf("proto_header_set_dst_id ERR_SUCCESS failed\n");
        return -2;
    }

    if (ut_common_compile_uint32(header->dst_id, 1)) {
        printf("proto_header_set_dst_id val failed\n");
        return -2;  
    }

    /* proto_header_set_checksum */
    ret = proto_header_set_checksum(header, 1);
    if (ut_common_compile_ret(ret, ERR_SUCCESS)) {
        printf("proto_header_set_checksum ERR_SUCCESS failed\n");
        return -2;
    }

    if (ut_common_compile_uint16(header->checksum, 1)) {
        printf("proto_header_set_checksum val failed\n");
        return -2; 
    }

    proto_header_deinit(header);

    return 0;
}

int proto_block_case(void)
{
    struct proto_block *block;
    int ret;

    block = proto_block_init(1, 1024);
    if (block == NULL) {
        return -1; 
    }

    ret = proto_block_set_type(block, 1);
    if (ut_common_compile_ret(ret, ERR_SUCCESS)) {
        printf("proto_block_set_type ERR_SUCCESS failed\n");
        return -2;
    }

    if (ut_common_compile_uint8(block->type, 1)) {
        printf("proto_block_set_type val failed\n");
        return -2;
    }

    ret = proto_block_reset_data(&block, (uint8_t *)"hello world", 11);
    if (ut_common_compile_ret(ret, ERR_SUCCESS)) {
        printf("proto_block_set_data ERR_SUCCESS failed\n");
        return -2;
    }

    if (ut_common_compile_uint16(block->len, 11)) {
        printf("proto_block_set_data len failed\n");
        return -2; 
    }

    if (ut_common_compile_uint8(block->data[0], 'h')) {
        printf("proto_block_set_data data failed\n");
        return -2; 
    }

    proto_block_deinit(block);

    return 0;
}

int main()
{
    int ret;

    ret = proto_header_case();
    if (ret) {
        printf("proto_header_case failed\n");
        return -1;
    } else {
        printf("proto_header_case success\n"); 
    }

    ret = proto_block_case();
    if (ret) {
        printf("proto_block_case failed\n");
        return -1;
    } else {
        printf("proto_block_case success\n");
    }

    return 0;
}
