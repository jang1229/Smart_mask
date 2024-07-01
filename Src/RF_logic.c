#include "RF_logic.h"
#include "main.h"
#include "RF_Task.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#define PATH_LOSS_EXPONENT 2.0 //일반적으로 쓰는 손실 지수 (상수)
#define REF_RSSI    -20  // 1M 거리기준 모듈에서 들어오는 RSSI값 

// 거리 계산 함수
double calculateDistance(double referenceRSSI, double measuredRSSI) 
{
    return pow(10, ((referenceRSSI - measuredRSSI) / (10 * PATH_LOSS_EXPONENT)));
}

//RF Send cmd  순차적으로 RF데이터 요청 
void RF_Cmd_Task(void)
{
  switch(ProRF.RF_Case)
  { 
		case 0 :	ProRF.RF_CmdFlag = SendCommand("AT+SEND=0,5,HELLO\r\n", "", 200);
		if(ProRF.RF_CmdFlag == 0)
			ProRF.RF_Case++;
		break;										
		
		case 1 :	ProRF.RF_CmdFlag = SendCommand("AT+SEND=1,5,HELLO\r\n", "", 200);
		if(ProRF.RF_CmdFlag == 0)
			ProRF.RF_Case++;
		break;	

    case 2 :	ProRF.RF_CmdFlag = SendCommand("AT+SEND=2,5,HELLO\r\n", "", 200);
		if(ProRF.RF_CmdFlag == 0)
			ProRF.RF_Case++;
		break;	

    case 3 :	ProRF.RF_CmdFlag = SendCommand("AT+SEND=3,5,HELLO\r\n", "", 200);
		if(ProRF.RF_CmdFlag == 0)
			ProRF.RF_Case++;
		break;										
		
		case 4 :	ProRF.RF_CmdFlag = SendCommand("AT+SEND=4,5,HELLO\r\n", "", 200);
		if(ProRF.RF_CmdFlag == 0)
			ProRF.RF_Case++;
    break;

    case 5 :	ProRF.RF_CmdFlag = SendCommand("AT+SEND=5,5,HELLO\r\n", "", 200);
		if(ProRF.RF_CmdFlag == 0)
			ProRF.RF_Case++;
		break;										
		
		case 6 :	ProRF.RF_CmdFlag = SendCommand("AT+SEND=6,5,HELLO\r\n", "", 200);
		if(ProRF.RF_CmdFlag == 0)
			ProRF.RF_Case++;
    break;
    
    case 7 :	ProRF.RF_CmdFlag = SendCommand("AT+SEND=7,5,HELLO\r\n", "", 200);
		if(ProRF.RF_CmdFlag == 0)
			ProRF.RF_Case++;
		break;										
		
		case 8 :	ProRF.RF_CmdFlag = SendCommand("AT+SEND=8,5,HELLO\r\n", "", 200);
		if(ProRF.RF_CmdFlag == 0)
			ProRF.RF_Case++;
    break;

    case 9 :	ProRF.RF_CmdFlag = SendCommand("AT+SEND=9,5,HELLO\r\n", "", 200);
		if(ProRF.RF_CmdFlag == 0)
			ProRF.RF_Case++;
		break;										
		
		case 10 :	ProRF.RF_CmdFlag = SendCommand("AT+SEND=10,5,HELLO\r\n", "", 200);
		if(ProRF.RF_CmdFlag == 0)
			ProRF.RF_Case++;
		break;	

    case 11 :	ProRF.RF_CmdFlag = SendCommand("AT+SEND=11,5,HELLO\r\n", "", 200);
		if(ProRF.RF_CmdFlag == 0)
			ProRF.RF_Case++;
		break;										
		
		case 12 :	ProRF.RF_CmdFlag = SendCommand("AT+SEND=12,5,HELLO\r\n", "", 200);
		if(ProRF.RF_CmdFlag == 0)
			ProRF.RF_Case++;
    break;

    case 13 :	ProRF.RF_CmdFlag = SendCommand("AT+SEND=13,5,HELLO\r\n", "", 200);
		if(ProRF.RF_CmdFlag == 0)
			ProRF.RF_Case++;
		break;										
		
		case 14 :	ProRF.RF_CmdFlag = SendCommand("AT+SEND=14,5,HELLO\r\n", "", 200);
		if(ProRF.RF_CmdFlag == 0)
			ProRF.RF_Case++;
    break;
    
    case 15 :	ProRF.RF_CmdFlag = SendCommand("AT+SEND=15,5,HELLO\r\n", "", 200);
		if(ProRF.RF_CmdFlag == 0)
			ProRF.RF_Case++;
		break;										
		
		case 16 :	ProRF.RF_CmdFlag = SendCommand("AT+SEND=16,5,HELLO\r\n", "", 200);
		if(ProRF.RF_CmdFlag == 0)
			ProRF.RF_Case++;
    break;
      
    case 17 :	ProRF.RF_CmdFlag = SendCommand("AT+SEND=17,5,HELLO\r\n", "", 200);
		if(ProRF.RF_CmdFlag == 0)
			ProRF.RF_Case++;
		break;										
		
		case 18 :	ProRF.RF_CmdFlag = SendCommand("AT+SEND=18,5,HELLO\r\n", "", 200);
		if(ProRF.RF_CmdFlag == 0)
			ProRF.RF_Case++;
    break;  									
		
		case 19 :	ProRF.RF_CmdFlag = SendCommand("AT+SEND=19,5,HELLO\r\n", "", 200);
		if(ProRF.RF_CmdFlag == 0)
			ProRF.RF_Case = 0;
    break;
         
  
  }
  
}


//RF Recive 받은 데이터 프로토콜에 맞게 수행하는 로직 
void Data_Send_Task(void)
{
    if(!strncmp((const char*)&ProRF.Buffer[4],"=0,", 3)) //id
    {
        if(!strncmp((const char*)&ProRF.Buffer[9],"0", 1)) //stop
        {
            Global.RF_Movement[0] = RF_STOP;
            
        }
        else if(!strncmp((const char*)&ProRF.Buffer[9],"1", 1))//move
        {
            Global.RF_Movement[0] = RF_MOVE;

        } 
        for(u8 i = 0; i < 3; i++)
        {
            ProRF.scan_distance[0][i] = ProRF.Buffer[11+i];
        }
        sscanf((const char*)ProRF.scan_distance[0],"%f",&ProRF.floatValue);
        Global.device_Distance_RF[0] = calculateDistance(REF_RSSI,ProRF.floatValue);

        memset(ProRF.Buffer, 0, sizeof(ProRF.Buffer));		
        Global.RF_MoveChk[0] = 0;		
    }
    else
    {
        Global.RF_MoveChk[0]++;
    }

    if(!strncmp((const char*)&ProRF.Buffer[4],"=1,", 3)) //id
    {
        if(!strncmp((const char*)&ProRF.Buffer[9],"0", 1)) //stop
        {
            Global.RF_Movement[1] = RF_STOP;

        }
        else if(!strncmp((const char*)&ProRF.Buffer[9],"1", 1))//move
        {
            Global.RF_Movement[1] = RF_MOVE;

        }
        
        for(u8 i = 0; i < 3; i++)
        {
            ProRF.scan_distance[1][i] = ProRF.Buffer[11+i];
        }
        sscanf((const char*)ProRF.scan_distance[1],"%f",&ProRF.floatValue);
        Global.device_Distance_RF[1] = calculateDistance(REF_RSSI,ProRF.floatValue);

        memset(ProRF.Buffer, 0, sizeof(ProRF.Buffer));		
        Global.RF_MoveChk[1] = 0;
    }
    else
    {
        Global.RF_MoveChk[1]++;
    }
    
    if(!strncmp((const char*)&ProRF.Buffer[4],"=2,", 3)) //id
    {
        if(!strncmp((const char*)&ProRF.Buffer[9],"0", 1)) //stop
        {
            Global.RF_Movement[2] = RF_STOP;

        }
        else if(!strncmp((const char*)&ProRF.Buffer[9],"1", 1))//move
        {
            Global.RF_Movement[2] = RF_MOVE;

        }
        
        for(u8 i = 0; i < 3; i++)
        {
            ProRF.scan_distance[2][i] = ProRF.Buffer[11+i];
        }
        sscanf((const char*)ProRF.scan_distance[2],"%f",&ProRF.floatValue);
        Global.device_Distance_RF[2] = calculateDistance(REF_RSSI,ProRF.floatValue);

        memset(ProRF.Buffer, 0, sizeof(ProRF.Buffer));		
        Global.RF_MoveChk[2] = 0;
    }
    else
    {
        Global.RF_MoveChk[2]++;
    }

    if(!strncmp((const char*)&ProRF.Buffer[4],"=3,", 3)) //id
    {
        if(!strncmp((const char*)&ProRF.Buffer[9],"0", 1)) //stop
        {
            Global.RF_Movement[3] = RF_STOP;

        }
        else if(!strncmp((const char*)&ProRF.Buffer[9],"1", 1))//move
        {
            Global.RF_Movement[3] = RF_MOVE;

        }        
        for(u8 i = 0; i < 3; i++)
        {
            ProRF.scan_distance[3][i] = ProRF.Buffer[11+i];
        }
        sscanf((const char*)ProRF.scan_distance[3],"%f",&ProRF.floatValue);
        Global.device_Distance_RF[3] = calculateDistance(REF_RSSI,ProRF.floatValue);

        memset(ProRF.Buffer, 0, sizeof(ProRF.Buffer));		
        Global.RF_MoveChk[3] = 0;
    }
    else
    {
        Global.RF_MoveChk[3]++;
    }

    if(!strncmp((const char*)&ProRF.Buffer[4],"=4,", 3)) //id
    {
        if(!strncmp((const char*)&ProRF.Buffer[9],"0", 1)) //stop
        {
            Global.RF_Movement[4] = RF_STOP;

        }
        else if(!strncmp((const char*)&ProRF.Buffer[9],"1", 1))//move
        {
            Global.RF_Movement[4] = RF_MOVE;

        }
        
        for(u8 i = 0; i < 3; i++)
        {
            ProRF.scan_distance[4][i] = ProRF.Buffer[11+i];
        }
        sscanf((const char*)ProRF.scan_distance[4],"%f",&ProRF.floatValue);
        Global.device_Distance_RF[4] = calculateDistance(REF_RSSI,ProRF.floatValue);

        memset(ProRF.Buffer, 0, sizeof(ProRF.Buffer));		
        Global.RF_MoveChk[4] = 0;
    }
    else
    {
        Global.RF_MoveChk[4]++;
    }

    if(!strncmp((const char*)&ProRF.Buffer[4],"=5,", 3)) //id
    {
        if(!strncmp((const char*)&ProRF.Buffer[9],"0", 1)) //stop
        {
            Global.RF_Movement[5] = RF_STOP;

        }
        else if(!strncmp((const char*)&ProRF.Buffer[9],"1", 1))//move
        {
            Global.RF_Movement[5] = RF_MOVE;

        }        
        for(u8 i = 0; i < 3; i++)
        {
            ProRF.scan_distance[5][i] = ProRF.Buffer[11+i];
        }
        sscanf((const char*)ProRF.scan_distance[5],"%f",&ProRF.floatValue);
        Global.device_Distance_RF[5] = calculateDistance(REF_RSSI,ProRF.floatValue);

        memset(ProRF.Buffer, 0, sizeof(ProRF.Buffer));	
        Global.RF_MoveChk[5] = 0;	
    }
    else
    {
        Global.RF_MoveChk[5]++;
    }
    
    if(!strncmp((const char*)&ProRF.Buffer[4],"=6,", 3)) //id
    {
        if(!strncmp((const char*)&ProRF.Buffer[9],"0", 1)) //stop
        {
            Global.RF_Movement[6] = RF_STOP;

        }
        else if(!strncmp((const char*)&ProRF.Buffer[9],"1", 1))//move
        {
            Global.RF_Movement[6] = RF_MOVE;

        }        
        for(u8 i = 0; i < 3; i++)
        {
            ProRF.scan_distance[6][i] = ProRF.Buffer[11+i];
        }
        sscanf((const char*)ProRF.scan_distance[6],"%f",&ProRF.floatValue);
        Global.device_Distance_RF[6] = calculateDistance(REF_RSSI,ProRF.floatValue);

        memset(ProRF.Buffer, 0, sizeof(ProRF.Buffer));	
        Global.RF_MoveChk[6] = 0;	
    }
    else
    {
        Global.RF_MoveChk[6]++;
    }

    if(!strncmp((const char*)&ProRF.Buffer[4],"=7,", 3)) //id
    {
        if(!strncmp((const char*)&ProRF.Buffer[9],"0", 1)) //stop
        {
            Global.RF_Movement[7] = RF_STOP;

        }
        else if(!strncmp((const char*)&ProRF.Buffer[9],"1", 1))//move
        {
            Global.RF_Movement[7] = RF_MOVE;

        }
        
        for(u8 i = 0; i < 3; i++)
        {
            ProRF.scan_distance[7][i] = ProRF.Buffer[11+i];
        }
        sscanf((const char*)ProRF.scan_distance[7],"%f",&ProRF.floatValue);
        Global.device_Distance_RF[7] = calculateDistance(REF_RSSI,ProRF.floatValue);
        
        memset(ProRF.Buffer, 0, sizeof(ProRF.Buffer));	
        Global.RF_MoveChk[7] = 0;
    
    }
    else
    {
        Global.RF_MoveChk[7]++;
    }

    if(!strncmp((const char*)&ProRF.Buffer[4],"=8,", 3)) //id
    {
        if(!strncmp((const char*)&ProRF.Buffer[9],"0", 1)) //stop
        {
            Global.RF_Movement[8] = RF_STOP;

        }
        else if(!strncmp((const char*)&ProRF.Buffer[9],"1", 1))//move
        {
            Global.RF_Movement[8] = RF_MOVE;

        }
        for(u8 i = 0; i < 3; i++)
        {
            ProRF.scan_distance[8][i] = ProRF.Buffer[11+i];
        }
        sscanf((const char*)ProRF.scan_distance[8],"%f",&ProRF.floatValue);
        Global.device_Distance_RF[8] = calculateDistance(REF_RSSI,ProRF.floatValue);

        memset(ProRF.Buffer, 0, sizeof(ProRF.Buffer));		
        Global.RF_MoveChk[8] = 0;
    }
    else
    {
        Global.RF_MoveChk[8]++;
    }

    if(!strncmp((const char*)&ProRF.Buffer[4],"=9,", 3)) //id
    {
        if(!strncmp((const char*)&ProRF.Buffer[9],"0", 1)) //stop
        {
            Global.RF_Movement[9] = RF_STOP;

        }
        else if(!strncmp((const char*)&ProRF.Buffer[9],"1", 1))//move
        {
            Global.RF_Movement[9] = RF_MOVE;

        }
        for(u8 i = 0; i < 3; i++)
        {
            ProRF.scan_distance[9][i] = ProRF.Buffer[11+i];
        }
        sscanf((const char*)ProRF.scan_distance[9],"%f",&ProRF.floatValue);
        Global.device_Distance_RF[9] = calculateDistance(REF_RSSI,ProRF.floatValue);

        memset(ProRF.Buffer, 0, sizeof(ProRF.Buffer));	
        Global.RF_MoveChk[9] = 0;	
    }
    else
    {
        Global.RF_MoveChk[9]++;
    }

    if(!strncmp((const char*)&ProRF.Buffer[4],"=10,", 4)) //id
        {
        if(!strncmp((const char*)&ProRF.Buffer[10],"0", 1)) //stop
        {
            Global.RF_Movement[10] = RF_STOP;

        }
        else if(!strncmp((const char*)&ProRF.Buffer[10],"1", 1))//move
        {
            Global.RF_Movement[10] = RF_MOVE;

        }
        for(u8 i = 0; i < 3; i++)
        {
            ProRF.scan_distance[10][i] = ProRF.Buffer[12+i];
        }
        sscanf((const char*)ProRF.scan_distance[10],"%f",&ProRF.floatValue);
        Global.device_Distance_RF[10] = calculateDistance(REF_RSSI,ProRF.floatValue);

        memset(ProRF.Buffer, 0, sizeof(ProRF.Buffer));		
        Global.RF_MoveChk[10] = 0;
        }
    else
    {
        Global.RF_MoveChk[10]++;
    }

    if(!strncmp((const char*)&ProRF.Buffer[4],"=11,", 4)) //id
    {
        if(!strncmp((const char*)&ProRF.Buffer[10],"0", 1)) //stop
        {
            Global.RF_Movement[11] = RF_STOP;

        }
        else if(!strncmp((const char*)&ProRF.Buffer[10],"1", 1))//move
        {
            Global.RF_Movement[11] = RF_MOVE;

        }
        for(u8 i = 0; i < 3; i++)
        {
            ProRF.scan_distance[11][i] = ProRF.Buffer[12+i];
        }
        sscanf((const char*)ProRF.scan_distance[11],"%f",&ProRF.floatValue);
        Global.device_Distance_RF[11] = calculateDistance(REF_RSSI,ProRF.floatValue);
        
        memset(ProRF.Buffer, 0, sizeof(ProRF.Buffer));
        Global.RF_MoveChk[11] = 0;		
    }
    else
    {
        Global.RF_MoveChk[11]++;
    }

    if(!strncmp((const char*)&ProRF.Buffer[4],"=12,", 4)) //id
    {
        if(!strncmp((const char*)&ProRF.Buffer[10],"0", 1)) //stop
        {
            Global.RF_Movement[12] = RF_STOP;

        }
        else if(!strncmp((const char*)&ProRF.Buffer[10],"1", 1))//move
        {
            Global.RF_Movement[12] = RF_MOVE;

        }
        
        for(u8 i = 0; i < 3; i++)
        {
            ProRF.scan_distance[12][i] = ProRF.Buffer[12+i];
        }
        sscanf((const char*)ProRF.scan_distance[12],"%f",&ProRF.floatValue);
        Global.device_Distance_RF[12] = calculateDistance(REF_RSSI,ProRF.floatValue);

        memset(ProRF.Buffer, 0, sizeof(ProRF.Buffer));		
        Global.RF_MoveChk[12] = 0;
    }
    else
    {
        Global.RF_MoveChk[12]++;
    }

    if(!strncmp((const char*)&ProRF.Buffer[4],"=13,", 4)) //id
    {
        if(!strncmp((const char*)&ProRF.Buffer[10],"0", 1)) //stop
        {
            Global.RF_Movement[13] = RF_STOP;

        }
        else if(!strncmp((const char*)&ProRF.Buffer[10],"1", 1))//move
        {
            Global.RF_Movement[13] = RF_MOVE;

        }        
        for(u8 i = 0; i < 3; i++)
        {
            ProRF.scan_distance[13][i] = ProRF.Buffer[12+i];
        }
        sscanf((const char*)ProRF.scan_distance[13],"%f",&ProRF.floatValue);
        Global.device_Distance_RF[13] = calculateDistance(REF_RSSI,ProRF.floatValue);

        memset(ProRF.Buffer, 0, sizeof(ProRF.Buffer));		
        Global.RF_MoveChk[13] = 0;
    }
    else
    {
        Global.RF_MoveChk[13]++;
    }
    
    if(!strncmp((const char*)&ProRF.Buffer[4],"=14,", 4)) //id
    {
        if(!strncmp((const char*)&ProRF.Buffer[10],"0", 1)) //stop
        {
            Global.RF_Movement[14] = RF_STOP;

        }
        else if(!strncmp((const char*)&ProRF.Buffer[10],"1", 1))//move
        {
            Global.RF_Movement[14] = RF_MOVE;

        }
        
        for(u8 i = 0; i < 3; i++)
        {
            ProRF.scan_distance[14][i] = ProRF.Buffer[12+i];
        }
        sscanf((const char*)ProRF.scan_distance[14],"%f",&ProRF.floatValue);
        Global.device_Distance_RF[14] = calculateDistance(REF_RSSI,ProRF.floatValue);

        memset(ProRF.Buffer, 0, sizeof(ProRF.Buffer));		
        Global.RF_MoveChk[14] = 0;
    }
    else
    {
        Global.RF_MoveChk[14]++;
    }

    if(!strncmp((const char*)&ProRF.Buffer[4],"=15,", 4)) //id
    {
        if(!strncmp((const char*)&ProRF.Buffer[10],"0", 1)) //stop
        {
            Global.RF_Movement[15] = RF_STOP;

        }
        else if(!strncmp((const char*)&ProRF.Buffer[10],"1", 1))//move
        {
            Global.RF_Movement[15] = RF_MOVE;

        }
        
        for(u8 i = 0; i < 3; i++)
        {
            ProRF.scan_distance[15][i] = ProRF.Buffer[12+i];
        }
        sscanf((const char*)ProRF.scan_distance[15],"%f",&ProRF.floatValue);
        Global.device_Distance_RF[15] = calculateDistance(REF_RSSI,ProRF.floatValue);
        
        memset(ProRF.Buffer, 0, sizeof(ProRF.Buffer));	
        Global.RF_MoveChk[15] = 0;	
    }
    else
    {
        Global.RF_MoveChk[15]++;
    }

    if(!strncmp((const char*)&ProRF.Buffer[4],"=16,", 4)) //id
        {
        if(!strncmp((const char*)&ProRF.Buffer[10],"0", 1)) //stop
        {
            Global.RF_Movement[16] = RF_STOP;

        }
        else if(!strncmp((const char*)&ProRF.Buffer[10],"1", 1))//move
        {
            Global.RF_Movement[16] = RF_MOVE;

        }
        
        for(u8 i = 0; i < 3; i++)
        {
            ProRF.scan_distance[16][i] = ProRF.Buffer[12+i];
        }
        sscanf((const char*)ProRF.scan_distance[16],"%f",&ProRF.floatValue);
        Global.device_Distance_RF[16] = calculateDistance(REF_RSSI,ProRF.floatValue);
        
        memset(ProRF.Buffer, 0, sizeof(ProRF.Buffer));		
        Global.RF_MoveChk[16] = 0;
        }
    else
    {
        Global.RF_MoveChk[16]++;
    }

    if(!strncmp((const char*)&ProRF.Buffer[4],"=17,", 4)) //id
        {
        if(!strncmp((const char*)&ProRF.Buffer[10],"0", 1)) //stop
        {
            Global.RF_Movement[17] = RF_STOP;

        }
        else if(!strncmp((const char*)&ProRF.Buffer[10],"1", 1))//move
        {
            Global.RF_Movement[17] = RF_MOVE;

        }
        
        for(u8 i = 0; i < 3; i++)
        {
            ProRF.scan_distance[17][i] = ProRF.Buffer[12+i];
        }
        sscanf((const char*)ProRF.scan_distance[17],"%f",&ProRF.floatValue);
        Global.device_Distance_RF[17] = calculateDistance(REF_RSSI,ProRF.floatValue);
            
        memset(ProRF.Buffer, 0, sizeof(ProRF.Buffer));	
        Global.RF_MoveChk[17] = 0;
        
        }
    else
    {
        Global.RF_MoveChk[17]++;
    }

    if(!strncmp((const char*)&ProRF.Buffer[4],"=18,", 4)) //id
    {
        if(!strncmp((const char*)&ProRF.Buffer[10],"0", 1)) //stop
        {
            Global.RF_Movement[18] = RF_STOP;

        }
        else if(!strncmp((const char*)&ProRF.Buffer[10],"1", 1))//move
        {
            Global.RF_Movement[18] = RF_MOVE;

        }

        for(u8 i = 0; i < 3; i++)
        {
                ProRF.scan_distance[18][i] = ProRF.Buffer[12+i];
        }
        sscanf((const char*)ProRF.scan_distance[18],"%f",&ProRF.floatValue);
        Global.device_Distance_RF[18] = calculateDistance(REF_RSSI,ProRF.floatValue);

        memset(ProRF.Buffer, 0, sizeof(ProRF.Buffer));	
        Global.RF_MoveChk[18] = 0;

    }
    else
    {
        Global.RF_MoveChk[18]++;
    }

    if(!strncmp((const char*)&ProRF.Buffer[4],"=19,", 4)) //id
    {   
        if(!strncmp((const char*)&ProRF.Buffer[10],"0", 1)) //stop
        {
            Global.RF_Movement[19] = RF_STOP;

        }
        else if(!strncmp((const char*)&ProRF.Buffer[10],"1", 1))//move
        {
            Global.RF_Movement[19] = RF_MOVE;

        }
        for(u8 i = 0; i < 3; i++)
        {
            ProRF.scan_distance[19][i] = ProRF.Buffer[12+i];
        }
        sscanf((const char*)ProRF.scan_distance[19],"%f",&ProRF.floatValue);
        Global.device_Distance_RF[19] = calculateDistance(REF_RSSI,ProRF.floatValue);
        
        memset(ProRF.Buffer, 0, sizeof(ProRF.Buffer));	
        Global.RF_MoveChk[19] = 0;
        
    }
    else
    {
        Global.RF_MoveChk[19]++;
    }
    // move change
    if(memcmp((const char*)Global.RF_PreMovement,(const char*)Global.RF_Movement,sizeof(Global.RF_Movement)) != 0)
    {
        Global.RF_moveflag = 1;
    }
  memset(&Global.RF_PreMovement, 0, sizeof(Global.RF_PreMovement));
  memcpy(Global.RF_PreMovement,Global.RF_Movement,sizeof(Global.RF_Movement));

}
