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

/* Copyright (c) 2015, Nordic Semiconductor ASA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *   * Neither the name of Nordic Semiconductor ASA nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef _NRF51422_PERIPHERALS_H
#define _NRF51422_PERIPHERALS_H


/* Software Interrupts */
#define SWI_PRESENT
#define SWI_COUNT 6

/* Radio */
#define RADIO_PRESENT
#define RADIO_COUNT 1

/* Peripheral to Peripheral Interconnect */
#define PPI_PRESENT
#define PPI_COUNT 1

#define PPI_CH_NUM 16
#define PPI_GROUP_NUM 4

/* Timer/Counter */
#define TIMER_PRESENT
#define TIMER_COUNT 3

#define TIMER0_MAX_SIZE 32
#define TIMER1_MAX_SIZE 16
#define TIMER2_MAX_SIZE 16

#define TIMER0_CC_NUM 4
#define TIMER1_CC_NUM 4
#define TIMER2_CC_NUM 4

/* Real Time Counter */
#define RTC_PRESENT
#define RTC_COUNT 2

#define RTC0_CC_NUM 3
#define RTC1_CC_NUM 4

/* RNG */
#define RNG_PRESENT
#define RNG_COUNT 1

/* Watchdog Timer */
#define WDT_PRESENT
#define WDT_COUNT 1

/* Temperature Sensor */
#define TEMP_PRESENT
#define TEMP_COUNT 1

/* Serial Peripheral Interface Master */
#define SPI_PRESENT
#define SPI_COUNT 2

/* Serial Peripheral Interface Slave with DMA */
#define SPIS_PRESENT
#define SPIS_COUNT 1

/* Two Wire Interface Master */
#define TWI_PRESENT
#define TWI_COUNT 2

/* Universal Asynchronous Receiver-Transmitter */
#define UART_PRESENT
#define UART_COUNT 1

/* Quadrature Decoder */
#define QDEC_PRESENT
#define QDEC_COUNT 1

/* Analog to Digital Converter */
#define ADC_PRESENT
#define ADC_COUNT 1

/* GPIO Tasks and Events */
#define GPIOTE_PRESENT
#define GPIOTE_COUNT 1

#define GPIOTE_CH_NUM 4

/* Low Power Comparator */
#define LPCOMP_PRESENT
#define LPCOMP_COUNT 1


#endif      // _NRF51422_PERIPHERALS_H