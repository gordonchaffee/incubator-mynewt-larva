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
#include <stdint.h>
#include "nimble/ble.h"

void
htole16(void *buf, uint16_t x)
{
    uint8_t *u8ptr;

    u8ptr = buf;
    u8ptr[0] = (uint8_t)x;
    u8ptr[1] = (uint8_t)(x >> 8);
}

void
htole32(void *buf, uint32_t x)
{
    uint8_t *u8ptr;

    u8ptr = buf;
    u8ptr[0] = (uint8_t)x;
    u8ptr[1] = (uint8_t)(x >> 8);
    u8ptr[2] = (uint8_t)(x >> 16);
    u8ptr[3] = (uint8_t)(x >> 24);
}

void
htole64(void *buf, uint64_t x)
{
    uint8_t *u8ptr;

    u8ptr = buf;
    u8ptr[0] = (uint8_t)x;
    u8ptr[1] = (uint8_t)(x >> 8);
    u8ptr[2] = (uint8_t)(x >> 16);
    u8ptr[3] = (uint8_t)(x >> 24);
    u8ptr[4] = (uint8_t)(x >> 32);
    u8ptr[5] = (uint8_t)(x >> 40);
    u8ptr[6] = (uint8_t)(x >> 48);
    u8ptr[7] = (uint8_t)(x >> 56);
}

uint16_t
le16toh(void *buf)
{
    uint16_t x;
    uint8_t *u8ptr;

    u8ptr = buf;
    x = u8ptr[0];
    x |= (uint16_t)u8ptr[1] << 8;

    return x;
}

uint32_t
le32toh(void *buf)
{
    uint32_t x;
    uint8_t *u8ptr;

    u8ptr = buf;
    x = u8ptr[0];
    x |= (uint32_t)u8ptr[1] << 8;
    x |= (uint32_t)u8ptr[2] << 16;
    x |= (uint32_t)u8ptr[3] << 24;

    return x;
}

uint64_t
le64toh(void *buf)
{
    uint64_t x;
    uint8_t *u8ptr;

    u8ptr = buf;
    x = u8ptr[0];
    x |= (uint64_t)u8ptr[1] << 8;
    x |= (uint64_t)u8ptr[2] << 16;
    x |= (uint64_t)u8ptr[3] << 24;
    x |= (uint64_t)u8ptr[4] << 32;
    x |= (uint64_t)u8ptr[5] << 40;
    x |= (uint64_t)u8ptr[6] << 48;
    x |= (uint64_t)u8ptr[7] << 54;

    return x;
}
