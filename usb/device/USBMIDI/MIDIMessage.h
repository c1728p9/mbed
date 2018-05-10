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

#ifndef MIDIMESSAGE_H
#define MIDIMESSAGE_H

#include "mbed.h"

#define MAX_MIDI_MESSAGE_SIZE 256 // Max message size. SysEx can be up to 65536 but 256 should be fine for most usage

// MIDI Message Format
//
// [ msg(4) | channel(4) ] [ 0 | n(7) ] [ 0 | m(7) ]
//
// MIDI Data Messages (Channel Specific)
//
// Message               msg n          m
// ---------------------------------------------
// Note Off              0x8 Key        Velocity
// Note On               0x9 Key        Velocity
// Polyphonic Aftertouch 0xA Key        Pressure
// Control Change        0xB Controller Value
// Program Change        0xC Program    -
// Channel Aftertouch    0xD Pressure   -
// Pitch Wheel           0xE LSB        MSB

#define CABLE_NUM (0<<4)

/** A MIDI message container */
class MIDIMessage {
public:

    MIDIMessage() : _data(new uint8_t[MAX_MIDI_MESSAGE_SIZE + 1]), _length(4) {}

    MIDIMessage(uint8_t *buf) : _data(new uint8_t[MAX_MIDI_MESSAGE_SIZE + 1]), _length(4)
    {
        for (int i = 0; i < 4; i++) {
            _data[i] = buf[i];
        }
    }

    MIDIMessage(const MIDIMessage& other)
    {
        *this = other;
    }

    MIDIMessage& operator=(const MIDIMessage& other)
    {
        _length = other._length;
        for (int i = 0; i < _length; i++) {
            _data[i] = other._data[i];
        }

         return *this;
     }

    ~MIDIMessage()
    {
        delete[] _data;
    }

    void from_raw(uint8_t *buf, int buf_len)
    {
        _length = buf_len + 1;
        if (_length > MAX_MIDI_MESSAGE_SIZE) {
            _length = MAX_MIDI_MESSAGE_SIZE;
        }

        // first byte keeped for retro-compatibility
        _data[0] = CABLE_NUM | 0x08;

        for (int i = 0; i < buf_len; i++) {
            _data[i + 1] = buf[i];
        }
    }

    // create messages

    /** Create a NoteOff message
     * @param key Key ID
     * @param velocity Key velocity (0-127, default = 127)
     * @param channel Key channel (0-15, default 0)
     * @returns A MIDIMessage
     */
    static MIDIMessage NoteOff(int key, int velocity = 127, int channel = 0)
    {
        MIDIMessage msg;
        msg._data[0] = CABLE_NUM | 0x08;
        msg._data[1] = 0x80 | (channel & 0x0F);
        msg._data[2] = key & 0x7F;
        msg._data[3] = velocity & 0x7F;
        return msg;
    }

    /** Create a NoteOn message
     * @param key Key ID
     * @param velocity Key velocity (0-127, default = 127)
     * @param channel Key channel (0-15, default 0)
     * @returns A MIDIMessage
     */
    static MIDIMessage NoteOn(int key, int velocity = 127, int channel = 0)
    {
        MIDIMessage msg;
        msg._data[0] = CABLE_NUM | 0x09;
        msg._data[1] = 0x90 | (channel & 0x0F);
        msg._data[2] = key & 0x7F;
        msg._data[3] = velocity & 0x7F;
        return msg;
    }

    /** Create a PolyPhonic Aftertouch message
     * @param key Key ID
     * @param pressure Aftertouch pressure (0-127)
     * @param channel Key channel (0-15, default 0)
     * @returns A MIDIMessage
     */
    static MIDIMessage PolyphonicAftertouch(int key, int pressure, int channel = 0)
    {
        MIDIMessage msg;
        msg._data[0] = CABLE_NUM | 0x0A;
        msg._data[1] = 0xA0 | (channel & 0x0F);
        msg._data[2] = key & 0x7F;
        msg._data[3] = pressure & 0x7F;
        return msg;
    }

    /** Create a Control Change message
     * @param control Controller ID
     * @param value Controller value (0-127)
     * @param channel Controller channel (0-15, default 0)
     * @returns A MIDIMessage
     */
    static MIDIMessage ControlChange(int control, int value, int channel = 0)
    {
        MIDIMessage msg;
        msg._data[0] = CABLE_NUM | 0x0B;
        msg._data[1] = 0xB0 | (channel & 0x0F);
        msg._data[2] = control & 0x7F;
        msg._data[3] = value & 0x7F;
        return msg;
    }

    /** Create a Program Change message
     * @param program Program ID
     * @param channel Channel (0-15, default 0)
     * @returns A MIDIMessage
     */
    static MIDIMessage ProgramChange(int program, int channel = 0)
    {
        MIDIMessage msg;
        msg._data[0] = CABLE_NUM | 0x0C;
        msg._data[1] = 0xC0 | (channel & 0x0F);
        msg._data[2] = program & 0x7F;
        msg._data[3] = 0x00;
        return msg;
    }

    /** Create a Channel Aftertouch message
     * @param pressure Pressure
     * @param channel Key channel (0-15, default 0)
     * @returns A MIDIMessage
     */
    static MIDIMessage ChannelAftertouch(int pressure, int channel = 0)
    {
        MIDIMessage msg;
        msg._data[0] = CABLE_NUM | 0x0D;
        msg._data[1] = 0xD0 | (channel & 0x0F);
        msg._data[2] = pressure & 0x7F;
        msg._data[3] = 0x00;
        return msg;
    }

    /** Create a Pitch Wheel message
     * @param pitch Pitch (-8192 - 8191, default = 0)
     * @param channel Channel (0-15, default 0)
     * @returns A MIDIMessage
     */
    static MIDIMessage PitchWheel(int pitch = 0, int channel = 0)
    {
        MIDIMessage msg;
        int p = pitch + 8192;    // 0 - 16383, 8192 is center
        msg._data[0] = CABLE_NUM | 0x0E;
        msg._data[1] = 0xE0 | (channel & 0x0F);
        msg._data[2] = p & 0x7F;
        msg._data[3] = (p >> 7) & 0x7F;
        return msg;
    }

    /** Create an All Notes Off message
     * @param channel Channel (0-15, default 0)
     * @returns A MIDIMessage
     */
    static MIDIMessage AllNotesOff(int channel = 0)
    {
        return ControlChange(123, 0, channel);
    }

    /** Create a SysEx message
    * @param data SysEx data (including 0xF0 .. 0xF7)
    * @param len SysEx data length
    * @returns A MIDIMessage
    */
    static MIDIMessage SysEx(uint8_t *data, int len)
    {
        MIDIMessage msg;// = MIDIMessage(data, len);
        msg.from_raw(data, len);
        return msg;
    }

    // decode messages

    /** MIDI Message Types */
    enum MIDIMessageType {
        ErrorType,
        NoteOffType,
        NoteOnType,
        PolyphonicAftertouchType,
        ControlChangeType,
        ProgramChangeType,
        ChannelAftertouchType,
        PitchWheelType,
        AllNotesOffType,
        SysExType
    };

    /** Read the message type
     *
     * @returns MIDIMessageType
     */
    MIDIMessageType type()
    {
        MIDIMessageType message_type;
        uint8_t min_size;
        switch ((_data[1] >> 4) & 0xF) {
            case 0x8:
                // message, channel
                // key
                // velocity
                min_size = 3;
                message_type = NoteOffType;
                break;
            case 0x9:
                // message, channel
                // key
                // velocity
                min_size = 3;
                message_type = NoteOnType;
                break;
            case 0xA:
                // message, channel
                // key
                // pressure
                min_size = 3;
                message_type = PolyphonicAftertouchType;
                break;
            case 0xB:
                // message, channel
                // controller
                min_size = 2;
                if (controller() < 120) { // standard controllers
                    message_type = ControlChangeType;
                } else if (controller() == 123) {
                    message_type = AllNotesOffType;
                } else {
                    message_type = ErrorType; // unsupported atm
                }
                break;
            case 0xC:
                // message, channel
                // program
                min_size = 2;
                message_type = ProgramChangeType;
                break;
            case 0xD:
                // message, channel
                // pressure
                min_size = 2;
                message_type = ChannelAftertouchType;
                break;
            case 0xE:
                // message, channel
                // pitch lsb
                // pitch msb
                min_size = 3;
                message_type = PitchWheelType;
                break;
            case 0xF:
                min_size = 2;
                message_type = SysExType;
                break;
            default:
                message_type = ErrorType;
                break;
        }


        if (_length < min_size) {
            // too small to be a valid message
            message_type = ErrorType;
        }
        return message_type;
    }

    /**
     * Read the channel number
     *
     * @return channel number or -1 on error
     */

    int channel()
    {
        return (_data[1] & 0x0F);
    }

    /** Read the key ID */
    int key()
    {
        MIDIMessageType msg_type = type();
        if ((msg_type != NoteOffType) &&
            (msg_type != NoteOnType) &&
            (msg_type != PolyphonicAftertouchType)) {
            return -1;
        }

        return _data[2] & 0x7F;
    }

    /** Read the velocity */
    int velocity()
    {
        MIDIMessageType msg_type = type();
        if ((msg_type != NoteOffType) &&
            (msg_type != NoteOnType)) {
            return -1;
        }

        return _data[3] & 0x7F;
    }

    /** Read the controller value */
    int value()
    {
        MIDIMessageType msg_type = type();
        if ((msg_type != ControlChangeType) &&
            (msg_type != AllNotesOffType)) {
            return -1;
        }

        // ControlChangeType
        // AllNotesOffType
        return _data[3] & 0x7F;
    }

    /** Read the aftertouch pressure */
    int pressure()
    {
        MIDIMessageType msg_type = type();
        if ((msg_type != PolyphonicAftertouchType) &&
            (msg_type != ChannelAftertouchType)) {
            return -1;
        }

        // PolyphonicAftertouchType
        if (type() == PolyphonicAftertouchType) {
            return _data[3] & 0x7F;
        } else {
            // ChannelAftertouchType
            return _data[2] & 0x7F;
        }
    }

    /** Read the controller number */
    int controller()
    {
        MIDIMessageType msg_type = type();
        if ((msg_type != ControlChangeType) &&
            (msg_type != AllNotesOffType)) {
            return -1;
        }

        return _data[2] & 0x7F;
    }

    /** Read the program number */
    int program()
    {
        MIDIMessageType msg_type = type();
        if (msg_type != ProgramChangeType) {
            return -1;
        }

        // ProgramChangeType
        return _data[2] & 0x7F;
    }

    /** Read the pitch value */
    int pitch()
    {
        MIDIMessageType msg_type = type();
        if (msg_type != PitchWheelType) {
            return -1;
        }

        // PitchWheelType
        int p = ((_data[3] & 0x7F) << 7) | (_data[2] & 0x7F);
        return p - 8192; // 0 - 16383, 8192 is center
    }

    uint8_t *_data;
    uint16_t _length;
};

#endif
