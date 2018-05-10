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

#ifndef USBMIDI_H
#define USBMIDI_H

/* These headers are included for child class. */
#include "USBDescriptor.h"
#include "USBDevice_Types.h"

#include "USBDevice.h"
#include "MIDIMessage.h"
#include "EventFlags.h"
#include "Mutex.h"
#include "Callback.h"

#define DEFAULT_CONFIGURATION (1)

/**
* USBMIDI example
*
* @code
* #include "mbed.h"
* #include "USBMIDI.h"
*
* USBMIDI midi;
*
* int main() {
*    while (1) {
*        for(int i=48; i<83; i++) {     // send some messages!
*            midi.write(MIDIMessage::NoteOn(i));
*            wait(0.25);
*            midi.write(MIDIMessage::NoteOff(i));
*            wait(0.5);
*        }
*    }
* }
* @endcode
*/
class USBMIDI: public USBDevice {
public:

    /**
    * Constructor
    *
    * @param vendor_id Your vendor_id
    * @param product_id Your product_id
    * @param product_release Your preoduct_release
    */
    USBMIDI(uint16_t vendor_id = 0x0700, uint16_t product_id = 0x0101, uint16_t product_release = 0x0001);

    /**
     * Send a MIDIMessage
     *
     * @param m The MIDIMessage to send
     */
    bool write(MIDIMessage m);

    bool readable();

    bool read(MIDIMessage *m);

    /**
     * Attach a callback for when a MIDIEvent is received
     *
     * @param fptr function pointer
     */
    void attach(Callback<void()> callback);


protected:

    virtual void callback_state_change(DeviceState new_state);

    virtual void callback_request(const setup_packet_t *setup);

    virtual void callback_request_xfer_done(const setup_packet_t *setup, bool aborted);

    virtual void callback_set_configuration(uint8_t configuration);

    virtual void callback_set_interface(uint16_t interface, uint8_t alternate);

    virtual const uint8_t *string_iproduct_desc();

    virtual const uint8_t *string_iinterface_desc();

    virtual const uint8_t *configuration_desc(uint8_t index);

private:
    static const uint32_t MaxSize = 64;

    uint8_t _bulk_buf[MaxSize];
    uint32_t _bulk_buf_pos;
    uint32_t _bulk_buf_size;

    bool _data_ready;
    uint8_t _data[MAX_MIDI_MESSAGE_SIZE + 1];
    uint8_t _cur_data;

    rtos::EventFlags _flags;
    rtos::Mutex _write_mutex;

    usb_ep_t _bulk_in;
    usb_ep_t _bulk_out;
    uint8_t _config_descriptor[0x65];

    Callback<void()> _callback;

    void _in_callback(usb_ep_t);
    void _out_callback(usb_ep_t);
    bool _next_message();
};

#endif
