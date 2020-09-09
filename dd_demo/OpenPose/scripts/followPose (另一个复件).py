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

import numpy as np
import cv2
import argparse

#from matplotlib import pyplot as plt

state = 0 #初始状态
cnt = 0 #检测手表次数

#特征匹配初始化
MIN_MATCH_COUNT = 5
template = cv2.imread('/home/robot/photos/watchdemo.jpg',0)
#template = cv2.resize(template, (72, 128), interpolation=cv2.INTER_CUBIC)
sift = cv2.xfeatures2d.SIFT_create()

#动作检测初始化
BODY_PARTS = { "Nose": 0, "Neck": 1, "RShoulder": 2, "RElbow": 3, "RWrist": 4,
           "LShoulder": 5, "LElbow": 6, "LWrist": 7, "RHip": 8, "RKnee": 9,
           "RAnkle": 10, "LHip": 11, "LKnee": 12, "LAnkle": 13, "REye": 14,
           "LEye": 15, "REar": 16, "LEar": 17, "Background": 18 }

POSE_PAIRS = [ ["Neck", "RShoulder"], ["Neck", "LShoulder"], ["RShoulder", "RElbow"],
           ["RElbow", "RWrist"], ["LShoulder", "LElbow"], ["LElbow", "LWrist"],
           ["Neck", "RHip"], ["RHip", "RKnee"], ["RKnee", "RAnkle"], ["Neck", "LHip"],
           ["LHip", "LKnee"], ["LKnee", "LAnkle"], ["Neck", "Nose"], ["Nose", "REye"],
           ["REye", "REar"], ["Nose", "LEye"], ["LEye", "LEar"] ]

net = cv2.dnn.readNetFromTensorflow("/home/robot/catkin_ws/src/dd_demo/OpenPose/scripts/graph_opt.pb")

def detect(frame):
    global state, cnt, count
    pub = rospy.Publisher('/kinect2/openpose', String, queue_size=10)
    
    try:
        frame.shape
    except:
        return
    else:
        if state == 0:
            state = 2
    
    if state == 2:
        #detect the watch
        kp1, des1 = sift.detectAndCompute(template,None)
        kp2, des2 = sift.detectAndCompute(frame,None)

        FLANN_INDEX_KDTREE = 0
        index_params = dict(algorithm = FLANN_INDEX_KDTREE, trees = 5)
        search_params = dict(checks = 50)
        flann = cv2.FlannBasedMatcher(index_params, search_params)
        matches = flann.knnMatch(des1,des2,k=2)

        good = []

        for m,n in matches:
            if m.distance < 0.7*n.distance:
                good.append(m)
        if len(good)>MIN_MATCH_COUNT:
            
            src_pts = np.float32([ kp1[m.queryIdx].pt for m in good ]).reshape(-1,1,2)
            dst_pts = np.float32([ kp2[m.trainIdx].pt for m in good ]).reshape(-1,1,2)
            
            M, mask = cv2.findHomography(src_pts, dst_pts, cv2.RANSAC, 5.0)
            matches_Mask = mask.ravel().tolist()
            h,w = template.shape

            pts = np.float32([ [0,0],[0,h-1],[w-1,h-1],[w-1,0] ]).reshape(-1,1,2)
            dst = cv2.perspectiveTransform(pts,M)
            cv2.polylines(frame,[np.int32(dst)],True,0,2, cv2.LINE_AA)
            #publish detected the watch
            pub.publish('has watch')
            state = 3
        else:
            print( "Not enough matches are found - %d/%d" % (len(good),MIN_MATCH_COUNT))
            #publish not detected the watch
            matches_Mask = None
        draw_params = dict(matchColor=(0,255,0), 
                           singlePointColor=None,
                           matchesMask=matches_Mask, 
                           flags=2)
        frame = cv2.drawMatches(template,kp1,frame,kp2,good,None,**draw_params)
    else:
        frameWidth = frame.shape[1]
        frameHeight = frame.shape[0]
        
        net.setInput(cv2.dnn.blobFromImage(frame, 1.0, (368, 368), (127.5, 127.5, 127.5), swapRB=True, crop=False))
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

            x = (frameWidth * point[0]) / out.shape[3]
            y = (frameHeight * point[1]) / out.shape[2]
            # Add a point if it's confidence is higher than threshold.
            points.append((int(x), int(y)) if conf > 0.2 else None)
        
        #falling down
        if state == 3:
            if points[1] is not None and points[8] is not None and points[11] is not None:
                #if points[8][1] - points[1][1] < frameHeight / 8 or points[11][1] - points[1][1] < frameHeight / 8 :
                if points[1] < frameHeight / 5 * 3:
                    print('detected the falling down\n')
                    pub.publish('fall down')
        
        #line each point
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
        
        #show the time of processing a image
        t, _ = net.getPerfProfile()
        freq = cv2.getTickFrequency() / 1000
        cv2.putText(frame, '%.2fms' % (t / freq), (10, 20), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 0))

    count = 0
    #show the image
    cv2.imshow('OpenPose using Opencv2', frame)
    if cv2.waitKey(3) & 0xFF == ord('q'):
        print('quit')

def callback(data):
    # define picture to_down' coefficient of ratio
    global count
    if count == 0:
        count = 1
        scaling_factor = 0.5
        bridge = CvBridge()
        cv_img = bridge.imgmsg_to_cv2(data, "bgr8")
        detect(cv_img)
    else:
        pass
 
def watchCB(data):
    global state
    state = 3
    
def displayOpenPose():
    rospy.init_node('OpenPose_display', anonymous=True)
    # make a video_object and init the video object
    global count
    count = 0
    rospy.Subscriber('/kinect2/qhd/image_color', Image, callback)
    rospy.Subscriber('/ijcai/watch', String, watchCB)
    rospy.spin()
 
if __name__ == '__main__':
    displayOpenPose()
