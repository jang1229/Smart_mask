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
 * Decawave DW1000Ng library for arduino.
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
 * @file DW1000Ng.cpp
 * Arduino driver library (source file) for the Decawave DW1000Ng UWB transceiver Module.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#if defined(__AVR__)
	#include <EEPROM.h>
#endif
#include "DW1000Ng.h"
#include "DW1000NgUtils.h"
#include "DW1000NgConstants.h"
#include "DW1000NgRegisters.h"
#include "SPIporting.h"
#include "main.h"
#include "board_driver.h"

	
/* anonymous namespace to host private-like variables and methods */


/* ########################### PRIVATE VARIABLES ################################# */
/* Validity matrix for 16 MHz PRF preamble codes */
const char preamble_validity_matrix_PRF16[8][2] = {
    {0, 0}, /* Channel 0 doesn't exist */
    {1, 2},
    {3, 4},
    {5, 6},
    {7, 8},
    {3, 4},
    {0, 0}, /* Channel 5 doesn't exist */
    {7, 8}
};

/* Validity matrix for 64 MHz PRF preamble codes */
const char preamble_validity_matrix_PRF64[8][4] = {
    {0, 0, 0, 0}, /* Channel 0 doesn't exist */
    {9, 10, 11, 12},
    {9, 10, 11, 12},
    {9, 10, 11, 12},
    {17, 18, 19, 20},
    {9, 10, 11, 12},
    {0, 0, 0, 0}, /* Channel 5 doesn't exist */
    {17, 18, 19, 20}
};

/* SPI select pin and interrupt pin*/
unsigned char _ss = 0xff;
unsigned char _irq = 0xff;
unsigned char _rst = 0xff;

/* IRQ callbacks */
SentHandlerFunction _handleSent = NULL;
ErrorHandlerFunction _handleError = NULL;
ReceivedHandlerFunction _handleReceived = NULL;
ReceiveFailedHandlerFunction _handleReceiveFailed = NULL;
ReceiveTimeoutHandlerFunction _handleReceiveTimeout = NULL;
ReceiveTimestampAvailableHandlerFunction _handleReceiveTimestampAvailable = NULL;

/* registers */
char       _syscfg[LEN_SYS_CFG];
char       _sysctrl[LEN_SYS_CTRL];
char       _sysstatus[LEN_SYS_STATUS];
char       _txfctrl[LEN_TX_FCTRL];
char       _sysmask[LEN_SYS_MASK];
char       _chanctrl[LEN_CHAN_CTRL];
char       _networkAndAddress[LEN_PANADR];

/* Temperature and Voltage monitoring */
char _vmeas3v3 = 0;
char _tmeas23C = 0;

/* Driver Internal State Trackers */
char        	_extendedFrameLength;
PacSize        	_pacSize;
PulseFrequency	_pulseFrequency;
DataRate        _dataRate;
PreambleLength	_preambleLength;
PreambleCode	_preambleCode;
Channel        	_channel;
bool     	_smartPower;
bool     	_frameCheck;
bool     	_debounceClockEnabled = false;
bool     	_nlos = false;
bool			_standardSFD = true;
bool     	_autoTXPower = true;
bool     	_autoTCPGDelay = true;
bool 		_wait4resp = false;
unsigned short		_antennaTxDelay = 0;
unsigned short		_antennaRxDelay = 0;

/* ############################# PRIVATE METHODS ################################### */

/*
* Write chars to the DW1000. Single chars can be written to registers via sub-addressing.
* @param[in] cmd
* 		The register address (see Chapter 7 in the DW1000 user manual).
* @param[in] offset
*		The offset to select register sub-parts for writing, or 0x00 to disable
* 		sub-adressing.
* @param[in] data
*		The data array to be written.
* @param[in] data_size
*		The number of chars to be written (take care not to go out of bounds of
* 		the register).
*/
// TODO offset really bigger than char?
void _writecharsToRegister(char cmd, unsigned short offset, char data[], unsigned short data_size) {
	char header[3];
	unsigned char headerLen = 1;
	
	// TODO proper error handling: address out of bounds
	// build SPI header
	if(offset == NO_SUB) {
		header[0] = WRITE | cmd;
	} else {
		header[0] = WRITE_SUB | cmd;
		if(offset < 128) {
			header[1] = (char)offset;
			headerLen++;
		} else {
			header[1] = RW_SUB_EXT | (char)offset;
			header[2] = (char)(offset >> 7);
			headerLen += 2;
		}
	}
	
	SPIporting_writeToSPI(_ss, headerLen, header, data_size, data);
}

/*
* Write Value in Hex or Int format to the DW1000. Single Value can be written to registers via sub-addressing.
* @param[in] cmd
* 		The register address (see Chapter 7 in the DW1000 user manual).
* @param[in] offset
*		The offset to select register sub-parts for writing, or 0x00 to disable
* 		sub-adressing.
* @param[in] data
*		The data Value to be written.
* @param[in] data_size
*		The number of chars to be written
*/
char DataChars[100];	
void _writeValueToRegister(char cmd, unsigned short offset, uint32_t data, unsigned short data_size) { 
	
	//char *datachars = (char *)malloc(data_size);
  //  if (datachars == NULL) return;
	
	
	DW1000NgUtils_writeValueTochars(DataChars, data, data_size);
	_writecharsToRegister(cmd, offset, DataChars, data_size);

	//free(datachars);
}

/*
* Write ONLY ONE chars to the DW1000.
* @param[in] cmd
* 		The register address (see Chapter 7 in the DW1000 user manual).
* @param[in] offset
*		The offset to select register sub-parts for writing, or 0x00 to disable
* 		sub-adressing.
* @param[in] data
*		The char to be written.
*/
void _writeSinglecharToRegister(char cmd, unsigned short offset, char data) {
	_writecharsToRegister(cmd, offset, &data, 1); // 1 as data_size because writes a single char
}

/*
* Read chars from the DW1000. Number of chars depend on register length.
* @param[in] cmd
* 		The register address (see Chapter 7 in the DW1000Ng user manual).
* @param[in] offset
*		The number of chars expected to be received.
* @param[out] data
*		The data array to be read into.
* @param[in] data_size 
*		The number of chars to be read. example-> 2 chars = 2 as input
*/
void _readcharsFromRegister(char cmd, unsigned short offset, char data[], unsigned short data_size) 
{    
	char header[3];
	unsigned char headerLen = 1;
	
	// build SPI header
	if(offset == NO_SUB) {
		header[0] = READ | cmd;
	} else {
		header[0] = READ_SUB | cmd;
		if(offset < 128) {
			header[1] = (char)offset;
			headerLen++;
		} else {
			header[1] = RW_SUB_EXT | (char)offset;
			header[2] = (char)(offset >> 7);
			headerLen += 2;
		}
	}

	SPIporting_readFromSPI(_ss, headerLen, header, data_size, data);
}

/*
* Write ONLY ONE bit, in a specific register, to the DW1000.
* @param[in] bitRegister
* 		The register address of the selected bit (see Chapter 7 in the DW1000 user manual).
* @param[in] RegisterOffset
*		The offset to select register sub-parts for writing, or 0x00 to disable sub-adressing.
* @param[in] bitRegister_LEN
* 		The lenght of the register to be written
* @param[in] selectedBit
* 		The position(in Int or Hex) of the bit in the register
* @param[in] value
*		The value of the bit to set. It, obviously, can be true/false or 1/0
*/
// 비트를 설정하는 함수
void bitSet(char *target, int bitPosition) {
    *target |= (1 << bitPosition);
}

// 비트를 클리어하는 함수
void bitClear(char *target, int bitPosition) {
    *target &= ~(1 << bitPosition);
}

void _writeBitToRegister(char bitRegister, unsigned short RegisterOffset, unsigned short bitRegister_LEN, unsigned short selectedBit, bool value) {
	unsigned short idx;
	unsigned char bitPosition;

	idx = selectedBit/8;
	if(idx >= bitRegister_LEN) {
		return; // TODO proper error handling: out of bounds
	}
	char targetchar; memset(&targetchar, 0, 1);
	bitPosition = selectedBit%8;
	_readcharsFromRegister(bitRegister, RegisterOffset+idx, &targetchar, 1);
	
	value ? bitSet(&targetchar, bitPosition) : bitClear(&targetchar, bitPosition);

	if(RegisterOffset == NO_SUB)
		RegisterOffset = 0x00;
		
	_writecharsToRegister(bitRegister, RegisterOffset+idx, &targetchar, 1);
}

// always 4 chars
// TODO why always 4 chars? can be different, see p. 58 table 10 otp memory map
void _readcharsOTP(unsigned short address, char data[]) {
	char addresschars[LEN_OTP_ADDR];
	
	// p60 - 6.3.3 Reading a value from OTP memory
	// chars of address
	addresschars[0] = (address & 0xFF);
	addresschars[1] = ((address >> 8) & 0xFF);
	// set address
	_writecharsToRegister(OTP_IF, OTP_ADDR_SUB, addresschars, LEN_OTP_ADDR);
	// switch into read mode
	_writeSinglecharToRegister(OTP_IF, OTP_CTRL_SUB, 0x03); // OTPRDEN | OTPREAD
	_writeSinglecharToRegister(OTP_IF, OTP_CTRL_SUB, 0x01); // OTPRDEN
	// read value/block - 4 chars
	_readcharsFromRegister(OTP_IF, OTP_RDAT_SUB, data, LEN_OTP_RDAT);
	// end read mode
	_writeSinglecharToRegister(OTP_IF, OTP_CTRL_SUB, 0x00);
}

void _enableClock(char clock) {
	char pmscctrl0[LEN_PMSC_CTRL0];
	memset(pmscctrl0, 0, LEN_PMSC_CTRL0);
	_readcharsFromRegister(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
	if(clock == SYS_AUTO_CLOCK) {
		pmscctrl0[0] = SYS_AUTO_CLOCK;
		pmscctrl0[1] &= 0xFE;
	} else if(clock == SYS_XTI_CLOCK) {
		pmscctrl0[0] &= 0xFC;
		pmscctrl0[0] |= SYS_XTI_CLOCK;
	} else if(clock == SYS_PLL_CLOCK) {
		pmscctrl0[0] &= 0xFC;
		pmscctrl0[0] |= SYS_PLL_CLOCK;
	} else if (clock == TX_PLL_CLOCK) {
		pmscctrl0[0] &= 0xCF;
		pmscctrl0[0] |= TX_PLL_CLOCK;
	} else if (clock == LDE_CLOCK) {
		pmscctrl0[0] = SYS_XTI_CLOCK;
		pmscctrl0[1] = 0x03;
	} else {
		// TODO deliver proper warning
	}
	_writecharsToRegister(PMSC, PMSC_CTRL0_SUB, pmscctrl0, 2);
}

/* Steps used to get Temp and Voltage */
void _vbatAndTempSteps() {
	char step1 = 0x80; _writecharsToRegister(RF_CONF, 0x11, &step1, 1);
	char step2 = 0x0A; _writecharsToRegister(RF_CONF, 0x12, &step2, 1);
	char step3 = 0x0F; _writecharsToRegister(RF_CONF, 0x12, &step3, 1);
	char step4 = 0x01; _writecharsToRegister(TX_CAL, NO_SUB, &step4, 1);
	char step5 = 0x00; _writecharsToRegister(TX_CAL, NO_SUB, &step5, 1);
}

/* AGC_TUNE1 - reg:0x23, sub-reg:0x04, table 24 */
void _agctune1() {
	char agctune1[LEN_AGC_TUNE1];
	if(_pulseFrequency == FREQ_16MHZ) {
		DW1000NgUtils_writeValueTochars(agctune1, 0x8870, LEN_AGC_TUNE1);
	} else if(_pulseFrequency == FREQ_64MHZ) {
		DW1000NgUtils_writeValueTochars(agctune1, 0x889B, LEN_AGC_TUNE1);
	} else {
		// TODO proper error/warning handling
	}
	_writecharsToRegister(AGC_TUNE, AGC_TUNE1_SUB, agctune1, LEN_AGC_TUNE1);
}

/* AGC_TUNE2 - reg:0x23, sub-reg:0x0C, table 25 */
void _agctune2() {
	char agctune2[LEN_AGC_TUNE2];
	DW1000NgUtils_writeValueTochars(agctune2, 0x2502A907L, LEN_AGC_TUNE2);
	_writecharsToRegister(AGC_TUNE, AGC_TUNE2_SUB, agctune2, LEN_AGC_TUNE2);
}

/* AGC_TUNE3 - reg:0x23, sub-reg:0x12, table 26 */
void _agctune3() {
	char agctune3[LEN_AGC_TUNE3];
	DW1000NgUtils_writeValueTochars(agctune3, 0x0035, LEN_AGC_TUNE3);
	_writecharsToRegister(AGC_TUNE, AGC_TUNE3_SUB, agctune3, LEN_AGC_TUNE3);
}

/* DRX_TUNE0b - reg:0x27, sub-reg:0x02, table 30 */
void _drxtune0b() {
	char drxtune0b[LEN_DRX_TUNE0b];
	if(_dataRate == RATE_110KBPS) {
		if(!_standardSFD) {
			DW1000NgUtils_writeValueTochars(drxtune0b, 0x0016, LEN_DRX_TUNE0b);
		} else {
			DW1000NgUtils_writeValueTochars(drxtune0b, 0x000A, LEN_DRX_TUNE0b);
		}
	} else if(_dataRate == RATE_850KBPS) {
		if(!_standardSFD) {
			DW1000NgUtils_writeValueTochars(drxtune0b, 0x0006, LEN_DRX_TUNE0b);
		} else {
			DW1000NgUtils_writeValueTochars(drxtune0b, 0x0001, LEN_DRX_TUNE0b);
		}
	} else if(_dataRate == RATE_6800KBPS) {
		if(!_standardSFD) {
			DW1000NgUtils_writeValueTochars(drxtune0b, 0x0002, LEN_DRX_TUNE0b);
		} else {
			DW1000NgUtils_writeValueTochars(drxtune0b, 0x0001, LEN_DRX_TUNE0b);
		}
	} else {
		// TODO proper error/warning handling
	}
	_writecharsToRegister(DRX_TUNE, DRX_TUNE0b_SUB, drxtune0b, LEN_DRX_TUNE0b);
}

/* DRX_TUNE1a - reg:0x27, sub-reg:0x04, table 31 */
void _drxtune1a() {
	char drxtune1a[LEN_DRX_TUNE1a];
	if(_pulseFrequency == FREQ_16MHZ) {
		DW1000NgUtils_writeValueTochars(drxtune1a, 0x0087, LEN_DRX_TUNE1a);
	} else if(_pulseFrequency == FREQ_64MHZ) {
		DW1000NgUtils_writeValueTochars(drxtune1a, 0x008D, LEN_DRX_TUNE1a);
	} else {
		// TODO proper error/warning handling
	}
	_writecharsToRegister(DRX_TUNE, DRX_TUNE1a_SUB, drxtune1a, LEN_DRX_TUNE1a);
}

/* DRX_TUNE1b - reg:0x27, sub-reg:0x06, table 32 */
void _drxtune1b() {
	char drxtune1b[LEN_DRX_TUNE1b];
	if(_preambleLength == LEN_1536 || _preambleLength == LEN_2048 ||
		_preambleLength == LEN_4096) {
		if(_dataRate == RATE_110KBPS) {
			DW1000NgUtils_writeValueTochars(drxtune1b, 0x0064, LEN_DRX_TUNE1b);
		} else {
			// TODO proper error/warning handling
		}
	} else if(_preambleLength != LEN_64) {
		if(_dataRate == RATE_850KBPS || _dataRate == RATE_6800KBPS) {
			DW1000NgUtils_writeValueTochars(drxtune1b, 0x0020, LEN_DRX_TUNE1b);
		} else {
			// TODO proper error/warning handling
		}
	} else {
		if(_dataRate == RATE_6800KBPS) {
			DW1000NgUtils_writeValueTochars(drxtune1b, 0x0010, LEN_DRX_TUNE1b);
		} else {
			// TODO proper error/warning handling
		}
	}
	_writecharsToRegister(DRX_TUNE, DRX_TUNE1b_SUB, drxtune1b, LEN_DRX_TUNE1b);
}

/* DRX_TUNE2 - reg:0x27, sub-reg:0x08, table 33 */
void _drxtune2() {
	char drxtune2[LEN_DRX_TUNE2];	
	if(_pacSize == SIZE_8) {
		if(_pulseFrequency == FREQ_16MHZ) {
			DW1000NgUtils_writeValueTochars(drxtune2, 0x311A002DL, LEN_DRX_TUNE2);
		} else if(_pulseFrequency == FREQ_64MHZ) {
			DW1000NgUtils_writeValueTochars(drxtune2, 0x313B006BL, LEN_DRX_TUNE2);
		} else {
			// TODO proper error/warning handling
		}
	} else if(_pacSize == SIZE_16) {
		if(_pulseFrequency == FREQ_16MHZ) {
			DW1000NgUtils_writeValueTochars(drxtune2, 0x331A0052L, LEN_DRX_TUNE2);
		} else if(_pulseFrequency == FREQ_64MHZ) {
			DW1000NgUtils_writeValueTochars(drxtune2, 0x333B00BEL, LEN_DRX_TUNE2);
		} else {
			// TODO proper error/warning handling
		}
	} else if(_pacSize == SIZE_32) {
		if(_pulseFrequency == FREQ_16MHZ) {
			DW1000NgUtils_writeValueTochars(drxtune2, 0x351A009AL, LEN_DRX_TUNE2);
		} else if(_pulseFrequency == FREQ_64MHZ) {
			DW1000NgUtils_writeValueTochars(drxtune2, 0x353B015EL, LEN_DRX_TUNE2);
		} else {
			// TODO proper error/warning handling
		}
	} else if(_pacSize == SIZE_64) {
		if(_pulseFrequency == FREQ_16MHZ) {
			DW1000NgUtils_writeValueTochars(drxtune2, 0x371A011DL, LEN_DRX_TUNE2);
		} else if(_pulseFrequency == FREQ_64MHZ) {
			DW1000NgUtils_writeValueTochars(drxtune2, 0x373B0296L, LEN_DRX_TUNE2);
		} else {
			// TODO proper error/warning handling
		}
	} else {
		// TODO proper error/warning handling
	}
	_writecharsToRegister(DRX_TUNE, DRX_TUNE2_SUB, drxtune2, LEN_DRX_TUNE2);
}

/* DRX_TUNE4H - reg:0x27, sub-reg:0x26, table 34 */
void _drxtune4H() {
	char drxtune4H[LEN_DRX_TUNE4H];
	if(_preambleLength == LEN_64) {
		DW1000NgUtils_writeValueTochars(drxtune4H, 0x0010, LEN_DRX_TUNE4H);
	} else {
		DW1000NgUtils_writeValueTochars(drxtune4H, 0x0028, LEN_DRX_TUNE4H);
	}
	_writecharsToRegister(DRX_TUNE, DRX_TUNE4H_SUB, drxtune4H, LEN_DRX_TUNE4H);
}

/* LDE_CFG1 - reg 0x2E, sub-reg:0x0806 */
void _ldecfg1() {
	char ldecfg1[LEN_LDE_CFG1];
	_nlos == true ? DW1000NgUtils_writeValueTochars(ldecfg1, 0x7, LEN_LDE_CFG1) : DW1000NgUtils_writeValueTochars(ldecfg1, 0xD, LEN_LDE_CFG1);
	_writecharsToRegister(LDE_IF, LDE_CFG1_SUB, ldecfg1, LEN_LDE_CFG1);
}

/* LDE_CFG2 - reg 0x2E, sub-reg:0x1806, table 50 */
void _ldecfg2() {
	char ldecfg2[LEN_LDE_CFG2];	
	if(_pulseFrequency == FREQ_16MHZ) {
		_nlos == true ? DW1000NgUtils_writeValueTochars(ldecfg2, 0x0003, LEN_LDE_CFG2) : DW1000NgUtils_writeValueTochars(ldecfg2, 0x1607, LEN_LDE_CFG2);
	} else if(_pulseFrequency == FREQ_64MHZ) {
		DW1000NgUtils_writeValueTochars(ldecfg2, 0x0607, LEN_LDE_CFG2);
	} else {
		// TODO proper error/warning handling
	}
	_writecharsToRegister(LDE_IF, LDE_CFG2_SUB, ldecfg2, LEN_LDE_CFG2);
}

/* LDE_REPC - reg 0x2E, sub-reg:0x2804, table 51 */
void _lderepc() {
	char lderepc[LEN_LDE_REPC];
	if(_preambleCode == CODE_1 || _preambleCode == CODE_2) {
		if(_dataRate == RATE_110KBPS) {
			DW1000NgUtils_writeValueTochars(lderepc, ((0x5998 >> 3) & 0xFFFF), LEN_LDE_REPC);
		} else {
			DW1000NgUtils_writeValueTochars(lderepc, 0x5998, LEN_LDE_REPC);
		}
	} else if(_preambleCode == CODE_3 || _preambleCode == CODE_8) {
		if(_dataRate == RATE_110KBPS) {
			DW1000NgUtils_writeValueTochars(lderepc, ((0x51EA >> 3) & 0xFFFF), LEN_LDE_REPC);
		} else {
			DW1000NgUtils_writeValueTochars(lderepc, 0x51EA, LEN_LDE_REPC);
		}
	} else if(_preambleCode == CODE_4) {
		if(_dataRate == RATE_110KBPS) {
			DW1000NgUtils_writeValueTochars(lderepc, ((0x428E >> 3) & 0xFFFF), LEN_LDE_REPC);
		} else {
			DW1000NgUtils_writeValueTochars(lderepc, 0x428E, LEN_LDE_REPC);
		}
	} else if(_preambleCode == CODE_5) {
		if(_dataRate == RATE_110KBPS) {
			DW1000NgUtils_writeValueTochars(lderepc, ((0x451E >> 3) & 0xFFFF), LEN_LDE_REPC);
		} else {
			DW1000NgUtils_writeValueTochars(lderepc, 0x451E, LEN_LDE_REPC);
		}
	} else if(_preambleCode == CODE_6) {
		if(_dataRate == RATE_110KBPS) {
			DW1000NgUtils_writeValueTochars(lderepc, ((0x2E14 >> 3) & 0xFFFF), LEN_LDE_REPC);
		} else {
			DW1000NgUtils_writeValueTochars(lderepc, 0x2E14, LEN_LDE_REPC);
		}
	} else if(_preambleCode == CODE_7) {
		if(_dataRate == RATE_110KBPS) {
			DW1000NgUtils_writeValueTochars(lderepc, ((0x8000 >> 3) & 0xFFFF), LEN_LDE_REPC);
		} else {
			DW1000NgUtils_writeValueTochars(lderepc, 0x8000, LEN_LDE_REPC);
		}
	} else if(_preambleCode == CODE_9) {
		if(_dataRate == RATE_110KBPS) {
			DW1000NgUtils_writeValueTochars(lderepc, ((0x28F4 >> 3) & 0xFFFF), LEN_LDE_REPC);
		} else {
			DW1000NgUtils_writeValueTochars(lderepc, 0x28F4, LEN_LDE_REPC);
		}
	} else if(_preambleCode == CODE_10 || _preambleCode == CODE_17) {
		if(_dataRate == RATE_110KBPS) {
			DW1000NgUtils_writeValueTochars(lderepc, ((0x3332 >> 3) & 0xFFFF), LEN_LDE_REPC);
		} else {
			DW1000NgUtils_writeValueTochars(lderepc, 0x3332, LEN_LDE_REPC);
		}
	} else if(_preambleCode == CODE_11) {
		if(_dataRate == RATE_110KBPS) {
			DW1000NgUtils_writeValueTochars(lderepc, ((0x3AE0 >> 3) & 0xFFFF), LEN_LDE_REPC);
		} else {
			DW1000NgUtils_writeValueTochars(lderepc, 0x3AE0, LEN_LDE_REPC);
		}
	} else if(_preambleCode == CODE_12) {
		if(_dataRate == RATE_110KBPS) {
			DW1000NgUtils_writeValueTochars(lderepc, ((0x3D70 >> 3) & 0xFFFF), LEN_LDE_REPC);
		} else {
			DW1000NgUtils_writeValueTochars(lderepc, 0x3D70, LEN_LDE_REPC);
		}
	} else if(_preambleCode == CODE_18 || _preambleCode == CODE_19) {
		if(_dataRate == RATE_110KBPS) {
			DW1000NgUtils_writeValueTochars(lderepc, ((0x35C2 >> 3) & 0xFFFF), LEN_LDE_REPC);
		} else {
			DW1000NgUtils_writeValueTochars(lderepc, 0x35C2, LEN_LDE_REPC);
		}
	} else if(_preambleCode == CODE_20) {
		if(_dataRate == RATE_110KBPS) {
			DW1000NgUtils_writeValueTochars(lderepc, ((0x47AE >> 3) & 0xFFFF), LEN_LDE_REPC);
		} else {
			DW1000NgUtils_writeValueTochars(lderepc, 0x47AE, LEN_LDE_REPC);
		}
	} else {
		// TODO proper error/warning handling
	}
	
	_writecharsToRegister(LDE_IF, LDE_REPC_SUB, lderepc, LEN_LDE_REPC);
}

/* TX_POWER (enabled smart transmit power control) - reg:0x1E, tables 19-20
* These values are based on a typical IC and an assumed IC to antenna loss of 1.5 dB with a 0 dBi antenna */
void _txpowertune() {
	char txpower[LEN_TX_POWER];
	if(_channel == CHANNEL_1 || _channel == CHANNEL_2) {
		if(_pulseFrequency == FREQ_16MHZ) {
			if(_smartPower) {
				#if DWM1000_OPTIMIZED
				DW1000NgUtils_writeValueTochars(txpower, 0x1B153555L, LEN_TX_POWER);
				#else
				DW1000NgUtils_writeValueTochars(txpower, 0x15355575L, LEN_TX_POWER);
				#endif
			} else {
				#if DWM1000_OPTIMIZED
				DW1000NgUtils_writeValueTochars(txpower, 0x55555555L, LEN_TX_POWER);
				#else
				DW1000NgUtils_writeValueTochars(txpower, 0x75757575L, LEN_TX_POWER);
				#endif
			}
		} else if(_pulseFrequency == FREQ_64MHZ) {
			if(_smartPower) {
				#if DWM1000_OPTIMIZED
				DW1000NgUtils_writeValueTochars(txpower, 0x0D072747L, LEN_TX_POWER);
				#else
				DW1000NgUtils_writeValueTochars(txpower, 0x07274767L, LEN_TX_POWER);
				#endif
			} else {
				#if DWM1000_OPTIMIZED
				DW1000NgUtils_writeValueTochars(txpower, 0x47474747L, LEN_TX_POWER);
				#else
				DW1000NgUtils_writeValueTochars(txpower, 0x67676767L, LEN_TX_POWER);
				#endif
			}
		} else {
			// TODO proper error/warning handling
		}
	} else if(_channel == CHANNEL_3) {
		if(_pulseFrequency == FREQ_16MHZ) {
			if(_smartPower) {
				#if DWM1000_OPTIMIZED
				DW1000NgUtils_writeValueTochars(txpower, 0x150F2F4FL, LEN_TX_POWER);
				#else
				DW1000NgUtils_writeValueTochars(txpower, 0x0F2F4F6FL, LEN_TX_POWER);
				#endif
			} else {
				#if DWM1000_OPTIMIZED
				DW1000NgUtils_writeValueTochars(txpower, 0x4F4F4F4FL, LEN_TX_POWER);
				#else
				DW1000NgUtils_writeValueTochars(txpower, 0x6F6F6F6FL, LEN_TX_POWER);
				#endif
			}
		} else if(_pulseFrequency == FREQ_64MHZ) {
			if(_smartPower) {
				#if DWM1000_OPTIMIZED
				DW1000NgUtils_writeValueTochars(txpower, 0x0B2B4B6BL, LEN_TX_POWER);
				#else
				DW1000NgUtils_writeValueTochars(txpower, 0x2B4B6B8BL, LEN_TX_POWER);
				#endif
			} else {
				#if DWM1000_OPTIMIZED
				DW1000NgUtils_writeValueTochars(txpower, 0x6B6B6B6BL, LEN_TX_POWER);
				#else
				DW1000NgUtils_writeValueTochars(txpower, 0x8B8B8B8BL, LEN_TX_POWER);
				#endif
			}
		} else {
			// TODO proper error/warning handling
		}
	} else if(_channel == CHANNEL_4) {
		if(_pulseFrequency == FREQ_16MHZ) {
			if(_smartPower) {
				#if DWM1000_OPTIMIZED
				DW1000NgUtils_writeValueTochars(txpower, 0x1F1F1F3FL, LEN_TX_POWER);
				#else
				DW1000NgUtils_writeValueTochars(txpower, 0x1F1F3F5FL, LEN_TX_POWER);
				#endif
			} else {
				#if DWM1000_OPTIMIZED
				DW1000NgUtils_writeValueTochars(txpower, 0x3F3F3F3FL, LEN_TX_POWER);
				#else
				DW1000NgUtils_writeValueTochars(txpower, 0x5F5F5F5FL, LEN_TX_POWER);
				#endif
			}
		} else if(_pulseFrequency == FREQ_64MHZ) {
			if(_smartPower) {
				#if DWM1000_OPTIMIZED
				DW1000NgUtils_writeValueTochars(txpower, 0x1A3A5A7AL, LEN_TX_POWER);
				#else
				DW1000NgUtils_writeValueTochars(txpower, 0x3A5A7A9AL, LEN_TX_POWER);
				#endif
			} else {
				#if DWM1000_OPTIMIZED
				DW1000NgUtils_writeValueTochars(txpower, 0x7A7A7A7AL, LEN_TX_POWER);
				#else
				DW1000NgUtils_writeValueTochars(txpower, 0x9A9A9A9AL, LEN_TX_POWER);
				#endif
			}
		} else {
			// TODO proper error/warning handling
		}
	} else if(_channel == CHANNEL_5) {
		if(_pulseFrequency == FREQ_16MHZ) {
			if(_smartPower) {
				#if DWM1000_OPTIMIZED
				DW1000NgUtils_writeValueTochars(txpower, 0x140E0828L, LEN_TX_POWER);
				#else
				DW1000NgUtils_writeValueTochars(txpower, 0x0E082848L, LEN_TX_POWER);
				#endif
			} else {
				#if DWM1000_OPTIMIZED
				DW1000NgUtils_writeValueTochars(txpower, 0x28282828L, LEN_TX_POWER);
				#else
				DW1000NgUtils_writeValueTochars(txpower, 0x48484848L, LEN_TX_POWER);
				#endif
			}
		} else if(_pulseFrequency == FREQ_64MHZ) {
			if(_smartPower) {
				#if DWM1000_OPTIMIZED
				DW1000NgUtils_writeValueTochars(txpower, 0x05254565L, LEN_TX_POWER);
				#else
				DW1000NgUtils_writeValueTochars(txpower, 0x25456585L, LEN_TX_POWER);
				#endif
			} else {
				#if DWM1000_OPTIMIZED
				DW1000NgUtils_writeValueTochars(txpower, 0x65656565L, LEN_TX_POWER);
				#else
				DW1000NgUtils_writeValueTochars(txpower, 0x85858585L, LEN_TX_POWER);
				#endif
			}
		} else {
			// TODO proper error/warning handling
		}
	} else if(_channel == CHANNEL_7) {
		if(_pulseFrequency == FREQ_16MHZ) {
			if(_smartPower) {
				#if DWM1000_OPTIMIZED
				DW1000NgUtils_writeValueTochars(txpower, 0x12325272L, LEN_TX_POWER);
				#else
				DW1000NgUtils_writeValueTochars(txpower, 0x32527292L, LEN_TX_POWER);
				#endif
			} else {
				#if DWM1000_OPTIMIZED
				DW1000NgUtils_writeValueTochars(txpower, 0x72727272L, LEN_TX_POWER);
				#else
				DW1000NgUtils_writeValueTochars(txpower, 0x92929292L, LEN_TX_POWER);
				#endif
			}
		} else if(_pulseFrequency == FREQ_64MHZ) {
			if(_smartPower) {
				#if DWM1000_OPTIMIZED
				DW1000NgUtils_writeValueTochars(txpower, 0x315191B1L, LEN_TX_POWER);
				#else
				DW1000NgUtils_writeValueTochars(txpower, 0x5171B1D1L, LEN_TX_POWER);
				#endif
			} else {
				#if DWM1000_OPTIMIZED
				DW1000NgUtils_writeValueTochars(txpower, 0xB1B1B1B1L, LEN_TX_POWER);
				#else
				DW1000NgUtils_writeValueTochars(txpower, 0xD1D1D1D1L, LEN_TX_POWER);
				#endif
			}
		} else {
			// TODO proper error/warning handling
		}
	} else {
		// TODO proper error/warning handling
	}
	_writecharsToRegister(TX_POWER, NO_SUB, txpower, LEN_TX_POWER);
}

/* RF_RXCTRLH - reg:0x28, sub-reg:0x0B, table 37 */
void _rfrxctrlh() {
	char rfrxctrlh[LEN_RF_RXCTRLH];
	if(_channel != CHANNEL_4 && _channel != CHANNEL_7) {
		DW1000NgUtils_writeValueTochars(rfrxctrlh, 0xD8, LEN_RF_RXCTRLH);
	} else {
		DW1000NgUtils_writeValueTochars(rfrxctrlh, 0xBC, LEN_RF_RXCTRLH);
	}
	_writecharsToRegister(RF_CONF, RF_RXCTRLH_SUB, rfrxctrlh, LEN_RF_RXCTRLH);
}

/* RX_TXCTRL - reg:0x28, sub-reg:0x0C */
void _rftxctrl() {
	char rftxctrl[LEN_RF_TXCTRL];
	if(_channel == CHANNEL_1) {
		DW1000NgUtils_writeValueTochars(rftxctrl, 0x00005C40L, LEN_RF_TXCTRL);
	} else if(_channel == CHANNEL_2) {
		DW1000NgUtils_writeValueTochars(rftxctrl, 0x00045CA0L, LEN_RF_TXCTRL);
	} else if(_channel == CHANNEL_3) {
		DW1000NgUtils_writeValueTochars(rftxctrl, 0x00086CC0L, LEN_RF_TXCTRL);
	} else if(_channel == CHANNEL_4) {
		DW1000NgUtils_writeValueTochars(rftxctrl, 0x00045C80L, LEN_RF_TXCTRL);
	} else if(_channel == CHANNEL_5) {
		DW1000NgUtils_writeValueTochars(rftxctrl, 0x001E3FE0L, LEN_RF_TXCTRL);
	} else if(_channel == CHANNEL_7) {
		DW1000NgUtils_writeValueTochars(rftxctrl, 0x001E7DE0L, LEN_RF_TXCTRL);
	} else {
		// TODO proper error/warning handling
	}
	_writecharsToRegister(RF_CONF, RF_TXCTRL_SUB, rftxctrl, LEN_RF_TXCTRL);
}

/* TC_PGDELAY - reg:0x2A, sub-reg:0x0B, table 40 */
void _tcpgdelaytune() {
	char tcpgdelay[LEN_TC_PGDELAY];	
	if(_channel == CHANNEL_1) {
		DW1000NgUtils_writeValueTochars(tcpgdelay, 0xC9, LEN_TC_PGDELAY);
	} else if(_channel == CHANNEL_2) {
		DW1000NgUtils_writeValueTochars(tcpgdelay, 0xC2, LEN_TC_PGDELAY);
	} else if(_channel == CHANNEL_3) {
		DW1000NgUtils_writeValueTochars(tcpgdelay, 0xC5, LEN_TC_PGDELAY);
	} else if(_channel == CHANNEL_4) {
		DW1000NgUtils_writeValueTochars(tcpgdelay, 0x95, LEN_TC_PGDELAY);
	} else if(_channel == CHANNEL_5) {
		DW1000NgUtils_writeValueTochars(tcpgdelay, 0xB5, LEN_TC_PGDELAY);
	} else if(_channel == CHANNEL_7) {
		DW1000NgUtils_writeValueTochars(tcpgdelay, 0x93, LEN_TC_PGDELAY);
	} else {
		// TODO proper error/warning handling
	}
	_writecharsToRegister(TX_CAL, TC_PGDELAY_SUB, tcpgdelay, LEN_TC_PGDELAY);
}

// FS_PLLCFG and FS_PLLTUNE - reg:0x2B, sub-reg:0x07-0x0B, tables 43-44
void _fspll() {
	char fspllcfg[LEN_FS_PLLCFG];
	char fsplltune[LEN_FS_PLLTUNE];
	if(_channel == CHANNEL_1) {
		DW1000NgUtils_writeValueTochars(fspllcfg, 0x09000407L, LEN_FS_PLLCFG);
		DW1000NgUtils_writeValueTochars(fsplltune, 0x1E, LEN_FS_PLLTUNE);
	} else if(_channel == CHANNEL_2 || _channel == CHANNEL_4) {
		DW1000NgUtils_writeValueTochars(fspllcfg, 0x08400508L, LEN_FS_PLLCFG);
		DW1000NgUtils_writeValueTochars(fsplltune, 0x26, LEN_FS_PLLTUNE);
	} else if(_channel == CHANNEL_3) {
		DW1000NgUtils_writeValueTochars(fspllcfg, 0x08401009L, LEN_FS_PLLCFG);
		DW1000NgUtils_writeValueTochars(fsplltune, 0x56, LEN_FS_PLLTUNE);
	} else if(_channel == CHANNEL_5 || _channel == CHANNEL_7) {
		DW1000NgUtils_writeValueTochars(fspllcfg, 0x0800041DL, LEN_FS_PLLCFG);
		DW1000NgUtils_writeValueTochars(fsplltune, 0xBE, LEN_FS_PLLTUNE);
	} else {
		// TODO proper error/warning handling
	}
	_writecharsToRegister(FS_CTRL, FS_PLLTUNE_SUB, fsplltune, LEN_FS_PLLTUNE);
	_writecharsToRegister(FS_CTRL, FS_PLLCFG_SUB, fspllcfg, LEN_FS_PLLCFG);
}

void _tune() {
	// these registers are going to be tuned/configured
	_agctune1();
	_agctune2();
	_agctune3();
	_drxtune0b();
	_drxtune1a();
	_drxtune1b();
	_drxtune2();
	_drxtune4H();
	_ldecfg1();
	_ldecfg2();
	_lderepc(); 
	if(_autoTXPower) _txpowertune();
	_rfrxctrlh();
	_rftxctrl();
	if(_autoTCPGDelay) _tcpgdelaytune();
	_fspll();
}

void _writeNetworkIdAndDeviceAddress() {
	_writecharsToRegister(PANADR, NO_SUB, _networkAndAddress, LEN_PANADR);
}

void _writeSystemConfigurationRegister() {
	_writecharsToRegister(SYS_CFG, NO_SUB, _syscfg, LEN_SYS_CFG);
}

void _writeChannelControlRegister() {
	_writecharsToRegister(CHAN_CTRL, NO_SUB, _chanctrl, LEN_CHAN_CTRL);
}

void _writeTransmitFrameControlRegister() {
	_writecharsToRegister(TX_FCTRL, NO_SUB, _txfctrl, LEN_TX_FCTRL);
}

void _writeSystemEventMaskRegister() {
	_writecharsToRegister(SYS_MASK, NO_SUB, _sysmask, LEN_SYS_MASK);
}

void _writeAntennaDelayRegisters() {
	char antennaTxDelaychars[2];
	char antennaRxDelaychars[2];
	DW1000NgUtils_writeValueTochars(antennaTxDelaychars, _antennaTxDelay, LEN_TX_ANTD);
	DW1000NgUtils_writeValueTochars(antennaRxDelaychars, _antennaRxDelay, LEN_LDE_RXANTD);
	_writecharsToRegister(TX_ANTD, NO_SUB, antennaTxDelaychars, LEN_TX_ANTD);
	_writecharsToRegister(LDE_IF, LDE_RXANTD_SUB, antennaRxDelaychars, LEN_LDE_RXANTD);
}

void _writeConfiguration() {
	// write all configurations back to device
	_writeSystemConfigurationRegister();
	_writeChannelControlRegister();
	_writeTransmitFrameControlRegister();
}

void _useExtendedFrameLength(bool val) {
	DW1000NgUtils_setBit(_syscfg, LEN_SYS_CFG, PHR_MODE_0_BIT, val);
	DW1000NgUtils_setBit(_syscfg, LEN_SYS_CFG, PHR_MODE_1_BIT, val);
}

void _setReceiverAutoReenable(bool val) {
	DW1000NgUtils_setBit(_syscfg, LEN_SYS_CFG, RXAUTR_BIT, val);
}

void _useFrameCheck(bool val) {
	_frameCheck = val;
}

void _setNlosOptimization(bool val) {
	_nlos = val;
	if(_nlos) {
		_ldecfg1();
		_ldecfg2();
	}
}

void _useSmartPower(bool smartPower) {
	_smartPower = smartPower;
	DW1000NgUtils_setBit(_syscfg, LEN_SYS_CFG, DIS_STXP_BIT, !smartPower);
	_writeSystemConfigurationRegister();
	if(_autoTXPower)
		_txpowertune();
}

void _setSFDMode(SFDMode mode) {
	switch(mode) {
		case STANDARD_SFD:
			DW1000NgUtils_setBit(_chanctrl, LEN_CHAN_CTRL, DWSFD_BIT, false);
			DW1000NgUtils_setBit(_chanctrl, LEN_CHAN_CTRL, TNSSFD_BIT, false);
			DW1000NgUtils_setBit(_chanctrl, LEN_CHAN_CTRL, RNSSFD_BIT, false);
			_standardSFD = true;
			break;
		case DECAWAVE_SFD:
			DW1000NgUtils_setBit(_chanctrl, LEN_CHAN_CTRL, DWSFD_BIT, true);
			DW1000NgUtils_setBit(_chanctrl, LEN_CHAN_CTRL, TNSSFD_BIT, true);
			DW1000NgUtils_setBit(_chanctrl, LEN_CHAN_CTRL, RNSSFD_BIT, true);
			_standardSFD = false;
			break;
		default:
			return; //TODO Proper error handling
	}
}

void _setChannel(Channel channel) {
	char chan = (char)channel;
	chan &= 0xF;
	_chanctrl[0] = ((chan | (chan << 4)) & 0xFF);

	_channel = channel;
}

void _setDataRate(DataRate data_rate) {
	char rate = (char)data_rate;
	rate &= 0x03;
	_txfctrl[1] &= 0x83;
	_txfctrl[1] |= (char)((rate << 5) & 0xFF);
	// special 110kbps flag
	if(data_rate == RATE_110KBPS) {
		DW1000NgUtils_setBit(_syscfg, LEN_SYS_CFG, RXM110K_BIT, true);
	} else {
		DW1000NgUtils_setBit(_syscfg, LEN_SYS_CFG, RXM110K_BIT, false);
	}
	_dataRate = data_rate;
}

void _setPulseFrequency(PulseFrequency frequency) {
	char freq = (char)frequency;
	freq &= 0x03;
	_txfctrl[2] &= 0xFC;
	_txfctrl[2] |= (char)(freq & 0xFF);
	_chanctrl[2] &= 0xF3;
	_chanctrl[2] |= (char)((freq << 2) & 0xFF);

	_pulseFrequency = frequency;
}

void _setPreambleLength(PreambleLength preamble_length) {
	char prealen = (char)preamble_length;
	prealen &= 0x0F;
	_txfctrl[2] &= 0xC3;
	_txfctrl[2] |= (char)((prealen << 2) & 0xFF);
	
	switch(preamble_length) {
		case LEN_64:
			_pacSize = SIZE_8;
			break;
		case LEN_128:
			_pacSize = SIZE_8;
			break;
		case LEN_256:
			_pacSize = SIZE_16;
			break;
		case LEN_512:
			_pacSize = SIZE_16;
			break;
		case LEN_1024:
			_pacSize = SIZE_32;
			break;
		default:
			_pacSize = SIZE_64; // In case of 1536, 2048 or 4096 preamble length.
	}
	
	_preambleLength = preamble_length;
}

void _setPreambleCode(PreambleCode preamble_code) {
	char preacode = (char)preamble_code;
	preacode &= 0x1F;
	_chanctrl[2] &= 0x3F;
	_chanctrl[2] |= ((preacode << 6) & 0xFF);
	_chanctrl[3] = 0x00;
	_chanctrl[3] = ((((preacode >> 2) & 0x07) | (preacode << 3)) & 0xFF);

	_preambleCode = preamble_code;
}

bool _checkPreambleCodeValidity() {
	char preacode = (char)_preambleCode;
	if(_pulseFrequency == FREQ_16MHZ) {
		for (int i = 0; i < 2; i++) {
			if(preacode == preamble_validity_matrix_PRF16[(int) _channel][i])
				return true;
		}
		return false;
	} else if (_pulseFrequency == FREQ_64MHZ) {
		for(int i = 0; i < 4; i++) {
			if(preacode == preamble_validity_matrix_PRF64[(int) _channel][i])
				return true;
		}
		return false;
	} else {
		return false; //TODO Proper error handling
	}
}

void _setValidPreambleCode() {
	PreambleCode preamble_code;

	switch(_channel) {
		case CHANNEL_1:
			preamble_code = _pulseFrequency == FREQ_16MHZ ? CODE_2 : CODE_10;
			break;
		case CHANNEL_3:
			preamble_code = _pulseFrequency == FREQ_16MHZ ? CODE_6 : CODE_10;
			break;
		case CHANNEL_4:
		case CHANNEL_7:
			preamble_code = _pulseFrequency == FREQ_16MHZ ? CODE_8 : CODE_18;
			break;
		case CHANNEL_2:
		case CHANNEL_5:
			preamble_code = _pulseFrequency == FREQ_16MHZ ? CODE_3 : CODE_10;
			break;
		default:
			return; //TODO Proper Error Handling
	}
	char preacode = (char)preamble_code;
	preacode &= 0x1F;
	_chanctrl[2] &= 0x3F;
	_chanctrl[2] |= ((preacode << 6) & 0xFF);
	_chanctrl[3] = 0x00;
	_chanctrl[3] = ((((preacode >> 2) & 0x07) | (preacode << 3)) & 0xFF);

	_preambleCode = preamble_code;
}

void _setNonStandardSFDLength() {
	switch(_dataRate) {
		case RATE_6800KBPS:
			_writeSinglecharToRegister(USR_SFD, SFD_LENGTH_SUB, 0x08);
			break;
		case RATE_850KBPS:
			_writeSinglecharToRegister(USR_SFD, SFD_LENGTH_SUB, 0x10);
			break;
		case RATE_110KBPS:
			_writeSinglecharToRegister(USR_SFD, SFD_LENGTH_SUB, 0x40);
			break;
		default:
			return; //TODO Proper error handling
	}
}

void _interruptOnSent(bool val) {
	DW1000NgUtils_setBit(_sysmask, LEN_SYS_MASK, TXFRS_BIT, val);
}

void _interruptOnReceived(bool val) {
	DW1000NgUtils_setBit(_sysmask, LEN_SYS_MASK, RXDFR_BIT, val);
	DW1000NgUtils_setBit(_sysmask, LEN_SYS_MASK, RXFCG_BIT, val);
}

void _interruptOnReceiveFailed(bool val) {
	DW1000NgUtils_setBit(_sysmask, LEN_SYS_STATUS, RXPHE_BIT, val);
	DW1000NgUtils_setBit(_sysmask, LEN_SYS_STATUS, RXFCE_BIT, val);
	DW1000NgUtils_setBit(_sysmask, LEN_SYS_STATUS, RXRFSL_BIT, val);
	DW1000NgUtils_setBit(_sysmask, LEN_SYS_STATUS, LDEERR_BIT, val);
}

void _interruptOnReceiveTimeout(bool val) {
	DW1000NgUtils_setBit(_sysmask, LEN_SYS_MASK, RXRFTO_BIT, val);
	DW1000NgUtils_setBit(_sysmask, LEN_SYS_MASK, RXPTO_BIT, val);
	DW1000NgUtils_setBit(_sysmask, LEN_SYS_MASK, RXSFDTO_BIT, val);
}

void _interruptOnReceiveTimestampAvailable(bool val) {
	DW1000NgUtils_setBit(_sysmask, LEN_SYS_MASK, LDEDONE_BIT, val);
}

void _interruptOnAutomaticAcknowledgeTrigger(bool val) {
	DW1000NgUtils_setBit(_sysmask, LEN_SYS_MASK, AAT_BIT, val);
}

void _manageLDE() {
	// transfer any ldo tune values
	char ldoTune[LEN_OTP_RDAT];
	unsigned short LDOTUNE_ADDRESS = 0x04;
	_readcharsOTP(LDOTUNE_ADDRESS, ldoTune); // TODO #define
	if(ldoTune[0] != 0) {
		// TODO tuning available, copy over to RAM: use OTP_LDO bit
	}
	// tell the chip to load the LDE microcode
	// TODO remove clock-related code (PMSC_CTRL) as handled separately
	char pmscctrl0[LEN_PMSC_CTRL0];
	char otpctrl[LEN_OTP_CTRL];
	memset(pmscctrl0, 0, LEN_PMSC_CTRL0);
	memset(otpctrl, 0, LEN_OTP_CTRL);
	_readcharsFromRegister(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
	_readcharsFromRegister(OTP_IF, OTP_CTRL_SUB, otpctrl, LEN_OTP_CTRL);
	pmscctrl0[0] = 0x01;
	pmscctrl0[1] = 0x03;
	otpctrl[0]   = 0x00;
	otpctrl[1]   = 0x80;
	_writecharsToRegister(PMSC, PMSC_CTRL0_SUB, pmscctrl0, 2);
	// uCode
	_enableClock(LDE_CLOCK);
	DelayMs(5);
	_writecharsToRegister(OTP_IF, OTP_CTRL_SUB, otpctrl, 2);
	DelayMs(1);
	_enableClock(SYS_AUTO_CLOCK);
	DelayMs(5);
	pmscctrl0[0] = 0x00;
	pmscctrl0[1] &= 0x02;
	_writecharsToRegister(PMSC, PMSC_CTRL0_SUB, pmscctrl0, 2);
}

/* Crystal calibration from OTP (if available)
* FS_XTALT - reg:0x2B, sub-reg:0x0E
* OTP(one-time-programmable) memory map - table 10 */
void _fsxtalt() {
	char fsxtalt[LEN_FS_XTALT];
	char buf_otp[4];
	_readcharsOTP(0x01E, buf_otp); //0x01E -> char[0]=XTAL_Trim
	if (buf_otp[0] == 0) {
		// No trim value available from OTP, use midrange value of 0x10
		DW1000NgUtils_writeValueTochars(fsxtalt, ((0x10 & 0x1F) | 0x60), LEN_FS_XTALT);
	} else {
		DW1000NgUtils_writeValueTochars(fsxtalt, ((buf_otp[0] & 0x1F) | 0x60), LEN_FS_XTALT);
	}
	// write configuration back to chip
	_writecharsToRegister(FS_CTRL, FS_XTALT_SUB, fsxtalt, LEN_FS_XTALT);
}

void _clearReceiveStatus() {
	// clear latched RX bits (i.e. write 1 to clear)
	DW1000NgUtils_setBit(_sysstatus, LEN_SYS_STATUS, RXDFR_BIT, true);
	DW1000NgUtils_setBit(_sysstatus, LEN_SYS_STATUS, RXFCG_BIT, true);
	DW1000NgUtils_setBit(_sysstatus, LEN_SYS_STATUS, RXPRD_BIT, true);
	DW1000NgUtils_setBit(_sysstatus, LEN_SYS_STATUS, RXSFDD_BIT, true);
	DW1000NgUtils_setBit(_sysstatus, LEN_SYS_STATUS, RXPHD_BIT, true);
	DW1000NgUtils_setBit(_sysstatus, LEN_SYS_STATUS, LDEDONE_BIT, true);
	_writecharsToRegister(SYS_STATUS, NO_SUB, _sysstatus, LEN_SYS_STATUS);
}

void _clearReceiveTimestampAvailableStatus() {
	DW1000NgUtils_setBit(_sysstatus, LEN_SYS_STATUS, LDEDONE_BIT, true);
	_writecharsToRegister(SYS_STATUS, NO_SUB, _sysstatus, LEN_SYS_STATUS);
}

void _clearReceiveTimeoutStatus() {
	DW1000NgUtils_setBit(_sysstatus, LEN_SYS_STATUS, RXRFTO_BIT, true);
	DW1000NgUtils_setBit(_sysstatus, LEN_SYS_STATUS, RXPTO_BIT, true);
	DW1000NgUtils_setBit(_sysstatus, LEN_SYS_STATUS, RXSFDTO_BIT, true);
	_writecharsToRegister(SYS_STATUS, NO_SUB, _sysstatus, LEN_SYS_STATUS);
}

void _clearReceiveFailedStatus() {
	DW1000NgUtils_setBit(_sysstatus, LEN_SYS_STATUS, RXPHE_BIT, true);
	DW1000NgUtils_setBit(_sysstatus, LEN_SYS_STATUS, RXFCE_BIT, true);
	DW1000NgUtils_setBit(_sysstatus, LEN_SYS_STATUS, RXRFSL_BIT, true);
	DW1000NgUtils_setBit(_sysstatus, LEN_SYS_STATUS, AFFREJ_BIT, true);
	DW1000NgUtils_setBit(_sysstatus, LEN_SYS_STATUS, LDEERR_BIT, true);
	_writecharsToRegister(SYS_STATUS, NO_SUB, _sysstatus, LEN_SYS_STATUS);
}

void _clearTransmitStatus() {
	DW1000NgUtils_setBit(_sysstatus, LEN_SYS_STATUS, AAT_BIT, true);
	DW1000NgUtils_setBit(_sysstatus, LEN_SYS_STATUS, TXFRB_BIT, true);
	DW1000NgUtils_setBit(_sysstatus, LEN_SYS_STATUS, TXPRS_BIT, true);
	DW1000NgUtils_setBit(_sysstatus, LEN_SYS_STATUS, TXPHS_BIT, true);
	DW1000NgUtils_setBit(_sysstatus, LEN_SYS_STATUS, TXFRS_BIT, true);
	_writecharsToRegister(SYS_STATUS, NO_SUB, _sysstatus, LEN_SYS_STATUS);
}

void _resetReceiver() {
	/* Set to 0 only bit 28 */
	_writeValueToRegister(PMSC, PMSC_SOFTRESET_SUB, 0xE0, LEN_PMSC_SOFTRESET);
	/* Set SOFTRESET to all ones */
	_writeValueToRegister(PMSC, PMSC_SOFTRESET_SUB, 0xF0, LEN_PMSC_SOFTRESET);
}

/* Internal helpers to read configuration */

void _readSystemConfigurationRegister() {
	_readcharsFromRegister(SYS_CFG, NO_SUB, _syscfg, LEN_SYS_CFG);
}

void _readSystemEventStatusRegister() {
	_readcharsFromRegister(SYS_STATUS, NO_SUB, _sysstatus, LEN_SYS_STATUS);
}

void _readNetworkIdAndDeviceAddress() {
	_readcharsFromRegister(PANADR, NO_SUB, _networkAndAddress, LEN_PANADR);
}

void _readSystemEventMaskRegister() {
	_readcharsFromRegister(SYS_MASK, NO_SUB, _sysmask, LEN_SYS_MASK);
}

void _readChannelControlRegister() {
	_readcharsFromRegister(CHAN_CTRL, NO_SUB, _chanctrl, LEN_CHAN_CTRL);
}

void _readTransmitFrameControlRegister() {
	_readcharsFromRegister(TX_FCTRL, NO_SUB, _txfctrl, LEN_TX_FCTRL);
}

bool _isTransmitDone() {
	return DW1000NgUtils_getBit(_sysstatus, LEN_SYS_STATUS, TXFRS_BIT);
}

bool _isReceiveTimestampAvailable() {
	return DW1000NgUtils_getBit(_sysstatus, LEN_SYS_STATUS, LDEDONE_BIT);
}

bool _isReceiveDone() {
	if(_frameCheck) {
		return (DW1000NgUtils_getBit(_sysstatus, LEN_SYS_STATUS, RXFCG_BIT) &&
				DW1000NgUtils_getBit(_sysstatus, LEN_SYS_STATUS, RXDFR_BIT));
	}
	return DW1000NgUtils_getBit(_sysstatus, LEN_SYS_STATUS, RXDFR_BIT);
}

bool _isReceiveFailed() {
	return (DW1000NgUtils_getBit(_sysstatus, LEN_SYS_STATUS, RXPHE_BIT) ||
			DW1000NgUtils_getBit(_sysstatus, LEN_SYS_STATUS, RXFCE_BIT) ||
			DW1000NgUtils_getBit(_sysstatus, LEN_SYS_STATUS, RXRFSL_BIT) ||
			DW1000NgUtils_getBit(_sysstatus, LEN_SYS_STATUS, LDEERR_BIT));
}

bool _isReceiveTimeout() {
	return (DW1000NgUtils_getBit(_sysstatus, LEN_SYS_STATUS, RXRFTO_BIT) || 
			DW1000NgUtils_getBit(_sysstatus, LEN_SYS_STATUS, RXPTO_BIT) || 
			DW1000NgUtils_getBit(_sysstatus, LEN_SYS_STATUS, RXSFDTO_BIT));
}

bool _isClockProblem() {
	return (DW1000NgUtils_getBit(_sysstatus, LEN_SYS_STATUS, CLKPLL_LL_BIT) ||
			DW1000NgUtils_getBit(_sysstatus, LEN_SYS_STATUS, RFPLL_LL_BIT));
}

void _disableSequencing() {
	_enableClock(SYS_XTI_CLOCK);
	char zero[2];
	DW1000NgUtils_writeValueTochars(zero, 0x0000, 2);
	_writecharsToRegister(PMSC, PMSC_CTRL1_SUB, zero, 2); // To re-enable write 0xE7
}

void _configureRFTransmitPowerSpectrumTestMode() {
	/* Enabled TXFEN, PLLFEN, LDOFEN and set TXRXSW to TX */
	char enable_mask[4];
	DW1000NgUtils_writeValueTochars(enable_mask, 0x005FFF00, LEN_RX_CONF_SUB);
	_writecharsToRegister(RF_CONF, RF_CONF_SUB, enable_mask, LEN_RX_CONF_SUB);
}

void _uploadConfigToAON() {
	/* Write 1 in UPL_CFG_BIT */
	_writeValueToRegister(AON, AON_CTRL_SUB, 0x04, LEN_AON_CTRL);
	/* Clear the register */
	_writeValueToRegister(AON, AON_CTRL_SUB, 0x00, LEN_AON_CTRL);
}

/* ####################### PUBLIC ###################### */

void DW1000Ng_initialize(unsigned char ss, unsigned char irq, unsigned char rst) {
	// generous initial init/wake-up-idle DelayMs
	DelayMs(5);
	_ss = ss;
	_irq = irq;
	_rst = rst;

	if(rst != 0xff) {
		// DW1000 data sheet v2.08 §5.6.1 page 20, the RSTn pin should not be driven high but left floating.
		//pinMode(_rst, INPUT);
		GPIO_ModeSwitch(DW1000_RSTn_GPIO_Port, DW1000_RSTn_Pin, GPIO_MODE_INPUT);
	}
	SPIporting_SPIinit();

	// pin and basic member setup
	// attach interrupt
	// TODO throw error if pin is not a interrupt pin

	if(_irq != 0xff)
		//attachInterrupt(digitalPinToInterrupt(_irq), DW1000Ng_interruptServiceRoutine, RISING);
		DW1000Ng_attachExtIrq(DW1000Ng_interruptServiceRoutine);
	
	SPIporting_SPIselect();
	// reset chip (either soft or hard)
	DW1000Ng_reset();
	
	SPIporting_setSPIspeed(SLOW);
	_enableClock(SYS_XTI_CLOCK);
	DelayMs(5);

	// Configure the CPLL lock detect
	_writeBitToRegister(EXT_SYNC, EC_CTRL_SUB, LEN_EC_CTRL, PLLLDT_BIT, true);

	// Configure XTAL trim
	_fsxtalt();

	// load LDE micro-code
	_manageLDE();

	// read the temp and vbat readings from OTP that were recorded during production test
	// see 6.3.1 OTP memory map
	char buf_otp[4];
	_readcharsOTP(0x008, buf_otp); // the stored 3.3 V reading
	_vmeas3v3 = buf_otp[0];
	_readcharsOTP(0x009, buf_otp); // the stored 23C reading
	_tmeas23C = buf_otp[0];

	_enableClock(SYS_AUTO_CLOCK);
	DelayMs(5);
	SPIporting_setSPIspeed(FAST);

	_readNetworkIdAndDeviceAddress();
	_readSystemConfigurationRegister();
	_readChannelControlRegister();
	_readTransmitFrameControlRegister();
	_readSystemEventMaskRegister();

	/* Cleared AON:CFG1(0x2C:0x0A) for proper operation of deepSleep */
	_writeValueToRegister(AON, AON_CFG1_SUB, 0x00, LEN_AON_CFG1);
	
}

void DW1000Ng_initializeNoInterrupt(unsigned char ss, unsigned char rst) {
	DW1000Ng_initialize(ss, 0xff, rst);
}

/* callback handler management. */
void DW1000Ng_attachErrorHandler(ErrorHandlerFunction handleError) {
	_handleError = handleError;
}

void DW1000Ng_attachSentHandler(SentHandlerFunction handleSent) {
	_handleSent = handleSent;
}

void DW1000Ng_attachReceivedHandler(ReceivedHandlerFunction handleReceived) {
	_handleReceived = handleReceived;
}

void DW1000Ng_attachReceiveFailedHandler(ReceiveFailedHandlerFunction handleReceiveFailed) {
	_handleReceiveFailed = handleReceiveFailed;
}

void DW1000Ng_attachReceiveTimeoutHandler(ReceiveTimeoutHandlerFunction handleReceiveTimeout) {
	_handleReceiveTimeout = handleReceiveTimeout;
}

void DW1000Ng_attachReceiveTimestampAvailableHandler(ReceiveTimestampAvailableHandlerFunction handleReceiveTimestampAvailable) {
	_handleReceiveTimestampAvailable = handleReceiveTimestampAvailable;
}

void DW1000Ng_interruptServiceRoutine() {
	// read current status and handle via callbacks
	_readSystemEventStatusRegister();
	if(_isClockProblem() /* TODO and others */ && _handleError != 0) {
		(*_handleError)();
	}
	if(_isTransmitDone()) {
		_clearTransmitStatus();
		if(_handleSent != NULL)
			(*_handleSent)();
	}
	if(_isReceiveTimestampAvailable()) {
		_clearReceiveTimestampAvailableStatus();
		if(_handleReceiveTimestampAvailable != NULL)
			(*_handleReceiveTimestampAvailable)();
	}
	if(_isReceiveFailed()) {
		_clearReceiveFailedStatus();
		DW1000Ng_forceTRxOff();
		_resetReceiver();
		if(_handleReceiveFailed != NULL)
			(*_handleReceiveFailed)();
	} else if(_isReceiveTimeout()) {
		_clearReceiveTimeoutStatus();
		DW1000Ng_forceTRxOff();
		_resetReceiver();
		if(_handleReceiveTimeout != NULL)
			(*_handleReceiveTimeout)();
	} else if(_isReceiveDone()) {
		_clearReceiveStatus();
		if(_handleReceived != NULL)
			(*_handleReceived)();
	}
}

bool DW1000Ng_isTransmitDone(){
	_readSystemEventStatusRegister();
	return _isTransmitDone();
}

void DW1000Ng_clearTransmitStatus() {
	_clearTransmitStatus();
}

bool DW1000Ng_isReceiveDone() {
	_readSystemEventStatusRegister();
	return _isReceiveDone();
}

void DW1000Ng_clearReceiveStatus() {
	_clearReceiveStatus();
}

bool DW1000Ng_isReceiveFailed() {
	_readSystemEventStatusRegister();
	return _isReceiveFailed();
}

void DW1000Ng_clearReceiveFailedStatus() {
	_clearReceiveFailedStatus();
	DW1000Ng_forceTRxOff();
	_resetReceiver();
}

bool DW1000Ng_isReceiveTimeout() {
	_readSystemEventMaskRegister();
	return _isReceiveTimeout();
}

void DW1000Ng_clearReceiveTimeoutStatus() {
	_clearReceiveTimeoutStatus();
	DW1000Ng_forceTRxOff();
	_resetReceiver();
}

void DW1000Ng_enableDebounceClock() {
	char pmscctrl0[LEN_PMSC_CTRL0];
	memset(pmscctrl0, 0, LEN_PMSC_CTRL0);
	_readcharsFromRegister(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
	DW1000NgUtils_setBit(pmscctrl0, LEN_PMSC_CTRL0, GPDCE_BIT, 1);
	DW1000NgUtils_setBit(pmscctrl0, LEN_PMSC_CTRL0, KHZCLKEN_BIT, 1);
	_writecharsToRegister(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
	_debounceClockEnabled = true;
}

void DW1000Ng_enableLedBlinking() {
	char pmscledc[LEN_PMSC_LEDC];
	memset(pmscledc, 0, LEN_PMSC_LEDC);
	_readcharsFromRegister(PMSC, PMSC_LEDC_SUB, pmscledc, LEN_PMSC_LEDC);
	DW1000NgUtils_setBit(pmscledc, LEN_PMSC_LEDC, BLNKEN, 1);
	_writecharsToRegister(PMSC, PMSC_LEDC_SUB, pmscledc, LEN_PMSC_LEDC);
}

void DW1000Ng_setGPIOMode(unsigned char msgp, unsigned char mode) {
	char gpiomode[LEN_GPIO_MODE];
	memset(gpiomode, 0, LEN_GPIO_MODE);
	_readcharsFromRegister(GPIO_CTRL, GPIO_MODE_SUB, gpiomode, LEN_GPIO_MODE);
	for (char i = 0; i < 2; i++){
		DW1000NgUtils_setBit(gpiomode, LEN_GPIO_MODE, msgp + i, (mode >> i) & 1);
	}
	_writecharsToRegister(GPIO_CTRL, GPIO_MODE_SUB, gpiomode, LEN_GPIO_MODE);
}

void DW1000Ng_applySleepConfiguration(sleep_configuration_t sleep_config) {
	char aon_wcfg[LEN_AON_WCFG];
	_readcharsFromRegister(AON, AON_WCFG_SUB, aon_wcfg, LEN_AON_WCFG);
	char aon_cfg0[1];
	memset(aon_cfg0, 0, 1);

	DW1000NgUtils_setBit(aon_wcfg, LEN_AON_WCFG, ONW_RADC_BIT, sleep_config.onWakeUpRunADC);
	DW1000NgUtils_setBit(aon_wcfg, LEN_AON_WCFG, ONW_RX_BIT, sleep_config.onWakeUpReceive);
	DW1000NgUtils_setBit(aon_wcfg, LEN_AON_WCFG, ONW_LEUI_BIT, sleep_config.onWakeUpLoadEUI);
	DW1000NgUtils_setBit(aon_wcfg, LEN_AON_WCFG, ONW_LDC_BIT, true);
	DW1000NgUtils_setBit(aon_wcfg, LEN_AON_WCFG, ONW_L64P_BIT, sleep_config.onWakeUpLoadL64Param);
	DW1000NgUtils_setBit(aon_wcfg, LEN_AON_WCFG, ONW_PRES_SLEEP_BIT, sleep_config.preserveSleep);
	DW1000NgUtils_setBit(aon_wcfg, LEN_AON_WCFG, ONW_LLDE_BIT, true);
	DW1000NgUtils_setBit(aon_wcfg, LEN_AON_WCFG, ONW_LLDO_BIT, true);
	_writecharsToRegister(AON, AON_WCFG_SUB, aon_wcfg, LEN_AON_WCFG);

	DW1000NgUtils_setBit(aon_cfg0, 1, WAKE_PIN_BIT, sleep_config.enableWakePIN);
	DW1000NgUtils_setBit(aon_cfg0, 1, WAKE_SPI_BIT, sleep_config.enableWakeSPI);
	DW1000NgUtils_setBit(aon_cfg0, 1, WAKE_CNT_BIT, false);
	DW1000NgUtils_setBit(aon_cfg0, 1, SLEEP_EN_BIT, sleep_config.enableSLP);
	_writecharsToRegister(AON, AON_CFG0_SUB, aon_cfg0, 1); //Deletes 3 bits of the unused LPCLKDIVA
}

/*Puts the device into sleep/deepSleep mode. This function also upload sleep config to AON. */
void DW1000Ng_deepSleep() {
	/* Clear the register */
	_writeValueToRegister(AON, AON_CTRL_SUB, 0x00, LEN_AON_CTRL);
	/* Write 1 in SAVE_BIT */
	_writeValueToRegister(AON, AON_CTRL_SUB, 0x02, LEN_AON_CTRL);
}

void DW1000Ng_spiWakeup(){
	char deviceId[LEN_DEV_ID];
	char expectedDeviceId[LEN_DEV_ID];
	DW1000NgUtils_writeValueTochars(expectedDeviceId, 0xDECA0130, LEN_DEV_ID);
	_readcharsFromRegister(DEV_ID, NO_SUB, deviceId, LEN_DEV_ID);
	if (memcmp(deviceId, expectedDeviceId, LEN_DEV_ID)) {
		//digitalWrite(_ss, LOW);
		// DelayMs(1);
		// digitalWrite(_ss, HIGH);
		// DelayMs(5);
		out_DW1000_CS(_OFF);
		DelayMs(1);
		out_DW1000_CS(_ON);
		DelayMs(5);
		DW1000Ng_setTxAntennaDelay(_antennaTxDelay);
		if (_debounceClockEnabled){
				DW1000Ng_enableDebounceClock();
		}
	}
}

void DW1000Ng_reset() {
	if(_rst == 0xff) { /* Fallback to Software Reset */
		DW1000Ng_softwareReset();
	} else {
		// DW1000Ng data sheet v2.08 §5.6.1 page 20, the RSTn pin should not be driven high but left floating.
		//pinMode(_rst, OUTPUT);
		//digitalWrite(_rst, LOW);
		//mills(2);
		//pinMode(_rst, INPUT);
		//mills(5);		
		GPIO_ModeSwitch(DW1000_RSTn_GPIO_Port, DW1000_RSTn_Pin, GPIO_MODE_OUTPUT_PP);
		out_DW1000_RSTn(_OFF);
		DelayMs(2);  // DW1000Ng data sheet v2.08 §5.6.1 page 20: nominal 50ns, to be safe take more time
		GPIO_ModeSwitch(DW1000_RSTn_GPIO_Port, DW1000_RSTn_Pin, GPIO_MODE_INPUT);
		DelayMs(5); // dw1000Ng data sheet v1.2 page 5: nominal 3 ms, to be safe take more time
	}
}

void DW1000Ng_softwareReset() {
	SPIporting_setSPIspeed(SLOW);
	
	/* Disable sequencing and go to state "INIT" - (a) Sets SYSCLKS to 01 */
	_disableSequencing();
	/* Clear AON and WakeUp configuration */
	_writeValueToRegister(AON, AON_WCFG_SUB, 0x00, LEN_AON_WCFG);
	_writeValueToRegister(AON, AON_CFG0_SUB, 0x00, LEN_AON_CFG0);
	// TODO change this with uploadToAON
	_writeValueToRegister(AON, AON_CTRL_SUB, 0x00, LEN_AON_CTRL);
	_writeValueToRegister(AON, AON_CTRL_SUB, 0x02, LEN_AON_CTRL);
	/* (b) Clear SOFTRESET to all zero’s */
	_writeValueToRegister(PMSC, PMSC_SOFTRESET_SUB, 0x00, LEN_PMSC_SOFTRESET);
	DelayMs(1);
	/* (c) Set SOFTRESET to all ones */
	_writeValueToRegister(PMSC, PMSC_SOFTRESET_SUB, 0xF0, LEN_PMSC_SOFTRESET);
}

/* ###########################################################################
* #### Pretty printed device information ####################################
* ######################################################################### */


void DW1000Ng_getPrintableDeviceIdentifier(char msgBuffer[]) {
	char data[LEN_DEV_ID];
	_readcharsFromRegister(DEV_ID, NO_SUB, data, LEN_DEV_ID);
	sprintf(msgBuffer, "%02X - model: %d, version: %d, revision: %d",
					(unsigned short)((data[3] << 8) | data[2]), data[1], (data[0] >> 4) & 0x0F, data[0] & 0x0F);
}

void DW1000Ng_getPrintableExtendedUniqueIdentifier(char msgBuffer[]) {
	char data[LEN_EUI];
	_readcharsFromRegister(EUI, NO_SUB, data, LEN_EUI);
	sprintf(msgBuffer, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
					data[7], data[6], data[5], data[4], data[3], data[2], data[1], data[0]);
}

void DW1000Ng_getPrintableNetworkIdAndShortAddress(char msgBuffer[]) {
	char data[LEN_PANADR];
	_readcharsFromRegister(PANADR, NO_SUB, data, LEN_PANADR);
	sprintf(msgBuffer, "PAN: %02X, Short Address: %02X",
					(unsigned short)((data[3] << 8) | data[2]), (unsigned short)((data[1] << 8) | data[0]));
}

void DW1000Ng_getPrintableDeviceMode(char msgBuffer[]) {
	unsigned short dr;
	unsigned char prf;
	unsigned short plen;
	unsigned char pcode;
	unsigned char ch;
	char chan_ctrl[LEN_CHAN_CTRL];
	char tx_fctrl[LEN_TX_FCTRL];
	_readcharsFromRegister(CHAN_CTRL, NO_SUB, chan_ctrl, LEN_CHAN_CTRL);
	_readcharsFromRegister(TX_FCTRL, NO_SUB, tx_fctrl, LEN_TX_FCTRL);
	/* Data Rate from 0x08 bits:13-14(tx_fctrl) */
	dr = (unsigned short)(tx_fctrl[1] >> 5 & 0x3);
	switch(dr) {
		case 0x00:
			dr = 110;
			break;
		case 0x01:
			dr = 850;
			break;
		case 0x02:
			dr = 6800;
			break;
		default:
			return; //TODO Error handling
	}
	/* PRF(16 or 64) from 0x1F bits:18-19(chan_ctrl) */
	prf = (unsigned char)(chan_ctrl[2] >> 2 & 0x03);
	if(prf == 0x01){
		prf = 16;
	} else if(prf == 0x02){
		prf = 64;
	} else{
		return; //TODO Error handling
	}
	/* PreambleLength from 0x08 bits:18-21(tx_fctrl) */
	plen = (unsigned short)(tx_fctrl[2] >> 2 & 0xF);
	switch(plen) {
		case 0x01:
			plen = 64;
			break;
		case 0x05:
			plen = 128;
			break;
		case 0x09:
			plen = 256;
			break;
		case 0x0D:
			plen = 512;
			break;
		case 0x02:
			plen = 1024;
			break;
		case 0x06:
			plen = 1536;
			break;
		case 0x0A:
			plen = 2048;
			break;
		case 0x03:
			plen = 4096;
			break;
		default:
			return; //TODO Error handling
	}
	/* Channel from 0x1F bits:0-4(tx_chan) */
	ch = (unsigned char)(chan_ctrl[0] & 0xF);
	/* Preamble Code from 0x1F bits:24-31(chan_ctrl) */
	pcode = (unsigned char)(chan_ctrl[3] >> 3 & 0x1F);
	sprintf(msgBuffer, "Data rate: %u kb/s, PRF: %u MHz, Preamble: %u symbols, Channel: #%u, Preamble code #%u" , dr, prf, plen, ch, pcode);
}

/* ###########################################################################
* #### DW1000Ng operation functions ###########################################
* ######################################################################### */

void DW1000Ng_setNetworkId(unsigned short val) {
	_networkAndAddress[2] = (char)(val & 0xFF);
	_networkAndAddress[3] = (char)((val >> 8) & 0xFF);
	_writeNetworkIdAndDeviceAddress();
}

void DW1000Ng_getNetworkId(char id[]) {
	_readNetworkIdAndDeviceAddress();
	id[0] = _networkAndAddress[2];
	id[1] = _networkAndAddress[3];
}

void DW1000Ng_setDeviceAddress(unsigned short val) {
	_networkAndAddress[0] = (char)(val & 0xFF);
	_networkAndAddress[1] = (char)((val >> 8) & 0xFF);
	_writeNetworkIdAndDeviceAddress();
}

void DW1000Ng_getDeviceAddress(char address[]) {
	_readNetworkIdAndDeviceAddress();
	address[0] = _networkAndAddress[0];
	address[1] = _networkAndAddress[1];
}

void DW1000Ng_setEUI(char eui[]) {
	char eui_char[LEN_EUI];
	char    reverseEUI[8];
	unsigned char     size = 8;

	DW1000NgUtils_convertToByte(eui, eui_char);
	
	//we reverse the address->

	for(unsigned char i    = 0; i < size; i++) {
		*(reverseEUI+i) = *(eui+size-i-1);
	}
	_writecharsToRegister(EUI, NO_SUB, reverseEUI, LEN_EUI);
}

void DW1000Ng_getEUI(char eui[]) {
	_readcharsFromRegister(EUI, NO_SUB, eui, LEN_EUI);
}

float DW1000Ng_getTemperature() {
	_vbatAndTempSteps();
	char sar_ltemp = 0; _readcharsFromRegister(TX_CAL, 0x04, &sar_ltemp, 1);
	return (sar_ltemp - _tmeas23C) * 1.14f + 23.0f;
}

float DW1000Ng_getBatteryVoltage() {
	_vbatAndTempSteps();
	char sar_lvbat = 0; _readcharsFromRegister(TX_CAL, 0x03, &sar_lvbat, 1);
	return (sar_lvbat - _vmeas3v3) / 173.0f + 3.3f;
}

void DW1000Ng_getTemperatureAndBatteryVoltage(float *temp, float *vbat) {
    _vbatAndTempSteps();
    DelayMs(1);
    char sar_lvbat = 0;
    _readcharsFromRegister(TX_CAL, 0x03, &sar_lvbat, 1);
    char sar_ltemp = 0;
    _readcharsFromRegister(TX_CAL, 0x04, &sar_ltemp, 1);
    
    // calculate voltage and temperature
    *vbat = (sar_lvbat - _vmeas3v3) / 173.0f + 3.3f;
    *temp = (sar_ltemp - _tmeas23C) * 1.14f + 23.0f;
}

void DW1000Ng_enableFrameFiltering(frame_filtering_configuration_t config) {
	DW1000NgUtils_setBit(_syscfg, LEN_SYS_CFG, FFEN_BIT, true);
	DW1000NgUtils_setBit(_syscfg, LEN_SYS_CFG, FFBC_BIT, config.behaveAsCoordinator);
	DW1000NgUtils_setBit(_syscfg, LEN_SYS_CFG, FFAB_BIT, config.allowBeacon);
	DW1000NgUtils_setBit(_syscfg, LEN_SYS_CFG, FFAD_BIT, config.allowData);
	DW1000NgUtils_setBit(_syscfg, LEN_SYS_CFG, FFAA_BIT, config.allowAcknowledgement);
	DW1000NgUtils_setBit(_syscfg, LEN_SYS_CFG, FFAM_BIT, config.allowMacCommand);
	DW1000NgUtils_setBit(_syscfg, LEN_SYS_CFG, FFAR_BIT, config.allowAllReserved);
	DW1000NgUtils_setBit(_syscfg, LEN_SYS_CFG, FFA4_BIT, config.allowReservedFour);
	DW1000NgUtils_setBit(_syscfg, LEN_SYS_CFG, FFA5_BIT, config.allowReservedFive);

	_writeSystemConfigurationRegister();
}

void DW1000Ng_disableFrameFiltering() {
	DW1000NgUtils_setBit(_syscfg, LEN_SYS_CFG, FFEN_BIT, false);
	_writeSystemConfigurationRegister();
}

void DW1000Ng_setDoubleBuffering(bool val) {
	DW1000NgUtils_setBit(_syscfg, LEN_SYS_CFG, DIS_DRXB_BIT, !val);
}

void DW1000Ng_setAntennaDelay(unsigned short value) {
	_antennaTxDelay = value;
	_antennaRxDelay = value;
	_writeAntennaDelayRegisters();
}

#if defined(__AVR__)
	void setAndSaveAntennaDelay(unsigned short DelayMs, unsigned char eeAddress) {
		EEPROM.put(eeAddress, DelayMs);
		EEPROM.end();
		setAntennaDelay(DelayMs);
	}

	unsigned short getSavedAntennaDelay(unsigned char eeAddress) {
		unsigned short DelayMs;
		EEPROM.get(eeAddress, DelayMs);
		EEPROM.end();
		return DelayMs;
	}

	unsigned short setAntennaDelayFromEEPROM(unsigned char eeAddress) {
		unsigned short DelayMs = getSavedAntennaDelay(eeAddress);
		setAntennaDelay(DelayMs);
	}
#endif

void DW1000Ng_setTxAntennaDelay(unsigned short value) {
	_antennaTxDelay = value;
	_writeAntennaDelayRegisters();	
}
void DW1000Ng_setRxAntennaDelay(unsigned short value) {
	_antennaRxDelay = value;
	_writeAntennaDelayRegisters();
}

unsigned short DW1000Ng_getTxAntennaDelay() {
	return _antennaTxDelay;
}
unsigned short DW1000Ng_getRxAntennaDelay() {
	return _antennaRxDelay;
}

void DW1000Ng_forceTRxOff() {
	memset(_sysctrl, 0, LEN_SYS_CTRL);
	DW1000NgUtils_setBit(_sysctrl, LEN_SYS_CTRL, TRXOFF_BIT, true);
	_writecharsToRegister(SYS_CTRL, NO_SUB, _sysctrl, LEN_SYS_CTRL);
}

void DW1000Ng_startReceive(TransmitMode mode) {
	memset(_sysctrl, 0, LEN_SYS_CTRL);
	DW1000NgUtils_setBit(_sysctrl, LEN_SYS_CTRL, SFCST_BIT, !_frameCheck);
	if(mode == DELAYED)
		DW1000NgUtils_setBit(_sysctrl, LEN_SYS_CTRL, RXDLYS_BIT, true);
	DW1000NgUtils_setBit(_sysctrl, LEN_SYS_CTRL, RXENAB_BIT, true);
	_writecharsToRegister(SYS_CTRL, NO_SUB, _sysctrl, LEN_SYS_CTRL);
}

void DW1000Ng_startTransmit(TransmitMode mode) {
	memset(_sysctrl, 0, LEN_SYS_CTRL);
	DW1000NgUtils_setBit(_sysctrl, LEN_SYS_CTRL, SFCST_BIT, !_frameCheck);
	if(mode == DELAYED)
		DW1000NgUtils_setBit(_sysctrl, LEN_SYS_CTRL, TXDLYS_BIT, true);
	if(_wait4resp)
		DW1000NgUtils_setBit(_sysctrl, LEN_SYS_CTRL, WAIT4RESP_BIT, true);

	DW1000NgUtils_setBit(_sysctrl, LEN_SYS_CTRL, TXSTRT_BIT, true);
	_writecharsToRegister(SYS_CTRL, NO_SUB, _sysctrl, LEN_SYS_CTRL);
}

void DW1000Ng_setInterruptPolarity(bool val) {
	DW1000NgUtils_setBit(_syscfg, LEN_SYS_CFG, HIRQ_POL_BIT, val);
	_writeSystemConfigurationRegister();
}

void DW1000Ng_applyConfiguration(device_configuration_t config) {
	DW1000Ng_forceTRxOff();

	_useExtendedFrameLength(config.extendedFrameLength);
	_setReceiverAutoReenable(config.receiverAutoReenable);
	_useSmartPower(config.smartPower);
	_useFrameCheck(config.frameCheck);
	_setNlosOptimization(config.nlos);
	_setSFDMode(config.sfd);
	_setChannel(config.channel);
	_setDataRate(config.dataRate);
	_setPulseFrequency(config.pulseFreq);
	_setPreambleLength(config.preambleLen);
	_setPreambleCode(config.preaCode);

	if(!_checkPreambleCodeValidity())
		_setValidPreambleCode();

	if(!_standardSFD)
		_setNonStandardSFDLength();

	// writes configuration to registers
	_writeConfiguration();
	// tune according to configuration
	_tune();
}

Channel DW1000Ng_getChannel() {
	return _channel;
}

PulseFrequency DW1000Ng_getPulseFrequency() {
	return _pulseFrequency;
}

void DW1000Ng_setPreambleDetectionTimeout(unsigned short pacSize) {
	char drx_pretoc[LEN_DRX_PRETOC];
	DW1000NgUtils_writeValueTochars(drx_pretoc, pacSize, LEN_DRX_PRETOC);
	_writecharsToRegister(DRX_TUNE, DRX_PRETOC_SUB, drx_pretoc, LEN_DRX_PRETOC);
}

void DW1000Ng_setSfdDetectionTimeout(unsigned short preambleSymbols) {
	char drx_sfdtoc[LEN_DRX_SFDTOC];
	DW1000NgUtils_writeValueTochars(drx_sfdtoc, preambleSymbols, LEN_DRX_SFDTOC);
	_writecharsToRegister(DRX_TUNE, DRX_SFDTOC_SUB, drx_sfdtoc, LEN_DRX_SFDTOC);
}

void DW1000Ng_setReceiveFrameWaitTimeoutPeriod(unsigned short timeMicroSeconds) {
	if (timeMicroSeconds > 0) {
		char rx_wfto[LEN_RX_WFTO];
		DW1000NgUtils_writeValueTochars(rx_wfto, timeMicroSeconds, LEN_RX_WFTO);
		_writecharsToRegister(RX_WFTO, NO_SUB, rx_wfto, LEN_RX_WFTO);
		/* enable frame wait timeout bit */
		DW1000NgUtils_setBit(_syscfg, LEN_SYS_CFG, RXWTOE_BIT, true);
		_writeSystemConfigurationRegister();
	} else {
		/* disable frame wait timeout bit */
		DW1000NgUtils_setBit(_syscfg, LEN_SYS_CFG, RXWTOE_BIT, false);
		_writeSystemConfigurationRegister();
	}
}

void DW1000Ng_applyInterruptConfiguration(interrupt_configuration_t interrupt_config) {
	DW1000Ng_forceTRxOff();

	_interruptOnSent(interrupt_config.interruptOnSent);
	_interruptOnReceived(interrupt_config.interruptOnReceived);
	_interruptOnReceiveFailed(interrupt_config.interruptOnReceiveFailed);
	_interruptOnReceiveTimeout(interrupt_config.interruptOnReceiveTimeout);
	_interruptOnReceiveTimestampAvailable(interrupt_config.interruptOnReceiveTimestampAvailable);
	_interruptOnAutomaticAcknowledgeTrigger(interrupt_config.interruptOnAutomaticAcknowledgeTrigger);

	_writeSystemEventMaskRegister();
}

void DW1000Ng_setWait4Response(uint32_t timeMicroSeconds) {
	_wait4resp = timeMicroSeconds == 0 ? false : true;

	/* Check if it overflows 20 bits */
	if(timeMicroSeconds > 1048575)
		timeMicroSeconds = 1048575;

	char W4R_TIME[LEN_ACK_RESP_T_W4R_TIME_SUB];
	DW1000NgUtils_writeValueTochars(W4R_TIME, timeMicroSeconds, LEN_ACK_RESP_T_W4R_TIME_SUB);
	W4R_TIME[2] &= 0x0F; 
	_writecharsToRegister(ACK_RESP_T, ACK_RESP_T_W4R_TIME_SUB, W4R_TIME, LEN_ACK_RESP_T_W4R_TIME_SUB);
}

// void setTXPower(char power[]) {
// 	//TODO Check char length
// 	_writecharsToRegister(TX_POWER, NO_SUB, power, LEN_TX_POWER);
// 	_autoTXPower = false;
// }

void DW1000Ng_setTXPower(int32_t power) {
	char txpower[LEN_TX_POWER];
	DW1000NgUtils_writeValueTochars(txpower, power, LEN_TX_POWER);
	//TODO Check char length
	_writecharsToRegister(TX_POWER, NO_SUB, txpower, LEN_TX_POWER);
	_autoTXPower = false;
}

// void setTXPower(DriverAmplifierValue driver_amplifier, TransmitMixerValue mixer) {
// 	char txpower[LEN_TX_POWER];
// 	char pwr = 0x00;

// 	pwr |= ((char) driver_amplifier << 5);
// 	pwr |= (char) mixer;

// 	for(int i = 0; i < LEN_TX_POWER; i++) {
// 		txpower[i] = pwr;
// 	}

// 	setTXPower(txpower);
// }

void DW1000Ng_setTXPowerAuto() {
	_autoTXPower = true;
	_txpowertune();
}

void DW1000Ng_setTCPGDelay(char tcpgdelay) {
	char tcpgchars[LEN_TC_PGDELAY];
	DW1000NgUtils_writeValueTochars(tcpgchars, tcpgdelay, LEN_TC_PGDELAY);
	_writecharsToRegister(TX_CAL, TC_PGDELAY_SUB, tcpgchars, LEN_TC_PGDELAY);
	_autoTCPGDelay = false;
}

void DW1000Ng_setTCPGDelayAuto() {
	_tcpgdelaytune();
	_autoTCPGDelay = true;
}

void DW1000Ng_enableTransmitPowerSpectrumTestMode(int32_t repeat_interval) {
	/* DW1000 clocks must be set to crystal speed so SPI rate have to be lowered and will
	not be increased again */
	SPIporting_setSPIspeed(SLOW);

	_disableSequencing();
	_configureRFTransmitPowerSpectrumTestMode();
	_enableClock(SYS_PLL_CLOCK);
	_enableClock(TX_PLL_CLOCK);

	if(repeat_interval < 4) 
		repeat_interval = 4;

	/* In diagnostic transmit power  mode (set next) the chars 31:0 only are used for DX_TIME register */
	char delaychars[4];
	DW1000NgUtils_writeValueTochars(delaychars, repeat_interval, 4);
	_writecharsToRegister(DX_TIME, NO_SUB, delaychars, 4);

	/* Enable Transmit Power Spectrum Test Mode */
	char diagnosticchars[2];
	DW1000NgUtils_writeValueTochars(diagnosticchars, 0x0010, LEN_DIAG_TMC);
	_writecharsToRegister(DIG_DIAG, DIAG_TMC_SUB, diagnosticchars, LEN_DIAG_TMC);
}

void DW1000Ng_setDelayedTRX(char futureTimechars[]) {
	/* the least significant 9-bits are ignored in DX_TIME in functional modes */
	_writecharsToRegister(DX_TIME, NO_SUB, futureTimechars, LEN_DX_TIME);
}

void DW1000Ng_setTransmitDataByte(char data[], unsigned short n) {
	if(_frameCheck) {
		n += 2; // two chars CRC-16
	}
	if(n > LEN_EXT_UWB_FRAMES) {
		return; // TODO proper error handling: frame/buffer size
	}
	if((n > LEN_UWB_FRAMES) && !_extendedFrameLength) {
		return; // TODO proper error handling: frame/buffer size
	}
	// transmit data and length
	_writecharsToRegister(TX_BUFFER, NO_SUB, data, n);
	
	/* Sets up transmit frame control length based on data length */
	_txfctrl[0] = (char)(n & 0xFF); // 1 char (regular length + 1 bit)
	_txfctrl[1] &= 0xE0;
	_txfctrl[1] |= (char)((n >> 8) & 0x03);  // 2 added bits if extended length
	_writeTransmitFrameControlRegister();
}

void DW1000Ng_setTransmitDataString(const char *data) {
    unsigned short n = strlen(data) + 1;
    char *datachars = (char *)malloc(n);
    
    if (datachars != NULL) {
        strcpy(datachars, data);
        DW1000Ng_setTransmitDataByte(datachars, n);
        free(datachars);
    } else {
        // 메모리 할당 실패 처리
    }
}


// TODO reorder
unsigned short DW1000Ng_getReceivedDataLength() {
	unsigned short len = 0;

	// 10 bits of RX frame control register
	char rxFrameInfo[LEN_RX_FINFO];
	_readcharsFromRegister(RX_FINFO, NO_SUB, rxFrameInfo, LEN_RX_FINFO);
	len = ((((unsigned short)rxFrameInfo[1] << 8) | (unsigned short)rxFrameInfo[0]) & 0x03FF);
	
	if(_frameCheck && len > 2) {
		return len-2;
	}
	return len;
}

void DW1000Ng_getReceivedData(char data[], unsigned short n) {
	if(n <= 0) {
		return;
	}
	_readcharsFromRegister(RX_BUFFER, NO_SUB, data, n);
}

// void getReceivedData(String& data) {
// 	unsigned short i;
// 	unsigned short n = getReceivedDataLength(); // number of chars w/o the two FCS ones
// 	if(n <= 0) { // TODO
// 		return;
// 	}
// 	char* datachars = (char*)malloc(n);
// 	getReceivedData(datachars, n);
// 	// clear string
// 	data.remove(0);
// 	data  = "";
// 	// append to string
// 	for(i = 0; i < n; i++) {
// 		data += (char)datachars[i];
// 	}
// 	free(datachars);
// }

uint64_t DW1000Ng_getTransmitTimestamp() {
	char data[LENGTH_TIMESTAMP];
	memset(data, 0 , LENGTH_TIMESTAMP);
	_readcharsFromRegister(TX_TIME, TX_STAMP_SUB, data, LEN_TX_STAMP);
	return DW1000NgUtils_charsAsValue(data, LEN_TX_STAMP);
}

uint64_t DW1000Ng_getReceiveTimestamp() {
	char data[LEN_RX_STAMP];
	memset(data, 0, LEN_RX_STAMP);
	_readcharsFromRegister(RX_TIME, RX_STAMP_SUB, data, LEN_RX_STAMP);
	return DW1000NgUtils_charsAsValue(data, LEN_RX_STAMP);
}

uint64_t DW1000Ng_getSystemTimestamp() {
	char data[LEN_SYS_TIME];
	memset(data, 0, LEN_SYS_TIME);
	_readcharsFromRegister(SYS_TIME, NO_SUB, data, LEN_SYS_TIME);
	return DW1000NgUtils_charsAsValue(data, LEN_SYS_TIME);		
}

float DW1000Ng_getReceiveQuality() {
	char         noisechars[LEN_STD_NOISE];
	char         fpAmpl2chars[LEN_FP_AMPL2];
	unsigned short     noise, f2;
	_readcharsFromRegister(RX_FQUAL, STD_NOISE_SUB, noisechars, LEN_STD_NOISE);
	_readcharsFromRegister(RX_FQUAL, FP_AMPL2_SUB, fpAmpl2chars, LEN_FP_AMPL2);
	noise = (unsigned short)noisechars[0] | ((unsigned short)noisechars[1] << 8);
	f2    = (unsigned short)fpAmpl2chars[0] | ((unsigned short)fpAmpl2chars[1] << 8);
	return (float)f2/noise;
}

float DW1000Ng_getFirstPathPower() {
	char         fpAmpl1chars[LEN_FP_AMPL1];
	char         fpAmpl2chars[LEN_FP_AMPL2];
	char         fpAmpl3chars[LEN_FP_AMPL3];
	char         rxFrameInfo[LEN_RX_FINFO];
	unsigned short     f1, f2, f3, N;
	float        A, corrFac;
	_readcharsFromRegister(RX_TIME, FP_AMPL1_SUB, fpAmpl1chars, LEN_FP_AMPL1);
	_readcharsFromRegister(RX_FQUAL, FP_AMPL2_SUB, fpAmpl2chars, LEN_FP_AMPL2);
	_readcharsFromRegister(RX_FQUAL, FP_AMPL3_SUB, fpAmpl3chars, LEN_FP_AMPL3);
	_readcharsFromRegister(RX_FINFO, NO_SUB, rxFrameInfo, LEN_RX_FINFO);
	f1 = (unsigned short)fpAmpl1chars[0] | ((unsigned short)fpAmpl1chars[1] << 8);
	f2 = (unsigned short)fpAmpl2chars[0] | ((unsigned short)fpAmpl2chars[1] << 8);
	f3 = (unsigned short)fpAmpl3chars[0] | ((unsigned short)fpAmpl3chars[1] << 8);
	N  = (((unsigned short)rxFrameInfo[2] >> 4) & 0xFF) | ((unsigned short)rxFrameInfo[3] << 4);

	if(_pulseFrequency == FREQ_16MHZ) {
		A       = 113.77;
		corrFac = 2.3334;
	} else {
		A       = 121.74;
		corrFac = 1.1667;
	}
	float estFpPwr = 10.0*log10(((float)f1*(float)f1+(float)f2*(float)f2+(float)f3*(float)f3)/((float)N*(float)N))-A;
	if(estFpPwr <= -88) {
		return estFpPwr;
	} else {
		// approximation of Fig. 22 in user manual for dbm correction
		estFpPwr += (estFpPwr+88)*corrFac;
	}
	return estFpPwr;
}

void DW1000Ng_set_max_5007()
{
	uint32_t reg;
	memset((char*)&reg, 0, 4);
	_readcharsFromRegister(0x26, 0x00, (char*)&reg, 4); // GPIO_CTRL
	reg |= 0x00014000; // PA��� ����
	reg |= 0x00050000; // LNA��� ����
	_writecharsToRegister(0x26, 0x00, (char*)&reg, 4);
//		reg = 0x1f1f1f1f;
	reg = 0xbfbfbfbf;
	_writecharsToRegister(TX_POWER, 0x00, (char*)&reg, 4);

#define PMSC_TXFINESEQ_OFFSET         0x26
	char pmsc_mask_dis[2];
	pmsc_mask_dis[0] = 0;
	pmsc_mask_dis[1] = 0;
	_writecharsToRegister(PMSC, PMSC_TXFINESEQ_OFFSET, pmsc_mask_dis, 2);

}

/*	
void get_gpio() 
{
	uint32_t reg;
	memset((char*)&reg, 0, 4);
	DW1000Ng__readcharsFromRegister(0x26, 0x00, (char*)&reg, 4); // GPIO_CTRL
	printfln(reg,BIN);
}

void set_gpio()
{
	uint32_t reg;
	memset((char*)&reg, 0, 4);
	DW1000Ng__readcharsFromRegister(0x26, 0x00, (char*)&reg, 4); // GPIO_CTRL
	reg |= 0x00014000; //7��8��PA���Բ�����Ҹ���Դϴ�.
	reg |= 0x00050000; //8 �� 9�� ���-RX/TX �׽�Ʈ��
	DW1000Ng__writecharsToRegister(0x26, 0x00, (char*)&reg, 4);
	reg = 0x1f1f1f1f;
	DW1000Ng__writecharsToRegister(TX_POWER, 0x00, (char*)&reg, 4);
}
*/
float DW1000Ng_getReceivePower() {
	char     cirPwrchars[LEN_CIR_PWR];
	char     rxFrameInfo[LEN_RX_FINFO];
	uint32_t twoPower17 = 131072;
	unsigned short C, N;
	float    A, corrFac;
	_readcharsFromRegister(RX_FQUAL, CIR_PWR_SUB, cirPwrchars, LEN_CIR_PWR);
	_readcharsFromRegister(RX_FINFO, NO_SUB, rxFrameInfo, LEN_RX_FINFO);
	C = (unsigned short)cirPwrchars[0] | ((unsigned short)cirPwrchars[1] << 8);
	N = (((unsigned short)rxFrameInfo[2] >> 4) & 0xFF) | ((unsigned short)rxFrameInfo[3] << 4);

	if(_pulseFrequency == FREQ_16MHZ) {
		A       = 113.77;
		corrFac = 2.3334;
	} else {
		A       = 121.74;
		corrFac = 1.1667;
	}
	
	float estRxPwr = 10.0*log10(((float)C*(float)twoPower17)/((float)N*(float)N))-A;
	if(estRxPwr <= -88) {
		return estRxPwr;
	} else {
		// approximation of Fig. 22 in user manual for dbm correction
		estRxPwr += (estRxPwr+88)*corrFac;
	}
	return estRxPwr;
}

#if DW1000NG_DEBUG
void getPrettychars(char data[], char msgBuffer[], unsigned short n) {
	unsigned short i, j, b;
	b = sprintf(msgBuffer, "Data, chars: %d\nB: 7 6 5 4 3 2 1 0\n", n); // TODO - type
	for(i = 0; i < n; i++) {
		char curchar = data[i];
		snprintf(&msgBuffer[b++], 2, "%d", (i+1));
		msgBuffer[b++] = (char)((i+1) & 0xFF);
		msgBuffer[b++] = ':';
		msgBuffer[b++] = ' ';
		for(j = 0; j < 8; j++) {
			msgBuffer[b++] = ((curchar >> (7-j)) & 0x01) ? '1' : '0';
			if(j < 7) {
				msgBuffer[b++] = ' ';
			} else if(i < n-1) {
				msgBuffer[b++] = '\n';
			} else {
				msgBuffer[b++] = '\0';
			}
		}
	}
	msgBuffer[b++] = '\0';
}

void getPrettychars(char cmd, unsigned short offset, char msgBuffer[], unsigned short n) {
	unsigned short i, j, b;
	char* readBuf = (char*)malloc(n);
	_readcharsFromRegister(cmd, offset, readBuf, n);
	b     = sprintf(msgBuffer, "Reg: 0x%02x, chars: %d\nB: 7 6 5 4 3 2 1 0\n", cmd, n);  // TODO - tpye
	for(i = 0; i < n; i++) {
		char curchar = readBuf[i];
		snprintf(&msgBuffer[b++], 2, "%d", (i+1));
		msgBuffer[b++] = (char)((i+1) & 0xFF);
		msgBuffer[b++] = ':';
		msgBuffer[b++] = ' ';
		for(j = 0; j < 8; j++) {
			msgBuffer[b++] = ((curchar >> (7-j)) & 0x01) ? '1' : '0';
			if(j < 7) {
				msgBuffer[b++] = ' ';
			} else if(i < n-1) {
				msgBuffer[b++] = '\n';
			} else {
				msgBuffer[b++] = '\0';
			}
		}
	}
	msgBuffer[b++] = '\0';
	free(readBuf);
}
#endif

