# name=Zombie Groovebox FM-1
# supportedDevices=Zombie Groovebox FM-1
"""Original MIT companion for Zombie Groovebox 0.8.x. Notes/macros use FL's normal MIDI path.

ALGO selects the first six channels in the current Channel Rack group.
SELECT controls BPM (Groovebox 0.8.2+); PRESET controls pan.
K1..K4: CC74/71/73/72, left unhandled for FL MIDI Learn/Multilink.
Channel 16 CC110/114/115/116 is reserved for selection, transport and relative BPM.
No MIDI notes are echoed back to FM-1. No project/files are changed by OnInit.
"""
import time
import channels
import device
import mixer
import transport
import plugins
import midi

# None: FL MIDI Learn / Multilink. Or four explicit plugin parameter indices.
MACRO_PARAMETER_INDICES = None
MACRO_CCS = (74, 71, 73, 72)

_last_feedback = None
_sent_at = -1.0
_seq = 0


def _pack7(data):
    result = bytearray()
    for at in range(0, len(data), 7):
        block = data[at:at + 7]
        result.append(sum(((value >> 7) & 1) << bit for bit, value in enumerate(block)))
        result.extend(value & 127 for value in block)
    return result


def _name_bytes(text):
    out = bytearray()
    for char in text:
        code = ord(char)
        if not (32 <= code < 127 or 0x400 <= code < 0x460):
            char = '?'
        encoded = char.encode('utf-8')
        if len(out) + len(encoded) > 47:
            break
        out.extend(encoded)
    return out


def _feedback(force=False):
    global _last_feedback, _sent_at, _seq
    if not device.isAssigned():
        return
    now = time.monotonic()
    if not force and now - _sent_at < .25:
        return
    count = channels.channelCount()
    selected = channels.selectedChannel(1)
    valid = 0 <= selected < count
    name = channels.getChannelName(selected) if valid else 'SELECT A PLUGIN'
    tempo = max(30, min(300, int(round(mixer.getCurrentTempo()))))
    volume = max(0, min(127, int(round(channels.getChannelVolume(selected) * 127)))) if valid else 0
    pan = max(-1.0, min(1.0, channels.getChannelPan(selected))) if valid else 0.0
    pan = max(0, min(127, int(round(64 + pan * (64 if pan < 0 else 63)))))
    payload = bytearray([int(bool(transport.isPlaying())), int(bool(transport.isRecording())),
                         tempo & 255, tempo >> 8, selected if valid and selected < 6 else 255,
                         volume, pan]) + _name_bytes(name)
    if not force and payload == _last_feedback and now - _sent_at < 1.0:
        return
    _seq = _seq % 127 + 1
    device.midiOutSysex(bytes([240,125,90,67,_seq,33,0,1,247]))
    _seq = _seq % 127 + 1
    frame = bytearray([0xF0, 0x7D, 0x5A, 0x43, _seq, 32]) + _pack7(payload) + bytearray([0xF7])
    device.midiOutSysex(bytes(frame))
    _last_feedback, _sent_at = payload, now


def OnInit():
    global _last_feedback, _sent_at
    _last_feedback, _sent_at = None, -1.0
    _feedback(True)


def OnIdle():
    _feedback()


def OnRefresh(flags):
    # Refreshes can be very frequent; the same rate limiter covers both callbacks.
    _feedback()


def OnSysEx(event):
    data = bytes(event.sysex)
    if len(data) >= 7 and data[:4] == bytes([0xF0, 0x7D, 0x5A, 0x43]) and data[5] in (32 | 64, 33 | 64):
        event.handled = True


def OnMidiMsg(event):
    if event.status & 0xF0 != 0xB0:
        return  # FL handles notes, velocity, releases and other normal MIDI messages.
    cc, value = event.data1, max(0, min(127, event.data2))
    ch = event.status & 15
    if ch == 15 and cc == 114:
        wanted = value >= 64
        if wanted != bool(transport.isPlaying()):
            transport.start() if wanted else transport.stop()
        event.handled = True
    elif ch == 15 and cc == 115:
        if (value >= 64) != bool(transport.isRecording()):
            transport.record()
        event.handled = True
    elif ch == 15 and cc == 116:
        delta = value - 64
        if delta:
            transport.globalTransport(midi.FPT_TempoJog, delta * 10,
                                      getattr(event, 'pmeflags', midi.PME_System))
        event.handled = True
    elif ch == 15 and cc == 110:
        if value < min(6, channels.channelCount()):
            channels.selectOneChannel(value)
        event.handled = True
    elif cc in MACRO_CCS and MACRO_PARAMETER_INDICES is not None:
        selected = channels.selectedChannel(1)
        if len(MACRO_PARAMETER_INDICES) == 4 and 0 <= selected < channels.channelCount() and plugins.isValid(selected):
            index = MACRO_PARAMETER_INDICES[MACRO_CCS.index(cc)]
            if isinstance(index, int) and 0 <= index < plugins.getParamCount(selected):
                plugins.setParamValue(value / 127.0, index, selected)
                event.handled = True
    elif cc in (7, 10):
        selected = channels.selectedChannel(1)
        if 0 <= selected < channels.channelCount():
            if cc == 7:
                channels.setChannelVolume(selected, value / 127.0)
            else:
                channels.setChannelPan(selected, (value - 64) / (64.0 if value < 64 else 63.0))
        event.handled = True


def OnDoFullRefresh():
    _feedback(True)
