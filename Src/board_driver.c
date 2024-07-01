/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "main.h"
#include "board_driver.h"
#include "cmsis_os.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported variables ---------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Exported functions ---------------------------------------------------------*/

ExtIrqHandler _handleExtIrq = NULL;

void DW1000Ng_attachExtIrq(ExtIrqHandler handleExtIrq) {
	_handleExtIrq = handleExtIrq;
}

void GPIO_ModeSwitch(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint32_t GPIO_Mode)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_Mode;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    
    HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void MY_SPI1_Init(u32 baud)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = baud;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}


void HAL_GPIO_EXTI_Callback(u16 pin)
{
	if(pin == GPIO_PIN_4)
	{
		if(_handleExtIrq != NULL)
		{
			_handleExtIrq();
		}
	}
   
}



unsigned char spi_readWrite_byte(unsigned char txData)
{
	unsigned char rxData;

	HAL_SPI_TransmitReceive(&hspi1,(uint8_t *)&txData, (uint8_t *)&rxData, 1, 10);

	return rxData;
}

void DelayMs(unsigned int Ms)
{
	DelayUs(Ms*1000);
}

void DelayUs(unsigned int Us)
{
	volatile unsigned int delay;

	delay = Us*8;
	while(delay--);
}

u32 GetTick(void)
{
	return RunningTime;
}

