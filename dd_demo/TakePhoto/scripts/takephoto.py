#!/usr/bin/env python
#coding=utf-8

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

state = 0
cnt = 0

def showimg(im):
    global state, cnt
    cnt += 1
    path = '/home/robot/photos/{}.jpg'.format(cnt)
    cv2.imshow("Kinect2", im);
    key = cv2.waitKey(3)
    if key & 0xff == ord('q'):
        cv2.destroyAllWindows()
        state = 1
    if key & 0xff == ord('s'):
        cv2.imwrite(path, im)

def callback(data):
    global state
    if state == 1:
        return
    scaling_factor = 0.5
    global count,bridge
    count = count + 1
    if count == 1:
        count = 0
        cv_img = bridge.imgmsg_to_cv2(data, "bgr8")
        showimg(cv_img)
    else:
        pass
 
def takephoto():
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
    takephoto()
