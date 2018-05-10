#include "mbed.h"
#include "USBMIDI.h"
#include "test_util.h"

USBMIDI midi;

void stub() {}

int main_midi()
{
    printf("Running MIDI test\r\n");
    midi.attach(stub);

    wait(5);

    while (1) {
        if (midi.readable()) {

            MIDIMessage message;
            midi.read(&message);

            if (message.type() == MIDIMessage::NoteOnType) {
                printf("Press %i\r\n", message.key());
            } else if (message.type() == MIDIMessage::NoteOffType) {
                printf("Release %i\r\n", message.key());
            }
        }
    }

    while (1) {
        for(int i=48; i<83; i++) {     // send some messages!
            midi.write(MIDIMessage::NoteOn(i));
            wait(0.25);
            midi.write(MIDIMessage::NoteOff(i));
            wait(0.5);
        }
    }

    printf("MIDI test passed\r\n");

    return 0;
}
