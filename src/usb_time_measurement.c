/*
 * time_measurement.c
 *
 *  Created on: Jun 9, 2021
 *      Author: florianfrank
 */

#include "usb_time_measurement.h"

#define start_timer()    *((volatile uint32_t*)0xE0001000) = 0x40000001  // Enable CYCCNT register
#define stop_timer()   *((volatile uint32_t*)0xE0001000) = 0x40000000  // Disable CYCCNT register
#define get_timer()   *((volatile uint32_t*)0xE0001004)               // Get value from CYCCNT register
#define reset_timer() *((volatile uint32_t*)0xE0001004) = 0x00        // Reset counter

#define CLOCK_FREQUENCY		168 // MHz

inline uint32_t USB_StartTimer()
{
    USB_ResetTimer();
	start_timer();
	return get_timer();
}


void USB_ResetTimer()
{
	reset_timer();
}

uint32_t USB_GetTimer()
{
	return get_timer();
}

uint32_t USB_TransformClockFrequencyToNs(uint32_t value)
{
	return (uint32_t)((float)value * (float)13.88/*5.95238095*/);
}

uint32_t USB_TransformClockFrequencyToMS(uint32_t value)
{
	return (uint32_t)((float)value * (float)13.88/*5.95238095*/)/1000/1000;
}
