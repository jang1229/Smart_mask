#include "main.h"
#include "cmsis_os.h"

#include "RF_Task.h"
#include "Control.h"
#include <stdio.h>
#include <string.h>
#include "RF_logic.h"

u8 Uart1RcvData;
RF_Control	ProRF;



void putChar1(unsigned char c)
{
  HAL_NVIC_DisableIRQ(USART1_IRQn);
  HAL_UART_Transmit(&huart1, &c, 1, 10);
  HAL_NVIC_EnableIRQ(USART1_IRQn);
}

void printfs1(char *s)
{
  unsigned int cnt=0;
  
  while(1)
  {
    if(*(s+cnt) == 0x00)   	return;
    else               		putChar1(*(s+cnt));
    cnt++;
  }
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) 
{   
  if(huart->Instance == USART1)
    UART1_Rx_Callback();
  
}


void UART1_Rx_Callback(void)
{
  HAL_UART_Receive_IT(&huart1, &Uart1RcvData, 1);
  RF_PROTOCOL(Uart1RcvData);
}

void RF_PROTOCOL(unsigned char Data)
{
  ProRF.CmdBuffer[ProRF.CmdBufferCnt++] = Data;
  if(ProRF.CmdBufferCnt > 150)
    ProRF.CmdBufferCnt = 0;
  
  if((ProRF.Start_Flag == 0) && ((Data == '+'))) // STX
  {
    ProRF.Start_Flag = 1;
    ProRF.BufferCnt = 0;
  }
  
  if(((ProRF.Buffer[ProRF.BufferCnt-1] == 0x0d) && (Data == 0x0A) || (Data == '#')) && ProRF.Start_Flag) // ETX
  {
    ProRF.Start_Flag = 0;
    ProRF.Data_UpDate_Flag = 1;
  }	
  
  if(ProRF.Start_Flag)
  {
    ProRF.Buffer[ProRF.BufferCnt] = Data;
    ProRF.BufferCnt++;
  }
}


int SendCommand(char *command, char *reply, uint16_t delay)
{
  HAL_UART_Transmit_IT(&huart1, (unsigned char *)command,
                       (uint16_t)strlen(command));
  
  
  //HAL_Delay(delay);
  osDelay(delay);

  if (strstr((char const*)ProRF.CmdBuffer, reply) != NULL)
  {
    memset(ProRF.CmdBuffer, 0, sizeof(ProRF.CmdBuffer));
    ProRF.CmdBufferCnt = 0;
    return 0;
  }
  memset(ProRF.CmdBuffer, 0, sizeof(ProRF.CmdBuffer));
  
  ProRF.CmdBufferCnt = 0;
  return 1;
}
void RF_Init(void)
{
    out_RF_EN(_ON_);
    out_RF_RST(Bit_RESET);
    out_RF_RST(Bit_SET);
    SendCommand("AT\r\n","",200);

	  Global.master_id = MASTER_ID;
	  memset(&ProRF.RF_TXBuffer, 0, sizeof(ProRF.RF_TXBuffer));
	  sprintf((char*)ProRF.RF_TXBuffer,"AT+ADDRESS=%d,\r\n",Global.master_id);
	  SendCommand((char*)ProRF.RF_TXBuffer,"",200);
    
	  memset(&ProRF.RF_TXBuffer, 0, sizeof(ProRF.RF_TXBuffer));
	  sprintf((char*)ProRF.RF_TXBuffer,"AT+BAND=921900000,M,\r\n");
	  SendCommand((char*)ProRF.RF_TXBuffer,"",200);

    out_RF_EN(_OFF_);
}


void RF_Task(void *argument)
{
    RF_Init();
  
		for(;;)
		{
			if(selectMode == RF)
			{
              RF_Cmd_Task();
              Data_Send_Task();
              osDelay(500);
			}
		}	
 
}