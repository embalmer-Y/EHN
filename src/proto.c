#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "proto.h"
#include "errno.h"

void proto_header_deinit(struct proto_header *header)
{
    free(header);
}

struct proto_header *proto_header_init(void)
{
    struct proto_header *header;
    header = (struct proto_header *)malloc(sizeof(struct proto_header));
    if (header == NULL) {
        return NULL; 
    }
    header->hop_limit = 0;
    header->heart_rate = PROTO_HEADER_HEART_RATE_DEFAULT;
    
    return header;
}

int proto_header_set_hop_limit(struct proto_header *header, uint8_t hop_limit)
{
    if (header == NULL) {
        return -ERR_INVALID_ARG;
    }
    
    if (hop_limit > PROTO_HEADER_HOP_LIMIT_MAX) {
        return -ERR_INVALID_ARG;
    }

    header->hop_limit = hop_limit;

    return ERR_SUCCESS;
}

int proto_header_set_cfg(struct proto_header *header, uint8_t cfg)
{
    if (header == NULL) {
        return -ERR_INVALID_ARG;
    }

    if (cfg > PROTO_HEADER_CFG_MAX) {
        return -ERR_INVALID_ARG; 
    }

    header->cfg_hdr = cfg;

    return ERR_SUCCESS;
}

int proto_header_set_priority(struct proto_header *header, uint8_t priority)
{
    if (header == NULL) {
        return -ERR_INVALID_ARG;
    }
    
    if (priority > PROTO_HEADER_PRIORITY_MAX) {
        return -ERR_INVALID_ARG; 
    }

    header->priority = priority;

    return ERR_SUCCESS;
}

int proto_header_set_heart_rate(struct proto_header *header, uint16_t heart_rate)
{
    if (header == NULL) {
        return -ERR_INVALID_ARG;
    }

    header->heart_rate = heart_rate;

    return ERR_SUCCESS;
}

int proto_header_set_src_id(struct proto_header *header, uint32_t src_id)
{
    if (header == NULL) {
        return -ERR_INVALID_ARG;
    }

    header->src_id = src_id;

    return ERR_SUCCESS;
}

int proto_header_set_dst_id(struct proto_header *header, uint32_t dst_id)
{
    if (header == NULL) {
        return -ERR_INVALID_ARG;
    }
    
    header->dst_id = dst_id;

    return ERR_SUCCESS;
}

int proto_header_set_len(struct proto_header *header, uint16_t len)
{
    if (header == NULL) {
        return -ERR_INVALID_ARG;
    }

    if (len < PROTO_HEADER_SIZE) {
        return -ERR_INVALID_ARG; 
    }

    header->len = len;

    return ERR_SUCCESS;
}

int proto_header_set_checksum(struct proto_header *header, uint16_t checksum)
{
    if (header == NULL) {
        return -ERR_INVALID_ARG;
    }
    
    header->checksum = checksum;

    return ERR_SUCCESS;
}

int proto_header_dump(struct proto_header *header)
{
    if (header == NULL) {
        return -ERR_INVALID_ARG;
    }
    printf("header->hop_limit: %d\n", header->hop_limit);
    printf("header->heart_rate: %d\n", header->heart_rate);
    printf("header->cfg_hdr: %d\n", header->cfg_hdr);
    printf("header->priority: %d\n", header->priority);
    printf("header->src_id: 0x%X\n", header->src_id);
    printf("header->dst_id: 0x%X\n", header->dst_id);
    printf("header->len: %d\n", header->len);
    printf("header->checksum: 0x%X\n", header->checksum);

    return ERR_SUCCESS; 
}

void proto_block_deinit(struct proto_block *block)
{
   if (block == NULL) {
       return; 
   }

   free(block); 
}

struct proto_block *proto_block_init(uint16_t type, uint16_t size)
{
    struct proto_block *block;
    block = (struct proto_block *)malloc(sizeof(struct proto_block) + size);
    if (block == NULL) {
        return NULL; 
    }

    block->type = type;
    block->len = size;

    return block;
}

int proto_block_set_type(struct proto_block *block, uint16_t type)
{
    if (block == NULL) {
        return -ERR_INVALID_ARG;
    }
    
    if (type > PROTO_BLOCK_TYPE_MAX) {
        return -ERR_INVALID_ARG;
    }

    block->type = type;

    return ERR_SUCCESS;
}

int proto_block_reset_data(struct proto_block **block, uint8_t *data, uint16_t size)
{
    uint16_t new_size;
    if (block == NULL || data == NULL) {
        printf("proto_block_reset_data block == NULL || data == NULL\n");
        return -ERR_INVALID_ARG;
    }

    if (sizeof(data) > size) {
        printf("proto_block_reset_data sizeof(data) < size\n");
        return -ERR_INVALID_ARG; 
    }

    new_size = sizeof(struct proto_block) + size;
    *block = (struct proto_block *)realloc(*block, new_size);
    if (*block == NULL) {
        return -ERR_NO_MEM;
    }

    (*block)->len = size;
    memcpy((*block)->data, data, size);

    return ERR_SUCCESS;
}
