# STM32_USB_Lib

A Library which allows to read and write to USB masstorage devices from STM32 boards. 

## Installation

## 1. Checkout the repository and build the library</li>

```bash
    git clone git@git.fim.uni-passau.de:frankfl/stm32_usb_lib.git
    cd stm32_usb_lib
    ./build.sh
```

## 2. Link library to an STM32 project</li>

<ul>
<li>2.1 Copy the library from stm32_usb_lib/lib/ to the linking location (e.g. <ProjectName>/lib)</li>

<li>2.3 Add the files usb_defines.h and usb_handler.h to the STM32 project</li>

<li>2.3 Open the project settings in STM32 cube IDE, specify the linking location and link the library</li>
<br>
<img src="./docs and specs/figures/Linker_Settings.png" width="600">

</ul>

## 3. Adjust the SystemClock settings

Activate PLL and set HSI (16 MHz RC oscillator) as clock source inside the **main.c**-file.

```c
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */

  	  /* Enable HSE Oscillator and activate PLL with HSE as source */
  	  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  	  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  	  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  	  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  	  RCC_OscInitStruct.PLL.PLLM = 8;
  	  RCC_OscInitStruct.PLL.PLLN = 336;
  	  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  	  RCC_OscInitStruct.PLL.PLLQ = 7;
  	  HAL_RCC_OscConfig (&RCC_OscInitStruct);

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}
```

## 4. Programming example

```c
#include "usb_handler.h"

USB_MS_Handle usbHandle;
USB_ERROR retInit = { USB_TIMEOUT, 0};

// Init USB communication
retInit = USB_InitConnection(&usbHandle);
if(retInt.m_ErrCode != USB_NO_ERROR)
    USB_ReturnErrorCodeStr(retInit)

// Wait until USB device is detected
USB_ERROR ret = { USB_TIMEOUT, 0};
do
{
    ret = USB_ExecuteStateMachine(&usbHandle, 1000);
    if(ret.m_ErrCode != USB_NO_ERROR && ret.m_ErrCode != USB_TIMEOUT)
        USB_ReturnErrorCodeStr(ret)
}while(ret.m_ErrCode != USB_NO_ERROR);

// Create or overwrite new file Test.txt on the connected masstorage device
uint32_t len = strlen("Hallo Welt!");
ret = USB_OpenWriteFile(&usbHandle, "Test.txt", (uint8_t*)"Hallo Welt!", &len, USB_OVERWRITE | USB_WRITE, FALSE);
if(ret.m_ErrCode != USB_NO_ERROR)
    USB_ReturnErrorCodeStr(ret)

// Append to an existing file and write continously
ret = USB_OpenFile(&usbHandle, "LogFile.txt", USB_APPEND | USB_WRITE);
if(ret.m_ErrCode != USB_NO_ERROR)
    USB_ReturnErrorCodeStr(ret)

uint8_t buffer[10];
memset(buffer, 0x55, 10);
uint32_t bufferLen = 10;

for(int i = 0; i < 10; i++)
{
    ret = USB_WriteData(&usbHandle, buffer, &bufferLen, true);
    if(ret.m_ErrCode != USB_NO_ERROR)
        USB_ReturnErrorCodeStr(ret)

    printf("Amount of data written %d\n", bufferLen):
}
USB_CloseFile(&usbHandle);
```
