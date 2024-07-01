#include "DW1000Ng.h"
#include "DW1000NgUtils.h"
#include "DW1000NgTime.h"
#include "DW1000NgConstants.h"
#include "board_driver.h"
#include <stdio.h>
#include "bno055.h"
#include "main.h"
void transmitPoll();

//jhw

uint16_t successRangingCount = 0;
uint32_t rangingCountPeriod = 0;
unsigned int prev_succeed_HAL_GetTick;


#define PRINT_PERIOD 100
#define ANCHOR_NO 20 // 앵커의 개수

unsigned int all_null_millis; // 앵커가 모두 인식안되기 시작한 시각
unsigned int received_millis[ANCHOR_NO];
int distance[ANCHOR_NO];

int id = 0;

// connection pins
const unsigned char PIN_SCK = 18;
const unsigned char PIN_MOSI = 23;
const unsigned char PIN_MISO = 19;
const unsigned char PIN_SS = 2;
const unsigned char PIN_RST = 15;
const unsigned char PIN_IRQ = 17;

// messages used in the ranging protocol
// TODO replace by enum
#define POLL 0
#define POLL_ACK 1
#define RANGE 2
#define RANGE_REPORT 3
#define RANGE_FAILED 255
// message flow state
volatile char expectedMsgId = POLL_ACK;
// message sent/received state
volatile bool sentAck = false;
volatile bool receivedAck = false;
// timestamps to remember
uint64_t timePollSent;
uint64_t timePollAckReceived;
uint64_t timeRangeSent;
// data buffer
#define LEN_DATA 30
char data[LEN_DATA];
// watchdog and reset period
uint32_t lastActivity;
//uint32_t resetPeriod = 250;
uint32_t resetPeriod = 100;
// reply times (same on both sides for symm. ranging)
unsigned short replyDelayTimeUS = 3000;

device_configuration_t DEFAULT_CONFIG = {
    false,
    true,
    true,
    true,
    false,
    STANDARD_SFD,
    CHANNEL_5,
    RATE_850KBPS,
    FREQ_16MHZ,
    LEN_256,
    CODE_3
};

interrupt_configuration_t DEFAULT_INTERRUPT_CONFIG = {
  true,
  true,
  true,
  false,
  true
};

void noteActivity()
{
  // update activity timestamp, so that we do not reach "resetPeriod"
  lastActivity = GetTick();
}

void resetInactive()
{
  // tag sends POLL and listens for POLL_ACK
  expectedMsgId = POLL_ACK;
  DW1000Ng_forceTRxOff();
  transmitPoll();
  noteActivity();
}

void handleSent()
{
  // status change on sent success
  sentAck = true;
}

void handleReceived()
{
  // status change on received success
  receivedAck = true;
}

unsigned int Tx_TagAngle = 0;
void transmitPoll()
{
  id++;
  if(id >= ANCHOR_NO )
    id = 0;

  data[16] = id;
  data[0] = POLL;

  //for ( int i = 0; i < ANCHOR_NO; i++)
  //{
  //  data[1+i*2] = distance[i]>>8;
   // data[1+i*2+1] = distance[i];
//    DW1000NgUtils_writeValueTochars(data + 1 + i * 2, distance[i], 2);
  //}
 // data[17] = (BNO055_Read1() >> 8) & 0xff;
 // data[18] = BNO055_Read1() & 0xff;
 // data[19] = (BatVoltag >> 8) & 0xff;
 // data[20] = BatVoltag & 0xff;

  DW1000Ng_setTransmitDataByte(data, LEN_DATA);
  DW1000Ng_startTransmit(IMMEDIATE);
}

void transmitRange()
{
  data[0] = RANGE;

  /* Calculation of future time */
  char futureTimechars[LENGTH_TIMESTAMP];

  timeRangeSent = DW1000Ng_getSystemTimestamp();
  timeRangeSent += DW1000NgTime_microsecondsToUWBTime(replyDelayTimeUS);
  DW1000NgUtils_writeValueTochars(futureTimechars, timeRangeSent, LENGTH_TIMESTAMP);
  DW1000Ng_setDelayedTRX(futureTimechars);
  timeRangeSent += DW1000Ng_getTxAntennaDelay();

  DW1000NgUtils_writeValueTochars(data + 1, timePollSent, LENGTH_TIMESTAMP);
  DW1000NgUtils_writeValueTochars(data + 6, timePollAckReceived, LENGTH_TIMESTAMP);
  DW1000NgUtils_writeValueTochars(data + 11, timeRangeSent, LENGTH_TIMESTAMP);
  DW1000Ng_setTransmitDataByte(data, LEN_DATA);
  DW1000Ng_startTransmit(DELAYED);
  //printf("Expect RANGE to be sent @ "); printfln(timeRangeSent.getAsFloat());
}


// ��ȣ �����.

void dwm1000_process()
{
  if (!sentAck && !receivedAck)
  {
    // check if inactive
    if (GetTick() - lastActivity > resetPeriod)
    {
      resetInactive();
    }
    return;
  }
  // continue on any success confirmation
  if (sentAck) {
    sentAck = false;
    DW1000Ng_startReceive(IMMEDIATE);
  }
  if (receivedAck)
  {
    receivedAck = false;
    // get message and parse
    DW1000Ng_getReceivedData(data, LEN_DATA);
    char msgId = data[0];
    if (msgId != expectedMsgId)
    {
      expectedMsgId = POLL_ACK;
      transmitPoll();
      return;
    }
    if (msgId == POLL_ACK)
    {
      timePollSent = DW1000Ng_getTransmitTimestamp();
      timePollAckReceived = DW1000Ng_getReceiveTimestamp();
      expectedMsgId = RANGE_REPORT;
      transmitRange();
      noteActivity();
    }


    else if (msgId == RANGE_REPORT)
    {
      float curRange;
      memcpy(&curRange, data + 1, 4);
      int id = data[16];
      distance[id] = (int)(curRange* 100.0);   
      distance[id]-=100; 

      received_millis[id] = GetTick();
      

      Global.device_ID[id] = data[16];
      Global.device_Distance_uwb[id] = distance[id];



      expectedMsgId = POLL_ACK;
      transmitPoll();
      noteActivity();
      all_null_millis = GetTick();
    }
    else if (msgId == RANGE_FAILED)
    {
      expectedMsgId = POLL_ACK;
      transmitPoll();
      noteActivity();
    }
  }
}

void init_dwm_1000_tag()
{

  // initialize the driver
  //DW1000Ng_initialize(PIN_SS, PIN_IRQ, 0xff);
  DW1000Ng_initialize(PIN_SS, PIN_IRQ, PIN_RST);
  //printf("DW1000Ng initialized ...\n\r");
  // general configuration
  DW1000Ng_applyConfiguration(DEFAULT_CONFIG);
  DW1000Ng_applyInterruptConfiguration(DEFAULT_INTERRUPT_CONFIG);

  DW1000Ng_setNetworkId(10);

  DW1000Ng_setAntennaDelay(16436);

  // DEBUG chip info and registers pretty printed
  char msg[128];
  DW1000Ng_getPrintableDeviceIdentifier(msg);
  DW1000Ng_getPrintableExtendedUniqueIdentifier(msg);
  DW1000Ng_getPrintableNetworkIdAndShortAddress(msg);
  DW1000Ng_getPrintableDeviceMode(msg);
  DW1000Ng_attachSentHandler(handleSent);
  DW1000Ng_attachReceivedHandler(handleReceived);

#ifdef USE_PA_LNA
//  DW1000Ng_set_max_5007();
#endif

  // anchor starts by transmitting a POLL message
  transmitPoll();
  noteActivity();

  for ( int i = 0; i < ANCHOR_NO; i++)
  {
    received_millis[i] = GetTick();
    distance[i] = -1000;
  }
}
void receiver() {
  DW1000Ng_forceTRxOff();
  // so we don't need to restart the receiver manually
  DW1000Ng_startReceive(IMMEDIATE);
}

void Anchor_init()
{
  DW1000Ng_initialize(PIN_SS, PIN_IRQ, PIN_RST);
  DW1000Ng_applyConfiguration(DEFAULT_CONFIG);
  DW1000Ng_applyInterruptConfiguration(DEFAULT_INTERRUPT_CONFIG);
  DW1000Ng_setDeviceAddress(1);
  DW1000Ng_setAntennaDelay(16436);
  DW1000Ng_set_max_5007();
  DW1000Ng_attachSentHandler(handleSent);
  DW1000Ng_attachReceivedHandler(handleReceived);
  // anchor starts in receiving mode, awaiting a ranging poll message

  receiver();
  noteActivity();
  
  // for first time ranging frequency computation
  rangingCountPeriod = GetTick();
  prev_succeed_HAL_GetTick = GetTick();

  for( int i = 0; i < ANCHOR_NO; i++)
  {
    distance[i] = -1000;
  }
}