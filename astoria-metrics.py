#!/usr/bin/env python

import serial
import os
import datetime
import collections
import numpy
import time
import socket
import argparse
import sys
from ConfigParser import SafeConfigParser
from subprocess import Popen

def sendToGraphite(opts, data):
    carbon_server = opts.get('default', 'carbon_server')
    carbon_port   = opts.getint('default', 'carbon_port')
    sock = socket.socket()

    try:
        #print "Sending metrics %s" % (data[0])
        sock.connect((carbon_server, carbon_port))
    except: 
        print "%s Unable to send metric data to %s:%s" % (datetime.datetime.now(), carbon_server, carbon_port )
        #return data

    for metric in data:
        sock.sendall(metric)
        #print "%s" % metric

    sock.close()
    return []

def update_state_file(state):
    f = open('/run/astoria-state', 'r+')
    f.seek(0)
    f.write(state)
    f.truncate()
        
    
def parse_args():
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("-c", "--config", help="Config file.", default="/etc/astoria-metrics.cfg")
    opts = parser.parse_args()

    """Parse config file arguments"""
    config = SafeConfigParser()

    if os.path.exists(os.path.expanduser(opts.config)):
        config.read(os.path.expanduser(opts.config))
    else:
        print "# No valid config file found at %s" % opts.config
        sys.exit(1)
    return config


def main():

    global opts
    opts = parse_args()
    sensor_error_limit = float(opts.get('default', 'sensor_error_limit'))

    ser = serial.Serial(port=opts.get('default', 'serialport'), timeout=0.5)
    last_stats  = datetime.datetime.now()
    metric_data = {}
    buffer = []
    
    while True:

        read =  ser.readline()
        if len(read) >= 1:
            if read[0] == "E":
                print "%s" % (read);
            else:
                print "%s %s" % (datetime.datetime.now(), read)
                parts = str(read).split(";")
                now = int(time.time())
                buffer = []
                if float(parts[7]) >= sensor_error_limit:
                    parts[7] = sensor_error_limit
                if float(parts[8]) >= sensor_error_limit:
                    parts[8] = sensor_error_limit
                if float(parts[9]) >= sensor_error_limit:
                    parts[9] = sensor_error_limit
                if float(parts[10]) >= sensor_error_limit:
                    parts[10] = sensor_error_limit
                if float(parts[11]) >= sensor_error_limit:
                    parts[11] = sensor_error_limit
                if float(parts[12]) >= sensor_error_limit:
                    parts[12] = sensor_error_limit
                buffer.append( "%s.uptime %s %s\n" % (opts.get('default', 'metricnamespace'), parts[0], now) )
                buffer.append( "%s.state.system.armed %s %s\n" % (opts.get('default', 'metricnamespace'), parts[1], now) )
                update_state_file(parts[1])
                buffer.append( "%s.state.system.active %s %s\n" % (opts.get('default', 'metricnamespace'), parts[2], now) )
                buffer.append( "%s.state.system.ready %s %s\n" % (opts.get('default', 'metricnamespace'), parts[3], now) )
                buffer.append( "%s.state.armed.ms %s %s\n" % (opts.get('default', 'metricnamespace'), parts[4], now) )
                buffer.append( "%s.state.ready.ms %s %s\n" % (opts.get('default', 'metricnamespace'), parts[5], now) )
                buffer.append( "%s.state.ssr.ms %s %s\n" % (opts.get('default', 'metricnamespace'), parts[6], now) )
                buffer.append( "%s.temp.probe.boiler %s %s\n" % (opts.get('default', 'metricnamespace'), float(parts[7]) / 100.0, now) )
                buffer.append( "%s.temp.probe.group_l %s %s\n" % (opts.get('default', 'metricnamespace'), float(parts[8]) / 100.0, now) )
                buffer.append( "%s.temp.probe.group_r %s %s\n" % (opts.get('default', 'metricnamespace'), float(parts[9]) / 100.0, now) )
                buffer.append( "%s.temp.probe.case_upper %s %s\n" % (opts.get('default', 'metricnamespace'), float(parts[10]) / 100.0, now) )
                buffer.append( "%s.temp.probe.case_lower %s %s\n" % (opts.get('default', 'metricnamespace'), float(parts[11]) / 100.0, now) )
                buffer.append( "%s.temp.probe.case_external %s %s\n" % (opts.get('default', 'metricnamespace'), float(parts[12]) / 100.0, now) )

        stats_delta = ((datetime.datetime.now() - last_stats).seconds * 1.0)
 
        if stats_delta > int(opts.get('default', 'interval')):
   
            last_stats = datetime.datetime.now()
            buffer = sendToGraphite(opts, buffer)

if __name__ == "__main__":
    main()
