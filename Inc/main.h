/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LCM_LED1_Pin GPIO_PIN_3
#define LCM_LED1_GPIO_Port GPIOE
#define LCM_LED2_Pin GPIO_PIN_4
#define LCM_LED2_GPIO_Port GPIOE
#define SW_PWR_ONOFF_Pin GPIO_PIN_5
#define SW_PWR_ONOFF_GPIO_Port GPIOE
#define Rotary_A_Pin GPIO_PIN_1
#define Rotary_A_GPIO_Port GPIOC
#define Rotary_B_Pin GPIO_PIN_2
#define Rotary_B_GPIO_Port GPIOC
#define Rotary_P_Pin GPIO_PIN_3
#define Rotary_P_GPIO_Port GPIOC
#define SW_SEL1_Pin GPIO_PIN_1
#define SW_SEL1_GPIO_Port GPIOA
#define SW_SEL2_Pin GPIO_PIN_2
#define SW_SEL2_GPIO_Port GPIOA
#define DW1000_CS_Pin GPIO_PIN_4
#define DW1000_CS_GPIO_Port GPIOA
#define DW1000_SCK_Pin GPIO_PIN_5
#define DW1000_SCK_GPIO_Port GPIOA
#define DW1000_MISO_Pin GPIO_PIN_6
#define DW1000_MISO_GPIO_Port GPIOA
#define DW1000_MOSI_Pin GPIO_PIN_7
#define DW1000_MOSI_GPIO_Port GPIOA
#define DW1000_WUP_Pin GPIO_PIN_5
#define DW1000_WUP_GPIO_Port GPIOC
#define DW1000_RSTn_Pin GPIO_PIN_0
#define DW1000_RSTn_GPIO_Port GPIOB
#define SD_SS_Pin GPIO_PIN_12
#define SD_SS_GPIO_Port GPIOB
#define SD_SCK_Pin GPIO_PIN_13
#define SD_SCK_GPIO_Port GPIOB
#define SD_MISO_Pin GPIO_PIN_14
#define SD_MISO_GPIO_Port GPIOB
#define SD_MOSI_Pin GPIO_PIN_15
#define SD_MOSI_GPIO_Port GPIOB
#define LCD_RST_Pin GPIO_PIN_10
#define LCD_RST_GPIO_Port GPIOD
#define RF_EN_Pin GPIO_PIN_7
#define RF_EN_GPIO_Port GPIOC
#define UWB_EN_Pin GPIO_PIN_8
#define UWB_EN_GPIO_Port GPIOC
#define PWR_ON_Pin GPIO_PIN_9
#define PWR_ON_GPIO_Port GPIOC
#define RF_RST_Pin GPIO_PIN_8
#define RF_RST_GPIO_Port GPIOA
#define IMU_INT_Pin GPIO_PIN_5
#define IMU_INT_GPIO_Port GPIOB
#define IMU_ADD_SEL_Pin GPIO_PIN_8
#define IMU_ADD_SEL_GPIO_Port GPIOB
#define IMU_RST_Pin GPIO_PIN_9
#define IMU_RST_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */



#define	_LOW_		0
#define	_HIGH_		1

#define	_OFF_		GPIO_PIN_RESET
#define	_ON_		GPIO_PIN_SET

#define	_OFF		GPIO_PIN_RESET
#define	_ON		  GPIO_PIN_SET

#define	OFF		GPIO_PIN_RESET
#define	ON  	GPIO_PIN_SET

#define	_DIS_		0
#define	_EN_		1

#define	_FALSE_		0
#define	_TRUE_		1

#define	Bit_SET			GPIO_PIN_SET
#define	Bit_RESET		GPIO_PIN_RESET

#define	RESET		0
#define	SET		1

#define	s8	char 	
#define s16 short	
#define	s32 int 	

#define	u8	unsigned char 	
#define u16 unsigned short	
#define	u32 unsigned int 

#define LCD_INTERVAL 1000

#define MASTER_ID 120

enum LCD_MODE
{
	UWB=0,
	RF,
	BOTH
};

enum UWB_Logic
{
  UWB_IDLE = 0,
  UWB_FAR,
  UWB_NEAR
};

enum RF_Logic
{
  RF_IDLE = 0,
  RF_STOP,
  RF_MOVE
};

#define B00		0
#define B01		1
#define B10		2
#define B11		3

#define in_RotaryA()				HAL_GPIO_ReadPin(Rotary_A_GPIO_Port,Rotary_A_Pin);
#define in_RotaryB()				HAL_GPIO_ReadPin(Rotary_B_GPIO_Port,Rotary_B_Pin);
#define in_RotaryP				HAL_GPIO_ReadPin(Rotary_P_GPIO_Port,Rotary_P_Pin);

#define out_LCM_LED1(x)         HAL_GPIO_WritePin(LCM_LED1_GPIO_Port, LCM_LED1_Pin, x);
#define out_LCM_LED2(x)         HAL_GPIO_WritePin(LCM_LED2_GPIO_Port, LCM_LED2_Pin, x);

//Main Power ON/OFF
#define out_PWR_OnOff(x)           HAL_GPIO_WritePin(PWR_ON_GPIO_Port, PWR_ON_Pin, x);

#define in_PWR_OnOff()              HAL_GPIO_ReadPin(SW_PWR_ONOFF_GPIO_Port,SW_PWR_ONOFF_Pin);

//MAX5007 FET ON/OFF
#define out_UWB_EN(x)           HAL_GPIO_WritePin(UWB_EN_GPIO_Port, UWB_EN_Pin, x);

#define out_SD_SS(x)           HAL_GPIO_WritePin(SD_SS_GPIO_Port, SD_SS_Pin, x);

#define out_LCD_RST(x)         HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, x);

//BNO055
#define out_IMU_RST(x)          HAL_GPIO_WritePin(IMU_RST_GPIO_Port, IMU_RST_Pin, x);
#define out_IMU_ADD_SEL(x)      HAL_GPIO_WritePin(IMU_ADD_SEL_GPIO_Port, IMU_ADD_SEL_Pin, x);
#define in_IMU_INT              HAL_GPIO_ReadPin(IMU_INT_GPIO_Port, IMU_INT_Pin);

//RYLR998
#define out_RF_RST(x)           HAL_GPIO_WritePin(RF_RST_GPIO_Port, RF_RST_Pin, x);
#define out_RF_EN(x)            HAL_GPIO_WritePin(RF_EN_GPIO_Port, RF_EN_Pin, x);

//MAX5007 //DWM1000
#define out_DW1000_RSTn(x)         HAL_GPIO_WritePin(DW1000_RSTn_GPIO_Port, DW1000_RSTn_Pin, x);
#define out_DW1000_WUP(x)          HAL_GPIO_WritePin(DW1000_WUP_GPIO_Port, DW1000_WUP_Pin, x);
#define out_DW1000_CS(x)          HAL_GPIO_WritePin(DW1000_CS_GPIO_Port, DW1000_CS_Pin, x);


#define in_SW_SEL1()              HAL_GPIO_ReadPin(SW_SEL1_GPIO_Port,SW_SEL1_Pin);
#define in_SW_SEL2()              HAL_GPIO_ReadPin(SW_SEL2_GPIO_Port,SW_SEL2_Pin);


extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

extern I2C_HandleTypeDef hi2c1;

extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;

extern UART_HandleTypeDef huart1;

extern SRAM_HandleTypeDef hsram1;

extern SRAM_HandleTypeDef hsram1;

extern u32 RunningTime;

extern unsigned int adc_buf[1];

extern float BatVoltag;
extern unsigned char Cur_BAT;
extern unsigned char BAT_chkflag;


extern u8 selectMode;

typedef struct _Global_ 
{
  u8 master_id;
  
  //rotary SW phase
  char CurDistancePhase;
  char PreDistancePhase;
  
  //distance_setting value //Rotary Value
  unsigned char Distance_UWB;
  unsigned char PreDistance_UWB;  

  float  Distance_RF;
  float  PreDistance_RF;  

  //push SW value
  char Pre_SW1;
  char Cur_SW1;
  
  char Pre_SW2;
  char Cur_SW2;

  char Pre_SW_PWR;
  char Cur_SW_PWR;
  
	u8 SW_PWR_RisingEdg;
	u8 SW_PWR_FallingEdg;
	u32 SW_PWR_RisingTime;
  
  u8 PWR_flag;

  u8 lcdFlag;
  u32 lcdTime;

  //LCD 모드 Flag
  u8 LCD_UWB_MODE;
  u8 LCD_RF_MODE;

  
  u8 UWB_Set_Value[10];
  u8 UWB_User_Value[10];
  
  u8 device_ID[20];
  double device_Distance_uwb[20];
  double device_PreDistance_uwb[20];
  

  
  u8 RF_Set_Value[10];
  u8 RF_User_Value[10];
  u8 RF_Movement_Value[20];

  u8 RF_PreMovement[20];
  u8 RF_Movement[20];
  u8 RF_MoveChk[20];
  double device_Distance_RF[20];

  u8 RF_userCnt;
  u8 RF_stopCnt;
  u8 RF_moveCnt;
  
  u8 RF_moveflag;
  
  
} GLOBAL;

extern GLOBAL Global;




/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
