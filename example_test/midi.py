import time
import rtmidi

midiout = rtmidi.MidiOut()
midiIn = rtmidi.MidiIn()
available_ports = midiout.get_ports()
print("PortsOut %s" % available_ports)
print("PortsIn %s" % midiIn.get_ports())

#exit(0)

if available_ports:
    midiout.open_port(1)
else:
    midiout.open_virtual_port("My virtual output")

note_on = [0x90, 60, 112] # channel 1, middle C, velocity 112
note_off = [0x80, 60, 0]
midiout.send_message(note_on)
time.sleep(0.5)
midiout.send_message(note_off)

del midiout