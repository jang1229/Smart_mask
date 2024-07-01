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
 * 
 * @file SPIporting.hpp
 * Arduino porting for the SPI interface.
*/

//#include <Arduino.h>
//#include <SPI.h>
#include "SPIporting.h"
#include "DW1000NgConstants.h"
#include "DW1000NgRegisters.h"
#include "board_driver.h"
#include "main.h"

void SPIporting_SPIinit() {
	
}

void SPIporting_SPIend() {
}

void SPIporting_SPIselect() {
	
}

//unsigned char spi_readWrite_byte(unsigned char txData);

void SPIporting_writeToSPI(unsigned char slaveSelectPIN, unsigned char headerLen, char header[], unsigned short dataLen, char data[]) {
	
	// _openSPI(slaveSelectPIN);
	// for(int i = 0; i < headerLen; i++) {
	// 	_spi->transfer(header[i]); // send header
	// }
	// for(int i = 0; i < dataLen; i++) {
	// 	_spi->transfer(data[i]); // write values
	// }
	// delayMicroseconds(5);
	// _closeSPI(slaveSelectPIN);
	
	out_DW1000_CS(_OFF);
	for(int i = 0; i < headerLen; i++) {
		spi_readWrite_byte((char)header[i]);
	}
	for(int i = 0; i < dataLen; i++) {
		spi_readWrite_byte((char)data[i]);
	}
	DelayUs(5);
	out_DW1000_CS(_ON);
}

void SPIporting_readFromSPI(unsigned char slaveSelectPIN, unsigned char headerLen, char header[], unsigned short dataLen, char data[]){
	// _openSPI(slaveSelectPIN);
	// for(int i = 0; i < headerLen; i++) {
	// 	_spi->transfer(header[i]); // send header
	// }
	// for(int i = 0; i < dataLen; i++) {
	// 	data[i] = _spi->transfer(0x00); // read values
	// }
	// delayMicroseconds(5);
	// _closeSPI(slaveSelectPIN);

	out_DW1000_CS(_OFF);
	for(int i = 0; i < headerLen; i++) {
		spi_readWrite_byte((char)header[i]);
	}
	for(int i = 0; i < dataLen; i++) {
		data[i] = spi_readWrite_byte(0x00);
	}
	DelayUs(5);
	out_DW1000_CS(_ON);
}

void SPIporting_setSPIspeed(SPIClock speed) {
	if(speed == FAST) 
	{
		MY_SPI1_Init(SPI_BAUDRATEPRESCALER_8);
	} 
	else if(speed == SLOW) 
	{
		MY_SPI1_Init(SPI_BAUDRATEPRESCALER_32);
	}
}
