
#include "Control.h"
#include "cmsis_os.h"
#include "main.h"
#include "lcd.h"
#include "LCD_Task.h"
#include "RF_Task.h"
#include "board_driver.h"
#include <string.h>

void boardRunning(void)
{
	if(RunningTime & 0x200)		out_LCM_LED1(_ON_)
	else						out_LCM_LED1(_OFF_);
}

float readValue = 0;
unsigned char AdcSize = 100;
float ADC_Channel[2][101];

typedef struct {
    double state;
    double estimate_error;
    double Q;
    double R;
} KalmanFilter;

KalmanFilter batFilter;

void kalman_init(KalmanFilter *kf, double initial_state, double initial_estimate_error, double process_noise, double measurement_noise) {
    kf->state = initial_state;
    kf->estimate_error = initial_estimate_error;
    kf->Q = process_noise;
    kf->R = measurement_noise;
}

// Kalman filter update
double kalman_update(KalmanFilter *kf, double measurement) {
    // Prediction Step
    double predicted_state = kf->state;
    double predicted_estimate_error = kf->estimate_error + kf->Q;

    // Update Step
    double K = predicted_estimate_error / (predicted_estimate_error + kf->R);
    kf->state = predicted_state + K * (measurement - predicted_state);
    kf->estimate_error = (1 - K) * predicted_estimate_error;

    return kf->state;
}

float removeMinMax(float * Data)
{
	int loop1, loop2 = 0;
	float temp = 0;
	float adcBuff[102];
	float Sum = 0;
	
	memcpy(&adcBuff[0], &Data[0], (sizeof(adcBuff)/4));
	
	for(loop1 = 0; loop1 < 100; loop1++)
	{
		for(loop2 = 0; loop2 < (100-loop1); loop2++)
		{
			if(adcBuff[loop2] > adcBuff[loop2+1])
			{
				temp = adcBuff[loop2];
				adcBuff[loop2] = adcBuff[loop2+1];
				adcBuff[loop2+1] = temp;
			}
		}
	}

	for(loop1 = 10; loop1 < 90; loop1++)
	{
		Sum += adcBuff[loop1];
	}
		
	return Sum/80;
}

float adc_1[20];
void get_raw_data(void)
{
    ADC_Channel[0][0] = (float)removeMinMax(&adc_1[0]);
}

float DataShift(unsigned char num)
{
	int loop1 = 0;
	float sum = 0;

	sum = removeMinMax(ADC_Channel[num]);
	
	for(loop1 = (AdcSize-1); loop1 > 0; loop1--)
	{
		ADC_Channel[num][loop1] = ADC_Channel[num][loop1-1];
	}

	return sum;
}


void Battery_ADC(void)
{
	
    double measurement;
    double estimated_state;
	
	readValue = DataShift(0);
	BatVoltag = ((((float)readValue/4096)*3.3) * 2.19186 * 100);
	measurement = (double)BatVoltag;

    estimated_state = kalman_update(&batFilter,measurement);
	BatVoltag = (float)estimated_state;

	Cur_BAT = (u8)(((BatVoltag+10-310)/(420-310)) * 100);

	if(Cur_BAT > 100) Cur_BAT = 100;
	else if(Cur_BAT < 1) Cur_BAT = 0;
}


//Rotary SW
void Scan_Rotary(void)
{
	char rotaryA;
  	char rotaryB;

	rotaryA = in_RotaryA();
	rotaryB = in_RotaryB();

	Global.CurDistancePhase = 0;
	Global.CurDistancePhase |= rotaryA<<1;
	Global.CurDistancePhase |= rotaryB;


	if(selectMode == UWB)
	{
		if((Global.PreDistancePhase == B11)&&(Global.CurDistancePhase == B01)) 
		{
			if(Global.Distance_UWB > 2)		 	 Global.Distance_UWB = Global.Distance_UWB - 2;
			else if(Global.Distance_UWB == 2)	 Global.Distance_UWB = 2;
		}
		else if((Global.PreDistancePhase == B11)&&(Global.CurDistancePhase == B10)) 
		{
			if(Global.Distance_UWB < 10)		 Global.Distance_UWB = Global.Distance_UWB + 2;
			else if(Global.Distance_UWB >= 10)	 Global.Distance_UWB = 10;

		}
	}
	else if(selectMode == RF)
	{
		if((Global.PreDistancePhase == B11)&&(Global.CurDistancePhase == B01)) 
		{
			if(Global.Distance_RF > 1)		 	 Global.Distance_RF = Global.Distance_RF - 0.1;
			else if(Global.Distance_RF == 1)	 Global.Distance_RF = 1;
		}
		else if((Global.PreDistancePhase == B11)&&(Global.CurDistancePhase == B10)) 
		{
			if(Global.Distance_RF < 5)		 Global.Distance_RF = Global.Distance_RF + 0.1;
			else if(Global.Distance_RF >= 5)	 Global.Distance_RF = 5;

		}

	}

	Global.PreDistancePhase = Global.CurDistancePhase;
}



//select PUSH SW
void Scan_Sel_SW(void)
{
	
	u8 SW_1;
	u8 SW_2;
	u8 SW_PWR;

	SW_1 = in_SW_SEL1();
	SW_2 = in_SW_SEL2();
	SW_PWR = in_PWR_OnOff();

	Global.Cur_SW1 = SW_1;
	Global.Cur_SW2 = SW_2;
	Global.Cur_SW_PWR = SW_PWR;

	if((Global.Pre_SW1 == 0)&& (Global.Cur_SW1 == 1))
	{
		if(selectMode != UWB)
		{
			//HAL_NVIC_DisableIRQ(USART1_IRQn);

			selectMode = UWB;
			//UWB_Main();

			out_RF_EN(_OFF_);
			out_UWB_EN(_ON_);
			init_dwm_1000_tag();

		}
	}
	else if((Global.Pre_SW2 == 0)&& (Global.Cur_SW2 == 1))
	{
		if(selectMode != RF)
		{			
			//HAL_NVIC_EnableIRQ(USART1_IRQn);
			
			selectMode = RF;
			//RF_Main();

			out_UWB_EN(_OFF_);
			out_RF_EN(_ON_);
				
			//RF module PWR On   
			out_RF_RST(Bit_RESET);
			out_RF_RST(Bit_SET);

			
			osDelay(200);
			SendCommand("AT+BAND=921900000,M,\r\n","",200);
			SendCommand("AT+BAND?,\r\n","",200);
			SendCommand("AT+ADDRESS?,\r\n","",200);

		}
	}
	
	Global.Pre_SW1 = Global.Cur_SW1;
	Global.Pre_SW2 = Global.Cur_SW2;

	
	//Pwr SW
	if(Global.Pre_SW_PWR != Global.Cur_SW_PWR)
	{
		if(Global.Cur_SW_PWR == _HIGH_)
		{
			Global.SW_PWR_RisingEdg = _ON_;
			Global.SW_PWR_FallingEdg = _OFF_;
		}

		else if(Global.Cur_SW_PWR == _LOW_)
		{
			Global.SW_PWR_RisingEdg = _OFF_;
			Global.SW_PWR_FallingEdg = _ON_;
			Global.SW_PWR_RisingTime = 0;
		}
	}
	Global.Pre_SW_PWR = Global.Cur_SW_PWR;
	
	//3sec -> power off
	if(Global.SW_PWR_RisingTime > 3000)
	{
		out_PWR_OnOff(_OFF_);
	}
	
}

u8 Rotary_A, Rotary_B;

void check_rotary(void)
{
	Rotary_A = in_RotaryA();
	Rotary_B = in_RotaryB();
}



void Control_Task(void *arg)
{
	kalman_init(&batFilter, 0.0, 1.0, 0.001, 0.1);

    for(;;)
    {		
		boardRunning();
		Scan_Sel_SW();
		if(BAT_chkflag)
		{
			get_raw_data();
			Battery_ADC();
			BAT_chkflag = 0;
		}
		osDelay(10);
    }
}