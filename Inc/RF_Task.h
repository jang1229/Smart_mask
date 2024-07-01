

typedef struct _RF_Control_ {

	unsigned char Buffer[200];
	unsigned char CmdBuffer[200];
	unsigned char SensorBuffer[30];
	unsigned char RF_TXBuffer[30];


	unsigned int CmdBufferCnt;
	unsigned int BufferCnt;
	unsigned char Start_Flag;
	unsigned char Data_UpDate_Flag;

	unsigned char RF_Case;
	unsigned char RF_CmdFlag;
	unsigned char RF_ConnectionFlag;

	unsigned char Weather_Case;
	
	u8 scan_distance[20][4];
	float floatValue;

} RF_Control;

extern RF_Control	ProRF;
extern u8 Uart1RcvData;

void putChar1(unsigned char c);
void printfs1(char *s);

void UART1_Rx_Callback(void);
void RF_PROTOCOL(unsigned char Data);
int SendCommand(char *command, char *reply, uint16_t delay);
void RF_Cmd_Task(void);
void Data_Send_Task(void);
void RF_Init(void);


void RF_Task(void *argument);
