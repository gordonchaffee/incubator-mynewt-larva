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

#include <errno.h>
#include <string.h>
#include "os/os.h"
#include "nimble/ble.h"
#include "ble_hs_priv.h"
#include "host/ble_att.h"
#include "host/ble_uuid.h"
#include "ble_l2cap_priv.h"
#include "ble_att_cmd.h"

int
ble_att_error_rsp_parse(void *payload, int len, struct ble_att_error_rsp *rsp)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_ERROR_RSP_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;

    if (u8ptr[0] != BLE_ATT_OP_ERROR_RSP) {
        return BLE_HS_EINVAL;
    }

    rsp->baep_req_op = u8ptr[1];
    rsp->baep_handle = le16toh(u8ptr + 2);
    rsp->baep_error_code = u8ptr[4];

    return 0;
}

int
ble_att_error_rsp_write(void *payload, int len, struct ble_att_error_rsp *rsp)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_ERROR_RSP_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;

    u8ptr[0] = BLE_ATT_OP_ERROR_RSP;
    u8ptr[1] = rsp->baep_req_op;
    htole16(u8ptr + 2, rsp->baep_handle);
    u8ptr[4] = rsp->baep_error_code;

    return 0;
}

int
ble_att_mtu_cmd_parse(void *payload, int len, struct ble_att_mtu_cmd *cmd)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_MTU_CMD_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;

    if (u8ptr[0] != BLE_ATT_OP_MTU_REQ &&
        u8ptr[0] != BLE_ATT_OP_MTU_RSP) {

        return BLE_HS_EINVAL;
    }

    cmd->bamc_mtu = le16toh(u8ptr + 1);

    return 0;
}

int
ble_att_mtu_req_write(void *payload, int len,
                      struct ble_att_mtu_cmd *cmd)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_MTU_CMD_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;

    u8ptr[0] = BLE_ATT_OP_MTU_REQ;
    htole16(u8ptr + 1, cmd->bamc_mtu);

    return 0;
}

int
ble_att_mtu_rsp_write(void *payload, int len, struct ble_att_mtu_cmd *cmd)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_MTU_CMD_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;

    u8ptr[0] = BLE_ATT_OP_MTU_RSP;
    htole16(u8ptr + 1, cmd->bamc_mtu);

    return 0;
}

int
ble_att_find_info_req_parse(void *payload, int len,
                            struct ble_att_find_info_req *req)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_FIND_INFO_REQ_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;

    if (u8ptr[0] != BLE_ATT_OP_FIND_INFO_REQ) {
        return BLE_HS_EINVAL;
    }

    req->bafq_start_handle = le16toh(u8ptr + 1);
    req->bafq_end_handle = le16toh(u8ptr + 3);

    return 0;
}

int
ble_att_find_info_req_write(void *payload, int len,
                            struct ble_att_find_info_req *req)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_FIND_INFO_REQ_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;

    u8ptr[0] = BLE_ATT_OP_FIND_INFO_REQ;
    htole16(u8ptr + 1, req->bafq_start_handle);
    htole16(u8ptr + 3, req->bafq_end_handle);

    return 0;
}

int
ble_att_find_info_rsp_parse(void *payload, int len,
                            struct ble_att_find_info_rsp *rsp)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_FIND_INFO_RSP_BASE_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;

    if (u8ptr[0] != BLE_ATT_OP_FIND_INFO_RSP) {
        return BLE_HS_EINVAL;
    }

    rsp->bafp_format = u8ptr[1];

    return 0;
}

int
ble_att_find_info_rsp_write(void *payload, int len,
                            struct ble_att_find_info_rsp *rsp)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_FIND_INFO_RSP_BASE_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;

    u8ptr[0] = BLE_ATT_OP_FIND_INFO_RSP;
    u8ptr[1] = rsp->bafp_format;

    return 0;
}

int
ble_att_find_type_value_req_parse(void *payload, int len,
                                  struct ble_att_find_type_value_req *req)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_FIND_TYPE_VALUE_REQ_BASE_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;

    if (u8ptr[0] != BLE_ATT_OP_FIND_TYPE_VALUE_REQ) {
        return BLE_HS_EINVAL;
    }

    req->bavq_start_handle = le16toh(u8ptr + 1);
    req->bavq_end_handle = le16toh(u8ptr + 3);
    req->bavq_attr_type = le16toh(u8ptr + 5);

    return 0;
}

int
ble_att_find_type_value_req_write(void *payload, int len,
                                  struct ble_att_find_type_value_req *req)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_FIND_TYPE_VALUE_REQ_BASE_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;

    u8ptr[0] = BLE_ATT_OP_FIND_TYPE_VALUE_REQ;
    htole16(u8ptr + 1, req->bavq_start_handle);
    htole16(u8ptr + 3, req->bavq_end_handle);
    htole16(u8ptr + 5, req->bavq_attr_type);

    return 0;
}

int
ble_att_read_type_req_parse(void *payload, int len,
                            struct ble_att_read_type_req *req)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_READ_TYPE_REQ_BASE_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;

    if (u8ptr[0] != BLE_ATT_OP_READ_TYPE_REQ) {
        return BLE_HS_EINVAL;
    }

    req->batq_start_handle = le16toh(u8ptr + 1);
    req->batq_end_handle = le16toh(u8ptr + 3);

    return 0;
}

int
ble_att_read_type_req_write(void *payload, int len,
                            struct ble_att_read_type_req *req)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_READ_TYPE_REQ_BASE_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;

    u8ptr[0] = BLE_ATT_OP_READ_TYPE_REQ;
    htole16(u8ptr + 1, req->batq_start_handle);
    htole16(u8ptr + 3, req->batq_end_handle);

    return 0;
}

int
ble_att_read_type_rsp_parse(void *payload, int len,
                            struct ble_att_read_type_rsp *rsp)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_READ_TYPE_RSP_BASE_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;

    rsp->batp_length = u8ptr[1];

    return 0;
}

int
ble_att_read_type_rsp_write(void *payload, int len,
                            struct ble_att_read_type_rsp *rsp)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_READ_TYPE_RSP_BASE_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;

    u8ptr[0] = BLE_ATT_OP_READ_TYPE_RSP;
    u8ptr[1] = rsp->batp_length;

    return 0;
}

int
ble_att_read_req_parse(void *payload, int len, struct ble_att_read_req *req)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_READ_REQ_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;

    if (u8ptr[0] != BLE_ATT_OP_READ_REQ) {
        return BLE_HS_EINVAL;
    }

    req->barq_handle = le16toh(u8ptr + 1);

    return 0;
}

int
ble_att_read_req_write(void *payload, int len, struct ble_att_read_req *req)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_READ_REQ_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;

    u8ptr[0] = BLE_ATT_OP_READ_REQ;
    htole16(u8ptr + 1, req->barq_handle);

    return 0;
}

int
ble_att_read_blob_req_parse(void *payload, int len,
                            struct ble_att_read_blob_req *req)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_READ_BLOB_REQ_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;

    if (u8ptr[0] != BLE_ATT_OP_READ_BLOB_REQ) {
        return BLE_HS_EINVAL;
    }

    req->babq_handle = le16toh(u8ptr + 1);
    req->babq_offset = le16toh(u8ptr + 3);

    return 0;
}

int
ble_att_read_blob_req_write(void *payload, int len,
                            struct ble_att_read_blob_req *req)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_READ_BLOB_REQ_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;

    u8ptr[0] = BLE_ATT_OP_READ_BLOB_REQ;
    htole16(u8ptr + 1, req->babq_handle);
    htole16(u8ptr + 3, req->babq_offset);

    return 0;
}

int
ble_att_read_mult_req_parse(void *payload, int len)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_READ_MULT_REQ_BASE_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;
    if (u8ptr[0] != BLE_ATT_OP_READ_MULT_REQ) {
        return BLE_HS_EINVAL;
    }

    return 0;
}

int
ble_att_read_mult_req_write(void *payload, int len)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_READ_MULT_REQ_BASE_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;
    u8ptr[0] = BLE_ATT_OP_READ_MULT_REQ;

    return 0;
}

int
ble_att_read_mult_rsp_parse(void *payload, int len)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_READ_MULT_RSP_BASE_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;
    if (u8ptr[0] != BLE_ATT_OP_READ_MULT_RSP) {
        return BLE_HS_EINVAL;
    }

    return 0;
}

int
ble_att_read_mult_rsp_write(void *payload, int len)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_READ_MULT_RSP_BASE_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;
    u8ptr[0] = BLE_ATT_OP_READ_MULT_RSP;

    return 0;
}

int
ble_att_read_group_type_req_parse(void *payload, int len,
                                  struct ble_att_read_group_type_req *req)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_READ_GROUP_TYPE_REQ_BASE_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;

    if (u8ptr[0] != BLE_ATT_OP_READ_GROUP_TYPE_REQ) {
        return BLE_HS_EINVAL;
    }

    req->bagq_start_handle = le16toh(u8ptr + 1);
    req->bagq_end_handle = le16toh(u8ptr + 3);

    return 0;
}

int
ble_att_read_group_type_req_write(void *payload, int len,
                                  struct ble_att_read_group_type_req *req)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_READ_GROUP_TYPE_REQ_BASE_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;

    u8ptr[0] = BLE_ATT_OP_READ_GROUP_TYPE_REQ;
    htole16(u8ptr + 1, req->bagq_start_handle);
    htole16(u8ptr + 3, req->bagq_end_handle);

    return 0;
}

int
ble_att_read_group_type_rsp_parse(void *payload, int len,
                                  struct ble_att_read_group_type_rsp *rsp)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_READ_GROUP_TYPE_RSP_BASE_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;

    if (u8ptr[0] != BLE_ATT_OP_READ_GROUP_TYPE_RSP) {
        return BLE_HS_EINVAL;
    }

    rsp->bagp_length = u8ptr[1];

    return 0;
}

int
ble_att_read_group_type_rsp_write(void *payload, int len,
                                  struct ble_att_read_group_type_rsp *rsp)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_READ_GROUP_TYPE_RSP_BASE_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;

    u8ptr[0] = BLE_ATT_OP_READ_GROUP_TYPE_RSP;
    u8ptr[1] = rsp->bagp_length;

    return 0;
}

int
ble_att_write_req_parse(void *payload, int len, struct ble_att_write_req *req)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_WRITE_REQ_BASE_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;

    if (u8ptr[0] != BLE_ATT_OP_WRITE_REQ) {
        return BLE_HS_EINVAL;
    }

    req->bawq_handle = le16toh(u8ptr + 1);

    return 0;
}

int
ble_att_write_cmd_parse(void *payload, int len, struct ble_att_write_req *req)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_WRITE_REQ_BASE_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;

    if (u8ptr[0] != BLE_ATT_OP_WRITE_CMD) {
        return BLE_HS_EINVAL;
    }

    req->bawq_handle = le16toh(u8ptr + 1);

    return 0;
}

int
ble_att_write_req_write(void *payload, int len, struct ble_att_write_req *req)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_WRITE_REQ_BASE_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;

    u8ptr[0] = BLE_ATT_OP_WRITE_REQ;
    htole16(u8ptr + 1, req->bawq_handle);

    return 0;
}

int
ble_att_write_cmd_write(void *payload, int len, struct ble_att_write_req *req)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_WRITE_REQ_BASE_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;

    u8ptr[0] = BLE_ATT_OP_WRITE_CMD;
    htole16(u8ptr + 1, req->bawq_handle);

    return 0;
}

static int
ble_att_prep_write_cmd_parse(void *payload, int len,
                             struct ble_att_prep_write_cmd *cmd, int is_req)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_PREP_WRITE_CMD_BASE_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;

    if (is_req) {
        if (u8ptr[0] != BLE_ATT_OP_PREP_WRITE_REQ) {
            return BLE_HS_EINVAL;
        }
    } else {
        if (u8ptr[0] != BLE_ATT_OP_PREP_WRITE_RSP) {
            return BLE_HS_EINVAL;
        }
    }

    cmd->bapc_handle = le16toh(u8ptr + 1);
    cmd->bapc_offset = le16toh(u8ptr + 3);

    return 0;
}

static int
ble_att_prep_write_cmd_write(void *payload, int len,
                             struct ble_att_prep_write_cmd *cmd, int is_req)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_PREP_WRITE_CMD_BASE_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;

    if (is_req) {
        u8ptr[0] = BLE_ATT_OP_PREP_WRITE_REQ;
    } else {
        u8ptr[0] = BLE_ATT_OP_PREP_WRITE_RSP;
    }
    htole16(u8ptr + 1, cmd->bapc_handle);
    htole16(u8ptr + 3, cmd->bapc_offset);

    return 0;
}

int
ble_att_prep_write_req_parse(void *payload, int len,
                             struct ble_att_prep_write_cmd *cmd)
{
    return ble_att_prep_write_cmd_parse(payload, len, cmd, 1);
}

int
ble_att_prep_write_req_write(void *payload, int len,
                             struct ble_att_prep_write_cmd *cmd)
{
    return ble_att_prep_write_cmd_write(payload, len, cmd, 1);
}

int
ble_att_prep_write_rsp_parse(void *payload, int len,
                             struct ble_att_prep_write_cmd *cmd)
{
    return ble_att_prep_write_cmd_parse(payload, len, cmd, 0);
}

int
ble_att_prep_write_rsp_write(void *payload, int len,
                             struct ble_att_prep_write_cmd *cmd)
{
    return ble_att_prep_write_cmd_write(payload, len, cmd, 0);
}

int
ble_att_exec_write_req_parse(void *payload, int len,
                             struct ble_att_exec_write_req *req)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_EXEC_WRITE_REQ_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;

    if (u8ptr[0] != BLE_ATT_OP_EXEC_WRITE_REQ) {
        return BLE_HS_EINVAL;
    }

    req->baeq_flags = u8ptr[1];

    return 0;
}

int
ble_att_exec_write_req_write(void *payload, int len,
                             struct ble_att_exec_write_req *req)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_EXEC_WRITE_REQ_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;
    u8ptr[0] = BLE_ATT_OP_EXEC_WRITE_REQ;
    u8ptr[1] = req->baeq_flags;

    return 0;
}

int
ble_att_exec_write_rsp_parse(void *payload, int len)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_EXEC_WRITE_RSP_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;
    if (u8ptr[0] != BLE_ATT_OP_EXEC_WRITE_RSP) {
        return BLE_HS_EINVAL;
    }

    return 0;
}

int
ble_att_exec_write_rsp_write(void *payload, int len)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_EXEC_WRITE_RSP_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;
    u8ptr[0] = BLE_ATT_OP_EXEC_WRITE_RSP;

    return 0;
}

int
ble_att_notify_req_parse(void *payload, int len,
                         struct ble_att_notify_req *req)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_NOTIFY_REQ_BASE_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;

    if (u8ptr[0] != BLE_ATT_OP_NOTIFY_REQ) {
        return BLE_HS_EINVAL;
    }

    req->banq_handle = le16toh(u8ptr + 1);

    return 0;

}

int
ble_att_notify_req_write(void *payload, int len,
                         struct ble_att_notify_req *req)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_NOTIFY_REQ_BASE_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;
    u8ptr[0] = BLE_ATT_OP_NOTIFY_REQ;
    htole16(u8ptr + 1, req->banq_handle);

    return 0;
}

int
ble_att_indicate_req_parse(void *payload, int len,
                           struct ble_att_indicate_req *req)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_INDICATE_REQ_BASE_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;

    if (u8ptr[0] != BLE_ATT_OP_INDICATE_REQ) {
        return BLE_HS_EINVAL;
    }

    req->baiq_handle = le16toh(u8ptr + 1);

    return 0;

}

int
ble_att_indicate_req_write(void *payload, int len,
                         struct ble_att_indicate_req *req)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_INDICATE_REQ_BASE_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;
    u8ptr[0] = BLE_ATT_OP_INDICATE_REQ;
    htole16(u8ptr + 1, req->baiq_handle);

    return 0;
}

int
ble_att_indicate_rsp_parse(void *payload, int len)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_INDICATE_RSP_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;
    if (u8ptr[0] != BLE_ATT_OP_INDICATE_RSP) {
        return BLE_HS_EINVAL;
    }

    return 0;
}

int
ble_att_indicate_rsp_write(void *payload, int len)
{
    uint8_t *u8ptr;

    if (len < BLE_ATT_INDICATE_RSP_SZ) {
        return BLE_HS_EMSGSIZE;
    }

    u8ptr = payload;
    u8ptr[0] = BLE_ATT_OP_INDICATE_RSP;

    return 0;
}
