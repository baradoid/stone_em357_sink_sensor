// *******************************************************************
//  common.h
//
//  sample app for Ember Stack API
//
//  Copyright 2005 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

#include PLATFORM_HEADER //compiler/micro specifics, types

#include "stack/include/ember.h"

#include "stack/include/error.h"
#include "hal/hal.h"
#include "app/util/serial/serial.h"
#include "stack/include/packet-buffer.h"
#include "app/util/common/form-and-join.h"
#include "app/util/counters/counters.h"

#ifdef USE_BOOTLOADER_LIB
  #include "app/util/bootload/bootload-utils.h"
#endif
// *******************************************************************
// Ember endpoint and interface configuration

// The application profile ID we use.  This profile ID is assigned to
// Ember Corp. and can only be used during development.
#define PROFILE_ID 0xC00F

// Numeric index for the first endpoint. Applications can use any
// endpoints between 1 and 238.
// 0 = ZDO, 239 = SPDO, 240 = EDO, 241-255 reserved
// This application uses only one endpoint. This constant makes the code
// easier to read.
#define ENDPOINT 1

// End Ember endpoint and interface configuration
// *******************************************************************

// *******************************************************************
// Application specific constants and globals

// All nodes running the sensor or sink application will belong to the
// SENSOR MULTICAST group and will use the same location in the 
// multicast table.

#define MULTICAST_ID            0x1111
#define MULTICAST_TABLE_INDEX   0

// The sensor nodes will store the sink address, and only the sink
// address, in the address table. The sink nodes will add entries to
// the address table for all sensor nodes.

#define SINK_ADDRESS_TABLE_INDEX             0

#define HELLO_MSG_SIZE 5

// **********************************
// define the message types

#define MSG_NONE              0

// protocol for hooking up a sensor to a sink
#define MSG_SINK_ADVERTISE       1 // sink: advertise service
#define MSG_SENSOR_SELECT_SINK   2 // sensor: selects one sink
#define MSG_SINK_READY           3 // sink: tells sensor it is ready to rx data
#define MSG_SINK_QUERY           4 // sleepy end devices: ask who the sink is

// data transmission from [sensor to sink]
#define MSG_DATA             10 // sensor: tx data

// multicast hello
#define MSG_MULTICAST_HELLO 100

#define MSG_HELLO 101
#define MSG_HELLO_REPLY 102

#define MSG_CLEAR_COUNTERS 103
#define MSG_CLEAR_COUNTERS_OK 104

//
// **********************************

// serial ports
#ifndef APP_SERIAL
#define APP_SERIAL   1
#endif//APP_SERIAL

// application timers are based on quarter second intervals, each
// quarter second is measured in millisecond ticks. This value defines
// the approximate number of millisecond ticks in a quarter second.
// Account for variations in system timers.
#ifdef AVR_ATMEGA_32
#define TICKS_PER_QUARTER_SECOND 225
#else
#define TICKS_PER_QUARTER_SECOND 250
#endif

// the time a sink waits before advertising.
// This is in quarter seconds.
#define TIME_BEFORE_SINK_ADVERTISE 240 // ~60 seconds

// This define determines how much data to send. It must be multiple of 2 
// (since we're sending two-byte integers). The maximum value for 
// SEND_DATA_SIZE is determined by the maximum transport payload 
// size, which varies if security is on or off. The sendData
// function in sensor.c checks this value and adjusts it if necessary.
// The default value for this define is set to 40 bytes.
#define SEND_DATA_SIZE  40 // size sent in bytes

// This define determines how often data is collected and sent to the sink.
// The value is in quarter seconds and the default setting is 80 qs = 20 sec.
// The minimum value allowed is 2 qa = 0.5 sec
//#define SEND_DATA_RATE  20 // rate sent in quarter-seconds
//#define SEND_DATA_RATE  120 // rate sent in quarter-seconds
//#define SEND_DATA_RATE  240 // 1 min - rate sent in quarter-seconds
#define SEND_DATA_RATE  1200 // 5 min - rate sent in quarter-seconds
//#define SEND_DATA_RATE  2400 // 10 min - rate sent in quarter-seconds
//#define SEND_DATA_RATE  3600 // 15 min - rate sent in quarter-seconds

// num of pkts to miss before deciding that other node is gone
#define MISS_PACKET_TOLERANCE    10

// buffer used to build up a message before being transmitted
extern int8u globalBuffer[];

// End application specific constants and globals
// *******************************************************************

// common utility functions
void sensorCommonSetupSecurity(void);
void printAddressTable(int8u tableSize);
void printMulticastTable(int8u tableSize);
void printEUI64(int8u port, EmberEUI64* eui);
void printExtendedPanId(int8u port, int8u *extendedPanId);
void printNodeInfo(void);
void printTokens(void);
void printChildTable(void);
#if EMBER_SECURITY_LEVEL == 5
  void sensorCommonPrintKeys(void);
#endif //EMBER_SECURITY_LEVEL == 5
void sensorCommonPrint16ByteKey(int8u* key);

// it is not recommended to ever hard code the network settings
// applications should always scan to determine the channel and panid
// to use. However, in some instances, this can aid in debugging
#define USE_HARDCODED_NETWORK_SETTINGS

#ifdef USE_HARDCODED_NETWORK_SETTINGS
  // when the network settings are hardcoded - set the channel, panid, and
  // power. 
  //#define APP_CHANNEL (26)
  //#define APP_PANID   (0x01ff)
  //#define APP_EXTENDED_PANID {'s','e','n','s','o','r',0,0}

  #define APP_CHANNEL (24)
  #define APP_PANID   (0x305A)
  //#define APP_EXTENDED_PANID {0x9D,0x38,0x36,0x49,0xAE,0x9B,0xB1,0xB4}

  //#define APP_PANID   (0x405A) 
  #define APP_EXTENDED_PANID {0x9D,0x38,0x36,0x49,0xAE,0x9B,0xB1,0xFA}
#else
  // when not using hardcoded settings, it is possible to ensure that
  // devices join to each other by setting an extended PAN ID below. 
  // Leaving the extended PAN ID as "0" means "use any"
  #define APP_EXTENDED_PANID {0x9D,0x38,0x36,0x49,0xAE,0x9B,0xB1,0xFA}
#endif //USE_HARDCODED_NETWORK_SETTINGS



#define APP_POWER   (3)


#if (defined(SENSOR_APP) || defined(SINK_APP)|| defined(SLEEPY_SENSOR_APP))
// ****************************************************************
// The following function are to support sleepy end devices

// adds a JIT (just-in-time) message of type msgType for all current children.
void appAddJitForAllChildren(int8u msgType, 
                             int8u* data, 
                             int8u dataLength);

// construct and send an APS message based on a stored JIT message
// this is called from emberPollHandler when transmitExpected is TRUE
void appSendJitToChild(EmberNodeId childId);

// print the variables that keep track of the JIT messages.
// (primarily used for debugging)
void jitMessageStatus(void);

// Handle queries from end devices for the sink by sending a 
// sink advertise in response
void handleSinkQuery(EmberNodeId childId);

#if defined(SINK_APP)
//#define mainSinkFound TRUE
#define sinkEUI emberGetEui64()
#define sinkShortAddress emberGetNodeId()

#endif

#if defined(SENSOR_APP) || defined(SLEEPY_SENSOR_APP)
// TRUE when a sink is found
extern boolean mainSinkFound;

extern EmberEUI64 sinkEUI;
extern EmberNodeId sinkShortAddress;
#endif

#endif // defined(SENSOR_APP) || defined(SINK_APP)

#ifdef USE_BOOTLOADER_LIB
// ****************************************************************
// the following functions are defined to support the em250 standalone 
// bootloader in the sensor sample application. 
// 
// The sensor application only makes use of passthru bootloading and does not 
// use clone or recovery bootload modes. The solution presented is not a 
// complete solution as the command line interface on the applications only 
// supports bootloading the first child in the child table or first entry in 
// the address table. (This could be solved with either an extension to the
// command line interface, or a different model for determining the EUI to
// be bootloaded). This is meant as a reference to show what is needed
// to support bootloading in an existing application. All code related to 
// bootloading is defined under USE_BOOTLOADER_LIB.
//
// Please see the standalone-bootloader-demo sample application for a 
// complete example of using the bootloader.

// called by a parent when a child should be bootloaded. The Bootload does
// not start until the next time the child wakes up.
void bootloadMySleepyChild(EmberEUI64 targetEui);

// called by a non-sleepy to bootload a neighbor. The bootload will attempt 
// to start immediately.
void bootloadMyNeighborRouter(EmberEUI64 targetEui);

// utility to determine if a device(based on EUI) is a child or not.
boolean isMyChild(EmberEUI64 candidateEui, int8u* childIndex);

// utility to determine if a device(based on EUI) is a neighbor or not
boolean isMyNeighbor(EmberEUI64 eui);

#endif //USE_BOOTLOADER_LIB

// Do a formatted conversion of a signed fixed point 32 bit number
// to decimal.  Make lots of assumtions.  Like minDig > dot;
// minDig < 20; etc.
void formatFixed(int8u* charBuffer, int32s value, int8u minDig, int8u dot, boolean showPlus);

typedef struct{
  int32u msgNum;
  int16u paport;
  int16u vcc;
  int16u isAcPower;
  int16u temp;
  int32u impCnt[4];
}TPayLoadData;

#define GPIO_MODE_ANALOG        0x0
#define GPIO_MODE_OUT_PUSH_PULL 0x1
#define GPIO_MODE_INPUT_FLOAT   0x4
#define GPIO_MODE_OUT_OD        0x5
#define GPIO_MODE_INPUT         0x8 
#define GPIO_MODE_ALT_OUT_PU    0x9 
#define GPIO_MODE_ALT_OUT_OD    0xD


#define GPIO_PULL_DOWN  0
#define GPIO_PULL_UP    1
//GPIO_PAOUT_REG
#define configPinMode(reg,port,pin,mode) \
            reg = (reg&(~port##pin##_CFG_MASK))|(mode<<port##pin##_CFG_BIT);

#define configPinOut(port,pin, state) \
            GPIO_##port##OUT_REG = (GPIO_##port##OUT_REG&(~port##pin##_MASK))|(state<<port##pin##_BIT)

#define togglePin(port,pin) \
            GPIO_##port##OUT_REG ^= port##pin##_MASK
            
#define configPinToInputPullUpDown(reg,port,pin,pull) \
            reg = (reg&(~port##pin##_CFG_MASK))|(0x8<<port##pin##_CFG_BIT); \
            GPIO_##port##OUT_REG = (GPIO_##port##OUT_REG&(~port##pin##_MASK))|(pull<<port##pin##_BIT)

#define configPinInputPullUp(reg,port,pin)      configPinToInputPullUpDown(reg, port, pin, GPIO_PULL_UP)
#define configPinInputPullDown(reg,port,pin)    configPinToInputPullUpDown(reg, port, pin, GPIO_PULL_DOWN)

void initPins();
void configRfFrontEnd();

//work with counters
typedef struct {
  int32u pinMask;
  int32u *gpioRegInAddr;
  boolean bPulseLasteState[3];
  int32u counterValue;
} TCounterAttr;

extern TCounterAttr counterAttr[4];

void initCounters();
//boolean getPulseState(TCounterAttr *attr);
#define TYPE_NORMAL 0
#define TYPE_SLEEPY 1
//void processCounter(TCounterAttr *attr, int8u cntType);
void processCountes();
void processSleepyCountes();



void sendDataCommon(int8u type);

void joinNetworkAsSleepySensor();
void joinNetworkAsRouter();

void printTimeStamp();
void printNetInfo(EmberNetworkParameters * networkParameters);
void pintNetState();
boolean isAcPower();

void sendHelloReply(EmberEUI64 eui);
