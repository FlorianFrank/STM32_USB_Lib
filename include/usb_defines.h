/*
 * usb_defines.h
 *
 *  Created on: Jun 22, 2021
 *      Author: florianfrank
 */

#ifndef INC_USB_DEFINES_H_
#define INC_USB_DEFINES_H_

#include <stdint.h>

char errorBuffer[512];

typedef enum {
	USB_NO_ERROR,
	USB_INTERFACE_CLOSED,
	USB_PARAM_ERROR,
	USB_LINK_ERROR,
	USB_NOT_SUPPORTED,
	USB_FATAL_ERROR,
	USB_BUSY,
	USB_SPEED_ERROR,
	USB_UNKNOWN_ERROR,
	USB_DISK_ERROR,
	USB_INTERNAL_ERROR,
	USB_FILE_NOT_READ,
	USB_FILE_UNAVAILABLE,
	USB_PATH_UNAVAILABLE,
	USB_INVALID_FILE_NAME,
	USB_ACCESS_DENIED,
	USB_FILE_EXISTS,
	USB_INVALID_OBJECT,
	USB_FILE_WRITE_PROTECTED,
	USB_INVALID_DRIVE,
	USB_NOT_ENABLED,
	USB_NO_FILESYSTEM,
	USB_TIMEOUT,
	USB_RESOURCE_LOCKED,
	USB_NOT_ENOUGH_CORE,
	USB_TO_MANY_OPEN_FILES
} USB_ERROR_CODE;

typedef struct {
	USB_ERROR_CODE m_ErrCode;
	int m_Line;
} USB_ERROR;

typedef enum {
  USB_IDLE = 0,
  USB_START,
  USB_RUNNING,
  USB_SELECT_CONFIG,
  USB_USER_CONNECTION,
  USB_CLASS_SELECTED,
  USB_UNRECOVERED
}USB_STATE;


typedef int BOOL;

#define FALSE 0
#define TRUE 1


struct
{
	BOOL m_Open;
	void* m_FileHandle; /* File object */
	char m_USBDISKPath[4];
	volatile USB_STATE m_USBState;
}typedef USB_MS_Handle;



#define USB_READ 			0x02	/* Open file with read access. */
#define USB_WRITE 			0x04	/* Open file with write access. */
#define USB_OPEN_IF_EXISTS  0x08    /* Open if file exists else return with error. */
#define USB_CREATE_NEW		0x10	/* Create new file if not exists, else return with error. */
#define USB_OVERWRITE		0x20	/* Always create new file, overwrite old one. */
#define USB_CREATE_OR_OPEN 	0x40    /* Open existing file. Create new one if not exists. */
#define USB_APPEND			0x80 	/* Open existing, set write pointer to the end of the file. */


#endif /* INC_USB_DEFINES_H_ */
