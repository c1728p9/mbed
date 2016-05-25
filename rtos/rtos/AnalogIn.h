/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
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
#ifndef RTOS_ANALOGIN_H
#define RTOS_ANALOGIN_H

#include "platform.h"

#if DEVICE_ANALOGIN

#include "analogin_api.h"
#include "Mutex.h"

#ifndef LEGACY_DRIVERS
namespace mbed {
#else
namespace rtos {
#endif

/** An analog input, used for reading the voltage on a pin
 *
 * Example:
 * @code
 * // Print messages when the AnalogIn is greater than 50%
 *
 * #include "mbed.h"
 *
 * AnalogIn temperature(p20);
 *
 * int main() {
 *     while(1) {
 *         if(temperature > 0.5) {
 *             printf("Too hot! (%f)", temperature.read());
 *         }
 *     }
 * }
 * @endcode
 */
extern rtos::Mutex _AnalogIn_mutex;
class AnalogIn {

public:

    /** Create an AnalogIn, connected to the specified pin
     *
     * @param pin AnalogIn pin to connect to
     * @param name (optional) A string to identify the object
     */
    AnalogIn(PinName pin) {
        _AnalogIn_mutex.lock();
        analogin_init(&_adc, pin);
        _AnalogIn_mutex.unlock();
    }

    /** Read the input voltage, represented as a float in the range [0.0, 1.0]
     *
     * @returns A floating-point value representing the current input voltage, measured as a percentage
     */
    float read() {
        float temp;
        _AnalogIn_mutex.lock();
        temp = analogin_read(&_adc);
        _AnalogIn_mutex.unlock();
        return temp;
    }

    /** Read the input voltage, represented as an unsigned short in the range [0x0, 0xFFFF]
     *
     * @returns
     *   16-bit unsigned short representing the current input voltage, normalised to a 16-bit value
     */
    unsigned short read_u16() {
        float temp;
        _AnalogIn_mutex.lock();
        temp = analogin_read_u16(&_adc);
        _AnalogIn_mutex.unlock();
        return temp;
    }

#ifdef MBED_OPERATORS
    /** An operator shorthand for read()
     *
     * The float() operator can be used as a shorthand for read() to simplify common code sequences
     *
     * Example:
     * @code
     * float x = volume.read();
     * float x = volume;
     *
     * if(volume.read() > 0.25) { ... }
     * if(volume > 0.25) { ... }
     * @endcode
     */
    operator float() {
        float temp;
        _AnalogIn_mutex.lock();
        temp = read();
        _AnalogIn_mutex.unlock();
        return temp;
    }
#endif

protected:
    analogin_t _adc;
};

} // namespace mbed


#endif

#endif
