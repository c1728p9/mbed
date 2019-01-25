/***************************************************************************//**
* \file cy_capsense.h
* \version 1.1
*
* \brief
* This file includes all the header files of the CapSense middleware.
*
********************************************************************************
* \copyright
* Copyright 2018-2019 Cypress Semiconductor Corporation
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
*******************************************************************************/

/**
********************************************************************************
* \mainpage Cypress CapSense Middleware Library
********************************************************************************
*
* CapSense is a Cypress capacitive sensing solution. Capacitive sensing can be 
* used in a variety of applications and products where conventional mechanical 
* buttons can be replaced with sleek human interfaces to transform the way 
* users interact with electronic systems. These include home appliances, and
* automotive, IoT, and industrial applications. CapSense supports multiple 
* interfaces (widgets) using both CSX and CSD sensing methods with robust 
* performance.
* 
********************************************************************************
* \section section_capsense_general General Description
********************************************************************************
*
* A CapSense solution includes a configuration wizard to create and configure 
* CapSense widgets, an API to control the design from the application firmware,
* and the CapSense Tuner tool for tuning, testing, and debugging for 
* easy and smooth design of human interfaces on customer products. 
*
* Include cy_capsense.h to get access to all functions and other declarations 
* in this library. If you are using the ModusToolbox CapSense Configurator, 
* you can include cycfg_capsense.h only.
* 
* \subsection subsection_capsense_features Features
*
* * Offers best-in-class signal-to-noise ratio (SNR)
* * Supports Self-Capacitance (CSD) and Mutual-Capacitance (CSX) 
*   sensing methods
* * Features SmartSense auto-tuning technology for CSD sensing to avoid 
*   complex manual tuning process
* * Supports various Widgets, such as Buttons, Matrix Buttons, Sliders, 
*   Touchpads, and Proximity Sensors
* * Provides ultra-low power consumption and liquid-tolerant capacitive 
*   sensing technology
* * Contains the integrated graphical CapSense Tuner tool for real-time tuning, 
*   testing, and debugging
* * Provides superior immunity against external noise and low-radiated 
*   emission
* * Offers best-in-class liquid tolerance
* * Supports one-finger and two-finger gestures 
*
* \subsection subsection_capsense_usage When to use a CapSense
* 
* CapSense has become a popular technology to replace conventional 
* mechanical- and optical- based user interfaces. There are fewer parts 
* involved, which saves cost and increases reliability, with no wear-and-tear. 
* The main advantages of CapSense compared with other solutions are:  
* robust performance in harsh environmental conditions and rejection of a 
* wide range of external noise sources. 
* 
* Use CapSense for:
* * Touch and gesture detection for various interfaces
* * Proximity detection for innovative user experiences and low-power 
*   optimization
* * Replacement for the IR-based proximity detection, which is sensitive 
*   to skin and colors
* * Contactless liquid-level sensing in a variety of applications
* * Touch-free operations in hazardous materials
* 
********************************************************************************
* \section section_capsense_configuration Configuration Considerations
********************************************************************************
*
* The CapSense operates on the top of the CapSense Sigma Delta (CSD) driver.
* Refer to the PDL API Reference Manual.
*
* This document includes the Application Programming Interface (API). It provides 
* descriptions of the functions in the firmware library, and descriptions 
* of the data structures (Register map) used by the firmware library.
* 
* The Application Programming Interface (API) routines allow controlling and 
* executing specific tasks using the CapSense middleware. The CapSense API 
* is described in the following sections:
* * \ref group_capsense_high_level
* * \ref group_capsense_low_level
* * \ref group_capsense_data_structure
* * \ref group_capsense_enums
* * \ref group_capsense_macros
* * \ref group_capsense_callbacks
* 
********************************************************************************
* \section section_capsense_misra MISRA-C Compliance
********************************************************************************
*
* <table class="doxtable">
*   <tr>
*     <th>MISRA Rule</th>
*     <th>Rule Class (Required/Advisory)</th>
*     <th>Rule Description</th>
*     <th>Description of Deviation(s)</th>
*   </tr>
*   <tr>
*     <td>11.4</td>
*     <td>A</td>
*     <td>A conversion should not be performed between a pointer to object 
*         and an integer type.</td>
*     <td>Such conversion is performed with CapSense context in two cases: 
*         interrupt handler and DeepSleepCallback function.
*         Both cases are verified on correct operation.</td>
*   </tr>
*   <tr>
*     <td>12.13</td>
*     <td>A</td>
*     <td>The increment (++) and decrement (--) operators should not be mixed 
*         with other operators in an expression.</td>
*     <td>These violations are reported for the GCC ARM optimized form of 
*         the "for" loop that have the following syntax:
*         for(index = COUNT; index --> 0u;)
*         It is used to improve performance.</td>
*   </tr>
*   <tr>
*     <td>1.2</td>
*     <td rowspan=2> R</td>
*     <td rowspan=2> Constant: Dereference of NULL pointer.</td>
*     <td rowspan=2> These violations are reported as a result of using of offset macros
*         of CSD Driver with corresponding documented violation 20.6. Refer 
*         to CSD Driver API Ref Guide.</td>
*   </tr>
*   <tr>
*     <td>20.3</td>
*   </tr>
* </table>
*
********************************************************************************
* \section section_capsense_more_information More Information
********************************************************************************
*
* Important information about the CapSense-technology overview, appropriate 
* Cypress device for the design, CapSense system and sensor design guidelines, 
* different interfaces and tuning guidelines necessary for a successful design 
* of a CapSense system is available in the Getting Started with CapSense
* document and the product-specific CapSense design guide. Cypress highly 
* recommends starting with these documents. They can be found on the 
* Cypress web site at www.cypress.com. For details about application notes, 
* code examples, and kits, see the References section in this datasheet.
* 
* For more information, refer to the following documents:
*
* * <a href="http://www.cypress.com/trm218176"><b>Technical Reference Manual
* (TRM)</b></a>
*
* * <a href="http://www.cypress.com/ds218787"><b>PSoC 63 with BLE Datasheet
* Programmable System-on-Chip datasheet</b></a>
*
* * <a href="http://www.cypress.com/an85951"><b>AN85951 PSoC 4 and PSoC 6
* MCU CapSense Design Guide for more details</b></a>
*
* * <a href="http://www.cypress.com/an210781"><b>AN210781 Getting Started
* with PSoC 6 MCU with Bluetooth Low Energy (BLE) Connectivity</b></a>
*
********************************************************************************
* \section section_capsense_changelog Changelog
********************************************************************************
*
* <table class="doxtable">
*   <tr><th>Version</th><th>Changes</th><th>Reason for Change</th></tr>
*   <tr>
*     <td>1.0</td>
*     <td>The initial version.</td>
*     <td></td>
*   </tr>
*   <tr>
*     <td>1.1</td>
*     <td>TBD</td>
*     <td></td>
*   </tr>
* </table>
*
* \defgroup group_capsense_high_level               High-level Functions
*
* \defgroup group_capsense_low_level                Low-level Functions
*
* \defgroup group_capsense_data_structure           CapSense Data Structure
*
* \defgroup group_capsense_structures               CapSense Structures
* \ingroup group_capsense_data_structure
* \brief The CapSense structures.
*
* \defgroup group_capsense_gesture_structures       Gesture Structures
* \ingroup group_capsense_data_structure
* \brief The Gesture-related structures.
*
* \defgroup group_capsense_enums                    Enumerated Types
*
* \defgroup group_capsense_macros                   Macros
*
* \defgroup group_capsense_macros_general           General Macros
*   \ingroup group_capsense_macros
*   \brief General macros
* \defgroup group_capsense_macros_settings          Settings Macros
*   \ingroup group_capsense_macros
*   \brief Settings macros
* \defgroup group_capsense_macros_pin               Pin-related Macros
*   \ingroup group_capsense_macros
*   \brief Pin-related macros
* \defgroup group_capsense_macros_process           Processing Macros
*   \ingroup group_capsense_macros
*   \brief Processing macros
* \defgroup group_capsense_macros_touch             Touch-related Macros
*   \ingroup group_capsense_macros
*   \brief Touch-related macros
* \defgroup group_capsense_macros_gesture           Gesture Macros
*   \ingroup group_capsense_macros
*   \brief Gesture macros
* \defgroup group_capsense_macros_miscellaneous     Miscellaneous Macros
*   \ingroup group_capsense_macros
*   \brief Miscellaneous macros
*
* \defgroup group_capsense_callbacks                Callbacks
*
* \cond SECTION_CAPSENSE_INTERNAL
* \defgroup group_capsense_internal                 Internal Functions
* \endcond
*
**/

/******************************************************************************/
/** \addtogroup group_capsense_high_level 
* \{
*
* High-level functions represent the highest abstraction layer of the 
* CapSense software component.
* 
* These functions perform tasks such as scanning, data processing, data 
* reporting and tuning interfaces. When performing a task, different 
* initialization is required based on a the sensing method or type of 
* widgets is automatically handled by these functions. Therefore, these 
* functions are sensing methods, features, and widget type agnostics. 
* 
* All the tasks required to implement a sensing system can be fulfilled 
* by the high-level functions. But, there is a set of 
* \ref group_capsense_low_level that provides access to lower level
* and specific tasks. If a design requires access to low-level tasks, 
* these functions can be used. The functions related to a given sensing 
* methods are not available if the corresponding method is disabled.
*
* \} */

/******************************************************************************/
/** \addtogroup group_capsense_low_level
* \{
*
* The low-level functions represent the lower layer of abstraction in 
* support of \ref group_capsense_high_level.
* 
* These functions also enable implementation of special case designs 
* requiring performance optimization and non-typical functionalities.
* 
* The functions that contain CSD or CSX in the name are specified for 
* that sensing method appropriately and should be used only with 
* dedicated widgets having that mode.
* All other functions are general to all sensing methods. Some of the 
* functions detect the sensing method used by the widget and execute 
* tasks as appropriate.
* 
* \} */

/******************************************************************************/
/** \addtogroup group_capsense_macros
* \{
*
* Specifies constants used in CapSense middleware.
*
* \} */

/******************************************************************************/
/** \addtogroup group_capsense_enums
* \{
*
* Documents CapSense related enumerated types.
*
* \} */

/******************************************************************************/
/** \cond SECTION_CAPSENSE_INTERNAL */
/** \addtogroup group_capsense_internal
* \{
*
* The section documents CapSense related internal function.
*
* These function should not be used in the application layer.
*
* \} \endcond */

/******************************************************************************/
/** \addtogroup group_capsense_data_structure
* \{
*
* The CapSense Data Structure organizes configuration parameters, input, and 
* output data shared among different FW modules within the CapSense. 
* 
* The key responsibilities of the Data Structure are as follows:
* * The Data Structure is the only data container in the Component.
* * It serves as storage for the configuration and the output data.
* * All CapSense modules use the data structure for the communication 
*   and data exchange.
* 
* The CapSense Data Structure is a composite of several smaller structures 
* (for global / common data, widget data, sensor data, and pin data). 
* Furthermore, the data is split between RAM and Flash to achieve a 
* reasonable balance between resources consumption and configuration / tuning 
* flexibility at runtime and compile time. A graphical representation of 
* the CapSense Data Structure is shown below.
* 
* Note that figure below shows a sample representation and documents the 
* high-level design of the data structure, it does not include all the 
* parameters and elements in each object.
*
* \image html capsense_ds.png "CapSense Data Structure" width=800px
* \image latex capsense_ds.png
* 
* CapSense Data Structure does not perform error checking on the data 
* written to CapSense Data Structure. It is the responsibility of application 
* layer to ensure register map rule are not violated while 
* modifying the value of data field in CapSense Data Structure.
*
* \} */

/******************************************************************************/
/** \addtogroup group_capsense_callbacks
* \{
*
* Callbacks allow a user to execute custom code called from CapSense 
* middleware when some events occur. CapSense supports two callbacks.
*
* <table>
*   <tr>
*     <th >Callback Name</th>
*     <th>Associated Register</th>
*     <th>Description</th>
*   </tr>
*   <tr>
*     <td>Start Sample</td>
*     <td>ptrSSCallback</td>
*     <td>It is called before each sensor scan triggering. This kind of callback 
*         could be used to implement a user-specific use cases like changing scan 
*         parameters depending on whether a sensor is going to be scanned.</td>
*   </tr>
*   <tr>
*     <td>End Of Scan</td>
*     <td>ptrEOSCallback</td>
*     <td>It is called after when sensor scan is finished and there is no other 
*         sensors in queue to be scanned.</td>
*   </tr>
* </table>
*
* \note
* These callbacks are called by the Cy_CapSense_InterruptHandler() function and
* lengthen its execution. Usually, Cy_CapSense_InterruptHandler() is called inside the
* CSD ISR. In such a case, the callbacks mentioned lengthen the execution of the CSD ISR as well.
*
* To assign user's functions to these callbacks present in the middleware, 
* perform the following:
*   
* * Write the function implementation (in any user file) using the following 
*   function prototype:
*   
*   <tt>void CallbackFunction(cy_stc_active_scan_sns_t * ptrActiveScan);</tt>
*   
* * Use the \ref Cy_CapSense_RegisterCallback() API to register the callback
*   function to be executed when the \ref cy_en_capsense_callback_event_t occurs, 
*   or assign the function to a corresponding CapSense Data Structure register, 
*   located in cy_stc_capsense_common_context_t:
*   
*   <tt>context-\>ptrCommonContext-\>\<Associated Register\> = \&CallbackFunction;</tt>
* 
* To unassign user's functions from these callbacks present in the middleware, 
* perform the following:
*
* * Use the \ref Cy_CapSense_UnRegisterCallback() API to unregister the callback
*   function that was previously assigned to be the executer when the specified
*   \ref cy_en_capsense_callback_event_t occurs, or assign NULL to a corresponding
*   CapSense Data Structure register, located in cy_stc_capsense_common_context_t:
*
*   <tt>context-\>ptrCommonContext-\>\<Associated Register\> = NULL;</tt>
*
* \} */


#if !defined(CY_CAPSENSE_H)
#define CY_CAPSENSE_H

#include "cy_device_headers.h"
#include "cy_capsense_common.h"
#include "cy_capsense_centroid.h"
#include "cy_capsense_control.h"
#include "cy_capsense_csd.h"
#include "cy_capsense_csx.h"
#include "cy_capsense_filter.h"
#include "cy_capsense_lib.h"
#include "cy_capsense_gesture_lib.h"
#include "cy_capsense_processing.h"
#include "cy_capsense_sensing.h"
#include "cy_capsense_structure.h"
#include "cy_capsense_tuner.h"

#ifndef CY_IP_MXCSDV2
    #error "The CSD driver is not supported on this device"
#endif

#endif /* CY_CAPSENSE_H */


/* [] END OF FILE */
