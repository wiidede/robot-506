#!/usr/bin/env python
#coding=utf-8

from __future__ import print_function

####ROS模块导入####
import rospy
from sensor_msgs.msg import Image
from cv_bridge import CvBridge, CvBridgeError
######导入结束#####

#import sys
#sys.path.remove('/opt/ros/kinetic/lib/python2.7/dist-packages')

import pyzbar.pyzbar as pyzbar
import numpy as np
import cv2

def detect(im):
    #读入图片
    #im = cv2.imread('../images/qr4.jpg')
    flag = 0;
    
    #查找二维码
    decodedObjects = pyzbar.decode(im)
    
    #打印结果
    for obj in decodedObjects:
        print('Type : ', obj.type)
        print('Data : ', obj.data, '\n')
        flag = 1
    
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
    if cv2.waitKey(3) & 0xFF == ord('q'):
        print('quit')
        return
    return flag

def callback(data):
    # define picture to_down' coefficient of ratio
    scaling_factor = 0.5
    global count,bridge
    count = count + 1
    if count == 1:
        count = 0
        cv_img = bridge.imgmsg_to_cv2(data, "bgr8")
        detect(cv_img)
    else:
        pass
 
def displayWebcam():
    rospy.init_node('webcam_display', anonymous=True)
 
    # make a video_object and init the video object
    global count,bridge
    count = 0
    bridge = CvBridge()
    rospy.Subscriber('/kinect2/qhd/image_color', Image, callback)
    rospy.spin()
 
if __name__ == '__main__':
    displayWebcam()
