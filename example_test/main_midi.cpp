#include "mbed.h"
#include "USBMIDI.h"
#include "test_util.h"

void release_sem(Semaphore *sem)
{
    sem->release();
}

void print_message(MIDIMessage &m)
{
    switch (m.type()) {
    case MIDIMessage::ErrorType:
        printf("MIDI ErrorType\r\n");
        break;
    case MIDIMessage::NoteOffType:
        printf("NoteOffType channel %i key %i velocity %i\r\n", m.channel(), m.key(), m.velocity());
        break;
    case MIDIMessage::NoteOnType:
        printf("NoteOnType channel %i key %i velocity %i\r\n", m.channel(), m.key(), m.velocity());
        break;
    case MIDIMessage::PolyphonicAftertouchType:
        printf("PolyphonicAftertouchType channel %i key %i pressure %i\r\n", m.channel(), m.key(), m.pressure());
        break;
    case MIDIMessage::ControlChangeType:
        printf("ControlChangeType channel %i controller %i value %i\r\n", m.channel(), m.controller(), m.value());
        break;
    case MIDIMessage::ProgramChangeType:
        printf("ProgramChangeType channel %i program %i\r\n", m.channel(), m.program());
        break;
    case MIDIMessage::ChannelAftertouchType:
        printf("ChannelAftertouchType channel %i pressure %i\r\n", m.channel(), m.pressure());
        break;
    case MIDIMessage::PitchWheelType:
        printf("PitchWheelType channel %i pitch %i\r\n", m.channel(), m.pitch());
        break;
    case MIDIMessage::ResetAllControllersType:
        printf("ResetAllControllersType channel %i controller %i value %i\r\n", m.channel(), m.controller(), m.value());
        break;
    case MIDIMessage::AllNotesOffType:
        printf("AllNotesOffType channel %i controller %i value %i\r\n", m.channel(), m.controller(), m.value());
        break;
    case MIDIMessage::SysExType:
        printf("SysExType size %i\r\n", m.length);
        break;
    }
}

int main_midi()
{
    printf("Running MIDI test\r\n");

    Semaphore can_read;
    USBMIDI midi;

    printf("Open a midi player and select \"Mbed Audio\" as your input device.\r\nThen press enter\r\n");
    test_util_wait_line();
    test_util_countdown(3);
    printf("Playing notes\r\n");

    for(int i=48; i<83; i++) {     // send some messages!
        midi.write(MIDIMessage::NoteOn(i));
        wait(0.1);
        midi.write(MIDIMessage::NoteOff(i));
        wait(0.2);
    }
    printf("Did you hear notes play?\r\n");
    MBED_ASSERT(test_util_get_response());
    printf("\r\n");

    printf("Open a midi player and select \"Mbed Audio\" as your output device.\r\nPlay a song for at least 10 seconds\r\n");
    midi.attach(Callback<void()>(&release_sem, &can_read));
    can_read.wait();

    Timer timer;
    timer.start();
    while (timer.read_ms() < 10000) {

        while (midi.readable()) {

            MIDIMessage message;
            midi.read(&message);
            print_message(message);
        }

        can_read.wait();
    }

    printf("MIDI test passed\r\n");

    return 0;
}
