#ifndef __TABLE_H__
#define __TABLE_H__

#include <stdint.h>

#include "config.h"
#include "proto.h"
#include "buff.h"
#include "route.h"

struct msg_table_ops {
    int (*init)(struct msg_table_config *config);
    void (*deinit)(void);
};

struct msg_table_rule {

};

struct msg_table {
    struct msg_table_ops *ops;
    struct msg_table_rule *rule;
};

#endif // __TABLE_H__
