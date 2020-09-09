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
path = "/home/robot/photos/person"  # 模型数据图片目录

total_image_name = []
total_face_encoding = []

#state - 0 - 等待识别
#state - 1 - 读取图片
#state - 2 - 读取完毕
#state - 3 - 开始识别
#state - 4 - 正在识别
state = 0
res_img = []
totalNames = []
nPerson = 0
cnt = 0 

def loadface():
    global state, nPerson
    for fn in os.listdir(path):  #fn 表示的是文件名q
        print(path + "/" + fn)
        faceImg = face_recognition.load_image_file(path + "/" + fn)
        faceEncoding = face_recognition.face_encodings(faceImg)
        if len(faceEncoding > 0) # 图片中是否有人脸
        {
            total_face_encoding.append(faceEncoding[0])
        }
        fn = fn[:(len(fn) - 4)]  #截取图片名（这里应该把images文件中的图片名命名为为人物名）
        total_image_name.append(fn)  #图片名字列表
        nPerson += 1
    state = 3

def detectImg(frame, name):
    #calculate the processing time of the function
    startTime = time.time()
    #print('\t detect start')
    global state, res_img, cnt
    try:
        frame.shape
    except:
        print('[detectImg] - invaild image received')
        return
    # Convert the image from BGR color (which OpenCV uses) to RGB color (which face_recognition uses)
    rgb_frame = frame[:, :, ::-1]
    # 发现在视频帧所有的脸和face_enqcodings
    face_locations = face_recognition.face_locations(rgb_frame)
    face_encodings = face_recognition.face_encodings(rgb_frame, face_locations)
    names = []

    # Loop through each face in this frame of video
    for (top, right, bottom, left), face_encoding in zip(face_locations, face_encodings):
        # See if the face is a match for the known face(s)
        matches = face_recognition.compare_faces(total_face_encoding, face_encoding)

        name = "Unknown"
        bSaveImg = False

        # If a match was found in total_face_encoding, just use the first one.
        # if True in matches:
        #     first_match_index = matches.index(True)
        #     name = total_image_name[first_match_index]

        # Or instead, use the known face with the smallest distance to the new face
        face_distances = face_recognition.face_distance(total_face_encoding, face_encoding)
        best_match_index = np.argmin(face_distances)
        if matches[best_match_index]:
            name = total_image_name[best_match_index]
            names.append(name)
            if name not in totalNames:
                totalNames.append(name)
                bSaveImg = True
                pub = rospy.Publisher('/whoiswho/end', String, queue_size=10)
                pub.publish(name)
                print("/whoiswho/end - publishing %s" % name)

        # Draw a box around the face
        cv2.rectangle(frame, (left, top), (right, bottom), (0, 0, 255), 2)

        # Draw a label with a name below the face
        cv2.rectangle(frame, (left, bottom - 35), (right, bottom), (0, 0, 255), cv2.FILLED)
        font = cv2.FONT_HERSHEY_DUPLEX
        cv2.putText(frame, name, (left + 6, bottom - 6), font, 1.0, (255, 255, 255), 1)

        if bSaveImg:
            cv2.imwrite("/home/robot/photos/person/%s_recognized.png" % name, frame)
    # 显示结果图像
    # 子线程无法使用imshow，不然会出问题
    res_img = frame
    #cv2.imshow('result', frame)
    #if cv2.waitKey(30) & 0xFF == ord('q'):
    #    print('please press ctrl + c to end the program')
    #calculate the processing time of the function
    endTime = time.time()
    print('processtime %s' % (endTime - startTime))
    #用于计数
    cnt += 1
    #遍历人名，打印，新的人名加入totalName
    for name in names:
        print('\t%s' % name)

    #用于结束人脸识别
    if len(totalNames) >= nPerson or cnt >= 100:
        state = 5
        pub = rospy.Publisher('/whoiswho/end', String, queue_size=10)
        pub.publish("finish")
        print("/whoiswho/end - publishing finish")
        cv2.destroyAllWindows()
    #print('\t detect end')
    else:
        state = 3

def imageCB(data):
    global state
    #print('[imageCB] - callback once %d'%state)
    scaling_factor = 0.5
    if state == 0:
        pass
    elif state == 1:
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
    if state == 0:
        state = 1
 
def whoiswho():
    rospy.init_node('whoiswho', anonymous=True)
    rospy.Subscriber('/kinect2/qhd/image_color', Image, imageCB)
    rospy.Subscriber('/whoiswho/start', String, stringCB)
    rospy.spin()
 
if __name__ == '__main__':
    whoiswho()
