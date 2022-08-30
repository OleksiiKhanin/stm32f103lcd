/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_cdc_if.h
  * @version        : v2.0_Cube
  * @brief          : Header for usbd_cdc_if.c file.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_CDC_IF_H__
#define __USBD_CDC_IF_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc.h"
#include "TaskMngr.h"

/** CDC Interface callback. */
extern USBD_CDC_ItfTypeDef USBD_Interface_fops_FS;

extern const void *const ReceiveUSBPackageLabel;
void setReceiveTimeoutUSB(u16 tick);

void clearUSB();

void writeUSB(BaseSize_t sz, byte_ptr buf);
void writeSymbUSB(u08 symb);
BaseSize_t readUSB(BaseSize_t sz, byte_ptr buf);


#ifdef __cplusplus
}
#endif

#endif /* __USBD_CDC_IF_H__ */

