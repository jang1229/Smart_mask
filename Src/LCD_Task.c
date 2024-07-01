#include "main.h"
#include "LCD_Task.h"
#include "lcd.h"
#include "cmsis_os.h"
#include "GUI.h"
#include "pic.h"
#include <string.h>
#include <stdio.h>
#include "board_driver.h"

//#include "dwm1000_tag.h"

#define Bank1_LCD_C    ((u32)0x60000000)    //disp Reg ADDR
#define Bank1_LCD_D    ((u32)0x60020000)	 //disp Data ADDR

u16 ColorTab[5]={RED,GREEN,BLUE,YELLOW,BRED};//������ɫ����
u16 ColornTab[8]={RED,MAGENTA,GREEN,DARKBLUE,BLUE,BLACK,LIGHTGREEN};

void DrawTestPage(u8 *str)
{
//���ƹ̶���up
LCD_Clear(WHITE);
LCD_Fill(0,0,lcddev.width,20,RED);
//���ƹ̶���down
LCD_Fill(0,lcddev.height-20,lcddev.width,lcddev.height,RED);
POINT_COLOR=WHITE;
Gui_StrCenter(0,2,WHITE,RED,str,16,1);//������ʾ
Gui_StrCenter(0,lcddev.height-18,WHITE,RED,"rookie@yandy.co.kr",16,1);//������ʾ
//���Ʋ�������
//LCD_Fill(0,20,lcddev.width,lcddev.height-20,WHITE);
}

/*****************************************************************************
 * @name       :void Test_Triangle(void)
 * @date       :2018-08-09 
 * @function   :triangle display and fill test
								Display red,green,blue,yellow,pink triangle boxes in turn,
								1500 milliseconds later,
								Fill the triangle in red,green,blue,yellow and pink in turn
 * @parameters :None
 * @retvalue   :None
******************************************************************************/
void Test_Triangle(void)
{
	u8 i=0;
	//DrawTestPage("����5:GUI Triangle������");
	LCD_Fill(0,20,lcddev.width,lcddev.height-20,WHITE);
	for(i=0;i<5;i++)
	{
		POINT_COLOR=ColorTab[i];
		Draw_Triangel(lcddev.width/2-80+(i*20),lcddev.height/2-20+(i*15),lcddev.width/2-50-1+(i*20),lcddev.height/2-20-52-1+(i*15),lcddev.width/2-20-1+(i*20),lcddev.height/2-20+(i*15));
	}
	HAL_Delay(1500);	
	LCD_Fill(0,20,lcddev.width,lcddev.height-20,WHITE); 
	for(i=0;i<5;i++)
	{
		POINT_COLOR=ColorTab[i];
		Fill_Triangel(lcddev.width/2-80+(i*20),lcddev.height/2-20+(i*15),lcddev.width/2-50-1+(i*20),lcddev.height/2-20-52-1+(i*15),lcddev.width/2-20-1+(i*20),lcddev.height/2-20+(i*15));
	}
	HAL_Delay(1500);
}
/*****************************************************************************
 * @name       :void main_test(void)
 * @date       :2018-08-09 
 * @function   :Drawing the main Interface of the Comprehensive Test Program
 * @parameters :None
 * @retvalue   :None
******************************************************************************/
u32 CntTest;
void main_test(void)
{
	//DrawTestPage("Smart_Mask_Anchor");	
	Gui_StrCenter(0,30,RED,BLUE,"Y&Y",16,1);//������ʾ
	Gui_StrCenter(0,80,RED,BLUE,"LCD_Test",24,1);//������ʾ	
	Gui_StrCenter(0,130,MAGENTA,BLUE,"3.5\" ILI9486 320X480",32,1);//������ʾ
	Gui_StrCenter(0,280,BLUE,BLUE,"rookie",32,1);//������ʾ
	//osDelay(1);		
    CntTest++;
}

/*****************************************************************************
 * @name       :void Test_Color(void)
 * @date       :2018-08-09 
 * @function   :Color fill test(white,black,red,green,blue)
 * @parameters :None
 * @retvalue   :None
******************************************************************************/
void Test_Color(void)
{
	//DrawTestPage("����1:��ɫ������");
	LCD_Fill(0,0,lcddev.width,lcddev.height,WHITE);
	Show_Str(20,30,BLUE,YELLOW,"BL Test",16,1);HAL_Delay(800);
	LCD_Fill(0,0,lcddev.width,lcddev.height,RED);
	Show_Str(20,30,BLUE,YELLOW,"RED ",16,1);HAL_Delay(800);
	LCD_Fill(0,0,lcddev.width,lcddev.height,GREEN);
	Show_Str(20,30,BLUE,YELLOW,"GREEN ",16,1);HAL_Delay(800);
	LCD_Fill(0,0,lcddev.width,lcddev.height,BLUE);
	Show_Str(20,30,RED,YELLOW,"BLUE ",16,1);HAL_Delay(800);
}

void Test_Circle(void)
{
	u8 i=0;
	//DrawTestPage("����4:GUI��Բ������");
	LCD_Fill(0,20,lcddev.width,lcddev.height-20,WHITE);
	for (i=0; i<5; i++)  
		gui_circle(lcddev.width/2-80+(i*25),lcddev.height/2-50+(i*25),ColorTab[i],30,0);
	HAL_Delay(1500);	
	LCD_Fill(0,20,lcddev.width,lcddev.height-20,WHITE); 
	for (i=0; i<5; i++) 
	  	gui_circle(lcddev.width/2-80+(i*25),lcddev.height/2-50+(i*25),ColorTab[i],30,1);
	HAL_Delay(1500);
}

/*****************************************************************************
 * @name       :void Pic_test(void)
 * @date       :2018-08-09 
 * @function   :picture display test
 * @parameters :None
 * @retvalue   :None
******************************************************************************/
void Pic_test(void)
{
	//DrawTestPage("����8:ͼƬ��ʾ����");
	//LCD_Fill(0,20,lcddev.width,lcddev.height-20,WHITE);
	Gui_Drawbmp16(30,30,gImage_qq);
	Show_Str(30+12,75,BLUE,YELLOW,"QQ",16,1);
	Gui_Drawbmp16(90,30,gImage_qq);
	Show_Str(90+12,75,BLUE,YELLOW,"QQ",16,1);
	Gui_Drawbmp16(150,30,gImage_qq);
	Show_Str(150+12,75,BLUE,YELLOW,"QQ",16,1);
	HAL_Delay(1200);
}


/*****************************************************************************
 * @name       :void English_Font_test(void)
 * @date       :2018-08-09 
 * @function   :English display test
 * @parameters :None
 * @retvalue   :None
******************************************************************************/
void English_Font_test(void)
{
	DrawTestPage("����6:Ӣ����ʾ����");
	Show_Str(10,30,BLUE,YELLOW,"6X12:abcdefghijklmnopqrstuvwxyz0123456789",12,0);
	Show_Str(10,45,BLUE,YELLOW,"6X12:ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789",12,1);
	Show_Str(10,60,BLUE,YELLOW,"6X12:~!@#$%^&*()_+{}:<>?/|-+.",12,0);
	Show_Str(10,80,BLUE,YELLOW,"8X16:abcdefghijklmnopqrstuvwxyz0123456789",16,0);
	Show_Str(10,100,BLUE,YELLOW,"8X16:ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789",16,1);
	Show_Str(10,120,BLUE,YELLOW,"8X16:~!@#$%^&*()_+{}:<>?/|-+.",16,0); 
	HAL_Delay(1200);
}

/*****************************************************************************
 * @name       :void Rotate_Test(void)
 * @date       :2018-08-09 
 * @function   :rotate test
 * @parameters :None
 * @retvalue   :None
******************************************************************************/
void Rotate_Test(void)
{
	u8 i=0;
	u8 *Direction[4]={"Rotation:0","Rotation:90","Rotation:180","Rotation:270"};
	
	for(i=0;i<4;i++)
	{
	LCD_direction(i);
	DrawTestPage("����9:��Ļ��ת����");
	Show_Str(20,30,BLUE,YELLOW,Direction[i],16,1);
	Gui_Drawbmp16(30,50,gImage_qq);
	HAL_Delay(1000);
	}
	LCD_direction(USE_HORIZONTAL);
}



// circle 좌표 
unsigned int circle_x[20] = {87, 174, 261, 350, 437,87, 174, 261, 350, 437,87, 174, 261, 350, 437,87, 174, 261, 350, 437};
unsigned int circle_y[20] = {137, 137, 137, 137, 137, 187, 187, 187, 187, 187, 237, 237, 237, 237, 237, 287, 287, 287, 287, 287};

unsigned int distanceCnt[20]; // 0~19번까지 통신 시도횟수 배열 
unsigned int distanceDisplay[20]; // 0~19번까지 동그라미 표시 하기위한 상태 배열 IDLE, FAR, NEAR 
unsigned int userCnt;


//UWB모드 동작 로직 
void LCD_Mode_UWB(void)
{
	//통신 된 유저 수 
	userCnt = 0;

	for(unsigned char Loop = 0;  Loop < 20; Loop++)
	{
		//if((Global.device_Distance_uwb[Loop] > 10) && (Global.device_Distance_uwb[Loop] <= (Global.Distance_UWB *100)))//distance Cm 

		if((Global.device_Distance_uwb[Loop] <= (Global.Distance_UWB *100)))//distance Cm 
		{
			if(Global.device_PreDistance_uwb[Loop] ==  Global.device_Distance_uwb[Loop]) // 움직임이 없을때 
			{

				if(distanceCnt[Loop] >= 11) // 11번 통신시도 횟수 동안 거리의 변화가 없으면 , 꺼진상태 Chk 
				{
					distanceCnt[Loop] = 0;
					Global.device_Distance_uwb[Loop] = 0;
					distanceDisplay[Loop] = UWB_IDLE;
				}
				else
					distanceCnt[Loop]++;

			}
			else if(Global.device_PreDistance_uwb[Loop] <=  Global.device_Distance_uwb[Loop]) // 전보다 거리가 커질 때
			{
				
				distanceDisplay[Loop] = UWB_FAR;
				distanceCnt[Loop] = 0;

			}
			else 	// 전보다 거리가 가까워질 때
			{
				distanceDisplay[Loop] = UWB_NEAR;
				distanceCnt[Loop] = 0;

			}

			Global.device_PreDistance_uwb[Loop] = Global.device_Distance_uwb[Loop];
		}
		else
			distanceDisplay[Loop] = UWB_IDLE;
	}


	// 멀어지는지 가까워지는지에 따라 동그라미 그리는 로직 
	for(unsigned char Loop = 0;  Loop < 20; Loop++)
	{
		if(distanceDisplay[Loop] == UWB_IDLE)	
		{
			gui_circle(circle_x[Loop],circle_y[Loop],WHITE,10,1);

		}
		else if(distanceDisplay[Loop] == UWB_FAR)	
		{
			gui_circle(circle_x[Loop],circle_y[Loop],BLUE,10,1);
			userCnt++;
			
		}
		else if(distanceDisplay[Loop] == UWB_NEAR)	
		{		
			gui_circle(circle_x[Loop],circle_y[Loop],RED,10,1);
			userCnt++;
		}
	}
    
	memset(&Global.UWB_User_Value, 0, sizeof(Global.UWB_User_Value));
	sprintf((char*)Global.UWB_User_Value,"%d Users ",userCnt);	
	Show_Str(330,57,BLACK,WHITE,Global.UWB_User_Value,24,0);


}

// UWB 모드 초기화면 
void UWB_Main(void)
{
		__disable_irq();

		//selectMode = UWB;

      	LCD_Clear(WHITE);		
		LCD_Battery();


		LCD_DrawLine(20,10,lcddev.width-20,10);
		LCD_DrawLine(20,50,lcddev.width-20,50);
		LCD_DrawLine(20,10,20,lcddev.height-10);
		LCD_DrawLine(lcddev.width-20,10,lcddev.width-20,lcddev.height-10);
		Gui_StrCenter(0,25,BLACK,BLUE,"Accident Investigation Status",24,1);//������ʾ
		LCD_DrawLine(20,lcddev.height-10,lcddev.width-20,lcddev.height-10);

		LCD_DrawLine(20,80,lcddev.width-20,80);
		LCD_DrawLine(20,110,lcddev.width-20,110);
		Show_Str(110,57,BLACK,WHITE,"USER",24,0);
		Show_Str(75,87,BLACK,WHITE,"Ditance Value",24,0);
		Show_Str(330,57,BLACK,WHITE,"0 Users ",24,0);

		Show_Str(350,87,BLACK,WHITE,Global.UWB_Set_Value,24,0); //2,4,6,8,10				

		LCD_DrawLine(20,160,lcddev.width-20,160);
		LCD_DrawLine(20,210,lcddev.width-20,210);
		LCD_DrawLine(20,260,lcddev.width-20,260);
		LCD_DrawLine(240,50,240,lcddev.height-10);
		LCD_DrawLine(64,110,64,lcddev.height-10);
		LCD_DrawLine(108,110,108,lcddev.height-10);
		LCD_DrawLine(152,110,152,lcddev.height-10);
		LCD_DrawLine(196,110,196,lcddev.height-10);
		LCD_DrawLine(196,110,196,lcddev.height-10);

		LCD_DrawLine(284,110,284,lcddev.height-10);
		LCD_DrawLine(328,110,328,lcddev.height-10);
		LCD_DrawLine(372,110,372,lcddev.height-10);
		LCD_DrawLine(416,110,416,lcddev.height-10);
		

		Show_Str(40,127,BLACK,WHITE,"0",24,0);
		Show_Str(127,127,BLACK,WHITE,"1",24,0);
		Show_Str(214,127,BLACK,WHITE,"2",24,0);
		Show_Str(301,127,BLACK,WHITE,"3",24,0);
		Show_Str(390,127,BLACK,WHITE,"4",24,0);
		Show_Str(40,177,BLACK,WHITE,"5",24,0);
		Show_Str(127,177,BLACK,WHITE,"6",24,0);
		Show_Str(214,177,BLACK,WHITE,"7",24,0);
		Show_Str(301,177,BLACK,WHITE,"8",24,0);
		Show_Str(388,177,BLACK,WHITE,"9",24,0);
		Show_Str(38,227,BLACK,WHITE,"10",24,0);
		Show_Str(125,227,BLACK,WHITE,"11",24,0);
		Show_Str(212,227,BLACK,WHITE,"12",24,0);
		Show_Str(299,227,BLACK,WHITE,"13",24,0);
		Show_Str(386,227,BLACK,WHITE,"14",24,0);
		Show_Str(38,277,BLACK,WHITE,"15",24,0);
		Show_Str(125,277,BLACK,WHITE,"16",24,0);
		Show_Str(212,277,BLACK,WHITE,"17",24,0);
		Show_Str(299,277,BLACK,WHITE,"18",24,0);
		Show_Str(386,277,BLACK,WHITE,"19",24,0);		
		__enable_irq();

}

//RF모드 동작 로직 

void LCD_Mode_RF(void)
{	
	Global.RF_userCnt = 0;
	Global.RF_stopCnt = 0;
	Global.RF_moveCnt = 0;

	//no signal chk
	for(u8 loop = 0; loop < 20; loop++)
	{
		if(Global.RF_Movement[loop] != RF_IDLE)
		{
			// 60번 이상 데이터가 들어오지 않으면 IDLE 상태 USER표시 X
			if(Global.RF_MoveChk[loop] > 60) 
			{
				Global.RF_Movement[loop] = RF_IDLE;
				Global.RF_MoveChk[loop] = 0;
			}
		}
		if(Global.RF_Movement[loop] == RF_IDLE)
		{
			Global.RF_MoveChk[loop] = 0;
		}
	}
	
	//status chk
	for(unsigned char loop = 0; loop < 20; loop++)
	{
		if(Global.device_Distance_RF[loop] <= Global.Distance_RF*1000) //distance_rf = M단위 KM로 변환위해 x 1000 
		{
			if(Global.RF_Movement[loop] == RF_STOP)
			{
				//users & stop users
				Global.RF_userCnt++;
				Global.RF_stopCnt++;
				
				
			}
			else if(Global.RF_Movement[loop] == RF_MOVE)
			{			
				//users & moving users
				Global.RF_userCnt++;
				Global.RF_moveCnt++;
			}
		}
				
	
	}		


	
	 
	//movement change -> Stoping ID Draw
	if(Global.RF_moveflag)
	{
		Global.RF_moveflag = 0;
		//user number
		memset(&Global.RF_User_Value, 0, sizeof(Global.RF_User_Value));
		sprintf((char*)Global.RF_User_Value,"%d Users",Global.RF_userCnt);
		Show_Str(330,80,BLACK,WHITE,Global.RF_User_Value,24,0);

		//move user, stop user
		memset(&Global.RF_Movement_Value, 0, sizeof(Global.RF_Movement_Value));
		sprintf((char*)Global.RF_Movement_Value,"MOVE %d, STOP %d",Global.RF_moveCnt,Global.RF_stopCnt);
		Show_Str(300,160,BLACK,WHITE,Global.RF_Movement_Value,24,0);

		//make stoping id string 
		make_stop_list();
		
		//clean stopping id 
		Gui_StrCenter(170,270,BLACK,WHITE,"                                                 ",24,0);
		if(TempStr[0] == 0)		Gui_StrCenter(170,270,BLACK,WHITE,"Not stop id",24,0);
		else					Gui_StrCenter(170,270,BLACK,WHITE,(unsigned char*)TempStr,24,0);
	}

}



// RF모드 초기화면 
void RF_Main(void)
{
		__disable_irq();
		//selectMode = RF;

      	LCD_Clear(WHITE);
		LCD_Battery();

		//DelayMs(100);

		LCD_DrawLine(20,10,lcddev.width-20,10);
		LCD_DrawLine(20,70,lcddev.width-20,70);

		LCD_DrawLine(20,10,20,lcddev.height-10);
		LCD_DrawLine(lcddev.width-20,10,lcddev.width-20,lcddev.height-10);

		Gui_StrCenter(0,35,BLACK,BLUE,"Accident Investigation Status",24,1);//������ʾ	
		LCD_DrawLine(20,lcddev.height-10,lcddev.width-20,lcddev.height-10);


		LCD_DrawLine(20,110,lcddev.width-20,110);
		LCD_DrawLine(20,150,lcddev.width-20,150);
		LCD_DrawLine(20,190,lcddev.width-20,190);
		Show_Str(110,80,BLACK,WHITE,"USER",24,0);
		Show_Str(75,120,BLACK,WHITE,"Ditance Value",24,0);
		Show_Str(90,160,BLACK,WHITE,"Movement",24,0);
		Show_Str(200,210,BLACK,WHITE,"Stopping ID",24,0);

		Show_Str(330,80,BLACK,WHITE,"0 Users",24,0);
		Show_Str(350,120,BLACK,WHITE,"1.00 Km",24,0); //2,4,6,8,10
		Show_Str(300,160,BLACK,WHITE,"MOVE 0, STOP 0",24,0);

		LCD_DrawLine(20,250,lcddev.width-20,250);

		LCD_DrawLine(240,70,240,190);
		__enable_irq();

}



// stopping id값 숫자별로 ',' 문자 넣어주는 로직 

//stopping ID value -> make string
char TempStr[60];
int TempStrCnt = 0;

void make_stop_list(void)
{
	char del = 0x2c;
	char ten=0, one=0;
	
	TempStrCnt = 0;
	memset(TempStr, 0, strlen(TempStr));

	for(int i=0 ;i<20 ;i++)
	{
		if(Global.RF_Movement[i] == RF_STOP)
		{	
			if(i>0)
			{
				one = i%10;
				ten = i/10;
				if(ten != 0) // ten
				{
					TempStr[TempStrCnt] = ten + 0x30; 
					TempStrCnt++;
				}

				TempStr[TempStrCnt] = one + 0x30; // one
				TempStrCnt++;
				
				TempStr[TempStrCnt] = del; // comma
				TempStrCnt++;
			}
			else if(i == 0)
			{
				TempStr[TempStrCnt] = 0x30; 
				TempStrCnt++;
				TempStr[TempStrCnt] = del; 
				TempStrCnt++;
			}			
		}
		
	}
	TempStr[TempStrCnt-1] = 0;
}



//배터리 display 로직 
void LCD_Battery(void)
{
	//need BAT value 
	if(Cur_BAT > 95)
	{		
		LCD_DrawFillRectangle_BAT(430,12,lcddev.width-21,22,GREEN);
		Show_Str(410,11,BLACK,WHITE,"100%",12,0);	
	}
	else if((Cur_BAT > 80) && (Cur_BAT <= 95))
	{
		LCD_DrawFillRectangle_BAT(430,12,lcddev.width-21,22,WHITE);
		LCD_DrawFillRectangle_BAT(430,12,lcddev.width-27,22,GREEN);
		
		if(Cur_BAT < 85) Show_Str(410,11,BLACK,WHITE,"85% ",12,0);			
		else if(Cur_BAT > 85) Show_Str(410,11,BLACK,WHITE,"95% ",12,0);
	}
	else if((Cur_BAT > 60) && (Cur_BAT <= 80))
	{
		LCD_DrawFillRectangle_BAT(430,12,lcddev.width-21,22,WHITE);
		LCD_DrawFillRectangle_BAT(430,12,lcddev.width-30,22,YELLOW);
		if(Cur_BAT < 70) Show_Str(410,11,BLACK,WHITE,"65% ",12,0);			
		else if(Cur_BAT > 70) Show_Str(410,11,BLACK,WHITE,"75% ",12,0);
	}
	else if((Cur_BAT > 50) && (Cur_BAT <= 60))
	{
		LCD_DrawFillRectangle_BAT(430,12,lcddev.width-21,22,WHITE);
		LCD_DrawFillRectangle_BAT(430,12,lcddev.width-37,22,YELLOW);
		Show_Str(410,11,BLACK,WHITE,"55% ",12,0);			
	}
	else if((Cur_BAT > 40) && (Cur_BAT <= 50))
	{
		LCD_DrawFillRectangle_BAT(430,12,lcddev.width-21,22,WHITE);
		LCD_DrawFillRectangle_BAT(430,12,lcddev.width-40,22,YELLOW);
		Show_Str(410,11,BLACK,WHITE,"45% ",12,0);			
	}
	else if((Cur_BAT > 20) && (Cur_BAT <= 40))
	{
		LCD_DrawFillRectangle_BAT(430,12,lcddev.width-21,22,WHITE);
		LCD_DrawFillRectangle_BAT(430,12,lcddev.width-42,22,RED);
		if(Cur_BAT < 30) Show_Str(410,11,BLACK,WHITE,"25% ",12,0);			
		else if(Cur_BAT > 30) Show_Str(410,11,BLACK,WHITE,"35% ",12,0);
	}
	else if((Cur_BAT > 0) && (Cur_BAT <= 20))
	{
		LCD_DrawFillRectangle_BAT(430,12,lcddev.width-21,22,WHITE);
		LCD_DrawFillRectangle_BAT(430,12,lcddev.width-45,22,RED);
		if(Cur_BAT < 10) Show_Str(410,11,BLACK,WHITE,"5% ",12,0);			
		else if(Cur_BAT > 10) Show_Str(410,11,BLACK,WHITE,"15% ",12,0);
	}


}


unsigned int LcdTime = 0;
u8 lcdUWBflag = 0;
u8 lcdRFflag = 0;

void LCD_Main(void)
{
	if(Global.lcdFlag)
	{
		switch (selectMode)
		{
			case UWB:	LCD_Mode_UWB();
						LCD_Battery();
						break;
			
			case RF:	LCD_Mode_RF();
						LCD_Battery();
						break;
		}
		Global.lcdFlag = 0;
	}

	//거리 옵션에 따른 디스플레이 로직 
	switch (selectMode)
	{
		case UWB:
			if(Global.PreDistance_UWB != Global.Distance_UWB)
			{
				memset(&Global.UWB_Set_Value, 0, sizeof(Global.UWB_Set_Value));
				sprintf((char*)Global.UWB_Set_Value,"%d M  ",Global.Distance_UWB);
				Show_Str(350,87,BLACK,WHITE,Global.UWB_Set_Value,24,0); //2,4,6,8,10				
				
			}
			Global.PreDistance_UWB = Global.Distance_UWB;
			break;
		
		case RF:
			if(Global.PreDistance_RF != Global.Distance_RF)
			{
				memset(&Global.RF_Set_Value, 0, sizeof(Global.RF_Set_Value));
				sprintf((char*)Global.RF_Set_Value,"%.2f Km  ",Global.Distance_RF);
				Show_Str(350,120,BLACK,WHITE,Global.RF_Set_Value,24,0); //2,4,6,8,10
				
			}
			Global.PreDistance_RF = Global.Distance_RF;
			break;

	}

	// 모드변경에 따른 기존화면 clear 하기위한 로직 
	if(selectMode == UWB)
	{
		if(lcdUWBflag == 0)
		{
			UWB_Main();
			lcdUWBflag = 1;
			lcdRFflag = 0;
		}
	}
	else if(selectMode == RF)
	{
		if(lcdRFflag == 0)
		{
			RF_Main();
			lcdRFflag = 1;
			lcdUWBflag = 0;
		}
	}


}



void LCD_Task(void *argument)
{
	TFT_LCD_Reset();
    TFT_LCD_Init();
	Battery_ADC();
	
	Global.Distance_UWB = 2;

	Global.PreDistance_RF = 1;
	Global.Distance_RF = 1;

	memset(&Global.UWB_Set_Value, 0, sizeof(Global.UWB_Set_Value));
	sprintf((char*)Global.UWB_Set_Value,"%d M  ",Global.Distance_UWB);

	UWB_Main();
	lcdUWBflag = 1;

	LCD_Battery();


    for(;;)
    {
		LCD_Main();
		osDelay(10);
    }
}