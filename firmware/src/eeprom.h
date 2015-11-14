/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : eeprom.h
* Author             : MCD Application Team, minor modification by M. Thomas
* Version            : V1.0.1
* Date               : 10/08/2007 (modified 27. April 2009)
* Description        : This file contains all the functions prototypes for the
*                      EEPROM emulation firmware library.
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __EEPROM_H
#define __EEPROM_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"



/* Exported constants --------------------------------------------------------*/
/* Page size */
#define PAGE_SIZE  (u16)0x400  /* 1KByte */

//* EEPROM start address in Flash */
#ifdef MOD_MTHOMAS_STMLIB
extern u32 _seemul;
#define EEPROM_START_ADDRESS  ((u32)&_seemul) /* start of emulated EEPROM def'd in linker-script */
/// #define EEPROM_START_ADDRESS  (u32)(0x08000000+((128-2)*1024))
#else
#define EEPROM_START_ADDRESS  (u32)0x0800F800 /* EEPROM emulation start address:
                                              after 64KByte of used Flash memory */
#endif /* MOD_MTHOMAS_STMLIB */

/* Pages 0 and 1 base and end addresses */
#define PAGE0_BASE_ADDRESS  (u32)(EEPROM_START_ADDRESS + (u16)0x0000)
#define PAGE0_END_ADDRESS   (u32)(EEPROM_START_ADDRESS + (PAGE_SIZE - 1))

#define PAGE1_BASE_ADDRESS  (u32)(EEPROM_START_ADDRESS + PAGE_SIZE)
#define PAGE1_END_ADDRESS   (u32)(EEPROM_START_ADDRESS + (2 * PAGE_SIZE - 1))

/* Used Flash pages for EEPROM emulation */
#define PAGE0    (u16)0x0000
#define PAGE1    (u16)0x0001

/* No valid page define */
#define NO_VALID_PAGE    (u16)0x00AB

/* Page status definitions */
#define ERASED             (u16)0xFFFF      /* PAGE is empty */
#define RECEIVE_DATA       (u16)0xEEEE      /* PAGE is marked to receive data */
#define VALID_PAGE         (u16)0x0000      /* PAGE containing valid data */

/* Valid pages in read and write defines */
#define READ_FROM_VALID_PAGE    (u8)0x00
#define WRITE_IN_VALID_PAGE     (u8)0x01

/* Page full define */
#define PAGE_FULL    (u8)0x80

/* Variables' number */
#define NumbOfVar  (u8)0x0F

/* Exported types ------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
FLASH_Status EE_Format(void);
u16 EE_ReadVariable(u16 VirtAddress, u16* Read_data);
u16 EE_WriteVariable(u16 VirtAddress, u16 Data);

#endif /* __EEPROM_H */

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
