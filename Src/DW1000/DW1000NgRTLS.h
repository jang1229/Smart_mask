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

#ifndef DW1000NgRTLS_H
#define DW1000NgRTLS_H

//#include <Arduino.h>
#include <stdbool.h> // For boolean type (using standard bool)
#include <stdint.h>

/* Frame control */
#define BLINK 0xC5
#define DATA 0x41
#define SHORT_SRC_AND_DEST 0x88
#define LONG_SRC_AND_DEST 0xCC
#define SHORT_SRC_LONG_DEST 0x8C
#define LONG_SRC_SHORT_DEST 0xC8

/* Application ID */
#define RTLS_APP_ID_LOW 0x9A
#define RTLS_APP_ID_HIGH 0x60
#define RTLS_APP_ID ((unsigned short)(RTLS_APP_ID_LOW | ((unsigned short)RTLS_APP_ID_HIGH << 8)))

/* Function code */
#define ACTIVITY_CONTROL 0x10
#define RANGING_INITIATION 0x20
#define RANGING_TAG_POLL 0x21
#define RANGING_TAG_FINAL_RESPONSE_EMBEDDED 0x23
#define RANGING_TAG_FINAL_RESPONSE_NO_EMBEDDED 0x25
#define RANGING_TAG_FINAL_SEND_TIME 0x27

/* Activity code */
#define ACTIVITY_FINISHED 0x00
#define RANGING_CONFIRM 0x01
#define RANGING_CONTINUE 0x02

/* BLINK Encoding Header */
#define BATTERY_GOOD 0x00
#define BATTERY_10_30_PERCENT 0x02
#define BATTERY_0_10_PERCENT 0x01
#define NO_BATTERY_STATUS 0x03

#define TEMPERATURE_DATA 0x20

#define EX_ID 0x80
#define NO_EX_ID 0x40

/* BLINK Ext Header */
#define BLINK_RATE_AND_LISTENING 0x01
#define TAG_LISTENING_NOW 0x02

typedef enum 
{
  NEXTACTIVITY_ACTIVITY_FINISHED = 0,
  NEXTACTIVITY_RANGING_CONFIRM
} NextActivity;

typedef struct {
    bool success;
    unsigned short target_anchor;
} RangeRequestResult;

typedef struct {
    bool success;
    bool next;
    unsigned short next_anchor;
    uint32_t new_blink_rate;
} RangeResult;

typedef struct {
    bool success;
    unsigned short new_blink_rate;
} RangeInfrastructureResult;

typedef struct {
    bool success;
    double range;
} RangeAcceptResult;



/*** TWR functions used in ISO/IEC 24730-62:2013, refer to the standard or the decawave manual for details about TWR ***/
char DW1000NgRTLS_increaseSequenceNumber();
void DW1000NgRTLS_transmitTwrShortBlink();
void DW1000NgRTLS_transmitRangingInitiation(char tag_eui[], char tag_short_address[]);
void DW1000NgRTLS_transmitPoll(char anchor_address[]);
void DW1000NgRTLS_transmitResponseToPoll(char tag_short_address[]);
void DW1000NgRTLS_lMessage(char anchor_address[], unsigned short reply_delay, uint64_t timePollSent, uint64_t timeResponseToPollReceived);
void DW1000NgRTLS_transmitRangingConfirm(char tag_short_address[], char next_anchor[]);
void DW1000NgRTLS_vityFinished(char tag_short_address[], char blink_rate[]);

bool DW1000NgRTLS_receiveFrame();
void DW1000NgRTLS_waitForTransmission();
/*** End of TWR functions ***/

/* Send a request range from tag to the rtls infrastructure */
RangeRequestResult DW1000NgRTLS_tagRangeRequest();

/* Used by an anchor to accept an incoming tagRangeRequest by means of the infrastructure
    NextActivity is used to indicate the tag what to do next after the ranging process (Activity finished is to return to blink (range request), 
    Continue range is to tell the tag to range a new anchor)
    value is the value relative to the next activity (Activity finished = new blink rante, continue range = new anchor address)
*/
RangeAcceptResult DW1000NgRTLS_anchorRangeAccept(NextActivity next, unsigned short value);

/* Used by tag to range after range request accept of the infrastructure 
    Target anchor is given after a range request success
    Finalmessagedelay is used in the process of TWR, a value of 1500 works on 8mhz-80mhz range devices,
    you could try to decrease it to improve system performance.
*/
RangeInfrastructureResult DW1000NgRTLS_tagRangeInfrastructure(unsigned short target_anchor, unsigned short finalMessageDelay);

/* Can be used as a single function start the localization process from the tag.
    Finalmessagedelay is the same as in function tagRangeInfrastructure
*/
RangeInfrastructureResult DW1000NgRTLS_tagTwrLocalize(unsigned short finalMessageDelay);

#endif