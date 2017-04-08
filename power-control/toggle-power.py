#!/usr/bin/env python

import time
import RPi.GPIO as GPIO
import argparse

def get_power_state():
    f = open('/run/astoria-state', 'r')
    state = f.read()
    if state == "1":
        return "ON"
    else:
        return "OFF"


def toggle_power_state():
    GPIO.setwarnings(False)
    GPIO.setmode(GPIO.BCM)
    POWER_SWITCH = 18
    GPIO.setup(POWER_SWITCH, GPIO.OUT)

    GPIO.output(POWER_SWITCH, False)
    time.sleep(0.1)
    GPIO.output(POWER_SWITCH, True)
    time.sleep(0.2)
    GPIO.cleanup()


def parse_args():
    """Parse command line arguments"""
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    parser.add_argument('--on', action='store_true', help='Turn on heating element')
    parser.add_argument('--off', action='store_true', help='Turn off heating element')
    opts = parser.parse_args()

    action = "ON"
    if opts.off:
        action = "OFF"

    return action

def main():
    action = parse_args()
    state = get_power_state()
    if action != state:
        toggle_power_state()
    time.sleep(3)
    state = get_power_state()
    if action != state:
        print "error setting state to %s; state is still %s" % (action, state)

if __name__ == '__main__':
    main()

