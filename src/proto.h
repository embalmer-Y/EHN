#ifndef __PROTO_H__
#define __PROTO_H__

#include <stdint.h>
#include <time.h>

#define PROTO_HEADER_SIZE 8
#define PROTO_HEADER_HOP_LIMIT_MAX 0x80    // 128 hops

#define PROTO_HEADER_CFG_MAX 3
#define PROTO_HEADER_CFG_MASK 0x18

#define PROTO_HEADER_PRIO_CNT 5
#define PROTO_HEADER_PRIORITY_MAX 4
#define PROTO_HEADER_PRIORITY_MASK 0x7

#define PROTO_HEADER_HEART_RATE_MAX 0xFFFF // 65535s
#define PROTO_HEADER_HEART_RATE_DEFAULT 60 // 60s

#define PROTO_HEADER_LEN_MAX 0xFFFF        // 65535 bytes

#define PROTO_BLOCK_TYPE_MAX 0xFFFF        // 65535 types
#define PROTO_BLOCK_LEN_MAX 0xFFFF         // 65535 bytes

enum proto_priority {
    PROTO_PRIO_LOW = 0,
    PROTO_PRIO_LEVEL1,
    PROTO_PRIO_LEVEL2,
    PROTO_PRIO_LEVEL3,
    PROTO_PRIO_LEVEL4,
};

/*
 protocol block schematic:
 32 bits                                   16 bits                                  0 bits
 +-------8bits--------+-------8bits--------+-------8bits-------+--------8bits-------+
 |                  type                   |                  len                   |
 +-------8bits--------+-------8bits--------+-------8bits-------+--------8bits-------+
 |                  data...                |
 +-------8bits--------+-------8bits--------+-------8bits-------+--------8bits-------+
*/
struct proto_block {
    uint16_t type;
    uint16_t len;
    uint8_t data[];
};

/*
 protocol header schematic:
 32 bits                                   16 bits                                  0 bits
 +---8bits---+--3bits--+--2bits--+--3bits--+-------8bits-------+--------8bits-------+
 | hop_limit | earmark | cfg_hdr |   prio  |                heart_rate              |
 +-------8bits--------+-------8bits--------+-------8bits-------+--------8bits-------+
 |                                      src_id                                      |
 +-------8bits--------+-------8bits--------+-------8bits-------+--------8bits-------+
 |                                      dst_id                                      |
 +-------8bits--------+-------8bits--------+-------8bits-------+--------8bits-------+
 |                   len                   |    check_sum(h)   |    check_sum(l)    |
 +-------8bits--------+-------8bits--------+-------8bits-------+--------8bits-------+
*/
struct proto_header {
    uint8_t hop_limit;
    struct {
        uint8_t earmark: 3;
        uint8_t cfg_hdr: 2;
        uint8_t priority: 3;
    };
    uint16_t heart_rate;
    uint32_t src_id;
    uint32_t dst_id;
    uint16_t len;
    uint16_t checksum;
};

void proto_block_deinit(struct proto_block *block);
struct proto_block *proto_block_init(uint16_t type, uint16_t size);
int proto_block_set_type(struct proto_block *block, uint16_t type);
int proto_block_reset_data(struct proto_block **block, uint8_t *data, uint16_t size);

int proto_header_dump(struct proto_header *header);
void proto_header_deinit(struct proto_header *header);
struct proto_header *proto_header_init(void);
int proto_header_set_hop_limit(struct proto_header *header, uint8_t hop_limit);
int proto_header_set_cfg(struct proto_header *header, uint8_t cfg);
int proto_header_set_priority(struct proto_header *header, uint8_t priority);
int proto_header_set_heart_rate(struct proto_header *header, uint16_t heart_rate);
int proto_header_set_src_id(struct proto_header *header, uint32_t src_id);
int proto_header_set_dst_id(struct proto_header *header, uint32_t dst_id);
int proto_header_set_len(struct proto_header *header, uint16_t len);
int proto_header_set_checksum(struct proto_header *header, uint16_t checksum);

#endif /* __PROTO_H__ */
