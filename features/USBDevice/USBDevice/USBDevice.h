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

#ifndef USBDEVICE_H
#define USBDEVICE_H

#include "mbed.h"
#include "USBDevice_Types.h"
#include "USBPhy.h"
#include "EventQueue.h"

class USBDevice;

struct endpoint_info_t
{
    void (USBDevice::*callback)(usb_ep_t endpoint);
    uint16_t max_packet_size;
    uint8_t flags;
    uint8_t pending;
};


class USBDevice: public  USBPhyEvents
{
public:
    typedef void (USBDevice::*ep_cb_t)(usb_ep_t endpoint);

    USBDevice(uint16_t vendor_id, uint16_t product_id, uint16_t product_release);
    USBDevice(USBPhy *phy, uint16_t vendor_id, uint16_t product_id, uint16_t product_release);

    void lock();
    void unlock();
    void assert_locked();

    /*
    * Check if the device is configured
    *
    * @returns true if configured, false otherwise
    */
    bool configured(void);

    /*
    * Connect a device
    *
    * @param blocking: block if not configured
    */
    void connect(bool blocking = true);

    /*
    * Disconnect a device
    */
    void disconnect(void);

    /*
    * Add an endpoint
    *
    * @param endpoint endpoint which will be added
    * @param maxPacket Maximum size of a packet which can be sent for this endpoint
    * @returns true if successful, false otherwise
    */
    bool endpoint_add(usb_ep_t endpoint, uint32_t maxPacket, uint8_t type=0, ep_cb_t callback=NULL);

    /*
    * Add an endpoint
    *
    * @param endpoint endpoint which will be added
    * @param maxPacket Maximum size of a packet which can be sent for this endpoint
    * @returns true if successful, false otherwise
    */
    template<typename T>
    bool endpoint_add(usb_ep_t endpoint, uint32_t maxPacket, uint8_t type, void (T::*method)(usb_ep_t endpoint)) {
        return endpoint_add(endpoint, maxPacket, type, static_cast<ep_cb_t>(method));
    }

    bool endpoint_remove(usb_ep_t endpoint);

    void endpoint_stall(usb_ep_t endpoint);

    void endpoint_unstall(usb_ep_t endpoint);

    /**
     * Get the current maximum size for this endpoint
     *
     * Return the currently configured maximum packet size, wMaxPacketSize,
     * for thie endpoint.
     */
    uint32_t endpoint_max_packet_size(usb_ep_t endpoint);

    /** Start a read on the given endpoint
     *
     * After the read is finished call read_start to get the result.
     *
     * @param endpoint endpoint to perform the read on
     * @return true if the read was started, false if no more reads can be started
     * @note Synchronization level: Interrupt safe
     */
    bool read_start(usb_ep_t endpoint);

    /**
     * Finish a read on the given endpoint
     *
     * Get the contents of a read started with read_start. To ensure all
     * the data from this endpoint is read make sure the buffer and size
     * passed is at least as big as the maximum packet for this endpoint.
     *
     * @param endpoint endpoint to read data from
     * @param buffer buffer to fill with read data
     * @param size The size of data that was read
     * @param max_size the total size of the data buffer. This bust be at least
     * the max packet size of this endpoint
     * @return true if the read was completed, otherwise false
     * @note Synchronization level: Interrupt safe
     */
    bool read_finish(usb_ep_t endpoint, uint8_t *buffer, uint32_t *size, uint32_t max_size);

    /**
     * Write a data to the given endpoint
     *
     * Write data to an endpoint.
     *
     * @param endpoint endpoint to write data to
     * @param buffer data to write
     * @param size the size of data to send. This must be less than or equal to the
     * max packet size of this endpoint
     *
     * @note Synchronization level: Interrupt safe
     */
    bool write(usb_ep_t endpoint, uint8_t *buffer, uint32_t size);

    /*
    * Get device descriptor.
    *
    * @returns pointer to the device descriptor
    */
    virtual const uint8_t * device_desc();

    /*
    * Get configuration descriptor
    *
    * @returns pointer to the configuration descriptor
    */
    virtual const uint8_t * configuration_desc(){return NULL;};

    /*
    * Get string lang id descriptor
    *
    * @return pointer to the string lang id descriptor
    */
    virtual const uint8_t * string_langid_desc();

    /*
    * Get string manufacturer descriptor
    *
    * @returns pointer to the string manufacturer descriptor
    */
    virtual const uint8_t * string_imanufacturer_desc();

    /*
    * Get string product descriptor
    *
    * @returns pointer to the string product descriptor
    */
    virtual const uint8_t * string_iproduct_desc();

    /*
    * Get string serial descriptor
    *
    * @returns pointer to the string serial descriptor
    */
    virtual const uint8_t * string_iserial_desc();

    /*
    * Get string configuration descriptor
    *
    * @returns pointer to the string configuration descriptor
    */
    virtual const uint8_t * string_iconfiguration_desc();

    /*
    * Get string interface descriptor
    *
    * @returns pointer to the string interface descriptor
    */
    virtual const uint8_t * string_iinterface_desc();

    /*
    * Get the length of the report descriptor
    *
    * @returns length of the report descriptor
    */
    virtual uint16_t report_desc_dength() { return 0; };

protected:

    /**
    * Called by USBDevice layer on bus reset.
    *
    * complete_reset must be called after
    * the device is fully reset.
    *
    * Warning: Called in ISR context
    */
    virtual void callback_reset() = 0;
    void complete_reset();

    /**
    * Called by USBDevice on Endpoint0 request.
    *
    * This is used to handle extensions to standard requests
    * and class specific requests.
    *
    * If this request is handled outside of USBDevice
    * then complete_request must be called to either
    * reject or continue this request.
    *
    * @return true if this request is handled outside
    * of USBDevice, false if handled by USBDevice.
    *
    * Warning: Called in ISR context
    */
    virtual bool callback_request() = 0;
    void complete_request(bool success);
    //void complete_request(uint8_t *data, uint32_t size, bool tx_n_rx);

    /**
    * Called by USBDevice on data stage completion
    *
    * Warning: Called in ISR context
    */
    virtual void callback_request_xfer_done() = 0;
    void complete_request_xfer_done(bool success);

    /*
    * Called by USBDevice layer in response to set_configuration.
    *
    * Upon reception of this command endpoints of the previous configuration
    * if any must be removed with endpoint_remove and new endpoint added with
    * endpoint_add.
    *
    * @param configuration Number of the configuration
    *
    * Warning: Called in ISR context
    */
    virtual void callback_set_configuration(uint8_t configuration) = 0;
    void complete_set_configuration(bool success);

    /*
    * Called by USBDevice layer in response to set_interface.
    *
    * Upon reception of this command endpoints of any previous interface
    * if any must be removed with endpoint_remove and new endpoint added with
    * endpoint_add.
    *
    * @param configuration Number of the configuration
    *
    * Warning: Called in ISR context
    */
    virtual void callback_set_interface(uint16_t interface, uint8_t alternate) = 0;
    void complete_set_interface(bool success);

    uint8_t * find_descriptor(uint8_t descriptorType);
    control_transfer_t * get_transfer_ptr(void);

    uint16_t vendor_id;
    uint16_t product_id;
    uint16_t product_release;
    uint8_t device_descriptor[18];

    endpoint_info_t endpoint_info[32 - 2];

private:
    virtual void start_process(void);
    void usbisr_thread(void);

    // USBPhyEvents
    virtual void connect_changed(unsigned int connected);
    virtual void suspend_changed(unsigned int suspended);
    virtual void sof(int frameNumber) {};
    virtual void reset(void);
    virtual void ep0_setup(void);
    virtual void ep0_out(void);
    virtual void ep0_in(void);
    virtual void out_callback(usb_ep_t endpoint);
    virtual void in_callback(usb_ep_t endpoint);

    bool request_get_descriptor(void);
    bool control_out(void);
    bool control_in(void);
    bool request_set_address(void);
    bool request_set_configuration(void);
    bool request_set_feature(void);
    bool request_clear_feature(void);
    bool request_get_status(void);
    bool request_setup(void);
    bool control_setup(void);
    void control_setup_continue();
    void decode_setup_packet(uint8_t *data, setup_packet_t *packet);
    bool request_get_configuration(void);
    bool request_get_interface(void);
    bool request_set_interface(void);


    //TODO - expose a locking API
    USBPhy *phy;
    control_transfer_t transfer;//TODO - move variables out of here
    usb_device_t device;

    uint16_t current_interface;
    uint8_t current_alternate;
    events::EventQueue *queue;
};


#endif
