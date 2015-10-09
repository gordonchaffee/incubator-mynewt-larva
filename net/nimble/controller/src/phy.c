/**
 * Copyright (c) 2015 Stack Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdint.h>
#include <assert.h>
#include "os/os.h"
#include "bsp/cmsis_nvic.h"
#include "controller/phy.h"
#include "controller/ll.h"
#include "hal/hal_cputime.h"
#include "mcu/nrf52.h"
#include "mcu/nrf52_bitfields.h"

/* To disable all radio interrupts */
#define NRF52_RADIO_IRQ_MASK_ALL    (0x34FF)

/* 
 * We configure the nrf52 with a 1 byte S0 field, 8 bit length field, and
 * zero bit S1 field. The preamble is 8 bits long.
 */
#define NRF52_LFLEN_BITS        (8)
#define NRF52_S0_LEN            (1)

/* Maximum length of frames */
#define NRF52_MAXLEN            (255)
#define NRF52_BALEN             (3)     /* For base address of 3 bytes */

/* Maximum tx power */
#define NRF52_TX_PWR_MAX_DBM    (4)
#define NRF52_TX_PWR_MIN_DBM    (-40)

/* BLE PHY data structure */
struct ble_phy_obj
{
    int8_t  txpwr_dbm;
    uint8_t chan;
    uint8_t mode;
};
struct ble_phy_obj g_ble_phy_data;

/* XXX: for now */
uint8_t g_ble_phy_rx_buffer[258];

/* Statistics */
struct ble_phy_statistics
{
    uint32_t tx_good;
    uint32_t tx_fail;
    uint32_t tx_bytes;
    uint32_t rx_starts;
    uint32_t rx_valid;
    uint32_t rx_crc_err;
    uint32_t phy_isrs;
};
struct ble_phy_statistics g_ble_phy_stats;

/**
 * cputime isr 
 *  
 * This is the global timer interrupt routine. 
 * 
 */
static void
ble_phy_isr(void)
{
    uint32_t x;

    /* Check interrupt source. If set, clear them */
    if (NRF_RADIO->EVENTS_DISABLED) {
        NRF_RADIO->EVENTS_DISABLED = 0;
        NRF_RADIO->INTENCLR = RADIO_INTENCLR_DISABLED_Msk; 

        if (g_ble_phy_data.mode == BLE_PHY_MODE_TX_RX) {
            /* Packet pointer needs to be reset */
            NRF_RADIO->PACKETPTR = (uint32_t)&g_ble_phy_rx_buffer[0];

            /* XXX: what other rx stuff? */
        }
    }

    /* XXX: dummy read since it was suggested by nordic */
    x = NRF_RADIO->STATE;
    assert(x < 0xFFFF);

    /* Count # of interrupts */
    ++g_ble_phy_stats.phy_isrs;
}

/**
 * ble phy init 
 *  
 * Initialize the PHY. This is expected to be called once. 
 * 
 * @return int 0: success; -1 otherwise
 */
int
ble_phy_init(void)
{
    uint32_t os_tmo;

    /* Make sure HFXO is started */
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART = 1;
    os_tmo = os_time_get() + (5 * (1000 / OS_TICKS_PER_SEC));
    while (1) {
        if (NRF_CLOCK->EVENTS_HFCLKSTARTED) {
            break;
        }
        if ((int32_t)(os_time_get() - os_tmo) > 0) {
            return -1;
        }
    }

    /* Set phy channel to an invalid channel so first set channel works */
    g_ble_phy_data.chan = BLE_PHY_NUM_CHANS;

    /* Toggle peripheral power to reset (just in case) */
    NRF_RADIO->POWER = 0;
    NRF_RADIO->POWER = 1;

    /* Disable all interrupts */
    NRF_RADIO->INTENCLR = NRF52_RADIO_IRQ_MASK_ALL;

    /* Set configuration registers */
    NRF_RADIO->MODE = RADIO_MODE_MODE_Ble_1Mbit;
    NRF_RADIO->PCNF0 = (NRF52_LFLEN_BITS << RADIO_PCNF0_LFLEN_Pos) |
                       (NRF52_S0_LEN << RADIO_PCNF0_S0LEN_Pos) |
                       (RADIO_PCNF0_PLEN_8bit << RADIO_PCNF0_PLEN_Pos);
    NRF_RADIO->PCNF1 = NRF52_MAXLEN | 
                       (RADIO_PCNF1_ENDIAN_Little <<  RADIO_PCNF1_ENDIAN_Pos) |
                       (NRF52_BALEN << RADIO_PCNF1_BALEN_Pos) |
                       RADIO_PCNF1_WHITEEN_Msk;

    /* Set base0 with the advertising access address */
    NRF_RADIO->BASE0 = (BLE_ACCESS_ADDR_ADV << 8) & 0xFFFFFF00;
    NRF_RADIO->PREFIX0 = (BLE_ACCESS_ADDR_ADV >> 24) & 0xFF;

    /* Configure the CRC registers */
    NRF_RADIO->CRCCNF = RADIO_CRCCNF_SKIPADDR_Msk | RADIO_CRCCNF_LEN_Three;

    /* Configure BLE poly */
    NRF_RADIO->CRCPOLY = 0x0100065B;

    /* Configure IFS */
    NRF_RADIO->TIFS = BLE_LL_IFS;

    /* Set isr in vector table and enable interrupt */
    NVIC_SetVector(RADIO_IRQn, (uint32_t)ble_phy_isr);
    NVIC_EnableIRQ(RADIO_IRQn);

    return 0;
}

int
ble_phy_tx(struct os_mbuf *txpdu, uint8_t mode)
{
    uint32_t state;

    /* XXX: do a double check on radio state */
    if (NRF_RADIO->STATE != RADIO_STATE_STATE_Disabled) {
        return -1;
    }

    /* Select tx address */
    if (g_ble_phy_data.chan < BLE_PHY_NUM_DATA_CHANS) {
        /* XXX: fix this */
        assert(0);
        NRF_RADIO->TXADDRESS = 0;
        NRF_RADIO->CRCINIT = 0;
    } else {
        NRF_RADIO->TXADDRESS = 0;
        NRF_RADIO->CRCINIT = BLE_LL_CRCINIT_ADV;
    }

    /* XXX: assume flat buffer for now */
    NRF_RADIO->PACKETPTR = (uint32_t)txpdu->om_data;

    /* Clear the ready, end and disabled events */
    NRF_RADIO->EVENTS_READY = 0;
    NRF_RADIO->EVENTS_END = 0;
    NRF_RADIO->EVENTS_DISABLED = 0;

    /* Enable shortcuts for transmit start/end. */
    if (mode == BLE_PHY_MODE_TX_RX) {
        NRF_RADIO->SHORTS = RADIO_SHORTS_END_DISABLE_Msk | 
                            RADIO_SHORTS_READY_START_Msk |
                            RADIO_SHORTS_DISABLED_RXEN_Msk;
        NRF_RADIO->INTENSET = RADIO_INTENSET_DISABLED_Msk; 
    } else {
        NRF_RADIO->SHORTS = RADIO_SHORTS_END_DISABLE_Msk | 
                            RADIO_SHORTS_READY_START_Msk;
    }
    g_ble_phy_data.mode = (uint8_t)mode;

    /* Trigger the start task. This should trigger start of packet */
    NRF_RADIO->TASKS_TXEN = 1;

    /* Read back radio state. If in TXRU, we are fine */
    state = NRF_RADIO->STATE;
    if ((state == RADIO_STATE_STATE_TxRu) || (state == RADIO_STATE_STATE_Tx)) {
        /* Increment number of transmitted frames */
        ++g_ble_phy_stats.tx_good;
        g_ble_phy_stats.tx_bytes += txpdu->om_len; /* XXX: use pkthdr */
    } else {
        /* Frame failed to transmit */
        ++g_ble_phy_stats.tx_fail;
    }

    return 0;
}

/**
 * ble phy txpwr set 
 *  
 * Set the transmit output power (in dBm). 
 *  
 * NOTE: If the output power specified is within the BLE limits but outside 
 * the chip limits, we "rail" the power level so we dont exceed the min/max 
 * chip values. 
 * 
 * @param dbm Power output in dBm.
 * 
 * @return int 0: success; -1 otherwise.
 */
int
ble_phy_txpwr_set(int dbm)
{
    /* Check valid range */
    assert(dbm <= BLE_PHY_MAX_PWR_DBM);

    /* "Rail" power level if outside supported range */
    if (dbm > NRF52_TX_PWR_MAX_DBM) {
        dbm = NRF52_TX_PWR_MAX_DBM;
    } else {
        if (dbm < NRF52_TX_PWR_MIN_DBM) {
            dbm = NRF52_TX_PWR_MIN_DBM;
        }
    }

    /* XXX: Should we do this only in a certain state? */
    NRF_RADIO->TXPOWER = dbm;

    return 0;
}

/**
 * ble phy txpwr get
 *  
 * Get the transmit power. 
 * 
 * @return int  The current PHY transmit power, in dBm
 */
int
ble_phy_txpwr_get(void)
{
    return g_ble_phy_data.txpwr_dbm;
}

/**
 * ble phy setchan 
 *  
 * Sets the logical frequency of the transceiver. The input parameter is the 
 * BLE channel index (0 to 39, inclusive). The NRF52 frequency register 
 * works like this: logical frequency = 2400 + FREQ (MHz). 
 *  
 * Thus, to get a logical frequency of 2402 MHz, you would program the 
 * FREQUENCY register to 2. 
 * 
 * @param chan This is the Data Channel Index or Advertising Channel index
 * 
 * @return int 0: success; -1 otherwise
 */
int
ble_phy_setchan(uint8_t chan)
{
    uint8_t freq;
    int     rc;

    assert(chan < BLE_PHY_NUM_CHANS);

    /* Check for valid channel range */
    if (chan >= BLE_PHY_NUM_CHANS) {
        return -1;
    }

    /* If the current channel is set, just return */
    if (g_ble_phy_data.chan == chan) {
        return 0;
    }

    /* Get correct nrf52 frequency */
    rc = 0;
    if (chan < BLE_PHY_NUM_DATA_CHANS) {
        if (chan < 11) {
            /* Data channel 0 starts at 2404. 0 - 10 are contiguous */
            freq = (BLE_PHY_DATA_CHAN0_FREQ_MHZ - 2400) + 
                   (BLE_PHY_CHAN_SPACING_MHZ * chan);
        } else {
            /* Data channel 11 starts at 2428. 0 - 10 are contiguous */
            freq = (BLE_PHY_DATA_CHAN0_FREQ_MHZ - 2400) + 
                   (BLE_PHY_CHAN_SPACING_MHZ * (chan + 1));
        }
    } else {
        switch (chan) {
        case 37:
            /* This advertising channel is at 2402 MHz */
            freq = BLE_PHY_CHAN_SPACING_MHZ;
            break;
        case 38:
            /* This advertising channel is at 2426 MHz */
            freq = BLE_PHY_CHAN_SPACING_MHZ * 13;
            break;
        case 39:
            /* This advertising channel is at 2480 MHz */
            freq = BLE_PHY_CHAN_SPACING_MHZ * 40;
            break;
        default:
            rc = -1;
            break;
        }
    }

    /* Set the frequency and the data whitening initial value */
    if (!rc) {
        g_ble_phy_data.chan = chan;
        NRF_RADIO->FREQUENCY = freq;
        NRF_RADIO->DATAWHITEIV = chan;
    }

    return rc;
}

void
ble_phy_disable(void)
{
    NRF_RADIO->INTENCLR = NRF52_RADIO_IRQ_MASK_ALL;
    NRF_RADIO->SHORTS = 0;
    NRF_RADIO->TASKS_DISABLE = 1;
}