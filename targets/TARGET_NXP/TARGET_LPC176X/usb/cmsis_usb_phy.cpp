
#include "USBPhyCMSIS.h"
#include "Driver_USBD.h"

static const usb_ep_table_t endpoint_table = {
    4096 - 32 * 4, // 32 words for endpoint buffers
    // +3 based added to interrupt and isochronous to ensure enough
    // space for 4 byte alignment
    {
        {USB_EP_ATTR_ALLOW_CTRL | USB_EP_ATTR_DIR_IN_AND_OUT, 1, 0},
        {USB_EP_ATTR_ALLOW_INT | USB_EP_ATTR_DIR_IN_AND_OUT,  1, 3},
        {USB_EP_ATTR_ALLOW_BULK | USB_EP_ATTR_DIR_IN_AND_OUT, 2, 0},
        {USB_EP_ATTR_ALLOW_ISO | USB_EP_ATTR_DIR_IN_AND_OUT,  1, 3},
        {USB_EP_ATTR_ALLOW_INT | USB_EP_ATTR_DIR_IN_AND_OUT,  1, 3},
        {USB_EP_ATTR_ALLOW_BULK | USB_EP_ATTR_DIR_IN_AND_OUT, 2, 0},
        {USB_EP_ATTR_ALLOW_ISO | USB_EP_ATTR_DIR_IN_AND_OUT,  1, 3},
        {USB_EP_ATTR_ALLOW_INT | USB_EP_ATTR_DIR_IN_AND_OUT,  1, 3},
        {USB_EP_ATTR_ALLOW_BULK | USB_EP_ATTR_DIR_IN_AND_OUT, 2, 0},
        {USB_EP_ATTR_ALLOW_ISO | USB_EP_ATTR_DIR_IN_AND_OUT,  1, 3},
        {USB_EP_ATTR_ALLOW_INT | USB_EP_ATTR_DIR_IN_AND_OUT,  1, 3},
        {USB_EP_ATTR_ALLOW_BULK | USB_EP_ATTR_DIR_IN_AND_OUT, 2, 0},
        {USB_EP_ATTR_ALLOW_ISO | USB_EP_ATTR_DIR_IN_AND_OUT,  1, 3},
        {USB_EP_ATTR_ALLOW_INT | USB_EP_ATTR_DIR_IN_AND_OUT,  1, 3},
        {USB_EP_ATTR_ALLOW_BULK | USB_EP_ATTR_DIR_IN_AND_OUT, 2, 0},
        {USB_EP_ATTR_ALLOW_BULK | USB_EP_ATTR_DIR_IN_AND_OUT, 2, 0}
    }
};

extern const ARM_DRIVER_USBD Driver_USBD0;

extern "C" void USBD_IRQ (void);

USBPhy *get_usb_phy()
{
    static USBPhyCMSIS usbphy(&Driver_USBD0, &endpoint_table,
                              USBD_IRQ, USB_IRQn);
    return &usbphy;
}

