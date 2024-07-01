#include "main.h"


typedef struct  
{										    
	u16 width;			
	u16 height;			
	u16 id;				
	u8  dir;				
	u16	 wramcmd;		
	u16  rramcmd;   
	u16  setxcmd;		
	u16  setycmd;		
}_lcd_dev; 	

extern _lcd_dev lcddev;
extern u16  POINT_COLOR;
extern u16  BACK_COLOR; 


#define WHITE       0xFFFF
#define BLACK      	0x0000	  
#define BLUE       	0x001F  
#define BRED        0XF81F
#define GRED 			 	0XFFE0
#define GBLUE			 	0X07FF
#define RED         0xF800
#define MAGENTA     0xF81F
#define GREEN       0x07E0
#define CYAN        0x7FFF
#define YELLOW      0xFFE0
#define BROWN 			0XBC40 
#define BRRED 			0XFC07 
#define GRAY  			0X8430 

#define DARKBLUE      	 0X01CF	
#define LIGHTBLUE      	 0X7D7C	
#define GRAYBLUE       	 0X5458
 
#define LIGHTGREEN     	0X841F 
#define LIGHTGRAY       0XEF5B 
#define LGRAY 			0XC618 

#define LGRAYBLUE      	0XA651 
#define LBBLUE          0X2B12 
	    														

#define LCD_W 320
#define LCD_H 480

#define USE_HORIZONTAL  	 0 
#define LCD_USE8BIT_MODEL    1	

void LCD_Init(void);
void LCD_WR_REG(unsigned char index);
void LCD_WR_DATA(u8 val);
void LCD_WriteRAM_Prepare(void);
void LCD_SetWindows(u16 xStar, u16 yStar,u16 xEnd,u16 yEnd);
void LCD_Clear(u16 Color);
void LCD_WriteReg(u16 LCD_Reg, u16 LCD_RegValue);
void LCD_direction(u8 direction);
void TFT_LCD_Reset(void);
void TFT_LCD_Init(void);
void Lcd_WriteData_16Bit(u16 Data);
void LCD_DrawPoint(u16 x,u16 y);
void LCD_SetCursor(u16 Xpos, u16 Ypos);




