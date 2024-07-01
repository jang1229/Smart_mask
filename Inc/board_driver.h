/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BOARD_DRIVER_H_
#define __BOARD_DRIVER_H_

/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* Exported types ------------------------------------------------------------*/
typedef void (*ExtIrqHandler)(void); 
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void DelayMs(unsigned int Ms);
void DelayUs(unsigned int Us);
void MY_SPI1_Init(u32 baud);
unsigned char spi_readWrite_byte(unsigned char txData);
void DW1000Ng_attachExtIrq(ExtIrqHandler handleExtIrq);
void GPIO_ModeSwitch(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint32_t GPIO_Mode);
u32 GetTick(void);
#endif