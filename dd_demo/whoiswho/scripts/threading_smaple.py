#!/usr/bin/env python
#coding=utf-8

import threading
import time

#import sys
#sys.path.remove('/opt/ros/kinetic/lib/python2.7/dist-packages')
state = 0

def function():
    global state
    for i in range(3):
        time.sleep(1)
        print(' i : %d \t the time is : %s \t'%(i + 1, time.ctime(time.time()) ))
    state = 0
        
while 1:
    if state == 0:
        state = 1
        t1 = threading.Thread(target = function, name = 't1')
        t1.start()
    time.sleep(0.1)
