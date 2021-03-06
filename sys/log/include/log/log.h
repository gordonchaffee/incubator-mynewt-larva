/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#ifndef __LOG_H__
#define __LOG_H__

#include "log/ignore.h"
#include "util/cbmem.h"

#include <os/queue.h>

struct log;

typedef int (*log_walk_func_t)(struct log *, void *arg, void *offset,
        uint16_t len);

typedef int (*lh_read_func_t)(struct log *, void *dptr, void *buf,
        uint16_t offset, uint16_t len);
typedef int (*lh_append_func_t)(struct log *, void *buf, int len);
typedef int (*lh_walk_func_t)(struct log *,
        log_walk_func_t walk_func, void *arg);
typedef int (*lh_flush_func_t)(struct log *);

#define LOG_TYPE_STREAM  (0)
#define LOG_TYPE_MEMORY  (1)
#define LOG_TYPE_STORAGE (2)

struct log_handler {
    int log_type;
    lh_read_func_t log_read;
    lh_append_func_t log_append;
    lh_walk_func_t log_walk;
    lh_flush_func_t log_flush;
    void *log_arg;
};

struct log_entry_hdr {
    int64_t ue_ts;
    uint16_t ue_level;
    uint16_t ue_module;
};
#define LOG_ENTRY_HDR_SIZE (sizeof(struct log_entry_hdr))

#define LOG_LEVEL_DEBUG    (0x01)
#define LOG_LEVEL_INFO     (0x02)
#define LOG_LEVEL_WARN     (0x04)
#define LOG_LEVEL_ERROR    (0x08)
#define LOG_LEVEL_CRITICAL (0x10)
/* Up to 7 custom log levels. */
#define LOG_LEVEL_PERUSER  (0x12)

/* Log module, eventually this can be a part of the filter. */
#define LOG_MODULE_DEFAULT          (0)
#define LOG_MODULE_OS               (1)
#define LOG_MODULE_NEWTMGR          (2)
#define LOG_MODULE_NIMBLE_CTLR      (3)
#define LOG_MODULE_NIMBLE_HOST      (4)
#define LOG_MODULE_PERUSER          (64)

/* Compile in Log Debug by default */
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_DEBUG
#endif

#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#define LOG_DEBUG(__l, __mod, __msg, ...) log_printf(__l, __mod, \
        LOG_LEVEL_DEBUG, __msg, ##__VA_ARGS__)
#else
#define LOG_DEBUG(__l, __mod, ...) IGNORE(__VA_ARGS__)
#endif

#if LOG_LEVEL <= LOG_LEVEL_INFO
#define LOG_INFO(__l, __mod, __msg, ...) log_printf(__l, __mod, \
        LOG_LEVEL_INFO, __msg, ##__VA_ARGS__)
#else
#define LOG_INFO(__l, __mod, ...) IGNORE(__VA_ARGS__)
#endif

#if LOG_LEVEL <= LOG_LEVEL_INFO
#define LOG_WARN(__l, __mod, __msg, ...) log_printf(__l, __mod, \
        LOG_LEVEL_WARN, __msg, ##__VA_ARGS__)
#else
#define LOG_WARN(__l, __mod, ...) IGNORE(__VA_ARGS__)
#endif

#if LOG_LEVEL <= LOG_LEVEL_ERROR
#define LOG_ERROR(__l, __mod, __msg, ...) log_printf(__l, __mod, \
        LOG_LEVEL_ERROR, __msg, ##__VA_ARGS__)
#else
#define LOG_ERROR(__l, __mod, ...) IGNORE(__VA_ARGS__)
#endif

#if LOG_LEVEL <= LOG_LEVEL_CRITICAL
#define LOG_CRITICAL(__l, __mod, __msg, ...) log_printf(__l, __mod, \
        LOG_LEVEL_CRITICAL, __msg, ##__VA_ARGS__)
#else
#define LOG_CRITICAL(__l, __mod, ...) IGNORE(__VA_ARGS__)
#endif

struct log {
    char *l_name;
    struct log_handler *l_log;
    uint16_t log_level;
    STAILQ_ENTRY(log) l_next;
};

/* Log system level functions (for all logs.) */
int log_init(void);
struct log *log_list_get_next(struct log *);

/* Log functions, manipulate a single log */
int log_register(char *name, struct log *log, struct log_handler *);
int log_append(struct log *, uint16_t, uint16_t, void *, uint16_t);

#define LOG_PRINTF_MAX_ENTRY_LEN (128)
void log_printf(struct log *log, uint16_t, uint16_t, char *, ...);
int log_read(struct log *log, void *dptr, void *buf, uint16_t off,
        uint16_t len);
int log_walk(struct log *log, log_walk_func_t walk_func,
        void *arg);
int log_flush(struct log *log);



/* Handler exports */
int log_cbmem_handler_init(struct log_handler *, struct cbmem *);
int log_console_handler_init(struct log_handler *);

#endif /* __LOG_H__ */
