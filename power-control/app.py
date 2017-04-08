#!/usr/bin/env python
import os
import robgracli
from flask import Flask, render_template, request
import time
import RPi.GPIO as GPIO

app = Flask(__name__)


def toggle_power():
    GPIO.setwarnings(False)
    GPIO.setmode(GPIO.BCM)
    POWER_SWITCH = 18
    GPIO.setup(POWER_SWITCH, GPIO.OUT)


    print "switching power state"
    GPIO.output(POWER_SWITCH, False)
    time.sleep(0.1)
    GPIO.output(POWER_SWITCH, True)
    time.sleep(0.2)
    GPIO.cleanup()
    time.sleep(0.8)

def get_power_state():
    f = open('/run/astoria-state', 'r')
    state = f.read()
    print state
    if state == "1":
        return "ON"
    else:
        return "OFF"



@app.route('/', methods=['GET','POST'])
def show_landing_page():
    power = get_power_state()
    if request.method == "POST":
        print request.form['action']
        if power != request.form['action']:
            toggle_power()
            power = get_power_state()

    return render_template('index.html', state=power)


if __name__ == "__main__":
    app.run(host='0.0.0.0',port=80)
