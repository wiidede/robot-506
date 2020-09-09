#!/usr/bin/env python
#coding=utf-8

####ROS模块导入####
import rospy
from sensor_msgs.msg import Image
from std_msgs.msg import String
from cv_bridge import CvBridge, CvBridgeError
######导入结束#####

#导入其他模块
import cv2
import face_recognition
import os
import threading
import time
import numpy as np

bridge = CvBridge()

#人脸检测初始化
path = "/home/robot/catkin_ws/src/dd_demo/whoiswho/images"  # 模型数据图片目录

total_image_name = []
total_face_encoding = []

#state - 0 - 等待识别
#state - 1 - 读取图片
#state - 2 - 读取完毕
#state - 3 - 开始识别
#state - 4 - 正在识别
state = 1
res_img = []

def loadface():
    global state
    for fn in os.listdir(path):  #fn 表示的是文件名q
        print(path + "/" + fn)
        total_face_encoding.append(
            face_recognition.face_encodings(
                face_recognition.load_image_file(path + "/" + fn))[0])
        fn = fn[:(len(fn) - 4)]  #截取图片名（这里应该把images文件中的图片名命名为为人物名）
        total_image_name.append(fn)  #图片名字列表
    state = 3

def detectImg(frame, name):
    #calculate the processing time of the function
    startTime = time.time()
    #print('\t detect start')
    global state, res_img
    try:
        frame.shape
    except:
        print('[detectImg] - invaild image received')
        return
    # Convert the image from BGR color (which OpenCV uses) to RGB color (which face_recognition uses)
    #frame = frame[:, :, ::-1]
    # 发现在视频帧所有的脸和face_enqcodings
    face_locations = face_recognition.face_locations(frame)
    face_encodings = face_recognition.face_encodings(frame, face_locations)
    names = []
    # 在这个视频帧中循环遍历每个人脸
    for (top, right, bottom, left), face_encoding in zip(
            face_locations, face_encodings): 
        # 看看面部是否与已知人脸相匹配。
        for i, v in enumerate(total_face_encoding):
            match = face_recognition.compare_faces(
                [v], face_encoding, tolerance=0.5)
            name = "Unknown"
            if match[0]:
                name = total_image_name[i]
                break
        # 画出一个框，框住脸
        cv2.rectangle(frame, (left, top), (right, bottom), (0, 0, 255), 2)
        # 画出一个带名字的标签，放在框下
        cv2.rectangle(frame, (left, bottom - 35), (right, bottom), (0, 0, 255),
                      cv2.FILLED)
        font = cv2.FONT_HERSHEY_DUPLEX
        cv2.putText(frame, name, (left + 6, bottom - 6), font, 1.0,
                    (255, 255, 255), 1)
        names.append(name)
    # 显示结果图像
    # 子线程无法使用imshow，不然会出问题
    res_img = frame
    #cv2.imshow('result', frame)
    #if cv2.waitKey(30) & 0xFF == ord('q'):
    #    print('please press ctrl + c to end the program')
    #calculate the processing time of the function
    endTime = time.time()
    print('processtime %s' % (endTime - startTime))
    for name in names:
        print('\t%s' % name)
    #print('\t detect end')
    state = 3

def imageCB(data):
    global state
    #print('[imageCB] - callback once %d'%state)
    scaling_factor = 0.5
    if state == 1:
        state = 2
        loadThread = threading.Thread(target = loadface)
        loadThread.start()
    elif state == 3:
        state = 4
        cv_img = bridge.imgmsg_to_cv2(data, "bgr8")
        #using threading to recognize the face
        detectThread = threading.Thread(target = detectImg, args = (cv_img, 'Name'))
        detectThread.start()
        global res_img
        try:
            res_img.shape
        except:
            print('nothing to show')
        else:
            cv2.imshow('result', res_img)
            cv2.waitKey(1)
    else:
        pass
    #print('[imageCB] - callback end')
    
def stringCB(data):
    global state
    state = 1
 
def whoiswho():
    rospy.init_node('whoiswho', anonymous=True)
    rospy.Subscriber('/kinect2/qhd/image_color', Image, imageCB)
    rospy.spin()
 
if __name__ == '__main__':
    whoiswho()
