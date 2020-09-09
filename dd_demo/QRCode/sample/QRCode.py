#!/usr/bin/env python3
#coding=utf-8

from __future__ import print_function

####ROS模块导入####

######导入结束#####

import sys
sys.path.remove('/opt/ros/kinetic/lib/python2.7/dist-packages')

import pyzbar.pyzbar as pyzbar
import numpy as np
import cv2

def detect():
    #读入图片
    im = cv2.imread('../images/qr4.jpg')
    
    #查找二维码
    decodedObjects = pyzbar.decode(im)
    
    #打印结果
    for obj in decodedObjects:
        print('Type : ', obj.type)
        print('Data : ', obj.data, '\n')
    
    #展示图片
    for decodedObject in decodedObjects:
        points = decodedObject.polygon

        if len(points) > 4:
            hull = cv2.convexHull(np.array([point for point in points], dtype=np.float32))
            hull = list(map(tuple, np.squeeze(hull)))
        else:
            hull = points;

        n = len(hull)

        for j in range(0, n):
            cv2.line(im, hull[j], hull[(j + 1) % n], (255, 0, 0), 3)

    cv2.imshow("Results", im);
    if cv2.waitKey(0) & 0xFF == ord('q'):
        print('hello')
        return
        

detect()
