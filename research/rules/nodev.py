#!/usr/bin/env python3
"""Write a copy of src/ with the three pool devices removed (for research/c2lines.py and
research/c2mem.py through U5_SRC).   python research/rules/nodev.py DEST
Prints the opening-brace line of each device function in the copy."""
import os, re, shutil, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rk  # noqa: E402
import u5  # noqa: E402
from kscan import remove_device  # noqa: E402

FUNCS = {'r1850.c': 'ULTIMA_1850_PrintString', 'town.c': 'TOWN_0f02_TileEffects', 'cmds.c': 'CMDS_0000_HoleUpRest'}

if __name__ == '__main__':
    dest = sys.argv[1]
    shutil.rmtree(dest, ignore_errors=True)
    shutil.copytree(u5.SRC, dest, ignore=shutil.ignore_patterns('*.OBJ', '*.obj', '*.COD', '*.cod'))
    for f, fn in FUNCS.items():
        raw = open(os.path.join(u5.SRC, f), encoding='utf-8', newline='').read()
        t = remove_device(f, raw.replace('\r\n', '\n'))
        open(os.path.join(dest, f), 'w', encoding='utf-8', newline='').write(t.replace('\n', '\r\n') if '\r\n' in raw else t)
        print(f, fn, 'brace line', rk.brace_line(t, fn))
