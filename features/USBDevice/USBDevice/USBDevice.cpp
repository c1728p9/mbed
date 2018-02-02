/* Copyright (c) 2010-2011 mbed.org, MIT License
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software
* and associated documentation files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or
* substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
* BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
* DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "stdint.h"

#include "USBEndpoints.h"
#include "USBDevice.h"
#include "USBDescriptor.h"
#include "mbed_shared_queues.h"
#include "USBPhyHw.h"

//#define DEBUG

/* Device status */
#define DEVICE_STATUS_SELF_POWERED  (1U<<0)
#define DEVICE_STATUS_REMOTE_WAKEUP (1U<<1)

/* Endpoint status */
#define ENDPOINT_STATUS_HALT        (1U<<0)

/* Standard feature selectors */
#define DEVICE_REMOTE_WAKEUP        (1)
#define ENDPOINT_HALT               (0)

/* Macro to convert wIndex endpoint number to physical endpoint number */
#define WINDEX_TO_PHYSICAL(endpoint) (endpoint)
#define EP_TO_INDEX(endpoint)   ((((endpoint & 0xf) << 1) | (endpoint & 0x80 ? 1 : 0)) - 2)


bool USBDevice::request_get_descriptor(void)
{
    bool success = false;
#ifdef DEBUG
    printf("get descr: type: %d\r\n", DESCRIPTOR_TYPE(transfer.setup.wValue));
#endif
    switch (DESCRIPTOR_TYPE(transfer.setup.wValue))
    {
        case DEVICE_DESCRIPTOR:
            if (device_desc() != NULL)
            {
                if ((device_desc()[0] == DEVICE_DESCRIPTOR_LENGTH) \
                    && (device_desc()[1] == DEVICE_DESCRIPTOR))
                {
#ifdef DEBUG
                    printf("device descr\r\n");
#endif
                    transfer.remaining = DEVICE_DESCRIPTOR_LENGTH;
                    transfer.ptr = (uint8_t*)device_desc();
                    transfer.direction = DEVICE_TO_HOST;
                    success = true;
                }
            }
            break;
        case CONFIGURATION_DESCRIPTOR:
            if (configuration_desc() != NULL)
            {
                if ((configuration_desc()[0] == CONFIGURATION_DESCRIPTOR_LENGTH) \
                    && (configuration_desc()[1] == CONFIGURATION_DESCRIPTOR))
                {
#ifdef DEBUG
                    printf("conf descr request\r\n");
#endif
                    /* Get wTotalLength */
                    transfer.remaining = configuration_desc()[2] \
                        | (configuration_desc()[3] << 8);

                    transfer.ptr = (uint8_t*)configuration_desc();
                    transfer.direction = DEVICE_TO_HOST;
                    success = true;
                }
            }
            break;
        case STRING_DESCRIPTOR:
#ifdef DEBUG
            printf("str descriptor\r\n");
#endif
            switch (DESCRIPTOR_INDEX(transfer.setup.wValue))
            {
                            case STRING_OFFSET_LANGID:
#ifdef DEBUG
                                printf("1\r\n");
#endif
                                transfer.remaining = string_langid_desc()[0];
                                transfer.ptr = (uint8_t*)string_langid_desc();
                                transfer.direction = DEVICE_TO_HOST;
                                success = true;
                                break;
                            case STRING_OFFSET_IMANUFACTURER:
#ifdef DEBUG
                                printf("2\r\n");
#endif
                                transfer.remaining =  string_imanufacturer_desc()[0];
                                transfer.ptr = (uint8_t*)string_imanufacturer_desc();
                                transfer.direction = DEVICE_TO_HOST;
                                success = true;
                                break;
                            case STRING_OFFSET_IPRODUCT:
#ifdef DEBUG
                                printf("3\r\n");
#endif
                                transfer.remaining = string_iproduct_desc()[0];
                                transfer.ptr = (uint8_t*)string_iproduct_desc();
                                transfer.direction = DEVICE_TO_HOST;
                                success = true;
                                break;
                            case STRING_OFFSET_ISERIAL:
#ifdef DEBUG
                                printf("4\r\n");
#endif
                                transfer.remaining = string_iserial_desc()[0];
                                transfer.ptr = (uint8_t*)string_iserial_desc();
                                transfer.direction = DEVICE_TO_HOST;
                                success = true;
                                break;
                            case STRING_OFFSET_ICONFIGURATION:
#ifdef DEBUG
                                printf("5\r\n");
#endif
                                transfer.remaining = string_iconfiguration_desc()[0];
                                transfer.ptr = (uint8_t*)string_iconfiguration_desc();
                                transfer.direction = DEVICE_TO_HOST;
                                success = true;
                                break;
                            case STRING_OFFSET_IINTERFACE:
#ifdef DEBUG
                                printf("6\r\n");
#endif
                                transfer.remaining = string_iinterface_desc()[0];
                                transfer.ptr = (uint8_t*)string_iinterface_desc();
                                transfer.direction = DEVICE_TO_HOST;
                                success = true;
                                break;
            }
            break;
        case INTERFACE_DESCRIPTOR:
#ifdef DEBUG
            printf("interface descr\r\n");
#endif
        case ENDPOINT_DESCRIPTOR:
#ifdef DEBUG
            printf("endpoint descr\r\n");
#endif
            /* TODO: Support is optional, not implemented here */
            break;
        default:
#ifdef DEBUG
            printf("ERROR\r\n");
#endif
            break;
    }

    return success;
}

void USBDevice::decode_setup_packet(uint8_t *data, setup_packet_t *packet)
{
    /* Fill in the elements of a setup_packet_t structure from raw data */
    packet->bmRequestType.dataTransferDirection = (data[0] & 0x80) >> 7;
    packet->bmRequestType.Type = (data[0] & 0x60) >> 5;
    packet->bmRequestType.Recipient = data[0] & 0x1f;
    packet->bRequest = data[1];
    packet->wValue = (data[2] | (uint16_t)data[3] << 8);
    packet->wIndex = (data[4] | (uint16_t)data[5] << 8);
    packet->wLength = (data[6] | (uint16_t)data[7] << 8);
}


bool USBDevice::control_out(void)
{
    /* Control transfer data OUT stage */
    uint8_t buffer[MAX_PACKET_SIZE_EP0];
    uint32_t packetSize;

    /* Check we should be transferring data OUT */
    if (transfer.direction != HOST_TO_DEVICE)
    {
         /* for other platforms, count on the HAL to handle this case */
         return false;
    }

    /* Read from endpoint */
    packetSize = phy->ep0_read_result(buffer);

    /* Check if transfer size is valid */
    if (packetSize > transfer.remaining)
    {
        /* Too big */
        return false;
    }

    /* Update transfer */
    transfer.ptr += packetSize;
    transfer.remaining -= packetSize;

    /* Check if transfer has completed */
    if (transfer.remaining == 0)
    {
        /* Transfer completed */
        if (transfer.notify)
        {
            /* Notify class layer. */
            USBCallback_requestCompleted(buffer, packetSize);
            transfer.notify = false;
        }
        /* Status stage */
        transfer.stage = CTRL_STAGE_STATUS;
        phy->ep0_write(NULL, 0);
    }
    else
    {
        phy->ep0_read();
    }

    return true;
}

bool USBDevice::control_in(void)
{
    /* Control transfer data IN stage */
    uint32_t packetSize;


    /* Check we should be transferring data IN */
    if (transfer.direction != DEVICE_TO_HOST)
    {
        return false;
    }

    if ((transfer.remaining == 0) && transfer.zlp) {
        /* ZLP will be sent below */
        transfer.zlp = false;
    }

    packetSize = transfer.remaining;

    if (packetSize > MAX_PACKET_SIZE_EP0)
    {
        packetSize = MAX_PACKET_SIZE_EP0;
    }

    /* Write to endpoint */
    phy->ep0_write(transfer.ptr, packetSize);

    /* Update transfer */
    transfer.ptr += packetSize;
    transfer.remaining -= packetSize;

    /* Send status if all the data has been sent
     * NOTE - Start the status stage immediately
     * after writing the last packet. Do not wait
     * for the next IN event, as this can be dropped
     * if the ACK by the host is corrupted.
     *
     * For more info on this see section
     * 8.5.3.2 of the USB2.0 specification.
     */
    if ((transfer.remaining == 0) && !transfer.zlp)
    {
        /* Transfer completed */
        if (transfer.notify)
        {
            /* Notify class layer. */
            USBCallback_requestCompleted(NULL, 0);
            transfer.notify = false;
        }
        /* Status stage */
        transfer.stage = CTRL_STAGE_STATUS;
        phy->ep0_read();

        /* Completed */
        return true;
    }


    return true;
}

bool USBDevice::request_set_address(void)
{
    /* Set the device address */
    phy->set_address(transfer.setup.wValue);

    if (transfer.setup.wValue == 0)
    {
        device.state = DEFAULT;
    }
    else
    {
        device.state = ADDRESS;
    }

    return true;
}

bool USBDevice::request_set_configuration(void)
{

    device.configuration = transfer.setup.wValue;
    /* Set the device configuration */
    if (device.configuration == 0)
    {
        /* Not configured */
        phy->unconfigure();
        device.state = ADDRESS;
    }
    else
    {
        if (USBCallback_setConfiguration(device.configuration))
        {
            /* Valid configuration */
            phy->configure();
            device.state = CONFIGURED;
        }
        else
        {
            return false;
        }
    }

    return true;
}

bool USBDevice::request_get_configuration(void)
{
    /* Send the device configuration */
    transfer.ptr = &device.configuration;
    transfer.remaining = sizeof(device.configuration);
    transfer.direction = DEVICE_TO_HOST;
    return true;
}

bool USBDevice::request_get_interface(void)
{
    /* Return the selected alternate setting for an interface */

    if (device.state != CONFIGURED)
    {
        return false;
    }

    /* Send the alternate setting */
    transfer.setup.wIndex = current_interface;
    transfer.ptr = &current_alternate;
    transfer.remaining = sizeof(current_alternate);
    transfer.direction = DEVICE_TO_HOST;
    return true;
}

bool USBDevice::request_set_interface(void)
{
    bool success = false;
    if(USBCallback_setInterface(transfer.setup.wIndex, transfer.setup.wValue))
    {
        success = true;
        current_interface = transfer.setup.wIndex;
        current_alternate = transfer.setup.wValue;
    }
    return success;
}

bool USBDevice::request_set_feature()
{
    bool success = false;

    if (device.state != CONFIGURED)
    {
        /* Endpoint or interface must be zero */
        if (transfer.setup.wIndex != 0)
        {
            return false;
        }
    }

    switch (transfer.setup.bmRequestType.Recipient)
    {
        case DEVICE_RECIPIENT:
            /* TODO: Remote wakeup feature not supported */
            break;
        case ENDPOINT_RECIPIENT:
            if (transfer.setup.wValue == ENDPOINT_HALT)
            {
                /* TODO: We should check that the endpoint number is valid */
                phy->endpoint_stall(
                    WINDEX_TO_PHYSICAL(transfer.setup.wIndex));
                success = true;
            }
            break;
        default:
            break;
    }

    return success;
}

bool USBDevice::request_clear_feature()
{
    bool success = false;

    if (device.state != CONFIGURED)
    {
        /* Endpoint or interface must be zero */
        if (transfer.setup.wIndex != 0)
        {
            return false;
        }
    }

    switch (transfer.setup.bmRequestType.Recipient)
    {
        case DEVICE_RECIPIENT:
            /* TODO: Remote wakeup feature not supported */
            break;
        case ENDPOINT_RECIPIENT:
            /* TODO: We should check that the endpoint number is valid */
            if (transfer.setup.wValue == ENDPOINT_HALT)
            {
                phy->endpoint_unstall( WINDEX_TO_PHYSICAL(transfer.setup.wIndex));
                success = true;
            }
            break;
        default:
            break;
    }

    return success;
}

bool USBDevice::request_get_status(void)
{
    static uint16_t status;
    bool success = false;

    if (device.state != CONFIGURED)
    {
        /* Endpoint or interface must be zero */
        if (transfer.setup.wIndex != 0)
        {
            return false;
        }
    }

    switch (transfer.setup.bmRequestType.Recipient)
    {
        case DEVICE_RECIPIENT:
            /* TODO: Currently only supports self powered devices */
            status = DEVICE_STATUS_SELF_POWERED;
            success = true;
            break;
        case INTERFACE_RECIPIENT:
            status = 0;
            success = true;
            break;
        case ENDPOINT_RECIPIENT:
            //TODO - need to handle this
//            /* TODO: We should check that the endpoint number is valid */
//            if (getEndpointStallState(
//                WINDEX_TO_PHYSICAL(transfer.setup.wIndex)))
//            {
//                status = ENDPOINT_STATUS_HALT;
//            }
//            else
//            {
                status = 0;
//            }
            success = true;
            break;
        default:
            break;
    }

    if (success)
    {
        /* Send the status */
        transfer.ptr = (uint8_t *)&status; /* Assumes little endian */
        transfer.remaining = sizeof(status);
        transfer.direction = DEVICE_TO_HOST;
    }

    return success;
}

bool USBDevice::request_setup(void)
{
    bool success = false;

    /* Process standard requests */
    if ((transfer.setup.bmRequestType.Type == STANDARD_TYPE))
    {
        switch (transfer.setup.bRequest)
        {
             case GET_STATUS:
                 success = request_get_status();
                 break;
             case CLEAR_FEATURE:
                 success = request_clear_feature();
                 break;
             case SET_FEATURE:
                 success = request_set_feature();
                 break;
             case SET_ADDRESS:
                success = request_set_address();
                 break;
             case GET_DESCRIPTOR:
                 success = request_get_descriptor();
                 break;
             case SET_DESCRIPTOR:
                 /* TODO: Support is optional, not implemented here */
                 success = false;
                 break;
             case GET_CONFIGURATION:
                 success = request_get_configuration();
                 break;
             case SET_CONFIGURATION:
                 success = request_set_configuration();
                 break;
             case GET_INTERFACE:
                 success = request_get_interface();
                 break;
             case SET_INTERFACE:
                 success = request_set_interface();
                 break;
             default:
                 break;
        }
    }

    return success;
}

bool USBDevice::control_setup(void)
{
    bool success = false;

    /* Control transfer setup stage */
    uint8_t buffer[MAX_PACKET_SIZE_EP0];

    phy->ep0_setup_read_result(buffer);

    /* Initialise control transfer state */
    decode_setup_packet(buffer, &transfer.setup);
    transfer.ptr = NULL;
    transfer.remaining = 0;
    transfer.direction = 0;
    transfer.zlp = false;
    transfer.notify = false;
    transfer.stage = CTRL_STAGE_SETUP;

#ifdef DEBUG
    printf("dataTransferDirection: %d\r\nType: %d\r\nRecipient: %d\r\nbRequest: %d\r\nwValue: %d\r\nwIndex: %d\r\nwLength: %d\r\n",transfer.setup.bmRequestType.dataTransferDirection,
                                                                                                                                   transfer.setup.bmRequestType.Type,
                                                                                                                                   transfer.setup.bmRequestType.Recipient,
                                                                                                                                   transfer.setup.bRequest,
                                                                                                                                   transfer.setup.wValue,
                                                                                                                                   transfer.setup.wIndex,
                                                                                                                                   transfer.setup.wLength);
#endif

    /* Class / vendor specific */
    success = USBCallback_request();

    if (!success)
    {
        /* Standard requests */
        if (!request_setup())
        {
#ifdef DEBUG
            printf("fail!!!!\r\n");
#endif
            return false;
        }
    }

    /* Check transfer size and direction */
    if (transfer.setup.wLength>0)
    {
        if (transfer.setup.bmRequestType.dataTransferDirection \
            == DEVICE_TO_HOST)
        {
            /* IN data stage is required */
            if (transfer.direction != DEVICE_TO_HOST)
            {
                return false;
            }

            /* Transfer must be less than or equal to the size */
            /* requested by the host */
            if (transfer.remaining > transfer.setup.wLength)
            {
                transfer.remaining = transfer.setup.wLength;
            }
        }
        else
        {

            /* OUT data stage is required */
            if (transfer.direction != HOST_TO_DEVICE)
            {
                return false;
            }

            /* Transfer must be equal to the size requested by the host */
            if (transfer.remaining != transfer.setup.wLength)
            {
                return false;
            }
        }
    }
    else
    {
        /* No data stage; transfer size must be zero */
        if (transfer.remaining != 0)
        {
            return false;
        }
    }

    /* Data or status stage if applicable */
    if (transfer.setup.wLength>0)
    {
        if (transfer.setup.bmRequestType.dataTransferDirection \
            == DEVICE_TO_HOST)
        {
            /* Check if we'll need to send a zero length packet at */
            /* the end of this transfer */
            if (transfer.setup.wLength > transfer.remaining)
            {
                /* Device wishes to transfer less than host requested */
                if ((transfer.remaining % MAX_PACKET_SIZE_EP0) == 0)
                {
                    /* Transfer is a multiple of EP0 max packet size */
                    transfer.zlp = true;
                }
            }

            /* IN stage */
            transfer.stage = CTRL_STAGE_DATA;
            control_in();
        }
        else
        {
            /* OUT stage */
            transfer.stage = CTRL_STAGE_DATA;
            phy->ep0_read();
        }
    }
    else
    {
        /* Status stage */
        transfer.stage = CTRL_STAGE_STATUS;
        phy->ep0_write(NULL, 0);
    }

    return true;
}

void USBDevice::reset(void)
{
    device.state = DEFAULT;
    device.configuration = 0;
    device.suspended = false;

    /* Call class / vendor specific busReset function */
    callback_reset();
}

void USBDevice::ep0_setup(void)
{
    /* Endpoint 0 setup event */
    if (!control_setup())
    {
        /* Protocol stall */
        phy->ep0_stall();
    }

    /* Return true if an OUT data stage is expected */
}

void USBDevice::ep0_out(void)
{
    if (transfer.stage == CTRL_STAGE_STATUS) {
        // No action needed on status stage
        return;
    }

    /* Endpoint 0 OUT data event */
    if (!control_out())
    {
        /* Protocol stall; this will stall both endpoints */
        phy->ep0_stall();
    }
}

void USBDevice::ep0_in(void)
{
#ifdef DEBUG
    printf("ep0_in\r\n");
#endif
    if (transfer.stage == CTRL_STAGE_STATUS) {
        // No action needed on status stage
        return;
    }

    /* Endpoint 0 IN data event */
    if (!control_in())
    {
        /* Protocol stall; this will stall both endpoints */
        phy->ep0_stall();
    }
}

void USBDevice::out_callback(uint8_t endpoint)
{
    (this->*(ep_callback[EP_TO_INDEX(endpoint)]))();
}

void USBDevice::in_callback(uint8_t endpoint)
{
    (this->*(ep_callback[EP_TO_INDEX(endpoint)]))();
}

bool USBDevice::configured(void)
{
    /* Returns true if device is in the CONFIGURED state */
    return (device.state == CONFIGURED);
}

void USBDevice::connect(bool blocking)
{
    /* Connect device */
    phy->connect();

    if (blocking) {
        /* Block if not configured */
        while (!configured());
    }
}

void USBDevice::disconnect(void)
{
    /* Disconnect device */
    phy->disconnect();
    
    /* Set initial device state */
    device.state = POWERED;
    device.configuration = 0;
    device.suspended = false;
}

control_transfer_t * USBDevice::get_transfer_ptr(void)
{
    return &transfer;
}

bool USBDevice::endpoint_add(uint8_t endpoint, uint32_t maxPacket, uint8_t type, ep_cb_t callback)
{
    if (phy->endpoint_add(endpoint, maxPacket, type)) {
        ep_callback[EP_TO_INDEX(endpoint)] = callback;
        return true;
    }
    return false;
}

bool USBDevice::endpoint_remove(uint8_t endpoint)
{
    ep_callback[EP_TO_INDEX(endpoint)] = NULL;
    return phy->endpoint_remove(endpoint);
}

void USBDevice::endpoint_stall(uint8_t endpoint)
{
    return phy->endpoint_stall(endpoint);
}

void USBDevice::endpoint_unstall(uint8_t endpoint)
{
    return phy->endpoint_unstall(endpoint);
}

uint8_t * USBDevice::find_descriptor(uint8_t descriptorType)
{
    /* Find a descriptor within the list of descriptors */
    /* following a configuration descriptor. */
    uint16_t wTotalLength;
    uint8_t *ptr;

    if (configuration_desc() == NULL)
    {
        return NULL;
    }

    /* Check this is a configuration descriptor */
    if ((configuration_desc()[0] != CONFIGURATION_DESCRIPTOR_LENGTH) \
            || (configuration_desc()[1] != CONFIGURATION_DESCRIPTOR))
    {
        return NULL;
    }

    wTotalLength = configuration_desc()[2] | (configuration_desc()[3] << 8);

    /* Check there are some more descriptors to follow */
    if (wTotalLength <= (CONFIGURATION_DESCRIPTOR_LENGTH+2))
    /* +2 is for bLength and bDescriptorType of next descriptor */
    {
        return NULL;
    }

    /* Start at first descriptor after the configuration descriptor */
    ptr = &(((uint8_t*)configuration_desc())[CONFIGURATION_DESCRIPTOR_LENGTH]);

    do {
        if (ptr[1] /* bDescriptorType */ == descriptorType)
        {
            /* Found */
            return ptr;
        }

        /* Skip to next descriptor */
        ptr += ptr[0]; /* bLength */
    } while (ptr < (configuration_desc() + wTotalLength));

    /* Reached end of the descriptors - not found */
    return NULL;
}


void USBDevice::connect_changed(unsigned int connected)
{
}

void USBDevice::suspend_changed(unsigned int suspended)
{
}


USBDevice::USBDevice(USBPhy *phy, uint16_t vendor_id, uint16_t product_id, uint16_t product_release) {
    this->vendor_id = vendor_id;
    this->product_id = product_id;
    this->product_release = product_release;

    this->phy = phy;
    current_interface = 0;
    current_alternate = 0;
    queue = mbed_highprio_event_queue();

    /* Set initial device state */
    device.state = POWERED;
    device.configuration = 0;
    device.suspended = false;

    this->phy->init(this);
};

USBDevice::USBDevice(uint16_t vendor_id, uint16_t product_id, uint16_t product_release) {
    static USBPhyHw hw_phy;
    this->vendor_id = vendor_id;
    this->product_id = product_id;
    this->product_release = product_release;

    this->phy = &hw_phy;
    current_interface = 0;
    current_alternate = 0;
    queue = mbed_highprio_event_queue();

    /* Set initial device state */
    device.state = POWERED;
    device.configuration = 0;
    device.suspended = false;

    this->phy->init(this);
};


bool USBDevice::readStart(uint8_t endpoint, uint32_t maxSize)
{
    return phy->endpoint_read(endpoint, maxSize) == EP_PENDING;
}


bool USBDevice::write(uint8_t endpoint, uint8_t * buffer, uint32_t size, uint32_t maxSize)
{
    EP_STATUS result;

    if (size > maxSize)
    {
        return false;
    }


    if(!configured()) {
        return false;
    }

    /* Send report */
    result = phy->endpoint_write(endpoint, buffer, size);

    if (result != EP_PENDING)
    {
        return false;
    }

    /* Wait for completion */
    do {
        result = phy->endpoint_write_result(endpoint);
    } while ((result == EP_PENDING) && configured());

    return (result == EP_COMPLETED);
}


bool USBDevice::writeNB(uint8_t endpoint, uint8_t * buffer, uint32_t size, uint32_t maxSize)
{
    EP_STATUS result;

    if (size > maxSize)
    {
        return false;
    }

    if(!configured()) {
        return false;
    }

    /* Send report */
    result = phy->endpoint_write(endpoint, buffer, size);

    if (result != EP_PENDING)
    {
        return false;
    }

    result = phy->endpoint_write_result(endpoint);

    return (result == EP_COMPLETED);
}



bool USBDevice::readEP(uint8_t endpoint, uint8_t * buffer, uint32_t * size, uint32_t maxSize)
{
    EP_STATUS result;

    if(!configured()) {
        return false;
    }

    /* Wait for completion */
    do {
        result = phy->endpoint_read_result(endpoint, buffer, size);
    } while ((result == EP_PENDING) && configured());

    return (result == EP_COMPLETED);
}


bool USBDevice::readEP_NB(uint8_t endpoint, uint8_t * buffer, uint32_t * size, uint32_t maxSize)
{
    EP_STATUS result;

    if(!configured()) {
        return false;
    }

    result = phy->endpoint_read_result(endpoint, buffer, size);

    return (result == EP_COMPLETED);
}



const uint8_t * USBDevice::device_desc() {
    uint8_t device_descriptor_temp[] = {
        DEVICE_DESCRIPTOR_LENGTH,       /* bLength */
        DEVICE_DESCRIPTOR,              /* bDescriptorType */
        LSB(USB_VERSION_2_0),           /* bcdUSB (LSB) */
        MSB(USB_VERSION_2_0),           /* bcdUSB (MSB) */
        0x00,                           /* bDeviceClass */
        0x00,                           /* bDeviceSubClass */
        0x00,                           /* bDeviceprotocol */
        MAX_PACKET_SIZE_EP0,            /* bMaxPacketSize0 */
        (uint8_t)(LSB(vendor_id)),                 /* idVendor (LSB) */
        (uint8_t)(MSB(vendor_id)),                 /* idVendor (MSB) */
        (uint8_t)(LSB(product_id)),                /* idProduct (LSB) */
        (uint8_t)(MSB(product_id)),                /* idProduct (MSB) */
        (uint8_t)(LSB(product_release)),           /* bcdDevice (LSB) */
        (uint8_t)(MSB(product_release)),           /* bcdDevice (MSB) */
        STRING_OFFSET_IMANUFACTURER,    /* iManufacturer */
        STRING_OFFSET_IPRODUCT,         /* iProduct */
        STRING_OFFSET_ISERIAL,          /* iSerialNumber */
        0x01                            /* bNumConfigurations */
    };
    MBED_ASSERT(sizeof(device_descriptor_temp) == sizeof(device_descriptor));
    memcpy(device_descriptor, device_descriptor_temp, sizeof(device_descriptor));
    return device_descriptor;
}

const uint8_t * USBDevice::string_langid_desc() {
    static const uint8_t string_langid_descriptor[] = {
        0x04,               /*bLength*/
        STRING_DESCRIPTOR,  /*bDescriptorType 0x03*/
        0x09,0x04,          /*bString Lang ID - 0x0409 - English*/
    };
    return string_langid_descriptor;
}

const uint8_t * USBDevice::string_imanufacturer_desc() {
    static const uint8_t string_imanufacturer_descriptor[] = {
        0x12,                                            /*bLength*/
        STRING_DESCRIPTOR,                               /*bDescriptorType 0x03*/
        'm',0,'b',0,'e',0,'d',0,'.',0,'o',0,'r',0,'g',0, /*bString iManufacturer - mbed.org*/
    };
    return string_imanufacturer_descriptor;
}

const uint8_t * USBDevice::string_iserial_desc() {
    static const uint8_t string_iserial_descriptor[] = {
        0x16,                                                           /*bLength*/
        STRING_DESCRIPTOR,                                              /*bDescriptorType 0x03*/
        '0',0,'1',0,'2',0,'3',0,'4',0,'5',0,'6',0,'7',0,'8',0,'9',0,    /*bString iSerial - 0123456789*/
    };
    return string_iserial_descriptor;
}

const uint8_t * USBDevice::string_iconfiguration_desc() {
    static const uint8_t string_iconfiguration_descriptor[] = {
        0x06,               /*bLength*/
        STRING_DESCRIPTOR,  /*bDescriptorType 0x03*/
        '0',0,'1',0,        /*bString iConfiguration - 01*/
    };
    return string_iconfiguration_descriptor;
}

const uint8_t * USBDevice::string_iinterface_desc() {
    static const uint8_t string_iinterface_descriptor[] = {
        0x08,               /*bLength*/
        STRING_DESCRIPTOR,  /*bDescriptorType 0x03*/
        'U',0,'S',0,'B',0,  /*bString iInterface - USB*/
    };
    return string_iinterface_descriptor;
}

const uint8_t * USBDevice::string_iproduct_desc() {
    static const uint8_t string_iproduct_descriptor[] = {
        0x16,                                                       /*bLength*/
        STRING_DESCRIPTOR,                                          /*bDescriptorType 0x03*/
        'U',0,'S',0,'B',0,' ',0,'D',0,'E',0,'V',0,'I',0,'C',0,'E',0 /*bString iProduct - USB DEVICE*/
    };
    return string_iproduct_descriptor;
}

void USBDevice::start_process() {
    queue->call(this, &USBDevice::usbisr_thread);
}

void USBDevice::usbisr_thread() {
    phy->process();
}
