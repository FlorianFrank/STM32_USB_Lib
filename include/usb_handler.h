/*
 * usb_handler.h
 *
 *  Created on: Jun 21, 2021
 *      Author: florianfrank
 */

#ifndef INC_USB_HANDLER_H_
#define INC_USB_HANDLER_H_

#include "usb_defines.h"


USB_ERROR USB_InitConnection(USB_MS_Handle* usbHandle);
USB_ERROR USB_DeInitConnection(USB_MS_Handle* usbHandle);

/* Basic functions */
USB_ERROR USB_MountDrive();
USB_ERROR USB_OpenFile(USB_MS_Handle* usbHandle, const char* fileName, int flags);
USB_ERROR USB_CloseFile(USB_MS_Handle* usbHandle);
USB_ERROR USB_WriteData(USB_MS_Handle* usbHandle, uint8_t *buffer, uint32_t *bufferLen, BOOL append);
USB_ERROR USB_ReadData(USB_MS_Handle* usbHandle, uint8_t *buffer, uint32_t *len);
USB_ERROR USB_SetLastBufferPos(USB_MS_Handle* usbHandle);


USB_ERROR USB_OpenWriteFile(USB_MS_Handle* usbHandle, const char* filename,
		uint8_t *buffer, uint32_t *bufferLen, int flags, BOOL keepOpen);
USB_ERROR USB_ExecuteStateMachine(USB_MS_Handle* usbHandle, int timeoutMS);

/** Helper functions **/
USB_ERROR_CODE USB_MAP_ErrCodeUSB(/*USB_MAP_ErrCodeUSB*/int errCode);
USB_ERROR_CODE USB_MAP_ErrCodeFileHandling(/*FRESULT*/int errCode);

const char* USB_ReturnErrorCodeStr(USB_ERROR err);

#endif /* INC_USB_HANDLER_H_ */
