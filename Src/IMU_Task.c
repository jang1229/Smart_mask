


#include "IMU_Task.h"
#include "main.h"
#include "cmsis_os.h"
#include "bno055.h"
#include <math.h>
#include <stdio.h>


//unsigned int BatVoltag = 0;


s8 BNO055_I2C_bus_write(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt);
s8 BNO055_I2C_bus_read(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt);
void BNO055_delay_msek(u32 msec);



/************** I2C buffer length******/

#define I2C_BUFFER_LEN 										8
#define BNO055_I2C_BUS_WRITE_ARRAY_INDEX ((u8)1)

I2C_HandleTypeDef *hi2c_sensor;


s8 I2C_routine(void)
{
	hi2c_sensor = &hi2c1;

	bno055.bus_write = BNO055_I2C_bus_write;
	bno055.bus_read = BNO055_I2C_bus_read;
	bno055.delay_msec = BNO055_delay_msek;

	bno055.dev_addr = BNO055_I2C_ADDR1;

  return BNO055_INIT_VALUE;
}

s8 BNO055_I2C_bus_write(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt)
{
    s32 BNO055_iERROR = BNO055_INIT_VALUE;
    u8 array[I2C_BUFFER_LEN];
    u8 stringpos = BNO055_INIT_VALUE;

    array[BNO055_INIT_VALUE] = reg_addr;
    for (stringpos = BNO055_INIT_VALUE; stringpos < cnt; stringpos++)
    {
        array[stringpos + BNO055_I2C_BUS_WRITE_ARRAY_INDEX] = *(reg_data + stringpos);
    }

    BNO055_iERROR = HAL_I2C_Master_Transmit(hi2c_sensor, (uint16_t)(bno055.dev_addr << 1), array, cnt+1, 1000);

    return (s8)BNO055_iERROR;
}

s8 BNO055_I2C_bus_read(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt)
{
    s32 BNO055_iERROR = BNO055_INIT_VALUE;
    u8 array[I2C_BUFFER_LEN] = { BNO055_INIT_VALUE };
    u8 stringpos = BNO055_INIT_VALUE;

    array[BNO055_INIT_VALUE] = reg_addr;

    BNO055_iERROR = HAL_I2C_Master_Transmit(hi2c_sensor, (uint16_t)(bno055.dev_addr << 1), array, 1, 1000);

    if (BNO055_iERROR == HAL_OK)
    {
    	BNO055_iERROR = HAL_I2C_Master_Receive(hi2c_sensor, (uint16_t)(bno055.dev_addr << 1), array, cnt, 1000);

    	if (BNO055_iERROR == HAL_OK)
    	{
				for (stringpos = BNO055_INIT_VALUE; stringpos < cnt; stringpos++)
				{
						*(reg_data + stringpos) = array[stringpos];
				}
    	}
    }

    return (s8)BNO055_iERROR;
}

void BNO055_delay_msek(u32 msec)
{
	HAL_Delay(msec);
}


s16 euler_data_h = BNO055_INIT_VALUE;
s16 euler_data_r = BNO055_INIT_VALUE;
s16 euler_data_p = BNO055_INIT_VALUE;

struct bno055_euler_t euler_hrp;

u8 op_mode = BNO055_INIT_VALUE;
u8 power_mode;
s16 TagAngle;
s16 BNO055_Read1(void)
{
  //
	bno055_read_euler_hrp(&euler_hrp);
	TagAngle = (s16)((float)euler_hrp.h*0.0625);
	return TagAngle;
}


void GetEuler_BNO055(s32 *RoX, s32 *RoY, s32 *RoZ)
{
    unsigned char dout[6];
    BNO055_I2C_bus_read(BNO055_I2C_ADDR1, BNO055_EULER_H_LSB_ADDR, dout, 6);
    *RoZ = dout[1] << 8 | dout[0]; // Heading
    *RoY = dout[3] << 8 | dout[2]; // Roll
    *RoX = dout[5] << 8 | dout[4]; // Pitch

    *RoZ = -(*RoZ);
    *RoY = -(*RoY);
    *RoX = -(*RoX);
}

void GetCalibData_BNO055(unsigned char *calib_data)
{
    op_mode = BNO055_OPERATION_MODE_CONFIG;
	bno055_set_operation_mode(op_mode);
    osDelay(20);

	HAL_I2C_Mem_Read(&hi2c1, (unsigned short)BNO055_I2C_ADDR1<<1, BNO055_ACCEL_OFFSET_X_LSB_ADDR, I2C_MEMADD_SIZE_8BIT, calib_data+1, 22, 3000);
    osDelay(20);
    calib_data[0] = BNO055_ACCEL_OFFSET_X_LSB_ADDR;
	
	op_mode = BNO055_OPERATION_MODE_NDOF;
	bno055_set_operation_mode(op_mode);
    osDelay(600);
}

unsigned char GetCalibState_BNO055()
{
    unsigned char state;
    BNO055_I2C_bus_read(BNO055_I2C_ADDR1, BNO055_CALIB_STAT_ADDR, &state, 1);
    return state;
}


unsigned char initial_calib_data[23] = {
            // #1
            //0x55, 0x1B, 0x00, 0xC4, 0xFF, 0xE7, 0xFF, 0x89, 0xFE, 0xA7, 0xFF, 0x4D, 0xFD, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE8, 0x03, 0x84, 0x03
            // #2
            //0x55, 0xED, 0xFF, 0x13, 0x00, 0xEA, 0xFF, 0x80, 0xFF, 0xE9, 0xFD, 0x2F, 0xFE, 0xFD, 0xFF, 0xFD, 0xFF, 0x00, 0x00,0xE8, 0x03, 0x21,0x03
            // #3
            //  0x55, 0xEF, 0xFF, 0xE2, 0xFF, 0x02, 0x00, 0xDB, 0xFF, 0x12, 0xFE, 0x0E, 0x01, 0xFE, 0xFF, 0xFE, 0xFF, 0x00, 0x00, 0xE8, 0x03, 0x97, 0x02
            // #4
            //0x55, 0xE9, 0xFF, 0x65, 0x00, 0xFF, 0xFF, 0x2F, 0x02, 0x26, 0x00, 0x66, 0xFD, 0x00, 0x00, 0x04, 0x00, 0x02, 0x00, 0xE8, 0x03, 0x36, 0x03
            // #5
            //0x55, 0xE5, 0xFF, 0xC4, 0xFF, 0xD8, 0xFF, 0x90, 0xFE, 0x93, 0xFF, 0x8B, 0xFD, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00, 0xE8, 0x03, 0x42, 0x03
            // #6
             0x55, 0xE4, 0xFF, 0xFD, 0xFF, 0xE6, 0xFF, 0x66, 0xFE, 0x13, 0x00, 0xD3, 0xFB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFD, 0xFF, 0xE8, 0x03, 0x6B, 0x03
            // #7
            //0x55, 0xF0, 0xFF, 0xCB, 0xFF, 0xDF, 0xFF, 0x35, 0x00, 0x8A, 0xFE, 0xFB, 0xFE, 0xFF, 0xFF, 0x00, 0x00, 0xFE, 0xFF, 0xE8, 0x03, 0x53, 0x03
            // #8
            //0x55, 0xE9, 0xFF, 0xD7, 0xFF, 0xEB, 0xFF, 0x3D, 0xFE, 0x8B, 0xFF, 0xFA, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFD, 0xFF, 0xE8, 0x03, 0x66, 0x03
            // #9
            // 0x55, 0xEC, 0xFF, 0x04, 0x00, 0xED, 0xFF, 0xE1, 0xFF, 0xB9, 0xFD, 0x09, 0x02, 0xFD, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xE8, 0x03, 0x0D, 0x03
            // #10
            // 0x55, 0xF2, 0xFF, 0x2B, 0x00, 0xEF, 0xFF, 0xB0, 0xFD, 0x72, 0xFF, 0xBC, 0x01, 0xFF, 0xFF, 0xFE, 0xFF, 0x04, 0x00, 0xE8, 0x03, 0x1B, 0x03
            
        };

unsigned char calib_data[23];
unsigned char calib_state;
u8 sendData[10];
unsigned char LED_Toggle = 0;
unsigned int LED_ToggleCnt = 0;


void IMU_Task(void *arg)
{
 #if 1
  out_IMU_ADD_SEL(Bit_RESET);
  //OUT_BL_IND(Bit_RESET);
  out_IMU_RST(Bit_RESET);
  osDelay(500);
  out_IMU_RST(Bit_SET);
  osDelay(500);			// �����̰� ª������... BNO055 ��ſ� ������ ��
  I2C_routine();
	
  bno055_init(&bno055);

  // BNO055 ���� �ʱ�ȭ
  op_mode = BNO055_OPERATION_MODE_CONFIG;
  bno055_set_operation_mode(op_mode);
  osDelay(50);


  power_mode = BNO055_POWER_MODE_NORMAL;
  bno055_set_power_mode(power_mode);
  //bno055_get_power_mode(&power_mode);
  osDelay(10);

  
   #if 1 
  bno055.dev_addr = BNO055_I2C_ADDR1;
/*
  sendData[0] = 0x41;
  sendData[1] = 0x21;
 //sendData[1] = 0x21;
  HAL_I2C_Master_Transmit(hi2c_sensor, (uint16_t)(bno055.dev_addr << 1), sendData, 2, 1000);
  osDelay(10);

  sendData[0] = 0x42;
  sendData[1] = 0x02;
  //sendData[1] = 0x04;
  HAL_I2C_Master_Transmit(hi2c_sensor, (uint16_t)(bno055.dev_addr << 1), sendData, 2, 1000);
  osDelay(10);
*/
  HAL_I2C_Master_Transmit(hi2c_sensor, (uint16_t)(bno055.dev_addr << 1), initial_calib_data, 23, 1000);
  osDelay(20);

  op_mode = BNO055_OPERATION_MODE_NDOF;
  bno055_set_operation_mode(op_mode);
  osDelay(50);
  #endif


  for(;;)
  {
  	BatVoltag = (unsigned int)((((float)adc_buf[0]/4096)*3.3) * 2.19186 * 100);

	if(BatVoltag < 320)
	{
	  if(LED_ToggleCnt > 500)
	  {
	  	LED_ToggleCnt = 0;
	  
		LED_Toggle ^= 1;
		if(LED_Toggle)	out_LCM_LED2(_ON)
		else			out_LCM_LED2(_OFF);
	  }
	  else
	  	LED_ToggleCnt++;		
	}
	else
		out_LCM_LED2(_OFF);

	osDelay(1);
    #if 0
    	calib_state = GetCalibState_BNO055();
		GetCalibData_BNO055(calib_data);
    	osDelay(500); // ���� �ð� �������� ���ڱ� Ķ���극�̼� ���¸� Ȯ��
    #endif

	#if 1
  		BNO055_Read1();
		osDelay(50);
  	#endif
  }

  #else
  OUT_ADD_SEL(Bit_RESET);
  OUT_BL_IND(Bit_RESET);
  OUT_BNO055_RSTn(Bit_RESET);
  osDelay(500);
  OUT_BNO055_RSTn(Bit_SET);
  osDelay(500);			// �����̰� ª������... BNO055 ��ſ� ������ ��
  I2C_routine();
	
  bno055_init(&bno055);

// BNO055 ���� �ʱ�ȭ
  op_mode = BNO055_OPERATION_MODE_CONFIG;
  bno055_set_operation_mode(op_mode);
  osDelay(50);

    bno055.dev_addr = BNO055_I2C_ADDR1;
/*
  sendData[0] = 0x41;
  sendData[1] = 0x21;
 //sendData[1] = 0x21;
  HAL_I2C_Master_Transmit(hi2c_sensor, (uint16_t)(bno055.dev_addr << 1), sendData, 2, 1000);
  osDelay(10);

  sendData[0] = 0x42;
  sendData[1] = 0x02;
  //sendData[1] = 0x04;
  HAL_I2C_Master_Transmit(hi2c_sensor, (uint16_t)(bno055.dev_addr << 1), sendData, 2, 1000);
  osDelay(10);
  */
  for(;;)
  {

    #if 1
    	calib_state = GetCalibState_BNO055();
		GetCalibData_BNO055(calib_data);
    	osDelay(1000); // ���� �ð� �������� ���ڱ� Ķ���극�̼� ���¸� Ȯ��
    #endif

	#if 1
  		BNO055_Read1();
  	#endif

	if(calib_state == 0xFF)
		while(1);
  }

  #endif
  /* USER CODE END BNO055_Task */
}




