/* mbed Microcontroller Library
 * Copyright (c) 2016-2016 ARM Limited
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef MBED_BLOCK_DEVICE_H
#define MBED_BLOCK_DEVICE_H

#include "mbed.h"

class BlockDevice {
public:
    virtual int init(void) = 0;
    virtual int uninit(void) = 0;
    virtual int read(uint8_t* buffer, uint32_t offset, uint32_t size) = 0;
    virtual int write(const uint8_t* buffer, uint32_t offset, uint32_t size) = 0;
    virtual int sync(void) = 0;
    virtual uint32_t block_size(void) = 0;
    virtual uint64_t total_size(void) = 0;
    virtual ~BlockDevice(void) {}
};

#endif
