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

#include <string.h>
#include <errno.h>
#include <limits.h>
#include "testutil/testutil.h"
#include "nimble/ble.h"
#include "host/ble_hs_test.h"
#include "host/ble_uuid.h"
#include "ble_hs_priv.h"
#include "ble_gatt_priv.h"
#include "ble_att_cmd.h"
#include "ble_hs_conn.h"
#include "ble_hs_test_util.h"

struct ble_gatt_read_test_attr {
    uint16_t conn_handle;
    uint16_t handle;
    uint8_t value_len;
    uint8_t value[BLE_ATT_ATTR_MAX_LEN];
};

#define BLE_GATT_READ_TEST_MAX_ATTRS    256

struct ble_gatt_read_test_attr
    ble_gatt_read_test_attrs[BLE_GATT_READ_TEST_MAX_ATTRS];
int ble_gatt_read_test_num_attrs;
int ble_gatt_read_test_complete;

uint16_t ble_gatt_read_test_bad_conn_handle;
int ble_gatt_read_test_bad_status;

uint16_t ble_gatt_read_test_mult_handles[BLE_GATT_READ_TEST_MAX_ATTRS];
uint8_t ble_gatt_read_test_mult_num_handles;

static void
ble_gatt_read_test_misc_init(void)
{
    ble_hs_test_util_init();
    ble_gatt_read_test_num_attrs = 0;
    ble_gatt_read_test_complete = 0;
    ble_gatt_read_test_bad_conn_handle = 0;
    ble_gatt_read_test_bad_status = 0;

    memset(&ble_gatt_read_test_attrs[0], 0,
           sizeof ble_gatt_read_test_attrs[0]);
}

static int
ble_gatt_read_test_cb(uint16_t conn_handle, struct ble_gatt_error *error,
                      struct ble_gatt_attr *attr, void *arg)
{
    struct ble_gatt_read_test_attr *dst;
    int *stop_after;

    stop_after = arg;

    if (error != NULL) {
        ble_gatt_read_test_bad_conn_handle = conn_handle;
        ble_gatt_read_test_bad_status = error->status;
        ble_gatt_read_test_complete = 1;
        return 0;
    }

    if (attr == NULL) {
        ble_gatt_read_test_complete = 1;
        return 0;
    }

    TEST_ASSERT_FATAL(ble_gatt_read_test_num_attrs <
                      BLE_GATT_READ_TEST_MAX_ATTRS);
    dst = ble_gatt_read_test_attrs + ble_gatt_read_test_num_attrs++;

    TEST_ASSERT_FATAL(attr->value_len <= sizeof dst->value);

    dst->conn_handle = conn_handle;
    dst->handle = attr->handle;
    dst->value_len = attr->value_len;
    memcpy(dst->value, attr->value, attr->value_len);

    if (stop_after != NULL && *stop_after > 0) {
        (*stop_after)--;
        if (*stop_after == 0) {
            ble_gatt_read_test_complete = 1;
            return 1;
        }
    }

    return 0;
}

static int
ble_gatt_read_test_long_cb(uint16_t conn_handle, struct ble_gatt_error *error,
                           struct ble_gatt_attr *attr, void *arg)
{
    struct ble_gatt_read_test_attr *dst;
    int *reads_left;

    reads_left = arg;

    if (error != NULL) {
        ble_gatt_read_test_bad_conn_handle = conn_handle;
        ble_gatt_read_test_bad_status = error->status;
        ble_gatt_read_test_complete = 1;
        return 0;
    }

    if (attr == NULL) {
        ble_gatt_read_test_complete = 1;
        return 0;
    }

    dst = ble_gatt_read_test_attrs + 0;

    TEST_ASSERT_FATAL(attr->value_len <= dst->value_len + sizeof dst->value);
    TEST_ASSERT(attr->offset == dst->value_len);

    if (attr->offset == 0) {
        dst->conn_handle = conn_handle;
        dst->handle = attr->handle;
    } else {
        TEST_ASSERT(conn_handle == dst->conn_handle);
        TEST_ASSERT(attr->handle == dst->handle);
    }
    memcpy(dst->value + dst->value_len, attr->value, attr->value_len);
    dst->value_len += attr->value_len;

    if (reads_left != NULL && *reads_left > 0) {
        (*reads_left)--;
        if (*reads_left == 0) {
            ble_gatt_read_test_complete = 1;
            return 1;
        }
    }

    return 0;
}

static int
ble_gatt_read_test_mult_cb(uint16_t conn_handle, struct ble_gatt_error *error,
                           uint16_t *attr_handles, uint8_t num_attr_handles,
                           uint8_t *attr_data, uint16_t attr_data_len,
                           void *arg)
{
    struct ble_gatt_read_test_attr *dst;
    int i;

    ble_gatt_read_test_mult_num_handles = num_attr_handles;
    for (i = 0; i < num_attr_handles; i++) {
        ble_gatt_read_test_mult_handles[i] = attr_handles[i];
    }

    if (error != NULL) {
        ble_gatt_read_test_bad_conn_handle = conn_handle;
        ble_gatt_read_test_bad_status = error->status;
        ble_gatt_read_test_complete = 1;
        return 0;
    }

    if (attr_data == NULL) {
        ble_gatt_read_test_complete = 1;
        return 0;
    }

    dst = ble_gatt_read_test_attrs + ble_gatt_read_test_num_attrs++;

    TEST_ASSERT_FATAL(attr_data_len <= sizeof dst->value);

    dst->conn_handle = conn_handle;
    dst->handle = 0;
    dst->value_len = attr_data_len;
    memcpy(dst->value, attr_data, attr_data_len);

    ble_gatt_read_test_complete = 1;

    return 0;
}

static void
ble_gatt_read_test_misc_rx_rsp_good_raw(struct ble_hs_conn *conn,
                                        uint8_t att_op,
                                        void *data, int data_len)
{
    struct ble_l2cap_chan *chan;
    uint8_t buf[1024];
    int rc;

    TEST_ASSERT_FATAL(data_len <= sizeof buf);

    /* Send the pending ATT Read Request. */
    ble_hs_test_util_tx_all();

    buf[0] = att_op;
    memcpy(buf + 1, data, data_len);

    chan = ble_hs_conn_chan_find(conn, BLE_L2CAP_CID_ATT);
    TEST_ASSERT_FATAL(chan != NULL);

    rc = ble_hs_test_util_l2cap_rx_payload_flat(conn, chan, buf,
                                                1 + data_len);
    TEST_ASSERT(rc == 0);
}

static void
ble_gatt_read_test_misc_rx_rsp_good(struct ble_hs_conn *conn,
                                    struct ble_gatt_attr *attr)
{
    ble_gatt_read_test_misc_rx_rsp_good_raw(conn, BLE_ATT_OP_READ_RSP,
                                            attr->value,
                                            attr->value_len);
}

static void
ble_gatt_read_test_misc_rx_rsp_bad(struct ble_hs_conn *conn,
                                   uint8_t att_error, uint16_t err_handle)
{
    /* Send the pending ATT Read Request. */
    ble_hs_test_util_tx_all();

    ble_hs_test_util_rx_att_err_rsp(conn, BLE_ATT_OP_READ_REQ, att_error,
                                    err_handle);
}

static int
ble_gatt_read_test_misc_uuid_rx_rsp_good(struct ble_hs_conn *conn,
                                         struct ble_gatt_attr *attrs)
{
    struct ble_att_read_type_rsp rsp;
    struct ble_l2cap_chan *chan;
    uint8_t buf[1024];
    int prev_len;
    int off;
    int rc;
    int i;

    if (ble_gatt_read_test_complete || attrs[0].handle == 0) {
        return 0;
    }

    /* Send the pending ATT Read By Type Request. */
    ble_hs_test_util_tx_all();

    rsp.batp_length = 2 + attrs[0].value_len;
    rc = ble_att_read_type_rsp_write(buf, sizeof buf, &rsp);
    TEST_ASSERT_FATAL(rc == 0);

    prev_len = 0;
    off = BLE_ATT_READ_TYPE_RSP_BASE_SZ;
    for (i = 0; attrs[i].handle != 0; i++) {
        if (prev_len != 0 && prev_len != attrs[i].value_len) {
            break;
        }
        prev_len = attrs[i].value_len;

        htole16(buf + off, attrs[i].handle);
        off += 2;

        memcpy(buf + off, attrs[i].value, attrs[i].value_len);
        off += attrs[i].value_len;
    }

    chan = ble_hs_conn_chan_find(conn, BLE_L2CAP_CID_ATT);
    TEST_ASSERT_FATAL(chan != NULL);

    rc = ble_hs_test_util_l2cap_rx_payload_flat(conn, chan, buf, off);
    TEST_ASSERT(rc == 0);

    return i;
}

static void
ble_gatt_read_test_misc_verify_good(struct ble_gatt_attr *attr)
{
    struct ble_hs_conn *conn;
    int rc;

    ble_gatt_read_test_misc_init();
    conn = ble_hs_test_util_create_conn(2, ((uint8_t[]){2,3,4,5,6,7,8,9}),
                                        NULL, NULL);

    rc = ble_gattc_read(conn->bhc_handle, attr->handle, ble_gatt_read_test_cb,
                        NULL);
    TEST_ASSERT_FATAL(rc == 0);

    ble_gatt_read_test_misc_rx_rsp_good(conn, attr);

    TEST_ASSERT(ble_gatt_read_test_num_attrs == 1);
    TEST_ASSERT(ble_gatt_read_test_attrs[0].conn_handle == conn->bhc_handle);
    TEST_ASSERT(ble_gatt_read_test_attrs[0].handle == attr->handle);
    TEST_ASSERT(ble_gatt_read_test_attrs[0].value_len == attr->value_len);
    TEST_ASSERT(memcmp(ble_gatt_read_test_attrs[0].value, attr->value,
                       attr->value_len) == 0);
}

static void
ble_gatt_read_test_misc_verify_bad(uint8_t att_status,
                                   struct ble_gatt_attr *attr)
{
    struct ble_hs_conn *conn;
    int rc;

    ble_gatt_read_test_misc_init();
    conn = ble_hs_test_util_create_conn(2, ((uint8_t[]){2,3,4,5,6,7,8,9}),
                                        NULL, NULL);

    rc = ble_gattc_read(conn->bhc_handle, attr->handle, ble_gatt_read_test_cb,
                        NULL);
    TEST_ASSERT_FATAL(rc == 0);

    ble_gatt_read_test_misc_rx_rsp_bad(conn, att_status, attr->handle);

    TEST_ASSERT(ble_gatt_read_test_num_attrs == 0);
    TEST_ASSERT(ble_gatt_read_test_bad_conn_handle == conn->bhc_handle);
    TEST_ASSERT(ble_gatt_read_test_bad_status ==
                BLE_HS_ERR_ATT_BASE + att_status);
    TEST_ASSERT(!ble_gattc_any_jobs());
}

static void
ble_gatt_read_test_misc_uuid_verify_good(uint16_t start_handle,
                                         uint16_t end_handle, void *uuid128,
                                         int stop_after,
                                         struct ble_gatt_attr *attrs)
{
    struct ble_hs_conn *conn;
    int num_read;
    int idx;
    int rc;
    int i;

    ble_gatt_read_test_misc_init();
    conn = ble_hs_test_util_create_conn(2, ((uint8_t[]){2,3,4,5,6,7,8,9}),
                                        NULL, NULL);

    rc = ble_gattc_read_by_uuid(conn->bhc_handle, start_handle, end_handle,
                                uuid128, ble_gatt_read_test_cb, &stop_after);
    TEST_ASSERT_FATAL(rc == 0);

    idx = 0;
    while (1) {
        num_read = ble_gatt_read_test_misc_uuid_rx_rsp_good(conn, attrs + idx);
        if (num_read == 0) {
            ble_hs_test_util_tx_all();
            ble_hs_test_util_rx_att_err_rsp(conn, BLE_ATT_OP_READ_TYPE_REQ,
                                            BLE_ATT_ERR_ATTR_NOT_FOUND,
                                            start_handle);
            break;
        }

        idx += num_read;
    }

    TEST_ASSERT(ble_gatt_read_test_complete);
    TEST_ASSERT(idx == ble_gatt_read_test_num_attrs);

    for (i = 0; i < idx; i++) {
        TEST_ASSERT(ble_gatt_read_test_attrs[i].conn_handle ==
                    conn->bhc_handle);
        TEST_ASSERT(ble_gatt_read_test_attrs[i].handle == attrs[i].handle);
        TEST_ASSERT(ble_gatt_read_test_attrs[i].value_len ==
                    attrs[i].value_len);
        TEST_ASSERT(memcmp(ble_gatt_read_test_attrs[i].value, attrs[i].value,
                           attrs[i].value_len) == 0);
    }
    TEST_ASSERT(!ble_gattc_any_jobs());
}

static void
ble_gatt_read_test_misc_long_verify_good(int max_reads,
                                         struct ble_gatt_attr *attr)
{
    struct ble_hs_conn *conn;
    int reads_left;
    int chunk_sz;
    int rem_len;
    int att_op;
    int off;
    int rc;

    ble_gatt_read_test_misc_init();
    conn = ble_hs_test_util_create_conn(2, ((uint8_t[]){2,3,4,5,6,7,8,9}),
                                        NULL, NULL);

    if (max_reads == 0) {
        max_reads = INT_MAX;
    }
    reads_left = max_reads;
    rc = ble_gattc_read_long(conn->bhc_handle, attr->handle,
                             ble_gatt_read_test_long_cb, &reads_left);
    TEST_ASSERT_FATAL(rc == 0);

    off = 0;
    rem_len = attr->value_len;
    do {
        if (rem_len > BLE_ATT_MTU_DFLT - 1) {
            chunk_sz = BLE_ATT_MTU_DFLT - 1;
        } else {
            chunk_sz = rem_len;
        }
        if (off == 0) {
            att_op = BLE_ATT_OP_READ_RSP;
        } else {
            att_op = BLE_ATT_OP_READ_BLOB_RSP;
        }
        ble_gatt_read_test_misc_rx_rsp_good_raw(conn, att_op,
                                                attr->value + off, chunk_sz);
        rem_len -= chunk_sz;
        off += chunk_sz;
    } while (rem_len > 0 && reads_left > 0);

    TEST_ASSERT(ble_gatt_read_test_complete);
    TEST_ASSERT(!ble_gattc_any_jobs());
    TEST_ASSERT(ble_gatt_read_test_attrs[0].conn_handle == conn->bhc_handle);
    TEST_ASSERT(ble_gatt_read_test_attrs[0].handle == attr->handle);
    if (reads_left > 0) {
        TEST_ASSERT(ble_gatt_read_test_attrs[0].value_len == attr->value_len);
    }
    TEST_ASSERT(memcmp(ble_gatt_read_test_attrs[0].value, attr->value,
                       ble_gatt_read_test_attrs[0].value_len) == 0);
}

static void
ble_gatt_read_test_misc_long_verify_bad(uint8_t att_status,
                                        struct ble_gatt_attr *attr)
{
    struct ble_hs_conn *conn;
    int rc;

    ble_gatt_read_test_misc_init();
    conn = ble_hs_test_util_create_conn(2, ((uint8_t[]){2,3,4,5,6,7,8,9}),
                                        NULL, NULL);

    rc = ble_gattc_read_long(conn->bhc_handle, attr->handle,
                             ble_gatt_read_test_long_cb, NULL);
    TEST_ASSERT_FATAL(rc == 0);

    ble_gatt_read_test_misc_rx_rsp_bad(conn, att_status, attr->handle);

    TEST_ASSERT(ble_gatt_read_test_num_attrs == 0);
    TEST_ASSERT(ble_gatt_read_test_bad_conn_handle == conn->bhc_handle);
    TEST_ASSERT(ble_gatt_read_test_bad_status ==
                BLE_HS_ERR_ATT_BASE + att_status);
    TEST_ASSERT(!ble_gattc_any_jobs());
}

static int
ble_gatt_read_test_misc_extract_handles(struct ble_gatt_attr *attrs,
                                        uint16_t *handles)
{
    int i;

    for (i = 0; attrs[i].handle != 0; i++) {
        handles[i] = attrs[i].handle;
    }
    return i;
}

static void
ble_gatt_read_test_misc_mult_verify_good(struct ble_gatt_attr *attrs)
{
    uint8_t expected_value[512];
    struct ble_hs_conn *conn;
    uint16_t handles[256];
    int num_attrs;
    int chunk_sz;
    int off;
    int rc;
    int i;

    ble_gatt_read_test_misc_init();
    conn = ble_hs_test_util_create_conn(2, ((uint8_t[]){2,3,4,5,6,7,8,9}),
                                        NULL, NULL);

    num_attrs = ble_gatt_read_test_misc_extract_handles(attrs, handles);

    off = 0;
    for (i = 0; i < num_attrs; i++) {
        if (attrs[i].value_len > BLE_ATT_MTU_DFLT - 1 - off) {
            chunk_sz = BLE_ATT_MTU_DFLT - 1 - off;
        } else {
            chunk_sz = attrs[i].value_len;
        }

        if (chunk_sz > 0) {
            memcpy(expected_value + off, attrs[i].value, chunk_sz);
            off += chunk_sz;
        }
    }

    rc = ble_gattc_read_mult(conn->bhc_handle, handles, num_attrs,
                             ble_gatt_read_test_mult_cb, NULL);
    TEST_ASSERT_FATAL(rc == 0);

    ble_gatt_read_test_misc_rx_rsp_good_raw(conn, BLE_ATT_OP_READ_MULT_RSP,
                                            expected_value, off);

    TEST_ASSERT(ble_gatt_read_test_complete);
    TEST_ASSERT(!ble_gattc_any_jobs());
    TEST_ASSERT(ble_gatt_read_test_attrs[0].conn_handle == conn->bhc_handle);
    TEST_ASSERT(ble_gatt_read_test_mult_num_handles == num_attrs);
    TEST_ASSERT(memcmp(ble_gatt_read_test_mult_handles, handles,
                       num_attrs * 2) == 0);
    TEST_ASSERT(ble_gatt_read_test_attrs[0].value_len == off);
    TEST_ASSERT(memcmp(ble_gatt_read_test_attrs[0].value, expected_value,
                       off) == 0);
}

static void
ble_gatt_read_test_misc_mult_verify_bad(uint8_t att_status,
                                        uint16_t err_handle,
                                        struct ble_gatt_attr *attrs)
{
    struct ble_hs_conn *conn;
    uint16_t handles[256];
    int num_attrs;
    int rc;

    ble_gatt_read_test_misc_init();
    conn = ble_hs_test_util_create_conn(2, ((uint8_t[]){2,3,4,5,6,7,8,9}),
                                        NULL, NULL);

    num_attrs = ble_gatt_read_test_misc_extract_handles(attrs, handles);

    rc = ble_gattc_read_mult(conn->bhc_handle, handles, num_attrs,
                             ble_gatt_read_test_mult_cb, NULL);
    TEST_ASSERT_FATAL(rc == 0);

    ble_gatt_read_test_misc_rx_rsp_bad(conn, att_status, err_handle);

    TEST_ASSERT(ble_gatt_read_test_num_attrs == 0);
    TEST_ASSERT(ble_gatt_read_test_bad_conn_handle == conn->bhc_handle);
    TEST_ASSERT(ble_gatt_read_test_mult_num_handles == num_attrs);
    TEST_ASSERT(memcmp(ble_gatt_read_test_mult_handles, handles,
                       num_attrs * 2) == 0);
    TEST_ASSERT(ble_gatt_read_test_bad_status ==
                BLE_HS_ERR_ATT_BASE + att_status);
    TEST_ASSERT(!ble_gattc_any_jobs());
}

TEST_CASE(ble_gatt_read_test_by_handle)
{
    /* Read a seven-byte attribute. */
    ble_gatt_read_test_misc_verify_good((struct ble_gatt_attr[]) { {
        .handle = 43,
        .value = (uint8_t[]){ 1,2,3,4,5,6,7 },
        .value_len = 7
    } });

    /* Read a one-byte attribute. */
    ble_gatt_read_test_misc_verify_good((struct ble_gatt_attr[]) { {
        .handle = 0x5432,
        .value = (uint8_t[]){ 0xff },
        .value_len = 1
    } });

    /* Read a 200-byte attribute. */
    ble_gatt_read_test_misc_verify_good((struct ble_gatt_attr[]) { {
        .handle = 815,
        .value = (uint8_t[200]){ 0 },
        .value_len = 200,
    } });

    /* Fail due to attribute not found. */
    ble_gatt_read_test_misc_verify_bad(BLE_ATT_ERR_ATTR_NOT_FOUND,
        (struct ble_gatt_attr[]) { {
            .handle = 719,
            .value = (uint8_t[]){ 1,2,3,4,5,6,7 },
            .value_len = 7
        } });

    /* Fail due to invalid PDU. */
    ble_gatt_read_test_misc_verify_bad(BLE_ATT_ERR_INVALID_PDU,
        (struct ble_gatt_attr[]) { {
            .handle = 65,
            .value = (uint8_t[]){ 0xfa, 0x4c },
            .value_len = 2
        } });
}

TEST_CASE(ble_gatt_read_test_by_uuid)
{
    /* Read a single seven-byte attribute. */
    ble_gatt_read_test_misc_uuid_verify_good(1, 100, BLE_UUID16(0x1234), 0,
        (struct ble_gatt_attr[]) { {
            .handle = 43,
            .value = (uint8_t[]){ 1,2,3,4,5,6,7 },
            .value_len = 7
        }, {
            0,
        } });

    /* Read two seven-byte attributes; one response. */
    ble_gatt_read_test_misc_uuid_verify_good(1, 100, BLE_UUID16(0x1234), 0,
        (struct ble_gatt_attr[]) { {
            .handle = 43,
            .value = (uint8_t[]){ 1,2,3,4,5,6,7 },
            .value_len = 7
        }, {
            .handle = 44,
            .value = (uint8_t[]){ 2,3,4,5,6,7,8 },
            .value_len = 7
        }, {
            0,
        } });

    /* Read two attributes; two responses. */
    ble_gatt_read_test_misc_uuid_verify_good(1, 100, BLE_UUID16(0x1234), 0,
        (struct ble_gatt_attr[]) { {
            .handle = 43,
            .value = (uint8_t[]){ 1,2,3,4,5,6,7 },
            .value_len = 7
        }, {
            .handle = 44,
            .value = (uint8_t[]){ 2,3,4 },
            .value_len = 3
        }, {
            0,
        } });

    /* Stop after three reads. */
    ble_gatt_read_test_misc_uuid_verify_good(1, 100, BLE_UUID16(0x1234), 3,
        (struct ble_gatt_attr[]) { {
            .handle = 43,
            .value = (uint8_t[]){ 1,2,3,4,5,6,7 },
            .value_len = 7
        }, {
            .handle = 44,
            .value = (uint8_t[]){ 2,3,4 },
            .value_len = 3
        }, {
            .handle = 45,
            .value = (uint8_t[]){ 2,3,4 },
            .value_len = 3
        }, {
            .handle = 46,
            .value = (uint8_t[]){ 3,4,5,6 },
            .value_len = 4
        }, {
            .handle = 47,
            .value = (uint8_t[]){ 2,3,4 },
            .value_len = 3
        }, {
            0,
        } });
}

TEST_CASE(ble_gatt_read_test_long)
{
    uint8_t data512[512];
    int i;

    for (i = 0; i < sizeof data512; i++) {
        data512[i] = i;
    }

    /* Read a seven-byte attribute. */
    ble_gatt_read_test_misc_long_verify_good(0, (struct ble_gatt_attr[]) { {
        .handle = 43,
        .value = data512,
        .value_len = 7
    } });

    /* Read a zero-byte attribute. */
    ble_gatt_read_test_misc_long_verify_good(0, (struct ble_gatt_attr[]) { {
        .handle = 43,
        .value = NULL,
        .value_len = 0
    } });

    /* Read a 60-byte attribute; three requests. */
    ble_gatt_read_test_misc_long_verify_good(0, (struct ble_gatt_attr[]) { {
        .handle = 34,
        .value = data512,
        .value_len = 60
    } });

    /* Stop after two reads. */
    ble_gatt_read_test_misc_long_verify_good(2, (struct ble_gatt_attr[]) { {
        .handle = 34,
        .value = data512,
        .value_len = 60
    } });

    /* Fail due to attribute not found. */
    ble_gatt_read_test_misc_long_verify_bad(BLE_ATT_ERR_ATTR_NOT_FOUND,
        (struct ble_gatt_attr[]) { {
            .handle = 719,
            .value = (uint8_t[]){ 1,2,3,4,5,6,7 },
            .value_len = 7
        } });
}

TEST_CASE(ble_gatt_read_test_mult)
{
    uint8_t data512[512];
    int i;

    for (i = 0; i < sizeof data512; i++) {
        data512[i] = i;
    }

    /* Read one attribute. */
    ble_gatt_read_test_misc_mult_verify_good((struct ble_gatt_attr[]) { {
        .handle = 43,
        .value = data512,
        .value_len = 7
    }, {
        0
    } });

    /* Read two attributes. */
    ble_gatt_read_test_misc_mult_verify_good((struct ble_gatt_attr[]) { {
        .handle = 43,
        .value = data512,
        .value_len = 7,
    }, {
        .handle = 44,
        .value = data512 + 7,
        .value_len = 4,
    }, {
        0
    } });

    /* Read two attributes (swap order). */
    ble_gatt_read_test_misc_mult_verify_good((struct ble_gatt_attr[]) { {
        .handle = 44,
        .value = data512 + 7,
        .value_len = 4,
    }, {
        .handle = 43,
        .value = data512,
        .value_len = 7,
    }, {
        0
    } });

    /* Read five attributes. */
    ble_gatt_read_test_misc_mult_verify_good((struct ble_gatt_attr[]) { {
        .handle = 43,
        .value = data512,
        .value_len = 7,
    }, {
        .handle = 44,
        .value = data512 + 7,
        .value_len = 4,
    }, {
        .handle = 145,
        .value = data512 + 11,
        .value_len = 2,
    }, {
        .handle = 191,
        .value = data512 + 13,
        .value_len = 3,
    }, {
        .handle = 352,
        .value = data512 + 16,
        .value_len = 4,
    }, {
        0
    } });

    /* Fail due to attribute not found. */
    ble_gatt_read_test_misc_mult_verify_bad(BLE_ATT_ERR_ATTR_NOT_FOUND, 719,
        (struct ble_gatt_attr[]) { {
            .handle = 719,
            .value = (uint8_t[]){ 1,2,3,4,5,6,7 },
            .value_len = 7
        }, {
            0
        } });
}

TEST_SUITE(ble_gatt_read_test_suite)
{
    ble_gatt_read_test_by_handle();
    ble_gatt_read_test_by_uuid();
    ble_gatt_read_test_long();
    ble_gatt_read_test_mult();
}

int
ble_gatt_read_test_all(void)
{
    ble_gatt_read_test_suite();

    return tu_any_failed;
}
