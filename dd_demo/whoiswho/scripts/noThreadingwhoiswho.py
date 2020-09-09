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

#人脸检测初始化
path = "/home/robot/catkin_ws/src/dd_demo/whoiswho/images"  # 模型数据图片目录

total_image_name = []
total_face_encoding = []

#state - 0 - 等待识别
#state - 1 - 读取图片
#state - 2 - 开始识别
state = 1

def loadface():
    for fn in os.listdir(path):  #fn 表示的是文件名q
        print(path + "/" + fn)
        total_face_encoding.append(
            face_recognition.face_encodings(
                face_recognition.load_image_file(path + "/" + fn))[0])
        fn = fn[:(len(fn) - 4)]  #截取图片名（这里应该把images文件中的图片名命名为为人物名）
        total_image_name.append(fn)  #图片名字列表

def detectImg(frame):
    global state
    try:
        frame.shape
    except:
        print('[detectImg] - invaild image received')
        return
    # 发现在视频帧所有的脸和face_enqcodings
    face_locations = face_recognition.face_locations(frame)
    face_encodings = face_recognition.face_encodings(frame, face_locations)
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
    # 显示结果图像
    cv2.imshow('Video', frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
	break

def imageCB(data):
    global state
    print('[imageCB] - callback once %d' % state)
    if state == 1:
        loadface()
        state = 2
    elif state == 2:
        bridge = CvBridge()
        cv_img = bridge.imgmsg_to_cv2(data, "bgr8")
        detectImg(cv_img)

    
def stringCB(data):
    global state
    state = 1
 
def whoiswho():
    rospy.init_node('whoiswho', anonymous=True)
    rospy.Subscriber('/kinect2/qhd/image_color', Image, imageCB)
    rospy.Subscriber('/whoiswho/facereco', String, stringCB)
    rospy.spin()
 
if __name__ == '__main__':
    whoiswho()
