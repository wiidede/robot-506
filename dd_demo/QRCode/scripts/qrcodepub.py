#!/usr/bin/env python
#coding=utf-8

from __future__ import print_function

####ROS模块导入####
import rospy
from sensor_msgs.msg import Image
from std_msgs.msg import String
from cv_bridge import CvBridge, CvBridgeError
######导入结束#####

#import sys
#sys.path.remove('/opt/ros/kinetic/lib/python2.7/dist-packages')

import pyzbar.pyzbar as pyzbar
import numpy as np
import cv2

state = 1

def detect(im):
    global state
    if state == 0:
        #读入图片
        #im = cv2.imread('../images/qr4.jpg')
        global pub
        
        #查找二维码
        decodedObjects = pyzbar.decode(im)
        
        #打印结果
        for obj in decodedObjects:
            print('Type : ', obj.type)
            print('Data : ', obj.data, '\n')
            if obj.type == 'QRCODE':
                pub.publish(obj.data)
                state = 1
            state = 1
        
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
    else:
        cv2.imwrite("/home/robot/catkin_ws/src/yolo/data/custom/img/detect.jpg", im)
        cv2.imshow("Results", im);
        if cv2.waitKey(3) & 0xFF == ord('q'):
            print('quit')

def callback(data):
    # define picture to_down' coefficient of ratio
    print('[qrcode] - callback once')
    scaling_factor = 0.5
    global count,bridge
    count = count + 1
    if count == 1:
        count = 0
        cv_img = bridge.imgmsg_to_cv2(data, "bgr8")
        detect(cv_img)
    else:
        pass
 
def displayQRCode():
    rospy.init_node('QRCode_display', anonymous=True)
 
    # make a video_object and init the video object
    global count, bridge
    count = 0
    bridge = CvBridge()
    rospy.Subscriber('/kinect2/hd/image_color', Image, callback)
    global pub
    pub = rospy.Publisher('/kinect2/qrcode', String, queue_size=10)

    rospy.spin()
 
if __name__ == '__main__':
    displayQRCode()
