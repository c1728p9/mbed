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
 * $Date:        15. December 2014
 * $Revision:    V1.00
 *
 * Driver:       Driver_USBD0
 * Project:      USB Full/Low-Speed Device Driver for Freescale MK64F
 * --------------------------------------------------------------------------
 * Use the following configuration settings in the middleware component
 * to connect to this driver.
 *
 *   Configuration Setting                Value
 *   ---------------------                -----
 *   Connect to hardware via Driver_USBD# = 0
 * -------------------------------------------------------------------------- */

/* History:
 *  Version 1.00
 *    Initial release for USB Device CMSIS Driver API v2.01, with double buffer
 *    support
 */

/* Defines used for driver configuration:
 *
 *   USBH_MAX_ENDPOINT_NUM: defines maximum number of Pipes that driver will
 *                          support, this value impacts driver memory
 *                          requirements
 *     - default value is 15 (+ Default Pipe)
 *     - maximum value of this define is 31
 */


#include <stdint.h>
#include <string.h>
#include "cmsis_os.h"

#include "Driver_USBD.h"

#include "fsl_device_registers.h"

#define OTG_FS_USBD_DRIVER_INITIALIZED (1U      )
#define OTG_FS_USBD_DRIVER_POWERED     (1U << 1U)
#define OTG_FS_USBD_DRIVER_CONNECTED   (1U << 2U)

extern  uint8_t  otg_fs_role;
extern  uint8_t  otg_fs_state;


// USB Device Driver ***********************************************************

#define ARM_USBD_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1,0) // USBD driver version

// Driver Version
static const ARM_DRIVER_VERSION usbd_driver_version = { ARM_USBD_API_VERSION, ARM_USBD_DRV_VERSION };

// Driver Capabilities
static const ARM_USBD_CAPABILITIES usbd_driver_capabilities = {
  0U, // VBUS Detection
  0U, // Event VBUS On
  0U  // Event VBUS Off
};

// Maximum number of Endpoints (excluding Endpoint 0, maximum value is 15)
#ifndef USBD_MAX_ENDPOINT_NUM
#define USBD_MAX_ENDPOINT_NUM           15U
#endif
#if    (USBH_MAX_ENDPOINT_NUM > 15)
#error  Too many Endpoints, maximum Endpoints that this driver supports is 15 !!!
#endif

// Maximum packet size (in bytes) for any Endpoint (reducing this value can save
// RAM used for Endpoint data buffers, but it must be at least as big as largest
// Endpoint maximum packet size)
#ifndef USBD_EP_MAX_PACKET_SIZE
#define USBD_EP_MAX_PACKET_SIZE         64U
#endif

#define EP_NUM(ep_addr)                (ep_addr & ARM_USB_ENDPOINT_NUMBER_MASK)
#define EP_ID(ep_addr)                 (EP_NUM(ep_addr) * 2U + ((ep_addr >> 7U) & 1U))

#define BD_OWN_MASK                    (1U    << 7U)
#define BD_DATA01_MASK                 (1U    << 6U)
#define BD_KEEP_MASK                   (1U    << 5U)
#define BD_NINC_MASK                   (1U    << 4U)
#define BD_DTS_MASK                    (1U    << 3U)
#define BD_BDT_STALL_MASK              (1U    << 2U)
#define BD_TOK_PID_MASK                (0x0FU << 2U)
#define BD_TOK_PID(idx)                ((BD[idx].stat & BD_TOK_PID_MASK) >> 2U)
#define BD_IDX(endp, tx, odd)          (((endp & 0x0FU) * 4U) + tx * 2U + odd)
#define BD_ID(ep_addr)                 (EP_NUM(ep_addr) * 4U + ((ep_addr >> 7U) & 1U) * 2U)
#define EP_ID_FROM_NUM(endp,tx)        (endp * 2U + tx)
#define BD_ID_FROM_NUM(endp,tx,odd)    (endp * 4U + tx * 2U + odd)

#define TOK_PID_SETUP                   0x0DU
#define TOK_PID_IN                      0x09U
#define TOK_PID_OUT                     0x01U

typedef struct _BD_t {                                  // Buffer Descriptor table entry
  uint32_t  cmd_stat;
  uint32_t  buf_addr;
} BD_t;

typedef struct _ENDPOINT_t {                            // Endpoint runtime information structure
  uint16_t  ep_max_packet_size;
  uint8_t   active;
  uint8_t  *data;
  uint32_t  num;
  uint32_t  num_transferred_total;
  uint16_t  num_transferring[2];
  uint8_t   next_data_toggle;
  uint8_t   last_bd_odd;
  uint8_t   next_bd_odd;
} ENDPOINT_t;

// Local variables and structures
static ARM_USBD_SignalDeviceEvent_t   SignalDeviceEvent;
static ARM_USBD_SignalEndpointEvent_t SignalEndpointEvent;

static ARM_USBD_STATE      usbd_device_state = { 0U };  // Device state

static uint32_t            setup_packet[2]   = { 0U };  // Setup packet data
static volatile uint8_t    setup_received    =   0U;    // Setup packet received
static volatile uint32_t   configured_ep;               // Bit mask of configured Endpoints

// Buffer Descriptor Table
static BD_t __align(512)   bd          [(USBD_MAX_ENDPOINT_NUM+1U)*2U*2U];

// Endpoints IN data buffers
static uint32_t            ep_in_data  [(USBD_MAX_ENDPOINT_NUM+1U)][2U][USBD_EP_MAX_PACKET_SIZE/4U] = { 0U };

// Endpoints runtime information
static volatile ENDPOINT_t ep          [(USBD_MAX_ENDPOINT_NUM+1U)*2U];


// Function prototypes
static uint16_t  USBD_GetFrameNumber (void);


// Local functions

/**
  \fn          void USBD_HW_Initialize (void)
  \brief       Initialize USB controller.
*/
static void USBD_HW_Initialize (void) {
  uint8_t i;

  USB0->USBTRC0      |=  USB_USBTRC0_USBRESET_MASK;     // Reset OTG FS Controller
  osDelay(1);                                           // wait for reset to finish

  USB0->CTL          &= ~USB_CTL_USBENSOFEN_MASK;       // Disable USB
  USB0->CTL          |=  USB_CTL_USBENSOFEN_MASK;       // Enable USB
  USB0->CTL          &= ~USB_CTL_HOSTMODEEN_MASK;       // Force Device mode
  USB0->CTL          |=  USB_CTL_ODDRST_MASK;           // Reset buffer descriptor table entry to EVEN
  USB0->CTL          &= ~USB_CTL_ODDRST_MASK;           // Clear buffer descriptor table entry reset

  USB0->USBCTRL       =  0U;                            // Reset USB CTRL register
  USB0->USBCTRL      &= ~USB_USBCTRL_SUSP_MASK;         // USB transceiver resume

  USB0->BDTPAGE1      = ((uint32_t)(bd) >>  8U) & 0xFEU;// Set BDT address
  USB0->BDTPAGE2      = ((uint32_t)(bd) >> 16U);
  USB0->BDTPAGE3      = ((uint32_t)(bd) >> 24U);

  for (i = 0U; i < 16U; i++) {                          // Clear all Endpoint Control registers
    USB0->ENDPOINT[i].ENDPT = 0U;
  }

  USB0->ADDR          =  0U;                            // Clear ADDR

  USB0->ERRSTAT       =  0xFFU;                         // Clear all error interrupts
  USB0->ERREN        |=  USB_ERREN_BTSERREN_MASK |      // Enable all error interrupts
                         USB_ERREN_DMAERREN_MASK |
                         USB_ERREN_BTOERREN_MASK |
                         USB_ERREN_DFN8EN_MASK   |
                         USB_ERREN_CRC16EN_MASK  |
                         USB_ERREN_CRC5EOFEN_MASK|
                         USB_ERREN_PIDERREN_MASK ;
  USB0->ISTAT         =  0xFFU;                         // Clear interrupt flags
  USB0->INTEN         =  USB_INTEN_USBRSTEN_MASK ;      // Enable USBRST interrupt
}

/**
  \fn          void USBD_HW_Reset (void)
  \brief       USB Bus Reset.
*/
static void USBD_Reset (void) {
  volatile ENDPOINT_t *ptr_ep;
  volatile BD_t       *ptr_bd;
  uint8_t              i;

  memset((void *)ep, 0, sizeof(ep));                    // Clear all Endpoints runtime information

  ptr_ep  = &ep[0];                                     // Initialize all starting odd bits
  for (i = 0U; i < USBD_MAX_ENDPOINT_NUM; i++) {
    ptr_ep->last_bd_odd = 1U; ptr_ep++;
    ptr_ep->last_bd_odd = 1U; ptr_ep++;
  }

  memset((void *)bd, 0, sizeof(bd));                    // Clear Buffer Descriptor Table

  ptr_ep  = &ep[0];
  ptr_ep->next_bd_odd = 1U;                             // Set next_bd_odd to odd for next transfer
  ptr_bd  = &bd[0];                                     // Setup Endpoint 0 OUT EVEN Buffer Descriptor Table (BDT) entry
  ptr_bd->buf_addr    = (uint32_t)setup_packet;
  ptr_bd->cmd_stat    = (8U << 16U) | BD_OWN_MASK | BD_DTS_MASK | BD_DATA01_MASK;
  ptr_bd += 2U;                                         // Skip Endpoint 0 OUT ODD BDT entry
                                                        // Set all initial Endpoints IN data sizes and buffer addresses
  for (i=0U; i<USBD_MAX_ENDPOINT_NUM; i++) {
                                                        // Endpoint IN EVEN BDT entry
    ptr_bd->buf_addr  = (uint32_t)(&ep_in_data[i][0][0]);
    ptr_bd->cmd_stat |= (USBD_EP_MAX_PACKET_SIZE << 16U);
    ptr_bd++;
                                                        // Endpoint IN ODD  BDT entry
    ptr_bd->buf_addr  = (uint32_t)(&ep_in_data[i][1][0]);
    ptr_bd->cmd_stat |= (USBD_EP_MAX_PACKET_SIZE << 16U);

    ptr_bd           += 3U;                             // Skip Endpoint OUT BDT entries
    // OUT Endpoints directly use provided RAM buffers except inital OUT for setup packet
  }
  configured_ep       = 1U;                             // Endpoint 0 OUT is configured to receive first SETUP packet

  USB0->ENDPOINT[0].ENDPT = USB_ENDPT_EPHSHK_MASK |     // Enable Endpoint handshaking
                            USB_ENDPT_EPRXEN_MASK ;     // Enable RX (OUT or SETUP) transfer

  USB0->CTL          |=  USB_CTL_ODDRST_MASK;           // Reset buffer descriptor table entry to EVEN
  USB0->CTL          &= ~USB_CTL_ODDRST_MASK;           // Clear buffer descriptor table entry reset

  USB0->ADDR          =  0U;                            // Clear ADDR

  USB0->ERRSTAT       =  0xFFU;                         // Clear all error interrupts
  USB0->ERREN        |=  USB_ERREN_BTSERREN_MASK |      // Enable all error interrupts
                         USB_ERREN_DMAERREN_MASK |
                         USB_ERREN_BTOERREN_MASK |
                         USB_ERREN_DFN8EN_MASK   |
                         USB_ERREN_CRC16EN_MASK  |
                         USB_ERREN_CRC5EOFEN_MASK|
                         USB_ERREN_PIDERREN_MASK ;
  USB0->ISTAT         =  0xFFU;                         // Clear interrupt flags
  USB0->INTEN         =  USB_ISTAT_STALL_MASK    |      // Enable STALL interrupt
                         USB_INTEN_SLEEPEN_MASK  |      // Enable SLEEP interrupt
                         USB_INTEN_TOKDNEEN_MASK |      // Enable TOKDNE interrupt
                         USB_INTEN_ERROREN_MASK  |      // Enable ERROR interrupt
                         USB_INTEN_USBRSTEN_MASK ;      // Enable USBRST interrupt
}

/**
  \fn          void USBD_HW_EndpointTransfer (uint8_t endp,
                                              uint8_t tx)
  \brief       Start transfer on Endpoint.
  \param[in]   endp:    Endpoint Number
  \param[in]   tx       Transmit or receive direction
                - value 0: Receive
                - value 1: Transmit
*/
static void USBD_HW_EndpointTransfer (uint8_t endp, uint8_t tx) {
  volatile ENDPOINT_t *ptr_ep;
  volatile BD_t       *ptr_bd;
  uint8_t             *data_addr;
  uint32_t             data_num, num_to_transfer;
  uint8_t              next_bd_odd, next_data_toggle;

  ptr_ep           = &ep[EP_ID_FROM_NUM(endp,tx)];
  next_bd_odd      =  ptr_ep->next_bd_odd;
  ptr_bd           = &bd[BD_ID_FROM_NUM(endp,tx,next_bd_odd)]; if (ptr_bd->cmd_stat & BD_OWN_MASK) return;
  data_addr        = (uint8_t *)(ptr_ep->data + ptr_ep->num_transferred_total + ptr_ep->num_transferring[!next_bd_odd]);
  data_num         =  ptr_ep->num - ptr_ep->num_transferred_total - ptr_ep->num_transferring[!next_bd_odd];
  next_data_toggle =  ptr_ep->next_data_toggle;

  if (data_num > ptr_ep->ep_max_packet_size) {
    num_to_transfer = ptr_ep->ep_max_packet_size;
  } else {
    num_to_transfer = data_num;
  }
  ptr_ep->num_transferring[next_bd_odd] = num_to_transfer;
  ptr_ep->next_data_toggle ^= 1U;
  ptr_ep->next_bd_odd      ^= 1U;

  if (tx) {                             // Endpoint IN
                                        // Copy data to RAM buffer to be sent
    memcpy(&ep_in_data[endp][next_bd_odd][0], data_addr, num_to_transfer);
  } else {                              // Endpoint OUT
                                        // Set address where data should be received
    ptr_bd->buf_addr = (uint32_t)data_addr;
  }
  ptr_bd->cmd_stat   = (num_to_transfer << 16U) | BD_OWN_MASK | BD_DTS_MASK | (BD_DATA01_MASK * next_data_toggle);
}


// USB Device Driver functions

/**
  \fn          ARM_DRIVER_VERSION USBD_GetVersion (void)
  \brief       Get driver version.
  \return      \ref ARM_DRIVER_VERSION
*/
static ARM_DRIVER_VERSION USBD_GetVersion (void) { return usbd_driver_version; }

/**
  \fn          ARM_USBD_CAPABILITIES USBD_GetCapabilities (void)
  \brief       Get driver capabilities.
  \return      \ref ARM_USBD_CAPABILITIES
*/
static ARM_USBD_CAPABILITIES USBD_GetCapabilities (void) { return usbd_driver_capabilities; }

/**
  \fn          int32_t USBD_Initialize (ARM_USBD_SignalDeviceEvent_t   cb_device_event,
                                        ARM_USBD_SignalEndpointEvent_t cb_endpoint_event)
  \brief       Initialize USB Device Interface.
  \param[in]   cb_device_event    Pointer to \ref ARM_USBD_SignalDeviceEvent
  \param[in]   cb_endpoint_event  Pointer to \ref ARM_USBD_SignalEndpointEvent
  \return      \ref execution_status
*/
static int32_t USBD_Initialize (ARM_USBD_SignalDeviceEvent_t   cb_device_event,
                                ARM_USBD_SignalEndpointEvent_t cb_endpoint_event) {

  if (otg_fs_state & OTG_FS_USBD_DRIVER_INITIALIZED) { return ARM_DRIVER_OK;    }
  if (otg_fs_state)                                  { return ARM_DRIVER_ERROR; }

  SignalDeviceEvent   = cb_device_event;
  SignalEndpointEvent = cb_endpoint_event;

  MPU_CESR = 0;                         // Disable MPU

  otg_fs_role   =  ARM_USB_ROLE_DEVICE;
  otg_fs_state |=  OTG_FS_USBD_DRIVER_INITIALIZED;

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USBD_Uninitialize (void)
  \brief       De-initialize USB Device Interface.
  \return      \ref execution_status
*/
static int32_t USBD_Uninitialize (void) {

  if (!(otg_fs_state & OTG_FS_USBD_DRIVER_INITIALIZED)) { return ARM_DRIVER_OK;    }
  if (  otg_fs_state & OTG_FS_USBD_DRIVER_POWERED     ) { return ARM_DRIVER_ERROR; }

  otg_fs_role   =  ARM_USB_ROLE_NONE;
  otg_fs_state &= ~OTG_FS_USBD_DRIVER_INITIALIZED;

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USBD_PowerControl (ARM_POWER_STATE state)
  \brief       Control USB Device Interface Power.
  \param[in]   state  Power state
  \return      \ref execution_status
*/
static int32_t USBD_PowerControl (ARM_POWER_STATE state) {

  if (!(otg_fs_state & OTG_FS_USBD_DRIVER_INITIALIZED)) { return ARM_DRIVER_ERROR; }

  switch (state) {
    case ARM_POWER_OFF:
      if (!(otg_fs_state & OTG_FS_USBD_DRIVER_POWERED)) { return ARM_DRIVER_OK; }
      otg_fs_state  &= ~OTG_FS_USBD_DRIVER_POWERED;
      NVIC_DisableIRQ(USB0_IRQn);                       // Disable interrupt
      USB0->USBCTRL |=  USB_USBCTRL_SUSP_MASK;          // Suspend USB transceiver
      USB0->CTL     &= ~USB_CTL_USBENSOFEN_MASK;        // Disable USB
      SIM->SCGC4    &= ~SIM_SCGC4_USBOTG_MASK;          // Disable OTG FS Clock
      usbd_device_state.vbus   = 0U;                    // USB Device no VBUS
      usbd_device_state.speed  = 0U;                    // USB Device speed (low)
      usbd_device_state.active = 0U;                    // USB Device not active
      break;

    case ARM_POWER_FULL:
      if (  otg_fs_state & OTG_FS_USBD_DRIVER_POWERED ) { return ARM_DRIVER_OK; }

      // Enable all clocks needed for USB to function, set USB clock to 48 MHz
      SIM->SOPT2    |=  SIM_SOPT2_USBSRC_MASK;          // MCGFLLCLK, MCGPLLCLK, or IRC48M used as src
      SIM->SCGC4    |=  SIM_SCGC4_USBOTG_MASK;          // Enable OTG FS Clock

      USBD_HW_Initialize();                             // Initialize USB controller

      otg_fs_state  |=  OTG_FS_USBD_DRIVER_POWERED;

      NVIC_ClearPendingIRQ(USB0_IRQn);                  // Clear interrupt
      NVIC_EnableIRQ(USB0_IRQn);                        // Enable interrupt
      break;

    default:
      return ARM_DRIVER_ERROR_UNSUPPORTED;
  }

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USBD_DeviceConnect (void)
  \brief       Connect USB Device.
  \return      \ref execution_status
*/
static int32_t USBD_DeviceConnect (void) {

  if (!(otg_fs_state & OTG_FS_USBD_DRIVER_POWERED) ) { return ARM_DRIVER_ERROR; }
  if (  otg_fs_state & OTG_FS_USBD_DRIVER_CONNECTED) { return ARM_DRIVER_OK;    }

  usbd_device_state.speed  =  ARM_USB_SPEED_FULL;
  usbd_device_state.active =  true;

  USB0->CONTROL     =  USB_CONTROL_DPPULLUPNONOTG_MASK; // Activate pull-up on D+

  otg_fs_state     |=  OTG_FS_USBD_DRIVER_CONNECTED;

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USBD_DeviceDisconnect (void)
  \brief       Disconnect USB Device.
  \return      \ref execution_status
*/
static int32_t USBD_DeviceDisconnect (void) {

  if (!(otg_fs_state & OTG_FS_USBD_DRIVER_POWERED)  ) { return ARM_DRIVER_ERROR; }
  if (!(otg_fs_state & OTG_FS_USBD_DRIVER_CONNECTED)) { return ARM_DRIVER_OK;    }

  USB0->CONTROL     =  0U;                              // De-activate pull-up on D+

  usbd_device_state.active =  false;

  otg_fs_state     &= ~OTG_FS_USBD_DRIVER_CONNECTED;

  return ARM_DRIVER_OK;
}

/**
  \fn          ARM_USBD_STATE USBD_DeviceGetState (void)
  \brief       Get current USB Device State.
  \return      Device State \ref ARM_USBD_STATE
*/
static ARM_USBD_STATE USBD_DeviceGetState (void) {
  return usbd_device_state;
}

/**
  \fn          int32_t USBD_DeviceRemoteWakeup (void)
  \brief       Trigger USB Remote Wakeup.
  \return      \ref execution_status
*/
static int32_t USBD_DeviceRemoteWakeup (void) {

  if (!(otg_fs_state & OTG_FS_USBD_DRIVER_CONNECTED)) { return ARM_DRIVER_ERROR; }

  USB0->CTL |=  USB_CTL_RESUME_MASK;
  osDelay(5);
  USB0->CTL &= ~USB_CTL_RESUME_MASK;

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USBD_DeviceSetAddress (uint8_t dev_addr)
  \brief       Set USB Device Address.
  \param[in]   dev_addr  Device Address
  \return      \ref execution_status
*/
static int32_t USBD_DeviceSetAddress (uint8_t dev_addr) {

  if (!(otg_fs_state & OTG_FS_USBD_DRIVER_CONNECTED)) { return ARM_DRIVER_ERROR; }

  USB0->ADDR = dev_addr & 0x7FU;

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USBD_ReadSetupPacket (uint8_t *setup)
  \brief       Read setup packet received over Control Endpoint.
  \param[out]  setup  Pointer to buffer for setup packet
  \return      \ref execution_status
*/
static int32_t USBD_ReadSetupPacket (uint8_t *setup) {

  if (!(otg_fs_state & OTG_FS_USBD_DRIVER_CONNECTED)) { return ARM_DRIVER_ERROR; }
  if (!setup_received)                                { return ARM_DRIVER_ERROR; }

  setup_received = 0U;
  memcpy(setup, setup_packet, 8);

  if (setup_received) {
    return ARM_DRIVER_ERROR;            // New setup packet was received while this was being read
  }

  USB0->CTL &= ~USB_CTL_TXSUSPENDTOKENBUSY_MASK;

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USBD_EndpointConfigure (uint8_t  ep_addr,
                                               uint8_t  ep_type,
                                               uint16_t ep_max_packet_size)
  \brief       Configure USB Endpoint.
  \param[in]   ep_addr  Endpoint Address
                - ep_addr.0..3: Address
                - ep_addr.7:    Direction
  \param[in]   ep_type  Endpoint Type (ARM_USB_ENDPOINT_xxx)
  \param[in]   ep_max_packet_size Endpoint Maximum Packet Size
  \return      \ref execution_status
*/
static int32_t USBD_EndpointConfigure (uint8_t  ep_addr,
                                       uint8_t  ep_type,
                                       uint16_t ep_max_packet_size) {
  volatile ENDPOINT_t *ptr_ep;
  uint32_t             ep_num, ep_dir, ep_msk, ep_mps;

  ptr_ep = &ep[EP_ID(ep_addr)];
  ep_num =  EP_NUM(ep_addr);
  ep_dir = (ep_addr & ARM_USB_ENDPOINT_DIRECTION_MASK) == ARM_USB_ENDPOINT_DIRECTION_MASK;
  ep_msk = (1U << ep_num) << (16U * ep_dir);
  ep_mps =  ep_max_packet_size & ARM_USB_ENDPOINT_MAX_PACKET_SIZE_MASK;

  if (!(otg_fs_state & OTG_FS_USBD_DRIVER_CONNECTED)) { return ARM_DRIVER_ERROR;      }
  if (ptr_ep->active)                                 { return ARM_DRIVER_ERROR_BUSY; }
  if (ep_num > USBD_MAX_ENDPOINT_NUM)                 { return ARM_DRIVER_ERROR;      }

  // Store maximum packet size of Endpoint
  ptr_ep->ep_max_packet_size    = ep_mps;

  // Clear current transfer information
  ptr_ep->active                = 0U;
  ptr_ep->data                  = NULL;
  ptr_ep->num                   = 0U;
  ptr_ep->num_transferred_total = 0U;
  ptr_ep->num_transferring[0]   = 0U;
  ptr_ep->num_transferring[1]   = 0U;
  ptr_ep->next_data_toggle      = 0U;

  configured_ep |=  ep_msk;             // Set flag in configured_ep for Endpoint we have configured

  USB0->ENDPOINT[ep_num].ENDPT |= USB_ENDPT_EPHSHK_MASK             |   // Enable Endpoint handshaking
                                  USB_ENDPT_EPRXEN_MASK * (!ep_dir) |   // Enable RX if OUT Endpoint
                                  USB_ENDPT_EPTXEN_MASK *   ep_dir;     // Enable TX if IN  Endpoint

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USBD_EndpointUnconfigure (uint8_t ep_addr)
  \brief       Unconfigure USB Endpoint.
  \param[in]   ep_addr  Endpoint Address
                - ep_addr.0..3: Address
                - ep_addr.7:    Direction
  \return      \ref execution_status
*/
static int32_t USBD_EndpointUnconfigure (uint8_t ep_addr) {
  volatile ENDPOINT_t *ptr_ep;
  volatile BD_t       *ptr_bd;
  uint32_t             ep_num, ep_dir, ep_msk;

  ptr_ep = &ep[EP_ID(ep_addr)];
  ep_num =  EP_NUM(ep_addr);
  ep_dir = (ep_addr & ARM_USB_ENDPOINT_DIRECTION_MASK) == ARM_USB_ENDPOINT_DIRECTION_MASK;
  ep_msk = (1U << ep_num) << (16U * ep_dir);

  if (!(otg_fs_state & OTG_FS_USBD_DRIVER_CONNECTED)) { return ARM_DRIVER_ERROR;      }
  if (ptr_ep->active)                                 { return ARM_DRIVER_ERROR_BUSY; }
  if (!(configured_ep & ep_msk))                      { return ARM_DRIVER_OK;         }
  if (ep_num > USBD_MAX_ENDPOINT_NUM)                 { return ARM_DRIVER_ERROR;      }

  ep_num =  EP_NUM(ep_addr);
  ep_dir = (ep_addr & ARM_USB_ENDPOINT_DIRECTION_MASK) == ARM_USB_ENDPOINT_DIRECTION_MASK;

  USB0->ENDPOINT[ep_num].ENDPT &= ~(USB_ENDPT_EPRXEN_MASK * (!ep_dir) | // Disable RX if OUT Endpoint
                                    USB_ENDPT_EPTXEN_MASK *   ep_dir) ; // Disable TX if IN  Endpoint

  configured_ep &= ~ep_msk;             // Clear flag in configured_ep for Endpoint we have unconfigured

  ptr_bd           = &bd[BD_ID(ep_addr)];
  ptr_bd->cmd_stat = 0U; ptr_bd++;      // Clear corresponding EVEN BDT entry
  ptr_bd->cmd_stat = 0U;                // Clear corresponding ODD  BDT entry

  // Clear Endpoint runtime information (leave last_bd_odd)
  ptr_ep->ep_max_packet_size    = 0U;
  ptr_ep->data                  = NULL;
  ptr_ep->num                   = 0U;
  ptr_ep->num_transferred_total = 0U;
  ptr_ep->num_transferring[0]   = 0U;
  ptr_ep->num_transferring[1]   = 0U;
  ptr_ep->next_data_toggle      = 0U;

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USBD_EndpointStall (uint8_t ep_addr, bool stall)
  \brief       Set/Clear Stall for USB Endpoint.
  \param[in]   ep_addr  Endpoint Address
                - ep_addr.0..3: Address
                - ep_addr.7:    Direction
  \param[in]   stall  Operation
                - \b false Clear
                - \b true Set
  \return      \ref execution_status
*/
static int32_t USBD_EndpointStall (uint8_t ep_addr, bool stall) {
  volatile ENDPOINT_t *ptr_ep;
  volatile BD_t       *ptr_bd;
  uint32_t             ep_num, ep_dir, ep_msk;

  ptr_ep = &ep[EP_ID(ep_addr)];
  ep_num =  EP_NUM(ep_addr);
  ep_dir = (ep_addr & ARM_USB_ENDPOINT_DIRECTION_MASK) == ARM_USB_ENDPOINT_DIRECTION_MASK;
  ep_msk = (1U << ep_num) << (16U * ep_dir);

  if (!(otg_fs_state & OTG_FS_USBD_DRIVER_CONNECTED)) { return ARM_DRIVER_ERROR;      }
  if (ptr_ep->active)                                 { return ARM_DRIVER_ERROR_BUSY; }
  if (!(configured_ep & ep_msk))                      { return ARM_DRIVER_ERROR;      }
  if (ep_num > USBD_MAX_ENDPOINT_NUM)                 { return ARM_DRIVER_ERROR;      }

  if (stall) {                          // Activate STALL
    USB0->ENDPOINT[ep_num].ENDPT |=  USB_ENDPT_EPSTALL_MASK;
  } else {                              // Clear STALL
    ptr_ep->next_data_toggle = 0U;      // Clear data toggle bit

    ptr_bd           = &bd[BD_ID(ep_addr)];
    ptr_bd->cmd_stat = 0U; ptr_bd++;    // Clear corresponding EVEN BDT entry
    ptr_bd->cmd_stat = 0U;              // Clear corresponding ODD  BDT entry

    USB0->ENDPOINT[ep_num].ENDPT &= ~USB_ENDPT_EPSTALL_MASK;
  }

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USBD_EndpointTransfer (uint8_t ep_addr, uint8_t *data, uint32_t num)
  \brief       Read data from or Write data to USB Endpoint.
  \param[in]   ep_addr  Endpoint Address
                - ep_addr.0..3: Address
                - ep_addr.7:    Direction
  \param[out]  data Pointer to buffer for data to read or with data to write
  \param[in]   num  Number of data bytes to transfer
  \return      \ref execution_status
*/
static int32_t USBD_EndpointTransfer (uint8_t ep_addr, uint8_t *data, uint32_t num) {
  volatile ENDPOINT_t *ptr_ep;
  uint32_t             ep_num, ep_dir, ep_msk;

  ptr_ep = &ep[EP_ID(ep_addr)];
  ep_num =  EP_NUM(ep_addr);
  ep_dir = (ep_addr & ARM_USB_ENDPOINT_DIRECTION_MASK) == ARM_USB_ENDPOINT_DIRECTION_MASK;
  ep_msk = (1U << ep_num) << (16U * ep_dir);

  if (!(otg_fs_state & OTG_FS_USBD_DRIVER_CONNECTED)) { return ARM_DRIVER_ERROR;      }
  if (ptr_ep->active)                                 { return ARM_DRIVER_ERROR_BUSY; }
  if (!(configured_ep & ep_msk))                      { return ARM_DRIVER_ERROR;      }
  if (ep_num > USBD_MAX_ENDPOINT_NUM)                 { return ARM_DRIVER_ERROR;      }

  ptr_ep->active                = 1U;
  ptr_ep->data                  = data;
  ptr_ep->num                   = num;
  ptr_ep->num_transferred_total = 0U;
  ptr_ep->num_transferring[0]   = 0U;
  ptr_ep->num_transferring[1]   = 0U;
  ptr_ep->next_bd_odd           = ptr_ep->last_bd_odd ^ 1U;     // Set next odd bit

  USBD_HW_EndpointTransfer(ep_num, ep_dir);             // Start the transaction

  return ARM_DRIVER_OK;
}

/**
  \fn          uint32_t USBD_EndpointTransferGetResult (uint8_t ep_addr)
  \brief       Get result of USB Endpoint transfer.
  \param[in]   ep_addr  Endpoint Address
                - ep_addr.0..3: Address
                - ep_addr.7:    Direction
  \return      number of successfully transferred data bytes
*/
static uint32_t USBD_EndpointTransferGetResult (uint8_t ep_addr) {

  if (EP_NUM(ep_addr) > USBD_MAX_ENDPOINT_NUM) { return 0U; }

  return (ep[EP_ID(ep_addr)].num_transferred_total);
}

/**
  \fn          int32_t USBD_EndpointTransferAbort (uint8_t ep_addr)
  \brief       Abort current USB Endpoint transfer.
  \param[in]   ep_addr  Endpoint Address
                - ep_addr.0..3: Address
                - ep_addr.7:    Direction
  \return      \ref execution_status
*/
static int32_t USBD_EndpointTransferAbort (uint8_t ep_addr) {
  volatile ENDPOINT_t *ptr_ep;
  volatile BD_t       *ptr_bd;
  uint32_t             ep_num, ep_dir, ep_msk;

  ptr_ep = &ep[EP_ID(ep_addr)];
  ep_num =  EP_NUM(ep_addr);
  ep_dir = (ep_addr & ARM_USB_ENDPOINT_DIRECTION_MASK) == ARM_USB_ENDPOINT_DIRECTION_MASK;
  ep_msk = (1U << ep_num) << (16U * ep_dir);

  if (!(otg_fs_state & OTG_FS_USBD_DRIVER_CONNECTED)) { return ARM_DRIVER_ERROR; }
  if (!(configured_ep & ep_msk))                      { return ARM_DRIVER_ERROR; }
  if (ep_num > USBD_MAX_ENDPOINT_NUM)                 { return ARM_DRIVER_ERROR; }

  ptr_ep->num                   = 0U;
  ptr_ep->num_transferred_total = 0U;
  ptr_ep->num_transferring[0]   = 0U;
  ptr_ep->num_transferring[1]   = 0U;
  ptr_ep->next_data_toggle      = 0U;

  ptr_bd           = &bd[BD_ID(ep_addr)];
  ptr_bd->cmd_stat =  0U; ptr_bd++;     // Clear corresponding EVEN BDT entry
  ptr_bd->cmd_stat =  0U;               // Clear corresponding ODD  BDT entry

  ptr_ep->active                = 0U;

  return ARM_DRIVER_OK;
}

/**
  \fn          uint16_t USBD_GetFrameNumber (void)
  \brief       Get current USB Frame Number.
  \return      Frame Number
*/
static uint16_t USBD_GetFrameNumber (void) {
  return ((((uint32_t)(USB0->FRMNUML)) | (((uint32_t)(USB0->FRMNUMH)) << 8)) & 0x07FFU);
}

/**
  \fn          void USBD_FS_IRQ (void)
  \brief       USB0 Device Interrupt Routine (IRQ).
*/
void USBD_FS_IRQ (void) {
  volatile ENDPOINT_t *ptr_ep;
  volatile BD_t       *ptr_bd;
  uint32_t             istat, stat, errstat;
  uint8_t              endp, tx, odd, token, en_setup;

  istat   = USB0->ISTAT & USB0->INTEN;                  // Read active interrupts
  stat    = USB0->STAT;
  errstat = USB0->ERRSTAT;
  USB0->ISTAT = istat;                                  // Clear interrupts

  en_setup = 0;

  if (istat & USB_ISTAT_USBRST_MASK) {                  // Reset interrupt
    if (otg_fs_state & OTG_FS_USBD_DRIVER_CONNECTED) {
      USBD_Reset();                                     // USB Bus Reset
      SignalDeviceEvent(ARM_USBD_EVENT_RESET);
    }
  }

  if (istat & USB_ISTAT_SLEEP_MASK) {                   // Suspend interrupt
    usbd_device_state.active = false;
    SignalDeviceEvent(ARM_USBD_EVENT_SUSPEND);
  }

  if (istat & USB_ISTAT_RESUME_MASK) {                  // Resume interrupt
    usbd_device_state.active = true;
    SignalDeviceEvent(ARM_USBD_EVENT_RESUME);
  }

  if (istat & USB_ISTAT_ERROR_MASK) {                   // Error interrupt
    USB0->ERRSTAT = errstat;                            // Clear all errors
  }

  if (istat & USB_ISTAT_STALL_MASK) {                   // Stall interrupt
    if (USB0->ENDPOINT[0].ENDPT & USB_ENDPT_EPSTALL_MASK) {
      USB0->ENDPOINT[0].ENDPT &= ~USB_ENDPT_EPSTALL_MASK;
      en_setup = 1U;                    // Enable reception of new SETUP Packet
    }
  }

  if (istat & USB_ISTAT_TOKDNE_MASK) {                  // Token interrupt
    endp     = (stat >> 4U) & 0x0FU;
    tx       = (stat >> 3U) & 0x01U;
    odd      = (stat >> 2U) & 0x01U;
    ptr_ep   = &ep[EP_ID_FROM_NUM(endp,tx)];
    ptr_bd   = &bd[BD_ID_FROM_NUM(endp,tx,odd)];
    ptr_ep->last_bd_odd = odd;
    token    = ((ptr_bd->cmd_stat >> 2U) & 0x0FU);
    switch (token) {
      case TOK_PID_SETUP:               // SETUP packet
        ptr_bd           = bd;          // Disable all BDT entries for Endpoint 0
        ptr_bd->cmd_stat = 0U; ptr_bd++;
        ptr_bd->cmd_stat = 0U; ptr_bd++;
        ptr_bd->cmd_stat = 0U; ptr_bd++;
        ptr_bd->cmd_stat = 0U;

        ep[0].next_data_toggle = 1U;    // Next OUT packet should be DATA1
        ep[1].next_data_toggle = 1U;    // Next IN  packet should be DATA1

        setup_received = 1U;            // Set SETUP packet pending
        SignalEndpointEvent(0, ARM_USBD_EVENT_SETUP);
        break;

      case TOK_PID_OUT:                 // OUT packet
        ptr_ep->num_transferred_total += (ptr_bd->cmd_stat >> 16U) & 0x3FFU;
        ptr_ep->num_transferring[odd]  =  0U;
        if (!endp && !ptr_ep->num && (setup_packet[0] & 0x80U)) {
           en_setup = 1U;               // If OUT ZLP (Device-to-Host request) enable reception of new SETUP Packet
        }
        if ((ptr_ep->num_transferred_total % ptr_ep->ep_max_packet_size) || (ptr_ep->num == ptr_ep->num_transferred_total)){
          // If all OUT data was received (required size or data terminated with ZLP or short packet), set ARM_USBD_EP_EVENT_OUT
          ptr_ep->active = 0U;          // Clear Endpoint busy flag
          SignalEndpointEvent((tx << 7U) | endp, ARM_USBD_EVENT_OUT);       // Send OUT event
        } else {
          // If this is not last transfer, enable next
          if ((ptr_ep->num > (ptr_ep->num_transferred_total + ptr_ep->num_transferring[!odd])) && (!ptr_ep->num_transferring[!odd])) {
            // If alternate BDT entry is empty and there is data to transfer, activate it
            USBD_HW_EndpointTransfer (endp, tx);
          }
          if (ptr_ep->num > (ptr_ep->num_transferred_total + ptr_ep->num_transferring[!odd])) {
            // If there is data to transfer activate it on current BDT entry
            USBD_HW_EndpointTransfer (endp, tx);
          }
        }
        break;

      case TOK_PID_IN:                  // IN Packet
        ptr_ep->num_transferred_total += ptr_ep->num_transferring[odd];
        ptr_ep->num_transferring[odd]  = 0U;
        if (!endp && !ptr_ep->num && !(setup_packet[0] & 0x80U)) {
          en_setup = 1U;                // If IN ZLP (Host-to-Device request) enable reception of new SETUP Packet
        }
        if (ptr_ep->num == ptr_ep->num_transferred_total) {
          // If all required IN data was sent, set ARM_USBD_EP_EVENT_IN
          ptr_ep->active = 0U;          // Clear Endpoint busy flag
          SignalEndpointEvent((tx << 7U) | endp, ARM_USBD_EVENT_IN);        // Send IN event
        } else {
          // If this is not last transfer, enable next
          if ((ptr_ep->num > (ptr_ep->num_transferred_total + ptr_ep->num_transferring[!odd])) && (!ptr_ep->num_transferring[!odd])) {
            // If alternate BDT entry is empty and there is data to transfer, activate it
            USBD_HW_EndpointTransfer (endp, tx);
          }
          if (ptr_ep->num > (ptr_ep->num_transferred_total + ptr_ep->num_transferring[!odd])) {
            // If there is data to transfer activate it on current BDT entry
            USBD_HW_EndpointTransfer (endp, tx);
          }
        }
        break;
    }
  }
  if (en_setup) {                       // If SETUP is expected, enable it's reception
    ptr_ep  = &ep[0];
    odd     =  ptr_ep->last_bd_odd ^ 1U;// Next is alternate of last
    ptr_bd  = &bd[odd];                 // Endpoint 0 OUT Buffer Descriptor Table (BDT) entry
    // Enable reception of SETUP Packet or OUT ZLP (DATA1)
    ptr_bd->buf_addr = (uint32_t)setup_packet;
    ptr_bd->cmd_stat = (8U << 16U) | BD_OWN_MASK | BD_DTS_MASK;
  }
}


ARM_DRIVER_USBD Driver_USBD0 = {
  USBD_GetVersion,
  USBD_GetCapabilities,
  USBD_Initialize,
  USBD_Uninitialize,
  USBD_PowerControl,
  USBD_DeviceConnect,
  USBD_DeviceDisconnect,
  USBD_DeviceGetState,
  USBD_DeviceRemoteWakeup,
  USBD_DeviceSetAddress,
  USBD_ReadSetupPacket,
  USBD_EndpointConfigure,
  USBD_EndpointUnconfigure,
  USBD_EndpointStall,
  USBD_EndpointTransfer,
  USBD_EndpointTransferGetResult,
  USBD_EndpointTransferAbort,
  USBD_GetFrameNumber
};
