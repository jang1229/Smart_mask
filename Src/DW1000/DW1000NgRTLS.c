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

//#include <Arduino.h>
#include "DW1000NgRTLS.h"
#include "DW1000Ng.h"
#include "DW1000NgUtils.h"
#include "DW1000NgTime.h"
#include "DW1000NgRanging.h"
#include "board_driver.h"

static char SEQ_NUMBER = 0;

char DW1000NgRTLS_increaseSequenceNumber(){
    return ++SEQ_NUMBER;
}

void DW1000NgRTLS_transmitTwrShortBlink() {
    char Blink[] = {BLINK, SEQ_NUMBER++, 0,0,0,0,0,0,0,0, NO_BATTERY_STATUS | NO_EX_ID, TAG_LISTENING_NOW};
    DW1000Ng_getEUI(&Blink[2]);
    DW1000Ng_setTransmitDataByte(Blink, sizeof(Blink));
    DW1000Ng_startTransmit(IMMEDIATE);
}

void DW1000NgRTLS_transmitRangingInitiation(char tag_eui[], char tag_short_address[]) {
    char RangingInitiation[] = {DATA, SHORT_SRC_LONG_DEST, SEQ_NUMBER++, 0,0, 0,0,0,0,0,0,0,0,  0,0, RANGING_INITIATION, 0,0};
    DW1000Ng_getNetworkId(&RangingInitiation[3]);
    memcpy(&RangingInitiation[5], tag_eui, 8);
    DW1000Ng_getDeviceAddress(&RangingInitiation[13]);
    memcpy(&RangingInitiation[16], tag_short_address, 2);
    DW1000Ng_setTransmitDataByte(RangingInitiation, sizeof(RangingInitiation));
    DW1000Ng_startTransmit(IMMEDIATE);
}

void DW1000NgRTLS_transmitPoll(char anchor_address[]){
    char Poll[] = {DATA, SHORT_SRC_AND_DEST, SEQ_NUMBER++, 0,0, 0,0, 0,0 , RANGING_TAG_POLL};
    DW1000Ng_getNetworkId(&Poll[3]);
    memcpy(&Poll[5], anchor_address, 2);
    DW1000Ng_getDeviceAddress(&Poll[7]);
    DW1000Ng_setTransmitDataByte(Poll, sizeof(Poll));
    DW1000Ng_startTransmit(IMMEDIATE);
}

void DW1000NgRTLS_transmitResponseToPoll(char tag_short_address[]) {
    char pollAck[] = {DATA, SHORT_SRC_AND_DEST, SEQ_NUMBER++, 0,0, 0,0, 0,0, ACTIVITY_CONTROL, RANGING_CONTINUE, 0, 0};
    DW1000Ng_getNetworkId(&pollAck[3]);
    memcpy(&pollAck[5], tag_short_address, 2);
    DW1000Ng_getDeviceAddress(&pollAck[7]);
    DW1000Ng_setTransmitDataByte(pollAck, sizeof(pollAck));
    DW1000Ng_startTransmit(IMMEDIATE);
}

void DW1000NgRTLS_transmitFinalMessage(char anchor_address[], unsigned short reply_delay, uint64_t timePollSent, uint64_t timeResponseToPollReceived) {
    /* Calculation of future time */
    char futureTimechars[LENGTH_TIMESTAMP];

    uint64_t timeFinalMessageSent = DW1000Ng_getSystemTimestamp();
    timeFinalMessageSent += DW1000NgTime_microsecondsToUWBTime(reply_delay);
    DW1000NgUtils_writeValueTochars(futureTimechars, timeFinalMessageSent, LENGTH_TIMESTAMP);
    DW1000Ng_setDelayedTRX(futureTimechars);
    timeFinalMessageSent += DW1000Ng_getTxAntennaDelay();

    char finalMessage[] = {DATA, SHORT_SRC_AND_DEST, SEQ_NUMBER++, 0,0, 0,0, 0,0, RANGING_TAG_FINAL_RESPONSE_EMBEDDED, 
        0,0,0,0,0,0,0,0,0,0,0,0
    };

    DW1000Ng_getNetworkId(&finalMessage[3]);
    memcpy(&finalMessage[5], anchor_address, 2);
    DW1000Ng_getDeviceAddress(&finalMessage[7]);

    DW1000NgUtils_writeValueTochars(finalMessage + 10, (uint32_t) timePollSent, 4);
    DW1000NgUtils_writeValueTochars(finalMessage + 14, (uint32_t) timeResponseToPollReceived, 4);
    DW1000NgUtils_writeValueTochars(finalMessage + 18, (uint32_t) timeFinalMessageSent, 4);
    DW1000Ng_setTransmitDataByte(finalMessage, sizeof(finalMessage));
    DW1000Ng_startTransmit(DELAYED);
}

void DW1000NgRTLS_transmitRangingConfirm(char tag_short_address[], char next_anchor[]) {
    char rangingConfirm[] = {DATA, SHORT_SRC_AND_DEST, SEQ_NUMBER++, 0,0, 0,0, 0,0, ACTIVITY_CONTROL, RANGING_CONFIRM, next_anchor[0], next_anchor[1]};
    DW1000Ng_getNetworkId(&rangingConfirm[3]);
    memcpy(&rangingConfirm[5], tag_short_address, 2);
    DW1000Ng_getDeviceAddress(&rangingConfirm[7]);
    DW1000Ng_setTransmitDataByte(rangingConfirm, sizeof(rangingConfirm));
    DW1000Ng_startTransmit(IMMEDIATE);
}

void DW1000NgRTLS_transmitActivityFinished(char tag_short_address[], char blink_rate[]) {
    /* I send the new blink rate to the tag */
    char rangingConfirm[] = {DATA, SHORT_SRC_AND_DEST, SEQ_NUMBER++, 0,0, 0,0, 0,0, ACTIVITY_CONTROL, ACTIVITY_FINISHED, blink_rate[0], blink_rate[1]};
    DW1000Ng_getNetworkId(&rangingConfirm[3]);
    memcpy(&rangingConfirm[5], tag_short_address, 2);
    DW1000Ng_getDeviceAddress(&rangingConfirm[7]);
    DW1000Ng_setTransmitDataByte(rangingConfirm, sizeof(rangingConfirm));
    DW1000Ng_startTransmit(IMMEDIATE);
}

static uint32_t DW1000NgRTLS_calculateNewBlinkRate(char frame[]) {
    uint32_t blinkRate = frame[11] + (((uint32_t)(frame[12] & 0x3F)) << 8);
    char multiplier = ((frame[12] & 0xC0) >> 6);
    
    if (multiplier == 0x01) {
        blinkRate *= 25;
    } else if (multiplier == 0x02) {
        blinkRate *= 1000;
    }

    return blinkRate;
}


void DW1000NgRTLS_waitForTransmission() {
    while(!DW1000Ng_isTransmitDone()) {
        #if defined(ESP8266)
        yield();
        #endif
    }
    DW1000Ng_clearTransmitStatus();
}

bool DW1000NgRTLS_receiveFrame() {
    DW1000Ng_startReceive(IMMEDIATE);
    while(!DW1000Ng_isReceiveDone()) {
        if(DW1000Ng_isReceiveTimeout() ) {
            DW1000Ng_clearReceiveTimeoutStatus();
            return false;
        }
        #if defined(ESP8266)
        yield();
        #endif
    }
    DW1000Ng_clearReceiveStatus();
    return true;
}

static bool DW1000NgRTLS_waitForNextRangingStep() {
    DW1000NgRTLS_waitForTransmission();
    if(!DW1000NgRTLS_receiveFrame()) return false;
    return true;
}

RangeRequestResult DW1000NgRTLS_tagRangeRequest() {
    DW1000NgRTLS_transmitTwrShortBlink();
    
    if (!DW1000NgRTLS_waitForNextRangingStep()) {
        RangeRequestResult result = { false, 0 };
        return result;
    }

    size_t init_len = DW1000Ng_getReceivedDataLength();
    
    char *init_recv = (char *)malloc(init_len);
    if (init_recv == NULL)
    {
      RangeRequestResult result = { false, 0 };
      return result;
    }
    
    DW1000Ng_getReceivedData(init_recv, init_len);

    if (!(init_len > 17 && init_recv[15] == RANGING_INITIATION)) {
        RangeRequestResult result = { false, 0 };
        return result;
    }

    DW1000Ng_setDeviceAddress(DW1000NgUtils_charsAsValue(&init_recv[16], 2));
    RangeRequestResult result = { 
        true, 
        (unsigned short)DW1000NgUtils_charsAsValue(&init_recv[13], 2)
    };
    
    free(init_recv);
    
    return result;
}


RangeResult DW1000NgRTLS_tagFinishRange(unsigned short anchor, unsigned short replyDelayUs) {
    RangeResult returnValue;

    char target_anchor[2];
    DW1000NgUtils_writeValueTochars(target_anchor, anchor, 2);
    DW1000NgRTLS_transmitPoll(target_anchor);

    /* Start of poll control for range */
    if (!DW1000NgRTLS_waitForNextRangingStep()) {
        returnValue.success = false;
        returnValue.next = false;
        returnValue.next_anchor = 0;
        returnValue.new_blink_rate = 0;
    } else {
        size_t cont_len = DW1000Ng_getReceivedDataLength();
        char *cont_recv = (char *)malloc(cont_len);
        if (cont_recv == NULL)
        {
            returnValue.success = false;
            returnValue.next = false;
            returnValue.next_anchor = 0;
            returnValue.new_blink_rate = 0;
            return returnValue;
        }

        DW1000Ng_getReceivedData(cont_recv, cont_len);

        if (cont_len > 10 && cont_recv[9] == ACTIVITY_CONTROL && cont_recv[10] == RANGING_CONTINUE) {
            /* Received Response to poll */
            DW1000NgRTLS_transmitFinalMessage(
                &cont_recv[7],
                replyDelayUs,
                DW1000Ng_getTransmitTimestamp(), // Poll transmit time
                DW1000Ng_getReceiveTimestamp()  // Response to poll receive time
            );

            if (!DW1000NgRTLS_waitForNextRangingStep()) {
                returnValue.success = false;
                returnValue.next = false;
                returnValue.next_anchor = 0;
                returnValue.new_blink_rate = 0;
            } else {
                size_t act_len = DW1000Ng_getReceivedDataLength();
                char *act_recv = (char *)malloc(act_len);
                if (act_recv == NULL)
                {
                    returnValue.success = false;
                    returnValue.next = false;
                    returnValue.next_anchor = 0;
                    returnValue.new_blink_rate = 0;
                    return returnValue;
                }

                DW1000Ng_getReceivedData(act_recv, act_len);

                if (act_len > 10 && act_recv[9] == ACTIVITY_CONTROL) {
                    if (act_len > 12 && act_recv[10] == RANGING_CONFIRM) {
                        returnValue.success = true;
                        returnValue.next = true;
                        returnValue.next_anchor = (unsigned short)DW1000NgUtils_charsAsValue(&act_recv[11], 2);
                        returnValue.new_blink_rate = 0;
                    } else if (act_len > 12 && act_recv[10] == ACTIVITY_FINISHED) {
                        returnValue.success = true;
                        returnValue.next = false;
                        returnValue.next_anchor = 0;
                        returnValue.new_blink_rate = DW1000NgRTLS_calculateNewBlinkRate(act_recv);
                    }
                } else {
                    returnValue.success = false;
                    returnValue.next = false;
                    returnValue.next_anchor = 0;
                    returnValue.new_blink_rate = 0;
                }
                free(act_recv);
            }
        } else {
            returnValue.success = false;
            returnValue.next = false;
            returnValue.next_anchor = 0;
            returnValue.new_blink_rate = 0;
        }

        free(cont_recv);
    }

    return returnValue;
}

RangeInfrastructureResult DW1000NgRTLS_tagRangeInfrastructure(unsigned short target_anchor, unsigned short finalMessageDelay) {
    RangeInfrastructureResult returnValue;

    RangeResult result = DW1000NgRTLS_tagFinishRange(target_anchor, finalMessageDelay);
    char keep_going = 1;

    if (!result.success) {
        keep_going = 0;
        returnValue.success = false;
        returnValue.new_blink_rate = 0;
    } else {
        while (result.success && result.next) {
            result = DW1000NgRTLS_tagFinishRange(result.next_anchor, finalMessageDelay);
            if (!result.success) {
                keep_going = 0;
                returnValue.success = false;
                returnValue.new_blink_rate = 0;
                break;
            }

            #if defined(ESP8266)
            if (keep_going == 1) {
                yield();
            }
            #endif
        }

        if (keep_going == 1) {
            if (result.success && result.new_blink_rate != 0) {
                keep_going = 0;
                returnValue.success = true;
                returnValue.new_blink_rate = (unsigned short)result.new_blink_rate;
            } else {
                if (!result.success) {
                    keep_going = 0;
                    returnValue.success = false;
                    returnValue.new_blink_rate = 0;
                } else {
                    // TODO. Handle this condition?
                }
            }
        }
    }
    return returnValue;
}

RangeInfrastructureResult DW1000NgRTLS_tagTwrLocalize(unsigned short finalMessageDelay) {
    RangeRequestResult request_result = DW1000NgRTLS_tagRangeRequest();

    if (request_result.success) {
        RangeInfrastructureResult result = DW1000NgRTLS_tagRangeInfrastructure(request_result.target_anchor, finalMessageDelay);

        if (result.success) {
            return result;
        }
    }

    RangeInfrastructureResult fail_result;
    fail_result.success = false;
    fail_result.new_blink_rate = 0;
    return fail_result;
}


RangeAcceptResult anchorRangeAccept(NextActivity next, unsigned short value) {
    RangeAcceptResult returnValue;

    double range;
    if(!DW1000NgRTLS_receiveFrame()) {
        returnValue.success = false;
        returnValue.range = 0;
    } else {

        size_t poll_len = DW1000Ng_getReceivedDataLength();
        char *poll_data = (char *)malloc(poll_len);
        if (poll_data == NULL)
        {
            returnValue.success = false;
            returnValue.range = 0;
            return returnValue;
        }
        
        DW1000Ng_getReceivedData(poll_data, poll_len);

        if(poll_len > 9 && poll_data[9] == RANGING_TAG_POLL) {
            uint64_t timePollReceived = DW1000Ng_getReceiveTimestamp();
            DW1000NgRTLS_transmitResponseToPoll(&poll_data[7]);
            DW1000NgRTLS_waitForTransmission();
            uint64_t timeResponseToPoll = DW1000Ng_getTransmitTimestamp();
            DelayUs(1500);

            if(!DW1000NgRTLS_receiveFrame()) {
                returnValue.success = false;
                returnValue.range = 0;
            } else {

                size_t rfinal_len = DW1000Ng_getReceivedDataLength();
                char *rfinal_data = (char *)malloc(rfinal_len);
                if (rfinal_data == NULL)
                {
                    returnValue.success = false;
                    returnValue.range = 0;
                    return returnValue;
                }

                DW1000Ng_getReceivedData(rfinal_data, rfinal_len);
                if(rfinal_len > 18 && rfinal_data[9] == RANGING_TAG_FINAL_RESPONSE_EMBEDDED) {
                    uint64_t timeFinalMessageReceive = DW1000Ng_getReceiveTimestamp();

                    char finishValue[2];
                    DW1000NgUtils_writeValueTochars(finishValue, value, 2);

                    if(next == NEXTACTIVITY_RANGING_CONFIRM) {
                        DW1000NgRTLS_transmitRangingConfirm(&rfinal_data[7], finishValue);
                    } else {
                        DW1000NgRTLS_transmitActivityFinished(&rfinal_data[7], finishValue);
                    }
                    
                    DW1000NgRTLS_waitForTransmission();

                    range = DW1000NgRanging_computeRangeAsymmetric(
                        DW1000NgUtils_charsAsValue(rfinal_data + 10, LENGTH_TIMESTAMP), // Poll send time
                        timePollReceived, 
                        timeResponseToPoll, // Response to poll sent time
                        DW1000NgUtils_charsAsValue(rfinal_data + 14, LENGTH_TIMESTAMP), // Response to Poll Received
                        DW1000NgUtils_charsAsValue(rfinal_data + 18, LENGTH_TIMESTAMP), // Final Message send time
                        timeFinalMessageReceive // Final message receive time
                    );

                    range = DW1000NgRanging_correctRange(range);

                    /* In case of wrong read due to bad device calibration */
                    if(range <= 0) 
                        range = 0.000001;

                    returnValue.success = false;
                    returnValue.range = 0;
                }
                free(rfinal_data);
            }
        }

        free(poll_data);
    }

    return returnValue;
}


RangeAcceptResult DW1000NgRTLS_anchorRangeAccept(NextActivity next, unsigned short value) {
    RangeAcceptResult returnValue;
    double range;
    char *poll_data = NULL;

    if (!DW1000NgRTLS_receiveFrame()) {
        returnValue.success = false;
        returnValue.range = 0;
    } else {
        size_t poll_len = DW1000Ng_getReceivedDataLength();
        poll_data = (char *)malloc(poll_len);
        if (poll_data == NULL) 
        {
            // 메모리 할당 실패 처리
            returnValue.success = false;
            returnValue.range = 0;

            return returnValue;
        }

        DW1000Ng_getReceivedData(poll_data, poll_len);

        if (poll_len > 9 && poll_data[9] == RANGING_TAG_POLL) {
            uint64_t timePollReceived = DW1000Ng_getReceiveTimestamp();
            DW1000NgRTLS_transmitResponseToPoll(&poll_data[7]);
            DW1000NgRTLS_waitForTransmission();
            uint64_t timeResponseToPoll = DW1000Ng_getTransmitTimestamp();
            DelayUs(1500);

            if (!DW1000NgRTLS_receiveFrame()) {
                returnValue.success = false;
                returnValue.range = 0;
            } else {
                size_t rfinal_len = DW1000Ng_getReceivedDataLength();
                
                char *rfinal_data = (char *)malloc(rfinal_len);
                if (rfinal_data == NULL)
                {
                    returnValue.success = false;
                    returnValue.range = 0;
                    return returnValue;
                }

                DW1000Ng_getReceivedData(rfinal_data, rfinal_len);

                if (rfinal_len > 18 && rfinal_data[9] == RANGING_TAG_FINAL_RESPONSE_EMBEDDED) {
                    uint64_t timeFinalMessageReceive = DW1000Ng_getReceiveTimestamp();

                    char finishValue[2];
                    DW1000NgUtils_writeValueTochars(finishValue, value, 2);

                    if (next == RANGING_CONFIRM) {
                        DW1000NgRTLS_transmitRangingConfirm(&rfinal_data[7], finishValue);
                    } else {
                        DW1000NgRTLS_transmitActivityFinished(&rfinal_data[7], finishValue);
                    }

                    DW1000NgRTLS_waitForTransmission();

                    range = DW1000NgRanging_computeRangeAsymmetric(
                        DW1000NgUtils_charsAsValue(rfinal_data + 10, LENGTH_TIMESTAMP), // Poll send time
                        timePollReceived,
                        timeResponseToPoll, // Response to poll sent time
                        DW1000NgUtils_charsAsValue(rfinal_data + 14, LENGTH_TIMESTAMP), // Response to Poll Received
                        DW1000NgUtils_charsAsValue(rfinal_data + 18, LENGTH_TIMESTAMP), // Final Message send time
                        timeFinalMessageReceive // Final message receive time
                    );

                    range = DW1000NgRanging_correctRange(range);

                    /* In case of wrong read due to bad device calibration */
                    if (range <= 0)
                        range = 0.000001;

                    returnValue.success = true;
                    returnValue.range = range;
                }
            }
        }

        free(poll_data);
    }

    return returnValue;
}