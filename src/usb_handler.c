/*
 * USB_Handler.c
 *
 *  Created on: Jun 21, 2021
 *      Author: florianfrank
 */

#include "usb_handler.h"
#include "usbh_diskio_dma.h"
#include "ff.h"
#include "usbh_def.h" 
#include "usb_time_measurement.h"


// TODO this variables could be specified locally?
FATFS USBDISKFatFs;
USBH_HandleTypeDef hUSBHost; /* USB Host handle */
extern HCD_HandleTypeDef hhcd;

/** Internally defined **/
void USB_StateCallback(USBH_HandleTypeDef *phost, uint8_t id);

/** 
 * @brief Callback function, which is invoked after a USB Host interrupt. 
 *        To detect, when devices connect, disconnect or change their state.
 */
extern void OTG_HS_IRQHandler(void)
{
	HAL_HCD_IRQHandler(&hhcd);
}


/**
 * @brief This function initializes the USB communication. It Links the driver, and runs the USB host progress.
 * @param usbHandle handle to read and write data to the USB mass storage device.
 * @return Error Handle containing USB_NO_ERROR if function was successful.
 */
USB_ERROR USB_InitConnection(USB_MS_Handle *usbHandle)
{


	if (!usbHandle)
	{
		return (USB_ERROR ) { USB_PARAM_ERROR, __LINE__ };

	}
	usbHandle->m_USBState = USB_IDLE;
	usbHandle->m_Open = FALSE;
	usbHandle->m_FileHandle = malloc(sizeof(FIL));

	// Link USB I/O driver
	if (FATFS_LinkDriver(&USBH_Driver, usbHandle->m_USBDISKPath) != 0)
	{
		return (USB_ERROR ) { USB_LINK_ERROR, __LINE__ };
	}

	// Initialize host library
	USBH_StatusTypeDef ret = USBH_Init(&hUSBHost, USB_StateCallback, 0);
	if (ret != USBH_OK)
	{
		return (USB_ERROR ) { USB_MAP_ErrCodeUSB(ret), __LINE__ };
	}

	ret = USBH_RegisterClass(&hUSBHost, USBH_MSC_CLASS);
	if (ret != USBH_OK)
	{
		return (USB_ERROR ) { USB_MAP_ErrCodeUSB(ret), __LINE__ };
	}

	// Start Host process
	ret = USBH_Start(&hUSBHost);
	if (ret != USBH_OK)
	{
		return (USB_ERROR ) {USB_MAP_ErrCodeUSB(ret), __LINE__ } ;
	}
	return (USB_ERROR ) {USB_NO_ERROR, __LINE__ } ;
}

/**
 * @brief This function deinitializes the USB communication. It Unlinks the driver and frees all resources.
 * @param usbHandle handle to read and write data to the USB mass storage device.
 * @return Error Handle containing USB_NO_ERROR if function was successful.
 */
USB_ERROR USB_DeInitConnection(USB_MS_Handle* usbHandle)
{
	if(!usbHandle)
		return (USB_ERROR) {USB_PARAM_ERROR, __LINE__};

	USB_ERROR ret = USB_CloseFile(usbHandle);
	if(ret.m_ErrCode != USB_NO_ERROR)
		return ret;
	if(FATFS_UnLinkDriver("0:") != 0)
		return (USB_ERROR ) {USB_LINK_ERROR, __LINE__ } ;
	free(usbHandle->m_FileHandle );
	//free(usbHandle);
	return (USB_ERROR ) {USB_NO_ERROR, __LINE__ } ;
}

/**
 * @brief This function executes the USB state machine. To wait until the USB device is connected.
 * @param usbHandle handle to read and write data to the USB mass storage device.
 * @param timeOut timeout in msto wait until the USB device is detected.
 * @return Error Handle containing USB_NO_ERROR if function was successful. 
 * 				 If device is not detected after timeout milliseconds return USB_TIMEOUT.
 */
USB_ERROR USB_ExecuteStateMachine(USB_MS_Handle* usbHandle, int timeoutMS)
{
	if(!usbHandle)
		return (USB_ERROR ) {USB_PARAM_ERROR, __LINE__ } ;

	int val = USB_StartTimer();
	while (usbHandle->m_USBState != USB_START) {

		/* USB Host Background task */
		hUSBHost.m_USBHandle = (void*)usbHandle;
		USBH_Process(&hUSBHost);
		if(USB_TransformClockFrequencyToMS(USB_GetTimer() - val) >= timeoutMS)
			return (USB_ERROR ) {USB_TIMEOUT, __LINE__ } ;
	}
	return (USB_ERROR ) {USB_NO_ERROR, __LINE__ } ;
}

/**
 * @brief This function mounts a detected USB drive.
 * @return Error Handle containing USB_NO_ERROR if function was successful. 
 */
USB_ERROR USB_MountDrive()
{
	return (USB_ERROR) {USB_MAP_ErrCodeFileHandling(f_mount(&USBDISKFatFs, "0:", 1)), __LINE__ };
}

/**
 * @brief This function opens a file on a already mounted USB drive.
 * @param usbHandle handle to read and write data to the USB mass storage device.
 * @param fileName name of the file to write. 
 * @param flags multiple parameters can be specified by combining them with a logical or operator. 
 * 				e.g. USB_WRITE | USB_APPEND means that the file is opened in write mode and the 
 * 				data is appended to the exiting content.
 * @return Error Handle containing USB_NO_ERROR if function was successful. 
 */
USB_ERROR USB_OpenFile(USB_MS_Handle* usbHandle, const char* fileName, int flags)
{
	int fopenFlag = 0x00;
	if((flags & USB_READ) >= USB_READ)
		fopenFlag |= FA_READ;
	if((flags & USB_WRITE) >= USB_WRITE)
		fopenFlag |= FA_WRITE;
	if((flags & USB_OPEN_IF_EXISTS) >= USB_OPEN_IF_EXISTS)
		fopenFlag |= FA_OPEN_EXISTING;
	if((flags & USB_CREATE_NEW) >= USB_CREATE_NEW)
		fopenFlag |= FA_CREATE_NEW;
	if((flags & USB_OVERWRITE) >= USB_OVERWRITE)
		fopenFlag |= FA_CREATE_ALWAYS;
	if((flags & USB_CREATE_OR_OPEN) >= USB_CREATE_OR_OPEN)
		fopenFlag |= FA_OPEN_ALWAYS;
	if((flags & USB_APPEND) >= USB_APPEND)
		fopenFlag |= FA_OPEN_APPEND;


	if(!usbHandle)
		return (USB_ERROR) {USB_PARAM_ERROR, __LINE__};

	USB_ERROR ret = (USB_ERROR) {USB_MAP_ErrCodeFileHandling(f_open((FIL*)usbHandle->m_FileHandle, fileName, fopenFlag)), __LINE__ };
	if(ret.m_ErrCode == USB_NO_ERROR)
		usbHandle->m_Open = TRUE;
	return ret;
}

/**
 * @brief This function closes an openend file on the USB device.
 * @param usbHandle handle to read and write data to the USB mass storage device.
 * @return Error Handle containing USB_NO_ERROR if function was successful. 
 */
USB_ERROR USB_CloseFile(USB_MS_Handle* usbHandle)
{
	if(!usbHandle)
		return (USB_ERROR) {USB_PARAM_ERROR, __LINE__};

	if(!usbHandle->m_Open)
		return (USB_ERROR) {USB_INTERFACE_CLOSED, __LINE__};

	USB_ERROR ret = (USB_ERROR) {USB_MAP_ErrCodeFileHandling(f_close((FIL*)usbHandle->m_FileHandle)), __LINE__ };
	if(ret.m_ErrCode == USB_NO_ERROR)
		usbHandle->m_Open = FALSE;
	return ret;
}


/**
 * @brief This function writes data on a file, previously opened by the USB_OpenFile function.
 * @param usbHandle handle to read and write data to the USB mass storage device.
 * @param buffer Buffer containing the data to write.
 * @param bufferLen Input: Length of the buffer to write, Output: Acutually written data.
 * @param append If append is enabled, the pointer is set to the end of the file and the data is appended to the existing content.
 * @return Error Handle containing USB_NO_ERROR if function was successful. 
 */
USB_ERROR USB_WriteData(USB_MS_Handle* usbHandle, uint8_t *buffer, uint32_t *bufferLen, BOOL append)
{
	if(!usbHandle)
		return (USB_ERROR) {USB_PARAM_ERROR, __LINE__};

	if(!usbHandle->m_Open)
		return (USB_ERROR) {USB_INTERFACE_CLOSED, __LINE__};

	if(append)
	{
		USB_ERROR_CODE ret =  USB_SetLastBufferPos(usbHandle).m_ErrCode;
		if(ret != USB_NO_ERROR)
			return (USB_ERROR) {ret, __LINE__};
	}

	uint32_t maxLen = *bufferLen;
	return (USB_ERROR) {USB_MAP_ErrCodeFileHandling(f_write((FIL*)usbHandle->m_FileHandle, buffer, maxLen, (void *)bufferLen)), __LINE__ };
}

/**
 * @brief This function sets the write pointer to the end of the file.
 * @param usbHandle handle to read and write data to the USB mass storage device.
 * @return Error Handle containing USB_NO_ERROR if function was successful. 
 * */
USB_ERROR USB_SetLastBufferPos(USB_MS_Handle* usbHandle)
{
	if(!usbHandle)
		return (USB_ERROR) {USB_INTERFACE_CLOSED, __LINE__};


	return (USB_ERROR) {USB_MAP_ErrCodeFileHandling(f_lseek((FIL*)usbHandle->m_FileHandle, f_size((FIL*)usbHandle->m_FileHandle))), __LINE__ };
}

/**
 * @brief This function reads data from a file, previously opened by the USB_OpenFile function.
 * @param usbHandle handle to read and write data to the USB mass storage device.
 * @param buffer location to copy the read data.
 * @param bufferLen Input: Length of the buffer to read, Output: Acutually read data.
 * @return Error Handle containing USB_NO_ERROR if function was successful. 
 * */
USB_ERROR USB_ReadData(USB_MS_Handle* usbHandle, uint8_t *buffer, uint32_t *len)
{
	uint32_t maxLen = *len;
	return (USB_ERROR) {USB_MAP_ErrCodeFileHandling(f_read((FIL*)usbHandle->m_FileHandle, buffer, maxLen, (void *)maxLen)), __LINE__ };
}

/**
 * @brief This function mounts the usb_device opens, writes and closes a file with one function call.
 * @param usbHandle handle to read and write data to the USB mass storage device.
 * @param filename name of the file to write.
 * @param buffer location containing the data to write.
 * @param bufferLen Input: Length of the buffer to write, Output: Acutually written data.
 * @param flags multiple parameters can be specified by combining them with a logical or operator. 
 * 				e.g. USB_WRITE | USB_APPEND means that the file is opened in write mode and the 
 * 				data is appended to the exiting content.
 * @param keepOpen If flag is true, do not close the file. This allows further write and read operations. 
 * 					The file must be closed manually outside this function.
 * @return Error Handle containing USB_NO_ERROR if function was successful. 
 * */
USB_ERROR USB_OpenWriteFile(USB_MS_Handle* usbHandle, const char* filename, uint8_t *buffer, uint32_t *bufferLen, int flags, BOOL keepOpen)
{

	/* Register the file system object to the FatFs module */
	USB_ERROR ret;
	ret =USB_MountDrive();
	if (ret.m_ErrCode != USB_NO_ERROR)
		return ret;

	ret = USB_OpenFile(usbHandle, filename, flags);
	if (ret.m_ErrCode != USB_NO_ERROR)
		return ret;

	ret = USB_WriteData(usbHandle, buffer, bufferLen, TRUE);
	if (ret.m_ErrCode != USB_NO_ERROR)
		return ret;

	if (!keepOpen)
	{
		ret = USB_CloseFile(usbHandle);
		if (ret.m_ErrCode != USB_NO_ERROR)
			return ret;
	}

	//FATFS_UnLinkDriver(usbHandle->USBDISKPath);
	return (USB_ERROR) {USB_NO_ERROR, __LINE__};
}

/**
 * @brief Internal function sets the current state of the USB interface.
 * @param phost USB Handle to set the state as local variable.
 * @param id Current state of the USB interface
 * */
void USB_StateCallback(USBH_HandleTypeDef *phost, uint8_t id)
{
	USB_MS_Handle *usbHandle= (USB_MS_Handle*)phost->m_USBHandle;
	if(!usbHandle)
		return;

	switch(id)
	{
	case HOST_USER_UNRECOVERED_ERROR:
		usbHandle->m_USBState = USB_UNRECOVERED;
		break;
	case HOST_USER_DISCONNECTION:
		usbHandle->m_USBState = USB_IDLE;
		USB_CloseFile(usbHandle);
		break;
	case HOST_USER_CLASS_ACTIVE:
		usbHandle->m_USBState = USB_START;break;
	case HOST_USER_CONNECTION:
		usbHandle->m_USBState = USB_USER_CONNECTION;
		break;
	case HOST_USER_SELECT_CONFIGURATION:
		usbHandle->m_USBState = USB_SELECT_CONFIG;
		break;
	case HOST_USER_CLASS_SELECTED:
		usbHandle->m_USBState = USB_CLASS_SELECTED;
		break;
  	default:
    	break;
  }
}

/**
 * @brief This function maps error codes of the STM32 USB Host library to the internal error codes of this library.
 * @param errCode Error code of type USB_MAP_ErrCodeUSB.
 * @return Converted error code.
 * */
USB_ERROR_CODE USB_MAP_ErrCodeUSB(int errCode){
	switch(errCode)
	{
	case USBH_OK:
		return USB_NO_ERROR;
	case USBH_BUSY:
		return USB_BUSY;
	case USBH_FAIL:
		return USB_FATAL_ERROR;
	case USBH_NOT_SUPPORTED:
		return USB_NOT_SUPPORTED;
	case USBH_UNRECOVERED_ERROR:
		return USB_FATAL_ERROR;
	case USBH_ERROR_SPEED_UNKNOWN:
		return USB_SPEED_ERROR;
	default:
		return USB_UNKNOWN_ERROR;
	}
}

/**
 * @brief This function maps error codes of the FATFs library to the internal error codes of this library.
 * @param errCode Error code of type FRESULT.
 * @return Converted error code.
 * */
USB_ERROR_CODE USB_MAP_ErrCodeFileHandling(int errCode)
{
	switch(errCode)
	{
	case FR_OK:
		return USB_NO_ERROR;
	case FR_DISK_ERR:
		return USB_DISK_ERROR;
	case FR_INT_ERR:
		return USB_INTERNAL_ERROR;
	case FR_NOT_READY:
		return USB_FILE_NOT_READ;
	case FR_NO_FILE:
		return USB_FILE_UNAVAILABLE;
	case FR_NO_PATH:
		return USB_PATH_UNAVAILABLE;
	case FR_INVALID_NAME:
		return USB_INVALID_FILE_NAME;
	case FR_DENIED:
		return USB_ACCESS_DENIED;
	case FR_EXIST:
		return USB_FILE_EXISTS;
	case FR_INVALID_OBJECT:
		return USB_INVALID_OBJECT;
	case FR_WRITE_PROTECTED:
		return USB_FILE_WRITE_PROTECTED;
	case FR_INVALID_DRIVE:
		return USB_INVALID_DRIVE;
	case FR_NOT_ENABLED:
		return USB_NOT_ENABLED;
	case FR_NO_FILESYSTEM:
		return USB_NO_FILESYSTEM;
	case FR_TIMEOUT:
		return USB_TIMEOUT;
	case FR_LOCKED:
		return USB_RESOURCE_LOCKED;
	case FR_NOT_ENOUGH_CORE:
		return USB_NOT_ENOUGH_CORE;
	case FR_TOO_MANY_OPEN_FILES:
		return USB_TO_MANY_OPEN_FILES;
	default:
		return USB_UNKNOWN_ERROR;
	}
}

/**
 * @brief This function converts USB_ERROR handles to strings. E.g. "Line 117: TIMEOUT"
 * @param err usb error code to convert
 * @return Line and error code as string representation.
 * */
const char* USB_ReturnErrorCodeStr(USB_ERROR err)
{
	switch(err.m_ErrCode)
	{
	case USB_NO_ERROR:
		sprintf(errorBuffer, "Line %d: %s\n", err.m_Line, "NO_ERROR");
		break;
	case USB_INTERFACE_CLOSED:
		sprintf(errorBuffer, "Line %d: %s\n", err.m_Line, "INTERFACE_CLOSED");
		break;
	case USB_PARAM_ERROR:
		sprintf(errorBuffer, "Line %d: %s\n", err.m_Line, "PARAM_ERROR");
		break;
	case USB_LINK_ERROR:
		sprintf(errorBuffer, "Line %d: %s\n", err.m_Line, "LINK_ERROR");
		break;
	case USB_NOT_SUPPORTED:
		sprintf(errorBuffer, "Line %d: %s\n", err.m_Line, "NOT_SUPPORTED");
		break;
	case USB_FATAL_ERROR:
		sprintf(errorBuffer, "Line %d: %s\n", err.m_Line, "FATAL_ERROR");
		break;
	case USB_BUSY:
		sprintf(errorBuffer, "Line %d: %s\n", err.m_Line, "BUSY");
		break;
	case USB_SPEED_ERROR:
		sprintf(errorBuffer, "Line %d: %s\n", err.m_Line, "SPEED_ERROR");
		break;
	case USB_UNKNOWN_ERROR:
		sprintf(errorBuffer, "Line %d: %s\n", err.m_Line, "UNKNOWN_ERROR");
		break;
	case USB_DISK_ERROR:
		sprintf(errorBuffer, "Line %d: %s\n", err.m_Line, "DISK_ERROR");
		break;
	case USB_INTERNAL_ERROR:
		sprintf(errorBuffer, "Line %d: %s\n", err.m_Line, "INTERNAL_ERROR");
		break;
	case USB_FILE_NOT_READ:
		sprintf(errorBuffer, "Line %d: %s\n", err.m_Line, "FILE_NOT_READ");
		break;
	case USB_FILE_UNAVAILABLE:
		sprintf(errorBuffer, "Line %d: %s\n", err.m_Line, "FILE_UNAVAILABLE");
		break;
	case USB_PATH_UNAVAILABLE:
		sprintf(errorBuffer, "Line %d: %s\n", err.m_Line, "PATH_UNAVAILABLE");
		break;
	case USB_INVALID_FILE_NAME:
		sprintf(errorBuffer, "Line %d: %s\n", err.m_Line, "INVALID_FILE_NAME");
		break;
	case USB_ACCESS_DENIED:
		sprintf(errorBuffer, "Line %d: %s\n", err.m_Line, "ACCESS_DENIED");
		break;
	case USB_FILE_EXISTS:
		sprintf(errorBuffer, "Line %d: %s\n", err.m_Line, "FILE_EXISTS");
		break;
	case USB_INVALID_OBJECT:
		sprintf(errorBuffer, "Line %d: %s\n", err.m_Line, "INVALID_OBJECT");
		break;
	case USB_FILE_WRITE_PROTECTED:
		sprintf(errorBuffer, "Line %d: %s\n", err.m_Line, "FILE_WRITE_PROTECTED");
		break;
	case USB_INVALID_DRIVE:
		sprintf(errorBuffer, "Line %d: %s\n", err.m_Line, "INVALID_DRIVE");
		break;
	case USB_NOT_ENABLED:
		sprintf(errorBuffer, "Line %d: %s\n", err.m_Line, "NOT_ENABLED");
		break;
	case USB_NO_FILESYSTEM:
		sprintf(errorBuffer, "Line %d: %s\n", err.m_Line, "NO_FILESYSTEM");
		break;
	case USB_TIMEOUT:
		sprintf(errorBuffer, "Line %d: %s\n", err.m_Line, "TIMEOUT");
		break;
	case USB_RESOURCE_LOCKED:
		sprintf(errorBuffer, "Line %d: %s\n", err.m_Line, "RESOURCE_LOCKED");
		break;
	case USB_NOT_ENOUGH_CORE:
		sprintf(errorBuffer, "Line %d: %s\n", err.m_Line, "NOT_ENOUGH_CORE");
		break;
	case USB_TO_MANY_OPEN_FILES:
		sprintf(errorBuffer, "Line %d: %s\n", err.m_Line, "TO_MANY_OPEN_FILES");
		break;
	default:
		return "UNKNOWN ERROR";
	}
	return errorBuffer;
}
