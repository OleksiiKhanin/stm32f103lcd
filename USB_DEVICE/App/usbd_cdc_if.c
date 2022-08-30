/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_cdc_if.c
  * @version        : v2.0_Cube
  * @brief          : Usb device for Virtual Com Port.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_if.h"

#define RX_DATA_SIZE  1024
#define TX_DATA_SIZE  128
#define PACKET_SIZE  64

typedef struct {
	byte_ptr data;
	BaseSize_t size;
} TxData_t;

static TxData_t TxBuffer[TX_DATA_SIZE];
static u08 RxBuffer[RX_DATA_SIZE];
static u08 tempRxBuffer[PACKET_SIZE];

static BaseSize_t transmitDataSize=0;

extern USBD_HandleTypeDef hUsbDeviceFS;


static int8_t CDC_Init_FS(void);
static int8_t CDC_DeInit_FS(void);
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Receive_FS(uint8_t* pbuf, uint32_t *Len);

USBD_CDC_ItfTypeDef USBD_Interface_fops_FS = {
											  CDC_Init_FS,
											  CDC_DeInit_FS,
											  CDC_Control_FS,
											  CDC_Receive_FS
										   };

static int8_t CDC_Init_FS(void) {
	CreateDataStruct(TxBuffer, sizeof(TxBuffer[0]), TX_DATA_SIZE);
	CreateDataStruct(RxBuffer, sizeof(RxBuffer[0]), RX_DATA_SIZE);
	USBD_CDC_SetRxBuffer(&hUsbDeviceFS, tempRxBuffer);
	return (USBD_OK);
}

static int8_t CDC_DeInit_FS(void) {
  return (USBD_OK);
}


static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
  /* USER CODE BEGIN 5 */
  switch(cmd)
  {
    case CDC_SEND_ENCAPSULATED_COMMAND:

    break;

    case CDC_GET_ENCAPSULATED_RESPONSE:

    break;

    case CDC_SET_COMM_FEATURE:

    break;

    case CDC_GET_COMM_FEATURE:

    break;

    case CDC_CLEAR_COMM_FEATURE:

    break;

  /*******************************************************************************/
  /* Line Coding Structure                                                       */
  /*-----------------------------------------------------------------------------*/
  /* Offset | Field       | Size | Value  | Description                          */
  /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
  /* 4      | bCharFormat |   1  | Number | Stop bits                            */
  /*                                        0 - 1 Stop bit                       */
  /*                                        1 - 1.5 Stop bits                    */
  /*                                        2 - 2 Stop bits                      */
  /* 5      | bParityType |  1   | Number | Parity                               */
  /*                                        0 - None                             */
  /*                                        1 - Odd                              */
  /*                                        2 - Even                             */
  /*                                        3 - Mark                             */
  /*                                        4 - Space                            */
  /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
  /*******************************************************************************/
    case CDC_SET_LINE_CODING:

    break;

    case CDC_GET_LINE_CODING:

    break;

    case CDC_SET_CONTROL_LINE_STATE:

    break;

    case CDC_SEND_BREAK:

    break;

  default:
    break;
  }

  return (USBD_OK);
  /* USER CODE END 5 */
}

const void *const ReceiveUSBPackageLabel = (const void* const) RxBuffer;

static void endOfReceive() {
	#ifdef SIGNALS_TASK
		emitSignal(ReceiveUSBPackageLabel, getCurrentSizeDataStruct(RxBuffer), RxBuffer);
	#endif
	execCallBack(ReceiveUSBPackageLabel);
}

static u16 nextByteTimeout = 0;
void setReceiveTimeoutUSB(u16 tick) { // Время ожидания следующего байта
	delTimerTask((TaskMng)endOfReceive,0,NULL);
	nextByteTimeout = tick;
}

static u08 CDC_Transmit_FS() {
	USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
	if (hcdc->TxState != 0){
		return USBD_BUSY;
	}
	byte_ptr buf = allocMem(transmitDataSize);
	if(buf == NULL) return USBD_FAIL;
	transmitDataSize = 0;
	BaseSize_t count = 0;
	TxData_t local;
	while(GetFromQ(&local,TxBuffer) == EVERYTHING_IS_OK) {
		for(BaseSize_t i = 0; i<local.size; i++) {
			buf[count] = local.data[i];
			count++;
		}
	}
	USBD_CDC_SetTxBuffer(&hUsbDeviceFS, buf, count);
	u08 res = USBD_CDC_TransmitPacket(&hUsbDeviceFS);
	freeMem(buf);
	execCallBack(CDC_Transmit_FS);
	return res;
}

static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len) {
	for(u32 i = 0; i < *Len; i++) {
		PutToBackQ(Buf+i, RxBuffer);
	}
	if(nextByteTimeout && !updateTimer((TaskMng)endOfReceive,0,NULL,nextByteTimeout)) {
		SetTimerTask((TaskMng)endOfReceive,0,NULL,nextByteTimeout);
	}
	USBD_CDC_SetRxBuffer(&hUsbDeviceFS, tempRxBuffer); // create a new buffer
	USBD_CDC_ReceivePacket(&hUsbDeviceFS); // wait next data
	return (USBD_OK);
}

static byte_ptr putByteToBuf(u08 byte){
	static unsigned char currentFreeByte;
	static unsigned char aTxBuffer[PACKET_SIZE];  // Буфер передачи отдельных байтов
	unsigned char tempCount = currentFreeByte + 1;
	tempCount = (tempCount<PACKET_SIZE)?tempCount:0; // Кольцевой буфер
	aTxBuffer[currentFreeByte] = byte; // Добавляем наш байт
	unsigned char* result = aTxBuffer+currentFreeByte; // Готовим указатель на добавленный байт
	currentFreeByte = tempCount;
	return result;
}

#include "String.h"
void writeUSB(BaseSize_t sz, byte_ptr buf) {
	if(!sz) sz = strSize((string_t)buf);
	transmitDataSize += sz;
	TxData_t temp = {size: sz, data: buf};
	if (PutToBackQ(&temp, TxBuffer) != EVERYTHING_IS_OK) {
		transmitDataSize -= sz;
		delTimerTask((TaskMng)CDC_Transmit_FS,0,NULL);
		SetTask((TaskMng)CDC_Transmit_FS,0,NULL);
	}
	else if(nextByteTimeout && !updateTimer((TaskMng)CDC_Transmit_FS,0,NULL,nextByteTimeout>>1)) {
		SetTimerTask((TaskMng)CDC_Transmit_FS,0,NULL,nextByteTimeout>>1);
	}
	changeCallBackLabel(writeUSB, CDC_Transmit_FS);
}

void writeSymbUSB(u08 symb) {
	byte_ptr tempPtr = putByteToBuf(symb);
	writeUSB(1, tempPtr);
}

BaseSize_t readUSB(BaseSize_t sz, byte_ptr out) {
	BaseSize_t i = 0;
	for(; i<sz; i++) {
		if(out != NULL) {
			if(GetFromQ((out+i),RxBuffer) != EVERYTHING_IS_OK) { out[i] = 0; break;}
		} else {
			if(DelFromQ(RxBuffer) != EVERYTHING_IS_OK) break;
		}
	}
	return i;
}

void clearUSB() {
	clearDataStruct(RxBuffer);
}
