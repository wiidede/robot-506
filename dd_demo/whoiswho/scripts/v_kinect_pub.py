#!/usr/bin/env python
#!coding=utf-8
 
#write by leo at 2018.04.26
#function: 
#1, Get live_video from the webcam.
#2, With ROS publish Image_info (to yolo and so on).
#3, Convert directly, don't need to save pic at local.
 
import rospy
from sensor_msgs.msg import Image
import cv2
import numpy as np
from cv_bridge import CvBridge, CvBridgeError
import sys
 
 
def webcamImagePub():
    # init ros_node
    rospy.init_node('webcam_puber', anonymous=True)
    # queue_size should be small in order to make it 'real_time'
    # or the node will pub the past_frame
    #img_pub = rospy.Publisher('webcam/image_raw', Image, queue_size=2)
    img_pub = rospy.Publisher('/kinect2/qhd/image_color', Image, queue_size=2)
    rate = rospy.Rate(5) # 5hz
 
    # make a video_object and init the video object
    cap = cv2.VideoCapture(0)
    # define picture to_down' coefficient of ratio
    scaling_factor = 0.5
    # the 'CVBridge' is a python_class, must have a instance.
    # That means "cv2_to_imgmsg() must be called with CvBridge instance"
    bridge = CvBridge()
 
    if not cap.isOpened():
        sys.stdout.write("Webcam is not available !")
        return -1
 
    count = 0
    # loop until press 'esc' or 'q'
    while not rospy.is_shutdown():
        ret, frame = cap.read()
        # resize the frame
        if ret:
            count = count + 1
        else:
            rospy.loginfo("Capturing image failed.")
        if count == 2:
            count = 0
            frame = cv2.resize(frame,None,fx=scaling_factor,fy=scaling_factor,interpolation=cv2.INTER_AREA)
            msg = bridge.cv2_to_imgmsg(frame, encoding="bgr8")
            img_pub.publish(msg)
            print '** publishing webcam_frame ***'	
        rate.sleep()
 
if __name__ == '__main__':
    try:
        webcamImagePub()
    except rospy.ROSInterruptException:
        pass
