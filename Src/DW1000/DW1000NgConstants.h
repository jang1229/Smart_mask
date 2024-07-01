/*
 * MIT License
 * 
 * Copyright (c) 2018 Michele Biondi, Andrea Salvatori
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#ifndef DW1000NgConstants_H
#define DW1000NgConstants_H

//#include <Arduino.h>

#define DW_GPIO_MODE 0
#define LED_MODE 1

#include <stdint.h>

#define TIME_RES     0.000015650040064103f
#define TIME_RES_INV 63897.6f

/* Speed of radio waves (light) [m/s] * timestamp resolution [~15.65ps] of DW1000Ng */
#define DISTANCE_OF_RADIO     0.0046917639786159f
#define DISTANCE_OF_RADIO_INV 213.139451293f

// timestamp char length - 40 bit -> 5 char
#define LENGTH_TIMESTAMP 5

// timer/counter overflow (40 bits) -> 4overflow approx. every 17.2 seconds
#define TIME_OVERFLOW 0x10000000000LL
#define TIME_MAX      0xffffffffffLL

// time factors (relative to [us]) for setting delayed transceive
// TODO use non float
#define SECONDS      1e6
#define MILLISECONDS 1e3
#define MICROSECONDS 1
#define NANOSECONDS  1e-3

/* preamble codes (CHAN_CTRL - RX & TX _CODE) - reg:0x1F, bits:31-27,26-22 */

typedef enum {
    CODE_1 = 1,
    CODE_2,
    CODE_3,
    CODE_4,
    CODE_5,
    CODE_6,
    CODE_7,
    CODE_8,
    CODE_9,
    CODE_10,
    CODE_11,
    CODE_12,
    CODE_17 = 17,
    CODE_18,
    CODE_19,
    CODE_20
} PreambleCode;


/* transmission/reception bit rate (TXBR) - reg:0x08, bits:14,13 */
typedef enum {
    RATE_110KBPS,
    RATE_850KBPS,
    RATE_6800KBPS
} DataRate;

/* transmission pulse frequency (TXPRF) - reg:0x08, bits:17,16
* 0x00 is 4MHZ, but receiver in DW1000Ng does not support it (!??) */
typedef enum {
    FREQ_16MHZ = 0x01,
    FREQ_64MHZ
} PulseFrequency;

/* preamble length (PE + TXPSR) - reg:0x08, bits:21,20,19,18 - table 16 */
typedef enum {
    LEN_64   = 0x01,
    LEN_128  = 0x05,
    LEN_256  = 0x09,
    LEN_512  = 0x0D,
    LEN_1024 = 0x02,
    LEN_1536 = 0x06,
    LEN_2048 = 0x0A,
    LEN_4096 = 0x03
} PreambleLength;

/* PAC size (DRX_TUNE2) - reg:0x08, sub-reg:0x27, bits:26,25 - table 33
* The value to program the sub-register changes in based of RXPRF */
typedef enum {
    SIZE_8  = 8,
    SIZE_16 = 16,
    SIZE_32 = 32,
    SIZE_64 = 64
} PacSize;

/* channel of operation (CHAN_CTRL - TX & RX _CHAN) - reg:0x1F, bits:3-0,7-4 */
typedef enum {
    CHANNEL_1 = 1,
    CHANNEL_2,
    CHANNEL_3,
    CHANNEL_4,
    CHANNEL_5,
    CHANNEL_7 = 7
} Channel;


/* Register is 6 bit, 7 = write, 6 = sub-adressing, 5-0 = register value
 * Total header with sub-adressing can be 15 bit. */
#define WRITE      0x80 // regular write
#define WRITE_SUB  0xC0 // write with sub address
#define READ       0x00 // regular read
#define READ_SUB   0x40 // read with sub address
#define RW_SUB_EXT 0x80 // R/W with sub address extension

/* clocks available. */
#define SYS_AUTO_CLOCK 0x00
#define SYS_XTI_CLOCK  0x01
#define SYS_PLL_CLOCK  0x02
#define TX_PLL_CLOCK   0x20
#define LDE_CLOCK      0x03

// typedef enum {
//     dB_18,
//     dB_15,
//     dB_12,
//     dB_9,
//     dB_6,
//     dB_3,
//     dB_0,
//     OFF
// } DriverAmplifierValue;

typedef enum {
    dB_0,
    dB_0_5,
    dB_1,
    dB_1_5,
    dB_2,
    dB_2_5,
    dB_3,
    dB_3_5,
    dB_4,
    dB_4_5,
    dB_5,
    dB_5_5,
    dB_6,
    dB_6_5,
    dB_7,
    dB_7_5,
    dB_8,
    dB_8_5,
    dB_9,
    dB_9_5,
    dB_10,
    dB_10_5,
    dB_11,
    dB_11_5,
    dB_12,
    dB_12_5,
    dB_13,
    dB_13_5,
    dB_14,
    dB_14_5,
    dB_15,
    dB_15_5
} TransmitMixerValue;


typedef enum {
    STANDARD_SFD,
    DECAWAVE_SFD
} SFDMode;

typedef enum {
    IMMEDIATE,
    DELAYED
} TransmitMode;

typedef enum {
    SLOW,
    FAST
} SPIClock;



#endif

