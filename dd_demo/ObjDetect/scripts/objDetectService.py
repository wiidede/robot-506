#!/usr/bin/env python
#coding=utf-8

from __future__ import print_function

####ROS模块导入####
import rospy
from sensor_msgs.msg import Image
from std_msgs.msg import String
from cv_bridge import CvBridge, CvBridgeError
from objdetect.srv import *
######导入结束#####
import numpy as np
import cv2

state = 0
#0 - 等待请求
#1 - 物品检测
#2 - 检测结束
count = 0
# 0 - start
# 1 - detectig
# 2 - end detect
cnt = 0

def server_srv():
    rospy.init_node("objdetect_server")
    s = rospy.Service("objdetect", objdetect, handle_function)
    rospy.loginfo("Ready to handle the request:")
    rospy.Subscriber('/kinect2/qhd/image_color', Image, callback)
    rospy.spin()

# Define the handle function to handle the request inputs
def handle_function(req):
    global state
    state = 1
    rospy.loginfo( 'Request age %d',req.cnt)
    #rospy.Subscriber('webcam/image_raw', Image, callback)
    while(1) :
        if state == 2:
            break
    return objdetectResponse("Hi. I' server!")
    
def callback(data):
    
    global state, count
    print('callback - state = {}, count = {}'.format(state, count))
    if state == 1 and count == 0:
        scaling_factor = 0.5
        global bridge
        bridge = CvBridge()
        cv_img = bridge.imgmsg_to_cv2(data, "bgr8")
        count = 1
        detect(cv_img)

def detect(target):
    global state, count, cnt
    print('detect - state')
    
    if state == 1:
        try:
            target.shape
            print('有图片')
        except:
            print('没有图片')
            count = 0
            return
        #特征匹配初始化
        path = '/home/robot/photos/{}.jpg'.format(cnt % 3 + 1)
        print(path)
        MIN_MATCH_COUNT = 6
        template = cv2.imread(path, 0)
        sift = cv2.xfeatures2d.SIFT_create()

        kp1, des1 = sift.detectAndCompute(template,None)
        kp2, des2 = sift.detectAndCompute(target,None)

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
            cv2.polylines(target,[np.int32(dst)],True,0,2, cv2.LINE_AA)
            #publish detected the watch
            print('detected')
            cnt = 0
            state = 2
        else:
            print( "Not enough matches are found - %d/%d" % (len(good),MIN_MATCH_COUNT))
            #publish not detected the watch
            cnt += 1
            if cnt > 1000 :
                print('not detect')
                cnt = 0
                state = 2
            matches_Mask = None
        draw_params = dict(matchColor=(0,255,0), 
                           singlePointColor=None,
                           matchesMask=matches_Mask, 
                           flags=2)
        target = cv2.drawMatches(template,kp1,target,kp2,good,None,**draw_params)
        count = 0
        #show the image
        cv2.imshow('OpenPose using Opencv2', target)
        if cv2.waitKey(3) & 0xFF == ord('q'):
            print('quit')

if __name__=="__main__":
    server_srv()
