/* mbed Microcontroller Library
 * Copyright (c) 2018-2018 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "USBPhyCMSIS.h"

const uint8_t EP0IN = 0x80;
const uint8_t EP0OUT = 0x00;

USBPhyCMSIS::USBPhyCMSIS(const ARM_DRIVER_USBD *phy,
                         const usb_ep_table_t *endpoint_table,
                         void (*isr_handler)(),
                         IRQn_Type irq_num)
    : _phy(phy), _table(endpoint_table), _events(NULL), _sof_on(false),
      _irq_handler(isr_handler), _irq(irq_num), _ep0_max_packet(8),
      _set_addr(false), _addr(0)
{
    _instance = this;
    NVIC_SetVector(_irq, (uint32_t)_usbisr);
}

USBPhyCMSIS::~USBPhyCMSIS()
{
    // Nothing to do
}

void USBPhyCMSIS::init(USBPhyEvents *events)
{
    _events = events;
    int32_t ret = _phy->Initialize(_event_handler_device, _event_handler_endpoint);
    MBED_ASSERT(ret == ARM_DRIVER_OK);

    if (ret == ARM_DRIVER_OK) {
        ret = _phy->PowerControl(ARM_POWER_FULL);
        MBED_ASSERT(ret == ARM_DRIVER_OK);
    }
}

void USBPhyCMSIS::deinit()
{
    int32_t ret = _phy->PowerControl(ARM_POWER_OFF);
    MBED_ASSERT(ret == ARM_DRIVER_OK);

    ret = _phy->Uninitialize();
    MBED_ASSERT(ret == ARM_DRIVER_OK);
}

bool USBPhyCMSIS::powered()
{
    ARM_USBD_STATE state = _phy->DeviceGetState();
    return state.vbus;
}

void USBPhyCMSIS::connect()
{
    int32_t ret = _phy->DeviceConnect();
    MBED_ASSERT(ret == ARM_DRIVER_OK);
}

void USBPhyCMSIS::disconnect()
{
    int32_t ret = _phy->DeviceDisconnect();
    MBED_ASSERT(ret == ARM_DRIVER_OK);
}

void USBPhyCMSIS::configure()
{
    // Nothing to do
}

void USBPhyCMSIS::unconfigure()
{
    // Nothing to do
}

void USBPhyCMSIS::sof_enable()
{
    _sof_on = true;
}

void USBPhyCMSIS::sof_disable()
{
    _sof_on = false;
}

void USBPhyCMSIS::set_address(uint8_t address)
{
    _addr = address;
    _set_addr = true;
}

void USBPhyCMSIS::remote_wakeup()
{
    int32_t ret = _phy->DeviceRemoteWakeup();
    MBED_ASSERT(ret == ARM_DRIVER_OK);
}

const usb_ep_table_t* USBPhyCMSIS::endpoint_table()
{
    return _table;
}

uint32_t USBPhyCMSIS::ep0_set_max_packet(uint32_t max_packet)
{
    _ep0_max_packet = max_packet;
    return max_packet;
}

void USBPhyCMSIS::ep0_setup_read_result(uint8_t *buffer, uint32_t size)
{
    MBED_ASSERT(size >= 8);
    int32_t ret = _phy->ReadSetupPacket(buffer);
    MBED_ASSERT(ret == ARM_DRIVER_OK);
}

void USBPhyCMSIS::ep0_read(uint8_t *data, uint32_t size)
{
    int32_t ret = _phy->EndpointTransfer(EP0OUT, data, size);
    MBED_ASSERT(ret == ARM_DRIVER_OK);
}

uint32_t USBPhyCMSIS::ep0_read_result()
{
    return _phy->EndpointTransferGetResult(EP0OUT);
}

void USBPhyCMSIS::ep0_write(uint8_t *data, uint32_t size)
{
    int32_t ret = _phy->EndpointTransfer(EP0IN, data, size);
    MBED_ASSERT(ret == ARM_DRIVER_OK);
}

void USBPhyCMSIS::ep0_stall()
{
    int32_t ret = _phy->EndpointStall(EP0IN, true);
    MBED_ASSERT(ret == ARM_DRIVER_OK);
    ret = _phy->EndpointStall(EP0OUT, true);
    MBED_ASSERT(ret == ARM_DRIVER_OK);
}

bool USBPhyCMSIS::endpoint_add(usb_ep_t endpoint, uint32_t max_packet, usb_ep_type_t type)
{
    int32_t ret = _phy->EndpointConfigure(endpoint, type, max_packet);
    MBED_ASSERT(ret == ARM_DRIVER_OK);
    return ret == ARM_DRIVER_OK;
}

void USBPhyCMSIS::endpoint_remove(usb_ep_t endpoint)
{
    _phy->EndpointUnconfigure(endpoint);
}

void USBPhyCMSIS::endpoint_stall(usb_ep_t endpoint)
{
    int32_t ret = _phy->EndpointStall(endpoint, true);
    MBED_ASSERT(ret == ARM_DRIVER_OK);
}

void USBPhyCMSIS::endpoint_unstall(usb_ep_t endpoint)
{
    int32_t ret = _phy->EndpointStall(endpoint, false);
    MBED_ASSERT(ret == ARM_DRIVER_OK);
}

bool USBPhyCMSIS::endpoint_read(usb_ep_t endpoint, uint8_t *data, uint32_t size)
{
    int32_t ret = _phy->EndpointTransfer(endpoint, data, size);
    return ret == ARM_DRIVER_OK;
}

uint32_t USBPhyCMSIS::endpoint_read_result(usb_ep_t endpoint)
{
    return _phy->EndpointTransferGetResult(endpoint);
}

bool USBPhyCMSIS::endpoint_write(usb_ep_t endpoint, uint8_t *data, uint32_t size)
{
    int32_t ret = _phy->EndpointTransfer(endpoint, data, size);
    return ret == ARM_DRIVER_OK;
}

void USBPhyCMSIS::endpoint_abort(usb_ep_t endpoint)
{
    int32_t ret = _phy->EndpointTransferAbort(endpoint);
    MBED_ASSERT(ret == ARM_DRIVER_OK);
}

void USBPhyCMSIS::process()
{
    if (_instance->_irq_handler) {
        _instance->_irq_handler();
    }
}

void USBPhyCMSIS::_usbisr()
{
    _instance->_events->start_process();
}

USBPhyCMSIS *USBPhyCMSIS::_instance = NULL;

void USBPhyCMSIS::_event_handler_device(uint32_t events)
{
    if (events & ARM_USBD_EVENT_VBUS_ON) {
        _instance->_events->power(true);
    }
    if (events & ARM_USBD_EVENT_VBUS_OFF) {
        _instance->_events->power(false);
    }
    if (events & ARM_USBD_EVENT_RESET) {
        _instance->_set_addr = false;
        _instance->_addr =  0;
        int32_t ret = _instance->_phy->EndpointConfigure(EP0IN, ARM_USB_ENDPOINT_CONTROL, _instance->_ep0_max_packet);
        MBED_ASSERT(ret == ARM_DRIVER_OK);
        ret = _instance->_phy->EndpointConfigure(EP0OUT, ARM_USB_ENDPOINT_CONTROL, _instance->_ep0_max_packet);
        MBED_ASSERT(ret == ARM_DRIVER_OK);
        _instance->_events->reset();
    }
    if (events & ARM_USBD_EVENT_SUSPEND) {
        _instance->_events->suspend(true);
    }
    if (events & ARM_USBD_EVENT_RESUME) {
        _instance->_events->suspend(false);
    }
}

void USBPhyCMSIS::_event_handler_endpoint(uint8_t ep_addr, uint32_t event)
{

    if ((ep_addr == EP0IN) || (ep_addr == EP0OUT)) {
        if (event & ARM_USBD_EVENT_IN) {
            if (_instance->_set_addr) {
                _instance->_set_addr = false;
                int32_t ret = _instance->_phy->DeviceSetAddress(_instance->_addr);
                MBED_ASSERT(ret == ARM_DRIVER_OK);
            }
            _instance->_events->ep0_in();
        }
        if (event & ARM_USBD_EVENT_OUT) {
            _instance->_events->ep0_out();
        }
        if (event & ARM_USBD_EVENT_SETUP) {
            _instance->_events->ep0_setup();
        }
    } else {
        if (event & ARM_USBD_EVENT_IN) {
             _instance->_events->in(ep_addr);
         }
         if (event & ARM_USBD_EVENT_OUT) {
             _instance->_events->out(ep_addr);
         }
    }
}
