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
#include "DW1000Ng.h"
#include "DW1000NgConstants.h"
#include "DW1000NgRanging.h"
#include "DW1000NgConstants.h"
#include "DW1000NgRTLS.h"

/* range bias tables - APS011 */
const double BIAS_TABLE[18][5] = {
    {61, -198, -110, -275, -295},
    {63, -187, -105, -244, -266},
    {65, -179, -100, -210, -235},
    {67, -163, -93, -176, -199},
    {69, -143, -82, -138, -150},
    {71, -127, -69, -95, -100},
    {73, -109, -51, -51, -58},
    {75, -84, -27, 0, 0},
    {77, -59, 0, 42, 49},
    {79, -31, 21, 97, 91},
    {81, 0, 35, 158, 127},
    {83, 36, 42, 210, 153},
    {85, 65, 49, 254, 175},
    {87, 84, 62, 294, 197},
    {89, 97, 71, 321, 233},
    {91, 106, 76, 339, 245},
    {93, 110, 81, 356, 264},
    {95, 112, 86, 394, 284}
};

/* asymmetric two-way ranging (more computation intense, less error prone) */
double DW1000NgRanging_computeRangeAsymmetric(
    uint64_t timePollSent,
    uint64_t timePollReceived,
    uint64_t timePollAckSent,
    uint64_t timePollAckReceived,
    uint64_t timeRangeSent,
    uint64_t timeRangeReceived
) {
    uint32_t timePollSent_32 = (uint32_t)timePollSent;
    uint32_t timePollReceived_32 = (uint32_t)timePollReceived;
    uint32_t timePollAckSent_32 = (uint32_t)timePollAckSent;
    uint32_t timePollAckReceived_32 = (uint32_t)timePollAckReceived;
    uint32_t timeRangeSent_32 = (uint32_t)timeRangeSent;
    uint32_t timeRangeReceived_32 = (uint32_t)timeRangeReceived;

    double round1 = (double)(timePollAckReceived_32 - timePollSent_32);
    double reply1 = (double)(timePollAckSent_32 - timePollReceived_32);
    double round2 = (double)(timeRangeReceived_32 - timePollAckSent_32);
    double reply2 = (double)(timeRangeSent_32 - timePollAckReceived_32);

    int64_t tof_uwb = (int64_t)((round1 * round2 - reply1 * reply2) / (round1 + round2 + reply1 + reply2));
    double distance = (double)(tof_uwb) * DISTANCE_OF_RADIO;

    return distance;
}

double DW1000NgRanging_correctRange(double range) {
    double result = 0;

    Channel currentChannel = DW1000Ng_getChannel();
    double rxPower = -(double)(DW1000Ng_getReceivePower());

    int index = DW1000Ng_getPulseFrequency() == FREQ_16MHZ ? 1 : 2;
    if (currentChannel == CHANNEL_4 || currentChannel == CHANNEL_7)
        index += 2;

    if (rxPower < BIAS_TABLE[0][0]) {
        result = range + BIAS_TABLE[0][index] * 0.001;
    } else if (rxPower >= BIAS_TABLE[17][0]) {
        result = range + BIAS_TABLE[17][index] * 0.001;
    } else {
        for (int i = 0; i < 17; i++) {
            if (rxPower >= BIAS_TABLE[i][0] && rxPower < BIAS_TABLE[i + 1][0]) {
                result = range + BIAS_TABLE[i][index] * 0.001;
                break;
            }
        }
    }

    return result;
}
