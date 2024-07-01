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
 * @file DW1000.h
 * Helper functions.
 */

#ifndef DW1000NgUtils_H
#define DW1000NgUtils_H

//#include <Arduino.h>
#include "DW1000NgConstants.h"

/**
Returns target bit value inside a char array

@param [in] data the char array
@param [in] n the length of the char array
@param [in] bit the bit position

returns bit value (true = 1, false = 0)
*/
bool DW1000NgUtils_getBit(char data[], unsigned short n, unsigned short bit);

/**
Sets the target bit value inside an array of chars

@param [in] data the char array
@param [in] n the length of the char array
@param [in] bit the bit position
@param [in] val the bit value

*/
void DW1000NgUtils_setBit(char data[], unsigned short n, unsigned short bit, bool val);

/**
Writes the target value inside a given char array.

@param [in] data the char array
@param [in] val the value to insert
@param [in] n the length of the char array
*/
void DW1000NgUtils_writeValueTochars(char data[], uint64_t val, unsigned char n);

/**
Gets the target char array value

@param [in] data the char array
@param [in] n the length of the char array

returns the char array value
*/
uint64_t DW1000NgUtils_charsAsValue(char data[], unsigned char n);

/**
Converts from char to 4 bits (hexadecimal)

@param [in] c the character

returns target value
*/

unsigned char DW1000NgUtils_nibbleFromChar(char c);

/**
Converts the target string to eui chars

@param [in] string The eui string (in format XX:XX:XX:XX:XX:XX:XX:XX)
@param [out] eui_char The eui chars
*/
void DW1000NgUtils_convertToByte(char string[], char* bytes);


#endif // HEADER_NAME_H



