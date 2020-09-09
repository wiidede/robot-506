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
import argparse

cnt = 0#摔倒计数

parser = argparse.ArgumentParser()
parser.add_argument('--input', help='Path to image or video. Skip to capture frames from camera')
parser.add_argument('--thr', default=0.2, type=float, help='Threshold value for pose parts heat map')
parser.add_argument('--width', default=368, type=int, help='Resize input to specific width.')
parser.add_argument('--height', default=368, type=int, help='Resize input to specific height.')

args = parser.parse_args()

BODY_PARTS = { "Nose": 0, "Neck": 1, "RShoulder": 2, "RElbow": 3, "RWrist": 4,
               "LShoulder": 5, "LElbow": 6, "LWrist": 7, "RHip": 8, "RKnee": 9,
               "RAnkle": 10, "LHip": 11, "LKnee": 12, "LAnkle": 13, "REye": 14,
               "LEye": 15, "REar": 16, "LEar": 17, "Background": 18 }

POSE_PAIRS = [ ["Neck", "RShoulder"], ["Neck", "LShoulder"], ["RShoulder", "RElbow"],
               ["RElbow", "RWrist"], ["LShoulder", "LElbow"], ["LElbow", "LWrist"],
               ["Neck", "RHip"], ["RHip", "RKnee"], ["RKnee", "RAnkle"], ["Neck", "LHip"],
               ["LHip", "LKnee"], ["LKnee", "LAnkle"], ["Neck", "Nose"], ["Nose", "REye"],
               ["REye", "REar"], ["Nose", "LEye"], ["LEye", "LEar"] ]

inWidth = args.width
inHeight = args.height

net = cv2.dnn.readNetFromTensorflow("/home/robot/catkin_ws/src/dd_demo/OpenPose/scripts/graph_opt.pb")

def detect(frame):
    global pub
    global cnt
    flag = 0#摔倒标记
    
    frameWidth = frame.shape[1]
    frameHeight = frame.shape[0]
    
    net.setInput(cv2.dnn.blobFromImage(frame, 1.0, (inWidth, inHeight), (127.5, 127.5, 127.5), swapRB=True, crop=False))
    out = net.forward()
    out = out[:, :19, :, :]  # MobileNet output [1, 57, -1, -1], we only need the first 19 elements

    assert(len(BODY_PARTS) == out.shape[1])

    points = []
    for i in range(len(BODY_PARTS)):
        # Slice heatmap of corresponging body's part.
        heatMap = out[0, i, :, :]

        # Originally, we try to find all the local maximums. To simplify a sample
        # we just find a global one. However only a single pose at the same time
        # could be detected this way.
        _, conf, _, point = cv2.minMaxLoc(heatMap)
        ###debug for the point
        #if conf > args.thr:#confidence is higher than threshold
        #    if point[1] > out.shape[3] / 2:#the point under the half of the image
        #        flag += 1
        #        if flag > 3:
        #            cnt += 1
        #            print('摔倒了{}'.format(cnt))
        #        #print('point:')
        #        #print(point)
        #        #print('out.shape:')
        #        #print(out.shape)
        #rospy.loginfo(point)
        ###end debug
        x = (frameWidth * point[0]) / out.shape[3]
        y = (frameHeight * point[1]) / out.shape[2]
        # Add a point if it's confidence is higher than threshold.
        points.append((int(x), int(y)) if conf > args.thr else None)
   
    #print(points)
    #print(points[1])
    #print(points[8])
    #print(points[11])
    if points[1] is not None and points[8] is not None and points[11] is not None:
        #print('points1:{}\tpoints1:{}\tpoints1:{}\tframeHeight:{}'.format(points[1], points[8], points[11], frameHeight))
        if points[8][1] - points[1][1] < frameHeight / 8 or points[11][1] - points[1][1] < frameHeight / 8 :
            print('detected the falling down\n')

    for pair in POSE_PAIRS:
        partFrom = pair[0]
        partTo = pair[1]
        assert(partFrom in BODY_PARTS)
        assert(partTo in BODY_PARTS)

        idFrom = BODY_PARTS[partFrom]
        idTo = BODY_PARTS[partTo]

        if points[idFrom] and points[idTo]:
            cv2.line(frame, points[idFrom], points[idTo], (0, 255, 0), 3)
            cv2.ellipse(frame, points[idFrom], (3, 3), 0, 0, 360, (0, 0, 255), cv2.FILLED)
            cv2.ellipse(frame, points[idTo], (3, 3), 0, 0, 360, (0, 0, 255), cv2.FILLED)

    t, _ = net.getPerfProfile()
    freq = cv2.getTickFrequency() / 1000
    cv2.putText(frame, '%.2fms' % (t / freq), (10, 20), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 0))

    cv2.imshow('OpenPose using Opencv2', frame)
    if cv2.waitKey(3) & 0xFF == ord('q'):
        print('quit')

def callback(data):
    # define picture to_down' coefficient of ratio
    #print('[openpose] - callback once')
    scaling_factor = 0.5
    global count,bridge
    count = count + 1
    if count == 1:
        count = 0
        cv_img = bridge.imgmsg_to_cv2(data, "bgr8")
        detect(cv_img)
    else:
        pass
 
def displayOpenPose():
    rospy.init_node('QRCode_display', anonymous=True)
 
    # make a video_object and init the video object
    global count, bridge
    count = 0
    bridge = CvBridge()
    rospy.Subscriber('/kinect2/qhd/image_color', Image, callback)
    global pub
    pub = rospy.Publisher('/kinect2/openpose', String, queue_size=10)

    rospy.spin()
 
if __name__ == '__main__':
    displayOpenPose()
