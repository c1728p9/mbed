#include "cmsis.h"
#include "device.h"
#include "mbed_assert.h"
#include "psoc6_utils.h"
#include "mbed_error.h"
#include "capsense_api.h"

#if defined(DEVICE_CAPSENSE)

void capsense_init(capsense_t *obj)
{
    /* Capture the CSD HW block and initialize it to the default state. */
    Cy_CapSense_Init(obj->ptrContext);
}


void capsense_free(void)
{
}


void capsense_enable(capsense_t *obj)
{
    Cy_CapSense_Enable(obj->ptrContext);
}


unsigned int capsense_is_busy(capsense_t *obj)
{
    unsigned int retVal;

    retVal = Cy_CapSense_IsBusy(obj->ptrContext);

    return(retVal);
}


void capsense_process_all_widgets(capsense_t *obj)
{
    Cy_CapSense_ProcessAllWidgets(obj->ptrContext);
}


void capsense_scan_all_widgets(capsense_t *obj)
{
    Cy_CapSense_ScanAllWidgets(obj->ptrContext);
}


unsigned int capsense_is_sensor_active(unsigned int wdgtIndex, unsigned int snsIndex, capsense_t *obj)
{
    unsigned int retVal;
    retVal = Cy_CapSense_IsSensorActive(wdgtIndex, snsIndex, obj->ptrContext);

    return(retVal);
}

void capsense_register_callback(cy_en_capsense_callback_event_t callbackType,
                                cy_capsense_callback_t callbackFunction,
                                capsense_t *obj)
{
    Cy_CapSense_RegisterCallback(callbackType, callbackFunction, obj->ptrContext);
}

#endif /* DEVICE_CAPSENSE */
