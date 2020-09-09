#!/usr/bin/env python
#coding=utf-8
import rospy
#导入mgs到pkg中
from std_msgs.msg import String

#回调函数输入的应该是msg
def callback(msg):
    rospy.loginfo('[QRCode Listener] - data : %s', msg.data)

def listener():
    rospy.init_node('QRCodeListener', anonymous=True)
    #Subscriber函数第一个参数是topic的名称，第二个参数是接受的数据类型 第三个参数是回调函数的名称
    rospy.Subscriber('/voice/tuling_nlu_topic', String, callback)
    rospy.spin()

if __name__ == '__main__':
    listener()

