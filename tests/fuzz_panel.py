"""Random panel, key, encoder and MIDI sessions on the host simulator; checks firmware invariants.

  python -X utf8 tests/fuzz_panel.py HOST_BINARY [--runs N] [--steps N] [--seed S] [--out DIR]

Build HOST_BINARY with -fsanitize=address,undefined to also catch memory errors. A failing run's script is kept
in DIR (default source/output/fuzz) so it replays exactly: HOST_BINARY DIR/fail-SEED.txt DIR
"""
import argparse, random, struct, subprocess, sys, zlib
from pathlib import Path

BUTTONS = ['FX', 'SEL', 'ENV', 'LFO', 'EDIT', 'GLO', 'HOME', 'SAVE', 'ARP', 'SEQ', 'PLAY', 'REC', 'OCT-', 'OCT+']
ENCODERS = ['SELECT', 'ALGO', 'PRESET', 'K1', 'K2', 'K3', 'K4']


def midi(r, sounding):
    ch = r.randrange(16)
    kind = r.choice(['on', 'on', 'off', 'cc', 'bend', 'clock', 'rt', 'pc'])
    if kind == 'on':
        note = r.randrange(128); sounding.add((ch, note))       # a held external note is legitimately held
        return f'midi USB {0x90 | ch:02X} {note:02X} {r.randrange(128):02X}'
    if kind == 'off': return f'midi USB {0x80 | ch:02X} {r.randrange(128):02X} 00'
    if kind == 'cc': return f'midi USB {0xB0 | ch:02X} {r.choice([1, 7, 10, 64, 74, 71, 73, 72, 110, 114, 115, 116, 120, 123, r.randrange(128)]):02X} {r.randrange(128):02X}'
    if kind == 'bend': return f'midi USB {0xE0 | ch:02X} {r.randrange(128):02X} {r.randrange(128):02X}'
    if kind == 'pc': return f'midi USB {0xC0 | ch:02X} {r.randrange(128):02X} 00'
    if kind == 'clock': return 'midi USB F8 00 00'
    return f'midi USB {r.choice(["FA", "FB", "FC"])} 00 00'


def frame(cmd, payload, seq=1):
    """Editor protocol frame without F0/F7: 7D 5A 43 seq cmd, then groups of (high bits, 7 low bytes)."""
    out = [0x7D, 0x5A, 0x43, seq & 127, cmd & 127]
    for i in range(0, len(payload), 7):
        group = payload[i:i + 7]; out.append(sum(((b >> 7) & 1) << j for j, b in enumerate(group))); out += [b & 127 for b in group]
    return 'sysex ' + ''.join(f'{b:02X}' for b in out)


EDGE = [0, 1, 2, 3, 4, 5, 6, 7, 15, 16, 17, 23, 24, 26, 27, 31, 32, 48, 63, 64, 65, 96, 127, 128, 129, 200, 254, 255]


def project(r):
    """A z_project (3452 B) that should pass valid_project, with an occasional edge value poked in."""
    p = bytearray(3452); struct.pack_into('<IHHBBBB', p, 0, 0x31434F5A, 3, r.randint(30, 300), r.randint(0, 60), r.randrange(6), r.randrange(2), 0)
    desc = [(0,7,0),(0,3,1),(0,127,64),(0,127,18),(0,127,100),(0,127,24),(0,127,2),(0,127,45),(0,127,90),(0,127,35),(0,127,35),(1,16,2),(0,127,0),(0,127,0),(0,127,15),(0,127,95),(-64,63,0),(0,127,30),(0,127,0),(0,13,0),(0,127,60),(0,1,0),(1,16,1),(1,64,16),(1,127,80),(-24,24,0),(0,127,127)]
    o = 12
    for t in range(6):
        for lo, hi, d in desc: struct.pack_into('<h', p, o, r.choice([lo, hi, d, r.randint(lo, hi)])); o += 2
        for i in range(64): p[o:o + 4] = bytes([r.randrange(128), r.choice([0, 0, r.randrange(128)]), r.randrange(128), r.randint(1, 127)]); o += 4
    p[o] = (r.randint(-3, 3)) & 255; o += 1
    sec = r.randrange(9); p[o] = sec; p[o + 1] = r.randrange(6 if sec == 6 else 4); cur = r.randrange(64); p[o + 2] = r.randrange(4); o += 3
    for t in range(6): p[o + t] = r.randrange(5); p[o + 6 + t] = r.randrange(4); p[o + 12 + t] = r.randint(1, 3); p[o + 18 + t] = r.randrange(2); p[o + 24 + t] = r.choice([255, r.randrange(96)])
    o += 30; p[o] = 0; p[o + 1] = cur; o += 2
    for i in range(6 * 64 * 3): p[o + i] = r.choice([0, 0, 0, r.randint(1, 128)])
    o += 6 * 64 * 3
    for i in range(6 * 64): p[o + i] = r.choice([0, 0, r.randrange(64)])
    o += 6 * 64; p[o:o + 5] = bytes([r.randrange(2), r.randrange(2), r.randrange(2), r.randrange(128), r.randrange(4)]); p[o + 6] = 0xD1 if r.random() < .5 else 0
    if r.random() < .3: p[r.randrange(len(p))] = r.choice(EDGE)
    return p


def drums(r):
    d = bytearray(3208); struct.pack_into('<IHBB', d, 0, 0x314D445A, 1, r.randrange(16), 0)
    for s in range(16): d[8 + s * 8:16 + s * 8] = bytes([r.randrange(128), r.randint(1, 127), r.randint(-24, 24) & 255, r.randint(-64, 63) & 255, r.randrange(2), r.randint(1, 64), 0, 0])
    for i in range(16 * 64): d[136 + i * 3:139 + i * 3] = bytes([r.choice([0, 0, r.randrange(128)]), r.randrange(128), r.randint(1, 4)])
    if r.random() < .3: d[r.randrange(len(d))] = r.choice(EDGE)
    return d


def upload(r, begin, chunk, commit, data):
    lines = [frame(begin, list(struct.pack('<I', zlib.crc32(bytes(data)))))]
    for at in range(0, len(data), 48): lines.append(frame(chunk, [at & 255, at >> 8] + list(data[at:at + 48])))   # host lines hold 255 hex chars
    return lines + [frame(commit, [])]


def editor(r):
    x = r.random()
    if x < .06: return upload(r, 16, 17, 18, project(r))
    if x < .10: return upload(r, 47, 17, 48, drums(r))
    cmd = r.choice([3, 4, 5, 6, 7, 8, 10, 19, 20, 21, 22, 23, 24, 25, 26, 27, 32, 33, 35, 36, 37, 42, 44, 45, 49, 1, 2, 11, 12, 13, 14, 15, 16, 17, 18, 28, 29, 30, 34, 39, 40, 41, 43, 46, 50, r.randrange(128)])
    if cmd in (9, 31, 38): cmd = 20            # saves are covered by SAVE; keep flash wear out of the loop
    return [frame(cmd, [r.choice(EDGE) if r.random() < .7 else r.randrange(256) for _ in range(r.choice([0, 1, 2, 3, 4, 5, 6, 7, 10, 11, r.randrange(60)]))], r.randrange(128))]


def session(seed, steps):
    r = random.Random(seed)
    held_b, held_k, sounding, lines = set(), set(), set(), ['wait 100']
    if r.random() < .5: lines.append('defaults')
    for n in range(steps):
        x = r.random()
        if x < .22:
            b = r.choice(BUTTONS)
            if b in held_b: lines.append(f'release {b}'); held_b.discard(b)
            elif r.random() < .6: lines.append(f'tap {b}')
            else: lines.append(f'press {b}'); held_b.add(b)
        elif x < .50:
            k = r.randrange(27)
            if k in held_k: lines.append(f'key {k} up'); held_k.discard(k)
            elif r.random() < .5: lines.append(f'tapkey {k} {r.randint(1, 127)}')
            else: lines.append(f'key {k} down {r.randint(1, 127)}'); held_k.add(k)
        elif x < .72:
            lines.append(f'spin {r.choice(ENCODERS)} {r.choice([-1, 1, -2, 2, -5, 5, -40, 40, r.randint(-200, 200)])}')
        elif x < .84: lines.append(midi(r, sounding))
        elif x < .90: lines += editor(r)
        elif x < .91: lines.append('reboot')                 # keys and buttons stay physically held
        else: lines.append(f'wait {r.choice([5, 20, 60, 200, 700])}')
        if n % 25 == 24: lines.append('invariants')
    lines.append('# end')                      # everything below releases and checks (kept by reducers)
    lines += [f'release {b}' for b in sorted(held_b)] + [f'key {k} up' for k in sorted(held_k)]
    lines += [f'midi USB {0x80 | ch:02X} {note:02X} 00' for ch, note in sorted(sounding)]
    for ch in range(16): lines.append(f'midi USB {0xB0 | ch:02X} 40 00')       # sustain pedals up
    lines += ['wait 100', 'invariants', 'stuckprep', 'wait 4000', 'stuckexpect', 'invariants', 'tap SAVE', 'wait 300', 'reboot', 'wait 200', 'invariants']
    return '\n'.join(lines) + '\n'


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('host'); ap.add_argument('--runs', type=int, default=200); ap.add_argument('--steps', type=int, default=300)
    ap.add_argument('--seed', type=int, default=1); ap.add_argument('--out', default='source/output/fuzz')
    a = ap.parse_args(); out = Path(a.out); out.mkdir(parents=True, exist_ok=True); failed = 0
    for seed in range(a.seed, a.seed + a.runs):
        script = out / f'run-{seed}.txt'; script.write_text(session(seed, a.steps), encoding='utf-8')
        r = subprocess.run([a.host, str(script), str(out)], capture_output=True, text=True, timeout=600)
        bad = r.returncode or 'runtime error' in r.stderr or 'ERROR: AddressSanitizer' in r.stderr
        if bad:
            failed += 1; keep = out / f'fail-{seed}.txt'; script.rename(keep)
            msg = [l for l in (r.stderr + r.stdout).splitlines() if 'FAIL' in l or 'error' in l.lower() or 'line ' in l][:6]
            print(f'FAIL seed {seed} rc={r.returncode}: ' + ' | '.join(msg)[:600], flush=True)
        else: script.unlink()
    print(f'{a.runs - failed}/{a.runs} fuzz sessions clean ({a.steps} actions each)')
    sys.exit(1 if failed else 0)


if __name__ == '__main__': main()
