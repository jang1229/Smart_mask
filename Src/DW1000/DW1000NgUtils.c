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

/*
 * Copyright (c) 2015 by Thomas Trojer <thomas@trojer.net>
 * Decawave DW1000 library for arduino.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @file DW1000.cpp
 * Helper functions.
 */

#include <stdint.h> // For uint16_t, uint8_t types
#include <string.h>
#include <stdbool.h> // For boolean type (using standard bool)
#include "DW1000NgUtils.h"
#include "DW1000NgConstants.h"
#include "DW1000NgRegisters.h"

void DW1000NgUtils_setBit(char data[], uint16_t n, uint16_t bit, bool val) {
    uint16_t idx;
    uint8_t shift;
    
    idx = bit / 8;
    if (idx >= n) {
        return; // TODO proper error handling: out of bounds
    }
    char* targetByte = &data[idx];
    shift = bit % 8;
    if (val) {
        *targetByte |= (1 << shift);
    } else {
        *targetByte &= ~(1 << shift);
    }
}

bool DW1000NgUtils_getBit(char data[], uint16_t n, uint16_t bit) {
    uint16_t idx;
    uint8_t shift;
    
    idx = bit / 8;
    if (idx >= n) {
        return false; // TODO proper error handling: out of bounds
    }
    char targetByte = data[idx];
    shift = bit % 8;
    
    return ((targetByte >> shift) & 1);
}
     
void DW1000NgUtils_writeValueTochars(char data[], uint64_t val, uint8_t n) {
    for (uint8_t i = 0; i < n; i++) {
        data[i] = (char)((val >> (i * 8)) & 0xFF);
    }
}

uint64_t DW1000NgUtils_charsAsValue(char data[], uint8_t n) {
    uint64_t value = 0;
    for (uint8_t i = 0; i < n; i++) {
        value |= ((uint64_t)data[i] << (i * 8));
    }
    return value;
}

uint8_t DW1000NgUtils_nibbleFromChar(char c) {
    if (c >= '0' && c <= '9') {
        return (uint8_t)(c - '0');
    }
    if (c >= 'a' && c <= 'f') {
        return (uint8_t)(c - 'a' + 10);
    }
    if (c >= 'A' && c <= 'F') {
        return (uint8_t)(c - 'A' + 10);
    }
    return 255;
}

void DW1000NgUtils_convertToByte(char string[], char* bytes) {
    char eui_byte[LEN_EUI];
    for (uint16_t i = 0; i < LEN_EUI; i++) {
        eui_byte[i] = (char)((DW1000NgUtils_nibbleFromChar(string[i * 3]) << 4) + DW1000NgUtils_nibbleFromChar(string[i * 3 + 1]));
    }
    memcpy(bytes, eui_byte, LEN_EUI);
}





