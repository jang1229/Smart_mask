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
 * @file DW1000Ng.h
 * Arduino driver library (header file) for the Decawave DW1000Ng UWB transceiver Module.
 */

#ifndef DW1000Ng_H
#define DW1000Ng_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h> // For boolean type (using standard bool)
//#include <Arduino.h>
//#include <SPI.h>
#include "DW1000NgConstants.h"
#include "DW1000NgConfiguration.h"
#include "DW1000NgCompileOptions.h"

typedef void (*SentHandlerFunction)(void); 
typedef void (*ErrorHandlerFunction)(void); 
typedef void (*ReceivedHandlerFunction)(void); 
typedef void (*ReceiveFailedHandlerFunction)(void); 
typedef void (*ReceiveTimeoutHandlerFunction)(void); 
typedef void (*ReceiveTimestampAvailableHandlerFunction)(void); 

/** 
Initiates and starts a sessions with a DW1000. If rst is not set or value 0xff, a soft resets (i.e. command
triggered) are used and it is assumed that no reset line is wired.

@param[in] ss  The SPI Selection pin used to identify the specific connection
@param[in] irq The interrupt line/pin that connects the Arduino.
@param[in] rst The reset line/pin for hard resets of ICs that connect to the Arduino. Value 0xff means soft reset.
*/
void DW1000Ng_initialize(unsigned char ss, unsigned char irq, unsigned char rst);

/** 
Initiates and starts a sessions with a DW1000 without interrupt. If rst is not set or value 0xff, a soft resets (i.e. command
triggered) are used and it is assumed that no reset line is wired.

@param[in] ss  The SPI Selection pin used to identify the specific connection
@param[in] rst The reset line/pin for hard resets of ICs that connect to the Arduino. Value 0xff means soft reset.
*/
void DW1000Ng_initializeNoInterrupt(unsigned char ss, unsigned char rst);

/** 
Enable debounce Clock, used to clock the LED blinking
*/
void DW1000Ng_enableDebounceClock();

/**
Enable led blinking feature
*/
void DW1000Ng_enableLedBlinking();

/**
Set DW1000's GPIO pins mode
*/
void DW1000Ng_setGPIOMode(unsigned char msgp, unsigned char mode);

/**
Applies the common sleep configuration and on-wake mode to the DW1000 for both DEEP_SLEEP and SLEEP modes.
ONW_LLDO_BIT and ONW_LLDE_BIT are 1 to default.

@param [in] config struct	The sleep/deepsleep configuration to apply to the DW1000
*/
void DW1000Ng_applySleepConfiguration(sleep_configuration_t sleep_config);

/**
Enter in DeepSleep. applySleepConfiguration must be called first.
Either spi wakeup or pin wakeup must be enabled.
-- In case of future implementation of Sleep mode, you must reset proper antenna DelayMs with setTxAntennaDelay() after wakeUp event. --
*/
void DW1000Ng_deepSleep();

/**
Wake-up from deep sleep by toggle chip select pin
*/
void DW1000Ng_spiWakeup();

/**
Resets all connected or the currently selected DW1000 chip.
Uses hardware reset or in case the reset pin is not wired it falls back to software Reset. 
*/
void DW1000Ng_reset();

/** 
Resets the currently selected DW1000 chip programmatically (via corresponding commands).
*/
void DW1000Ng_softwareReset();

/** 
(Re-)set the network identifier which the selected chip should be associated with. This
setting is important for certain MAC address filtering rules.
This is also referred as PanId

@param[in] val An arbitrary numeric network identifier.
*/
void DW1000Ng_setNetworkId(unsigned short val);

/**
Gets the network identifier (a.k.a PAN id) set for the device

@param[out] id the chars that represent the PAN id (2 chars)
*/
void DW1000Ng_getNetworkId(char id[]);

/** 
(Re-)set the device address (i.e. short address) for the currently selected chip. This
setting is important for certain MAC address filtering rules.

@param[in] val An arbitrary numeric device address.
*/
void DW1000Ng_setDeviceAddress(unsigned short val);

/**
Gets the short address identifier set for the device

@param[out] address the chars that represent the short address of the device(2 chars)
*/
void DW1000Ng_getDeviceAddress(char address[]);

/**
Sets the device Extended Unique Identifier.
This is a long identifier of the device.

@param[in] eui A string containing the eui in its normal notation using columns.
*/
void DW1000Ng_setEUI(char eui[]);

/**
Sets the transmission power of the device.
Be careful to respect your current country limitations.

@param[in] power  chars (written as a 32-bit number) that represent the power
*/

/**
Gets the device Extended Unique Identifier.

@param[out] eui The 8 bytes of the EUI.
*/
void DW1000Ng_getEUI(char eui[]);


void DW1000Ng_setTXPower(int32_t power);

/**
Automatically sets power in respect to the current device settings.
This should be guaranteed to set power under -41.3 dBm / MHz (legal limit in most countries).
*/
void DW1000Ng_setTXPowerAuto();

/**
Sets the pulse generator DelayMs value.
You should use the setTCPGDelayAuto() function.
*/
void DW1000Ng_setTCPGDelay(char tcpg_delay);

/**
Automatically sets pulse generator DelayMs value
*/
void DW1000Ng_setTCPGDelayAuto();

/** 
Enables transmit power spectrum test mode that is used for Transmit Power regulatory testing 

@param [in] repeat_interval the interval to repeat the transmission
*/
void DW1000Ng_enableTransmitPowerSpectrumTestMode(int32_t repeat_interval);

/**
Sets a DelayMs for transmission and receive

@param [in] futureTimechars the timestamp in chars of the time of the transmission (in UWB time)
*/
void DW1000Ng_setDelayedTRX(char futureTimechars[]);

/**
Sets the transmission chars inside the tx buffer of the DW1000

@param [in] data the chars to transmit
@param [in] n the length of the array of chars
*/
void DW1000Ng_setTransmitDataByte(char data[], unsigned short n);

/**
Sets the transmission chars inside the tx buffer of the DW1000 based on the input string

@param [in] data the string to transmit
*/
void DW1000Ng_setTransmitDataString(const char *data);

/**
Gets the received chars and stores them in a char array

@param [out] data The array of char to store the data
@param [out] n The length of the char array
*/
void DW1000Ng_getReceivedData(char data[], unsigned short n);

/**
Calculates the length of the received data

returns the length of the data
*/
unsigned short DW1000Ng_getReceivedDataLength();

/**
Calculates the latest transmission timestamp

return the last transmission timestamp
*/
uint64_t DW1000Ng_getTransmitTimestamp();

/**
Calculates the latest receive timestamp

return the last receive timestamp
*/
uint64_t DW1000Ng_getReceiveTimestamp();

/**
Calculates the current system timestamp inside the DW1000

return the system timestamp
*/
uint64_t DW1000Ng_getSystemTimestamp();

/* receive quality information. (RX_FSQUAL) - reg:0x12 */

/**
Gets the receive power of the device (last receive)

returns the last receive power of the device
*/
float DW1000Ng_getReceivePower();

/**
Gets the power of the first path

returns the first path power
*/ 
float DW1000Ng_getFirstPathPower();

/**
Gets the last receive quality

returns last receive quality
*/
float DW1000Ng_getReceiveQuality();

/**
Sets both tx and rx antenna DelayMs value

@param [in] value the DelayMs in UWB time
*/
void DW1000Ng_setAntennaDelay(unsigned short value);

#if defined(__AVR__)
	/**
	Sets both tx and rx antenna DelayMs value, and saves it in the EEPROM for future use

	@param [in] value the DelayMs in UWB time
	@param [in] the EEPROM offset at which the DelayMs is saved
	*/
	void setAndSaveAntennaDelay(unsigned short DelayMs, unsigned char eeAddress = 0);

	/**
	Gets the saved antenna DelayMs value from EEPROM

	returns the value of the DelayMs saved in the EEPROM in UWB time

	@param [in] the EEPROM offset at which the DelayMs is saved
	*/
	unsigned short getSavedAntennaDelay(unsigned char eeAddress = 0);

	/**
	Sets the saved antenna DelayMs value from EEPROM as the configured DelayMs

	@param [in] the EEPROM offset at which the DelayMs is saved
	*/
	unsigned short setAntennaDelayFromEEPROM(unsigned char eeAddress = 0);
#endif

/**
Sets the tx antenna DelayMs value

@param [in] value the DelayMs in UWB time
*/
void DW1000Ng_setTxAntennaDelay(unsigned short value);

/**
Sets the rx antenna DelayMs value

@param [in] value the DelayMs in UWB time
*/
void DW1000Ng_setRxAntennaDelay(unsigned short value);

/**
Gets the tx antenna DelayMs value

returns the value of the DelayMs in UWB time
*/
unsigned short DW1000Ng_getTxAntennaDelay();

/**
Gets the rx antenna DelayMs value

returns the value of the DelayMs in UWB time
*/
unsigned short DW1000Ng_getRxAntennaDelay();

/**
Sets the function for error event handling

@param [in] handleError the target function
*/
void DW1000Ng_attachErrorHandler(ErrorHandlerFunction handleError);

/**
Sets the function for end of transission event handling

@param [in] handleSent the target function
*/
void DW1000Ng_attachSentHandler(SentHandlerFunction handleSent);

/**
Sets the function for end of receive event handling

@param [in] handleReceived the target function
*/

void DW1000Ng_attachReceivedHandler(ReceivedHandlerFunction handleReceived);

/**
Sets the function for receive error event handling

@param [in] handleReceiveFailed the target function
*/
void DW1000Ng_attachReceiveFailedHandler(ReceiveFailedHandlerFunction handleReceiveFailed);

/**
Sets the function for receive timeout event handling

@param [in] handleReceiveTimeout the target function
*/
void DW1000Ng_attachReceiveTimeoutHandler(ReceiveTimeoutHandlerFunction handleReceiveTimeout);

/**
Sets the function for receive timestamp availabe event handling

@param [in] handleReceiveTimestampAvailable the target function
*/
void DW1000Ng_attachReceiveTimestampAvailableHandler(ReceiveTimestampAvailableHandlerFunction handleReceiveTimestampAvailable);


/**
Handles dw1000 events triggered by interrupt
By default this is attached to the interrupt pin callback
*/
void DW1000Ng_interruptServiceRoutine();

bool DW1000Ng_isTransmitDone();

void DW1000Ng_clearTransmitStatus();

bool DW1000Ng_isReceiveDone();

void DW1000Ng_clearReceiveStatus();

bool DW1000Ng_isReceiveFailed();

void DW1000Ng_clearReceiveFailedStatus();

bool DW1000Ng_isReceiveTimeout();

void DW1000Ng_clearReceiveTimeoutStatus();

/**
Stops the transceiver immediately, this actually sets the device in Idle mode.
*/
void DW1000Ng_forceTRxOff();

/**
Sets the interrupt polarity

By default this is set to true by the DW1000

@param [in] val True here means active high
*/
void DW1000Ng_setInterruptPolarity(bool val);

/**
Applies the target configuration to the DW1000

@param [in] config the configuration to apply to the DW1000
*/
void DW1000Ng_applyConfiguration(device_configuration_t config);

/**
Enables the interrupts for the target events

@param [in] interrupt_config the interrupt map to use
*/
void DW1000Ng_applyInterruptConfiguration(interrupt_configuration_t interrupt_config);

/**
Gets the current channel in use

returns the current channel
*/
Channel DW1000Ng_getChannel();

/**
Gets the current PRF of the device

returns the current PRF
*/
PulseFrequency DW1000Ng_getPulseFrequency();

/**
Sets the timeout for Raceive Frame.

@param[in] Pac size based on current preamble lenght - 1
*/
void DW1000Ng_setPreambleDetectionTimeout(unsigned short pacSize);

/**
Sets the timeout for SFD detection.
The recommended value is: PreambleLenght + SFD + 1.
The default value is 4096+64+1

@param[in] the sfd detection timeout 
*/
void DW1000Ng_setSfdDetectionTimeout(unsigned short preambleSymbols);

/**
Sets the timeout for Raceive Frame. Must be sets in idle mode.
Allow the external microprocessor to enter a low power state awaiting a valid receive frame.

@param[in] time in μs. units = ~1μs(1.026μs). 0 to disable 
*/
void DW1000Ng_setReceiveFrameWaitTimeoutPeriod(unsigned short timeMicroSeconds);

/**
Sets the device in receive mode

@param [in] mode IMMEDIATE or DELAYED receive
*/
void DW1000Ng_startReceive(TransmitMode mode);

/**
Sets the device in transmission mode

@param [in] mode IMMEDIATE or DELAYED transmission
*/
void DW1000Ng_startTransmit(TransmitMode mode);
	
/**
Gets the temperature inside the DW1000 Device

returns The temperature 
*/
float DW1000Ng_getTemperature();

/**
Gets the voltage in input of the DW1000

returns The input voltage
*/
float DW1000Ng_getBatteryVoltage();

/**
Gets both temperature and voltage with a single read

@param [out] temp the temperature
@param [out] vbat the input voltage
*/ 
void DW1000Ng_getTemperatureAndBatteryVoltage(float *temp, float *vbat);

/**
Enables the frame filtering functionality using the provided configuration.
Messages must be formatted using 802.15.4-2011 format.

@param [in] config frame filtering configuration
*/
void DW1000Ng_enableFrameFiltering(frame_filtering_configuration_t config);

/**
Disables the frame filtering functionality
*/
void DW1000Ng_disableFrameFiltering();

/**
WARNING: this just sets the relative bits inside the register.
You must refer to the DW1000 User manual to activate it properly.
*/
void DW1000Ng_setDoubleBuffering(bool val);

/**
Enables frames up to 1023 char length

@param [in] val true or false
*/
void DW1000Ng_useExtendedFrameLength(bool val);

/**
Sets the time before the device enters receive after a transmission.
Use 0 here to deactivate it.

@param[in] time in μs. units = ~1μs(1.026μs)
*/
void DW1000Ng_setWait4Response(uint32_t timeMicroSeconds);

#if DW1000NG_PRINTABLE

/* ##### Print device id, address, etc. ###################################### */
/** 
Generates a String representation of the device identifier of the chip. That usually 
are the letters "DECA" plus the	version and revision numbers of the chip.

@param[out] msgBuffer The String buffer to be filled with printable device information.
	Provide 128 chars, this should be sufficient.
*/
void DW1000Ng_getPrintableDeviceIdentifier(char msgBuffer[]);

/** 
Generates a String representation of the extended unique identifier (EUI) of the chip.

@param[out] msgBuffer The String buffer to be filled with printable device information.
	Provide 128 chars, this should be sufficient.
*/
void DW1000Ng_getPrintableExtendedUniqueIdentifier(char msgBuffer[]);

/** 
Generates a String representation of the short address and network identifier currently
defined for the respective chip.

@param[out] msgBuffer The String buffer to be filled with printable device information.
	Provide 128 chars, this should be sufficient.
*/
void DW1000Ng_getPrintableNetworkIdAndShortAddress(char msgBuffer[]);

/** 
Generates a String representation of the main operational settings of the chip. This
includes data rate, pulse repetition frequency, preamble and channel settings.

@param[out] msgBuffer The String buffer to be filled with printable device information.
	Provide 128 chars, this should be sufficient.
*/
void DW1000Ng_getPrintableDeviceMode(char msgBuffer[]);
#endif

#if DW1000NG_DEBUG
void getPrettychars(char data[], char msgBuffer[], unsigned short n);
void getPrettychars(char cmd, unsigned short offset, char msgBuffer[], unsigned short n);
#endif

void DW1000Ng_set_max_5007();

void _readcharsFromRegister(char cmd, unsigned short offset, char data[], unsigned short data_size);

#endif