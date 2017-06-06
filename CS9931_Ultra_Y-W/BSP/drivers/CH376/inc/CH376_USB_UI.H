#ifndef __CH376_USB_UI_H__
#define __CH376_USB_UI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include  <stdint.h>
#include  "string.h"
#include	"FILE_SYS.H"
#include  "HAL.H"
	
/* ********************************************************************************************************************* */
/* 常用类型和常量定义 */

#ifndef		TRUE
#define		TRUE	1
#define		FALSE	0
#endif
#ifndef		NULL
#define		NULL	0
#endif
#ifndef		SUCCESS
#define		SUCCESS	1
#define		FAIL   	0
#endif
	
#define   MAX_RETERY      (10)
#define   UNKNOWDEVICE    (255)

#ifndef UINT8
typedef unsigned char                UINT8;
#endif
#ifndef UINT16
typedef unsigned short               UINT16;
#endif
#ifndef UINT32
typedef unsigned long                UINT32;
#endif
#ifndef PUINT8
typedef unsigned char               *PUINT8;
#endif
#ifndef PUINT16
typedef unsigned short              *PUINT16;
#endif
#ifndef PUINT32
typedef unsigned long               *PUINT32;
#endif
#ifndef UINT8V
typedef unsigned char volatile       UINT8V;
#endif
#ifndef PUINT8V
typedef unsigned char volatile      *PUINT8V;
#endif

uint8_t OPEN_FILE(char *fliepath);
uint8_t WAIT_DEVICE_CONNECT(void);
uint8_t USB_DEVICE_INIT(void);

#endif
