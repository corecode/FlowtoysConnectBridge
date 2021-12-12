#!/bin/env python3
from __future__ import print_function

target_usbinfo = "VID:PID=10c4:ea60"
target_command = ["--chip", "esp32", "--baud", "921600",
                  "--before", "default_reset",
                  "--after", "hard_reset", "write_flash",
                  "-z",
                  "--flash_mode", "dio",
                  "--flash_freq", "80m",
                  "--flash_size", "detect",
                  "0xe000", "boot_app0.bin",
                  "0x1000", "bootloader_dio_80m.bin",
                  "0x10000", "FlowtoysConnectBridge.ino.bin",
                  "0x8000", "FlowtoysConnectBridge.ino.partitions.bin"]

import sys
import argparse
from serial.tools.list_ports import grep as grep_serial
import esptool
import os

# deal with pyinstaller
if getattr(sys, 'frozen', False) and hasattr(sys, '_MEIPASS'):
    os.chdir(sys._MEIPASS)

def run_esptool(port: str, cmd: list[str]):
    fullcmd = ['--port', port] + cmd
    print("esptool.py " + ' '.join(fullcmd))
    esptool.main(fullcmd)

def main(argv: list[str]):
    ports = [p.device for p in grep_serial(target_usbinfo)]

    parser = argparse.ArgumentParser()
    parser.add_argument('--wipe', action='store_true', default=False, help='wipe flash completely')
    parser.add_argument('--port', action='append', default=None, help='port to write firmware to (multiple possible) (detected: %s)' % (', '.join(ports) or "nothing"))
    args = parser.parse_args(argv)

    if args.port is None:
        args.port = ports

    if len(args.port) == 0:
        print("did not find a serial port for target", file=sys.stderr)
        return 1

    for p in args.port:
        if args.wipe:
            run_esptool(p, ['erase_flash'])
        command = target_command
        run_esptool(p, command)

    return 0

if __name__ == '__main__':
    try:
        sys.exit(main(sys.argv[1:]))
    except Exception as e:
        print(e, file=sys.stderr)
        sys.exit(1)
