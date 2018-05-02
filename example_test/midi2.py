import mido
from mido import Message

#print("Output names: %s" % mido.get_output_names())
midi_out = mido.open_output('Mbed Audio 1')

msg = Message('note_on')
midi_out.send(msg)
msg = Message('note_off')
midi_out.send(msg)

#msg = Message('sysex', data=bytearray(b'ABC')*100)
#midi_out.send(msg)
midi_out.close()