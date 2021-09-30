/*
 * time_measurment.h
 *
 *  Created on: Jun 9, 2021
 *      Author: florianfrank
 */

#ifndef INC_TIME_MEASUREMENT_H_
#define INC_TIME_MEASUREMENT_H_

#include <stdint.h>

uint32_t USB_StartTimer();
void USB_ResetTimer();
uint32_t USB_GetTimer();
uint32_t USB_TransformClockFrequencyToNs(uint32_t value);
uint32_t USB_TransformClockFrequencyToMS(uint32_t value);

#endif /* INC_TIME_MEASUREMENT_H_ */

