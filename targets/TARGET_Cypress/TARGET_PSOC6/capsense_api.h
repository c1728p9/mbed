#ifndef MBED_CAPSENSE_API_H
#define MBED_CAPSENSE_API_H

#include "device.h"

#if defined(DEVICE_CAPSENSE)

#ifdef __cplusplus
extern "C" {
#endif

void capsense_init(capsense_t *obj);
void capsense_free(void);

void capsense_enable(capsense_t *obj);
unsigned int capsense_is_busy(capsense_t *obj);
void capsense_process_all_widgets(capsense_t *obj);
void capsense_scan_all_widgets(capsense_t *obj);
unsigned int capsense_is_sensor_active(unsigned int wdgtIndex, unsigned int snsIndex, capsense_t *obj);
void capsense_register_callback(cy_en_capsense_callback_event_t callbackType,
                                cy_capsense_callback_t callbackFunction,
                                capsense_t *obj);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_CAPSENSE */

#endif /* MBED_CAPSENSE_API_H */
