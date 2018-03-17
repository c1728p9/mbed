
#include "USBPhyCMSIS.h"
#include "Driver_USBD.h"

#define ALLOW_ALL_ENDPOINTS (USB_EP_ATTR_ALLOW_BULK | USB_EP_ATTR_ALLOW_INT | USB_EP_ATTR_ALLOW_ISO)

static const usb_ep_table_t endpoint_table = {
    1, // No cost per endpoint - everything allocated up front
    {
        {USB_EP_ATTR_ALLOW_CTRL | USB_EP_ATTR_DIR_IN_AND_OUT, 0, 0},
        {ALLOW_ALL_ENDPOINTS | USB_EP_ATTR_DIR_IN_AND_OUT,    0, 0},
        {ALLOW_ALL_ENDPOINTS | USB_EP_ATTR_DIR_IN_AND_OUT,    0, 0},
        {ALLOW_ALL_ENDPOINTS | USB_EP_ATTR_DIR_IN_AND_OUT,    0, 0},
        {ALLOW_ALL_ENDPOINTS | USB_EP_ATTR_DIR_IN_AND_OUT,    0, 0},
        {ALLOW_ALL_ENDPOINTS | USB_EP_ATTR_DIR_IN_AND_OUT,    0, 0},
        {ALLOW_ALL_ENDPOINTS | USB_EP_ATTR_DIR_IN_AND_OUT,    0, 0},
        {ALLOW_ALL_ENDPOINTS | USB_EP_ATTR_DIR_IN_AND_OUT,    0, 0},
        {ALLOW_ALL_ENDPOINTS | USB_EP_ATTR_DIR_IN_AND_OUT,    0, 0},
        {ALLOW_ALL_ENDPOINTS | USB_EP_ATTR_DIR_IN_AND_OUT,    0, 0},
        {ALLOW_ALL_ENDPOINTS | USB_EP_ATTR_DIR_IN_AND_OUT,    0, 0},
        {ALLOW_ALL_ENDPOINTS | USB_EP_ATTR_DIR_IN_AND_OUT,    0, 0},
        {ALLOW_ALL_ENDPOINTS | USB_EP_ATTR_DIR_IN_AND_OUT,    0, 0},
        {ALLOW_ALL_ENDPOINTS | USB_EP_ATTR_DIR_IN_AND_OUT,    0, 0},
        {ALLOW_ALL_ENDPOINTS | USB_EP_ATTR_DIR_IN_AND_OUT,    0, 0},
        {ALLOW_ALL_ENDPOINTS | USB_EP_ATTR_DIR_IN_AND_OUT,    0, 0}
    }
};

extern const ARM_DRIVER_USBD Driver_USBD0;

extern "C" void USBD_FS_IRQ (void);

USBPhy *get_usb_phy()
{
    static USBPhyCMSIS usbphy(&Driver_USBD0, &endpoint_table,
                              USBD_FS_IRQ, USB0_IRQn);
    return &usbphy;
}

