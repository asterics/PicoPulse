# pip install keyboard pyserial pystray pillow pyinstaller  
# pyinstaller --onefile --noconsole pulseControl.py

import sys
import threading
import serial
import serial.tools.list_ports
import winsound
import time
import re
import keyboard
import pystray
from PIL import Image, ImageDraw
import os
import json
import argparse

active = False
ser = None
speed=0

if sys.stderr is None:
    sys.stderr = open(os.devnull, 'w')
if sys.stdout is None:
    sys.stdout = open(os.devnull, 'w')
if sys.stdin is None:
    sys.stdin = open(os.devnull, 'r')


def play_tone(freq, duration=200):
    winsound.Beep(freq, duration)

def send_command(cmd):
    if ser and ser.is_open:
        ser.write(cmd)

def handle_serial_input(ser):
    while True:
        try:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if line:
                print(f"Received: {line}")
                match = re.match(r'^pulseActive: (\d+)', line)
                if match:
                    value = int(match.group(1))
                    play_tone(400+600*value)
                match = re.match(r'^pulseTime: (\d+)', line)
                if match:
                    value = int(match.group(1))
                    play_tone(1500-int(value/7),100)

        except serial.SerialException as e:
            print(f"Serial error: {e}")
            break

def find_pulse_device():
    ports = serial.tools.list_ports.comports()
    check_response_command = "pulseControl\n"
    expected_response = "OK\n"
    timeout_seconds = 1

    for port in ports:
        try:
            print(f"Checking {port.device}...", end=" ")
            ser = serial.Serial(port.device, baudrate=115200, timeout=timeout_seconds)
            time.sleep(0.1)  # Wait a moment after opening the port
            ser.write(check_response_command.encode())

            response = ser.readline().decode(errors="ignore")
            if response == expected_response:
                print("Found pulse device!")
                return ser  # Port stays open
            else:
                ser.close()
                print("No valid response.")
        except (serial.SerialException, OSError) as e:
            print(f"Skipping ({e})")
    
    print("Error: Pulse device not found.")
    return None

def pulse_toggleState():
    send_command(b'toggle\n')
 
def pulse_toggleClick():
    send_command(b'click\n')

def pulse_faster():
    send_command(b'faster\n')
    
def pulse_slower():
    send_command(b'slower\n')

def start_hotkey_listener():
    keyboard.add_hotkey('shift+f11', pulse_toggleState)
    keyboard.add_hotkey('shift+f10', pulse_faster)
    keyboard.add_hotkey('shift+f9', pulse_slower)
    keyboard.add_hotkey('shift+f8', pulse_toggleClick)
    keyboard.wait()

def create_image(color):
    image = Image.new('RGB', (64, 64), "white")
    draw = ImageDraw.Draw(image)
    draw.ellipse((16, 16, 48, 48), fill=color)
    return image

def tray_app():
    global ser

    key_listener_thread = threading.Thread(target=start_hotkey_listener, daemon=True)
    key_listener_thread.start()

    port_listener_thread = threading.Thread(target=handle_serial_input, args=(ser,), daemon=True)
    port_listener_thread.start()

    def on_exit(icon, item):
        icon.stop()

    icon = pystray.Icon("PulseControl")
    icon.icon = create_image("green")
    icon.menu = pystray.Menu(
        pystray.MenuItem("On/Off (Shift+F11)", pulse_toggleState),
        pystray.MenuItem("Faster (Shift+F10)", pulse_faster),
        pystray.MenuItem("Slower  (Shift+F9)", pulse_slower),
        pystray.MenuItem("Click on/off  (Shift+F8)", pulse_toggleClick),
        pystray.MenuItem("Quit", on_exit))
    icon.run()

if __name__ == "__main__":

    ser = find_pulse_device()
    if ser and ser.is_open:
        tray_app()
    else:
        print("Pulse Device not found - exit.")