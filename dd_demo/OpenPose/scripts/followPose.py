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

#from matplotlib import pyplot as plt

state = 0 #初始状态
cnt = 0 #检测手表次数

#特征匹配初始化
MIN_MATCH_COUNT = 8
template = cv2.imread('/home/robot/photos/1.jpg',0)
try :
    template.shape
except:
    print("image not found")
#template = cv2.resize(template, (72, 128), interpolation=cv2.INTER_CUBIC)
sift = cv2.xfeatures2d.SIFT_create()

def detect(frame):
    global state, cnt, count
    pub_op = rospy.Publisher('/kinect2/openpose', String, queue_size=10)
    
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

            #pts = np.float32([ [0,0],[0,h-1],[w-1,h-1],[w-1,0] ]).reshape(-1,1,2)
            #dst = cv2.perspectiveTransform(pts,M)
            #cv2.polylines(frame,[np.int32(dst)],True,0,2, cv2.LINE_AA)
            #publish detected the watch
            pub_op.publish('has watch')
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
    elif state == 3:
        #save the photo
        path = '/home/robot/photos/pose.jpg'
        cv2.imwrite(path, frame)



    count = 0
    #show the image
    cv2.imshow('follow detecting', frame)
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
    rospy.init_node('follow-detect', anonymous=True)
    # make a video_object and init the video object
    global count
    count = 0
    rospy.Subscriber('/kinect2/qhd/image_color', Image, callback)
    rospy.Subscriber('/ijcai/watch', String, watchCB)
    rospy.spin()
 
if __name__ == '__main__':
    displayOpenPose()
