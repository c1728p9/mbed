/* -----------------------------------------------------------------------------
 * Copyright (c) 2013-2014 ARM Ltd.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software. Permission is granted to anyone to use this
 * software for any purpose, including commercial applications, and to alter
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *
 * $Date:        18. June 2014
 * $Revision:    V1.00
 *
 * Project:      USB common (Device and Host) module for NXP LPC17xx
 * -------------------------------------------------------------------------- */

/* History:
 *  Version 1.00
 *    - Initial release
 */

#include "mbed_toolchain.h"
#include "LPC17xx.h"
#include "PIN_LPC17xx.h"

#include "Driver_USB.h"

#include "RTE_Device.h"

volatile uint32_t usb_role = ARM_USB_ROLE_NONE;

MBED_WEAK void USBH_IRQ (void) {};
MBED_WEAK void USBD_IRQ (void) {};

/**
  \fn          void USB_IRQHandler (void)
  \brief       USB Interrupt Routine (IRQ).
*/
void USB_IRQHandler (void) {
  switch (usb_role) {
    case ARM_USB_ROLE_HOST:
      USBH_IRQ ();
      break;
    case ARM_USB_ROLE_DEVICE:
      USBD_IRQ ();
      break;
    case ARM_USB_ROLE_NONE:
      break;
  }
}

/**
  \fn          void USB_PinsConfigure (void)
  \brief       Configure USB pins
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t USB_PinsConfigure (void) {

  switch (usb_role) {
    // Host pins
    case ARM_USB_ROLE_HOST:
      // P1.19 - !USB_PPWR
      if (PIN_Configure (1, 19, PIN_FUNC_2, PIN_PINMODE_PULLUP,   PIN_PINMODE_NORMAL) == -1)
        return -1;
      // P1.22 -  USB_PWRD
      if (PIN_Configure (1, 22, PIN_FUNC_2, PIN_PINMODE_TRISTATE, PIN_PINMODE_NORMAL) == -1)
        return -1;
      // P1.27 - !USB_OVRCR
      if (PIN_Configure (1, 27, PIN_FUNC_2, PIN_PINMODE_TRISTATE, PIN_PINMODE_NORMAL) == -1)
        return -1;
      break;

    // Device pins
    case ARM_USB_ROLE_DEVICE:
      // P2.9 SoftConnect
      if (PIN_Configure (2, 9,  PIN_FUNC_1, PIN_PINMODE_PULLDOWN, PIN_PINMODE_NORMAL) == -1)
        return -1;
      // P1.30 VBUS
      if (PIN_Configure (1, 30, PIN_FUNC_2, PIN_PINMODE_TRISTATE, PIN_PINMODE_NORMAL) == -1)
        return -1;
      break;

    default: return -1;
  }

  // Common pins configuration

  // P1.18 GoodLink
  if (PIN_Configure (1, 18, PIN_FUNC_1, PIN_PINMODE_PULLUP,   PIN_PINMODE_NORMAL) == -1)
    return -1;  
  // P0.29 D+
  if (PIN_Configure (0, 29, PIN_FUNC_1, PIN_PINMODE_TRISTATE, PIN_PINMODE_NORMAL) == -1)
    return -1;  
  // P0.30 D-
  if (PIN_Configure (0, 30, PIN_FUNC_1, PIN_PINMODE_TRISTATE, PIN_PINMODE_NORMAL) == -1)
    return -1;

  return 0;
}

/**
  \fn          void USB_PinsUnconfigure (void)
  \brief       Unconfigure USB pins
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t USB_PinsUnconfigure (void) {

  switch (usb_role) {
    // Host pins
    case ARM_USB_ROLE_HOST:
      // P1.19 - !USB_PPWR
      if (PIN_Configure (1, 19, PIN_FUNC_0, PIN_PINMODE_PULLUP, PIN_PINMODE_NORMAL) == -1)
        return -1;
      // P1.22 -  USB_PWRD
      if (PIN_Configure (1, 22, PIN_FUNC_0, PIN_PINMODE_PULLUP, PIN_PINMODE_NORMAL) == -1)
        return -1;
      // P1.27 - !USB_OVRCR
      if (PIN_Configure (1, 27, PIN_FUNC_0, PIN_PINMODE_PULLUP, PIN_PINMODE_NORMAL) == -1)
        return -1;
      break;

    // Device pins
    case ARM_USB_ROLE_DEVICE:
      // P2.9 SoftConnect
      if (PIN_Configure (2, 9, PIN_FUNC_0,  PIN_PINMODE_PULLUP, PIN_PINMODE_NORMAL) == -1)
        return -1;
      // P1.30 VBUS
      if (PIN_Configure (1, 30, PIN_FUNC_0, PIN_PINMODE_PULLUP, PIN_PINMODE_NORMAL) == -1)
        return -1;
      break;

    default: return -1;
  }

  // Common pins configuration

  // P1.18 GoodLink
  if (PIN_Configure (1, 18, PIN_FUNC_0, PIN_PINMODE_PULLUP, PIN_PINMODE_NORMAL) == -1)
    return -1;  
  // P0.29 D+
  if (PIN_Configure (0, 29, PIN_FUNC_0, PIN_PINMODE_PULLUP, PIN_PINMODE_NORMAL) == -1)
    return -1;  
  // P0.30 D-
  if (PIN_Configure (0, 30, PIN_FUNC_0, PIN_PINMODE_PULLUP, PIN_PINMODE_NORMAL) == -1)
    return -1;

  return 0;
}
