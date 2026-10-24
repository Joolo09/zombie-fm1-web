# SPDX-License-Identifier: MIT
"""Original Zombie Groovebox 0.8 controller profile for Live 11/12 (Python 3).

Uses Live's installed _Framework, without distributing or copying it.
Notes are never forwarded to the script or echoed to the hardware.
Parameters use EncoderElement mappings so Live handles MIDI mapping/automation.
No track arming, routing or project changes occur merely by loading the profile.
"""
import time
import Live
from _Framework.ControlSurface import ControlSurface
from _Framework.EncoderElement import EncoderElement
from _Framework.InputControlElement import MIDI_CC_TYPE
from .settings import MACRO_PARAMETER_INDICES, RECORD_TARGET

MACRO_CCS = (74, 71, 73, 72)
CONTROL_CCS = (110, 114, 115, 116)


def pack7(data):
    out = []
    for at in range(0, len(data), 7):
        block = data[at:at + 7]
        out.append(sum(((value >> 7) & 1) << bit for bit, value in enumerate(block)))
        out.extend(value & 127 for value in block)
    return out


def screen_name(text):
    out = bytearray()
    for char in str(text):
        code = ord(char)
        if not (32 <= code < 127 or 0x400 <= code < 0x460):
            char = '?'
        raw = char.encode('utf-8')
        if len(out) + len(raw) > 47:
            break
        out.extend(raw)
    return list(out)


def midi_value(parameter, default=0):
    if parameter is None or parameter.max <= parameter.min:
        return default
    return max(0, min(127, int(round((parameter.value - parameter.min) * 127.0 / (parameter.max - parameter.min)))))


class ZombieCoreFM1(ControlSurface):
    def __init__(self, c_instance):
        if len(MACRO_PARAMETER_INDICES) != 4 or any(not isinstance(n,int) or n < 0 for n in MACRO_PARAMETER_INDICES):
            raise ValueError('Configure exactly four nonnegative parameter indices')
        if RECORD_TARGET not in ('arrangement','session'):
            raise ValueError('RECORD_TARGET must be arrangement or session')
        self._ready = False
        super().__init__(c_instance)
        self._host = c_instance
        self._bindings = None
        self._sent_at = -1.0
        self._last_feedback = None
        self._seq = 0
        self._last_error = None
        # The hardware can send on any configured track channel, not just channel 1.
        with self.component_guard():
            self._encoders = tuple(tuple(EncoderElement(MIDI_CC_TYPE, channel, cc,
                Live.MidiMap.MapMode.absolute, name='ZC_CC%d_CH%d' % (cc, channel + 1))
                for channel in range(16)) for cc in MACRO_CCS + (7, 10))
        self._ready = True
        self._refresh_bindings()

    def build_midi_map(self, midi_map_handle):
        super().build_midi_map(midi_map_handle)
        for cc in CONTROL_CCS:
            Live.MidiMap.forward_midi_cc(self._host.handle(), midi_map_handle, 15, cc)
        # No forward_midi_note: Track input and Live's record/monitor path own the keys.

    def _target(self):
        song = self.song()
        track = song.view.selected_track
        device = getattr(getattr(track, 'view', None), 'selected_device', None)
        return song, track, device

    def _refresh_bindings(self):
        song, track, device = self._target()
        parameters = tuple(device.parameters) if device is not None else ()
        macros = tuple(parameters[index] if 0 <= index < len(parameters) and
                       parameters[index].is_enabled else None for index in MACRO_PARAMETER_INDICES)
        mixer = getattr(track, 'mixer_device', None)
        bindings = macros + (getattr(mixer, 'volume', None), getattr(mixer, 'panning', None))
        if bindings != self._bindings:
            with self.component_guard():
                for group, parameter in zip(self._encoders, bindings):
                    for encoder in group:
                        if parameter is None:
                            encoder.release_parameter()
                        else:
                            encoder.connect_to(parameter)
            self._bindings = bindings

    def _command(self, command, payload):
        self._seq = self._seq % 127 + 1
        self._send_midi(tuple([240, 125, 90, 67, self._seq, command] + pack7(payload) + [247]))

    def _feedback(self, force=False):
        now = time.monotonic()
        if not force and now - self._sent_at < .25:
            return
        song, track, device = self._target()
        tracks = tuple(song.tracks)
        selected = tracks.index(track) if track in tracks and tracks.index(track) < 6 else 255
        tempo = max(30, min(300, int(round(song.tempo))))
        mixer = getattr(track, 'mixer_device', None)
        volume = midi_value(getattr(mixer, 'volume', None))
        pan = getattr(mixer, 'panning', None)
        pan_value = max(-1.0, min(1.0, pan.value)) if pan is not None else 0.0
        pan = max(0, min(127, int(round(64 + pan_value * (64 if pan_value < 0 else 63)))))
        recording = song.session_record if RECORD_TARGET == 'session' else song.record_mode
        name = device.name if device is not None else getattr(track, 'name', 'SELECT DEVICE')
        payload = [int(bool(song.is_playing)), int(bool(recording)), tempo & 255, tempo >> 8,
                   selected, volume, pan] + screen_name(name)
        if not force and payload == self._last_feedback and now - self._sent_at < 1.0:
            return
        self._command(33, [2])  # Optional host label: Ableton. Zombie Core 0.6.1.
        self._command(32, payload)
        self._last_feedback, self._sent_at = payload, now

    def update_display(self):
        super().update_display()
        if not self._ready:
            return
        try:
            self._refresh_bindings()
            self._feedback()
            self._last_error = None
        except (RuntimeError, AttributeError) as error:
            # A deleted/replaced Live object can disappear between two callbacks.
            if str(error) != self._last_error:
                self.log_message('Zombie Groovebox FM-1: ' + str(error))
                self._last_error = str(error)

    def receive_midi(self, midi_bytes):
        data = tuple(midi_bytes)
        if len(data) >= 7 and data[:4] == (240, 125, 90, 67) and data[5] in (96, 97):
            return
        if len(data) == 3 and data[0] == 0xBF and data[1] in CONTROL_CCS:
            cc, value = data[1], data[2]
            if not 0 <= value <= 127:
                return
            song = self.song()
            if cc == 110:
                tracks = tuple(song.tracks)
                if value < min(6, len(tracks)):
                    song.view.selected_track = tracks[value]
            elif cc == 114:
                wanted = value >= 64
                if wanted != bool(song.is_playing):
                    song.start_playing() if wanted else song.stop_playing()
            elif cc == 116:
                song.tempo = max(30.0, min(300.0, song.tempo + value - 64))
            elif cc == 115:
                wanted = value >= 64
                if RECORD_TARGET == 'session':
                    if wanted != bool(song.session_record):
                        song.session_record = wanted
                elif wanted != bool(song.record_mode):
                    song.record_mode = wanted
            self._refresh_bindings()
            self._feedback(True)
            return
        super().receive_midi(data)

    def disconnect(self):
        self._ready = False
        with self.component_guard():
            for group in self._encoders:
                for encoder in group:
                    encoder.release_parameter()
        self._bindings = None
        super().disconnect()
