/*********************************************************************
* Software License Agreement (BSD License)
* 
*  Copyright (c) 2017-2020, Waterplus http://www.6-robot.com
*  All rights reserved.
* 
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
* 
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of the WaterPlus nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
* 
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  FOOTPRINTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*********************************************************************/
/* @author Zhang Wanjie                                             */
#include <ros/ros.h>
#include <std_msgs/String.h>
#include <sound_play/SoundRequest.h>
#include "xfyun_waterplus/IATSwitch.h"
#include "wpb_home_tutorials/Follow.h"

#define STATE_READY     0   //有限状态机里的准备状态
#define STATE_HAND      1   //有限状态机里的举手状态
#define STATE_WATCH     2   //有限状态机里的腕表状态
#define STATE_DOWN      3   //有限状态机里的摔倒状态
#define STATE_END       4   //有限状态机里的摔倒状态

//订阅主题和服务的变量和结构体
static ros::Publisher spk_pub;
static std_msgs::String strSpeak;
static ros::ServiceClient follow_start;
static ros::ServiceClient follow_stop;
static ros::ServiceClient follow_resume;
static ros::ServiceClient clientIAT;
static xfyun_waterplus::IATSwitch srvIAT;
static int nState = STATE_READY;    //有限状态机的初始状态
static int nWaitCnt = 10;           //倒计时时间

//TTS
static void speak(std::string inStr)
{
    strSpeak.data = inStr;
    spk_pub.publish(strSpeak);
}

//语音识别结果的回调函数
void KeywordCB(const std_msgs::String::ConstPtr & msg)
{
    
}

//openpose detect callback function
void PoseCB(const std_msgs::String::ConstPtr & msg)
{
    ROS_INFO("[PoseCB] - %s", msg->data.c_str());
    if (msg->data == "hang up")
    {
        nState = STATE_WATCH;
        speak("Please put your hand before the camera");
    }
    if (msg->data == "has watch")
    {
        nState = STATE_DOWN;
        speak("you have taken the watch");
        wpb_home_tutorials::Follow srv_start;
        srv_start.request.thredhold = 0.7;  //目标跟随的距离值,单位为米
        follow_start.call(srv_start);
    }
    if (msg->data == "no watch")
    {
        nState = STATE_DOWN;
        speak("please take the watch");
        wpb_home_tutorials::Follow srv_start;
        srv_start.request.thredhold = 0.7;  //目标跟随的距离值,单位为米
        follow_start.call(srv_start);
    }
    if (msg->data == "fall down")
    {
        nState = STATE_END;
        speak("you have fallen down");
    }
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "wpb_home_follow");  //程序初始化

    ros::NodeHandle n;
    ros::Subscriber sub_sr = n.subscribe("/xfyun/iat", 10, KeywordCB);  //订阅讯飞语音识别结果主题
    ros::Subscriber sub_pose = n.subscribe("/kinect2/openpose", 10, PoseCB);  //订阅openpose结果主题
    spk_pub = n.advertise<std_msgs::String>("/xfyun/tts", 10); //开辟一个主题,用来语音发音

    clientIAT = n.serviceClient<xfyun_waterplus::IATSwitch>("xfyun_waterplus/IATSwitch");   //连接语音识别开关服务
    follow_start = n.serviceClient<wpb_home_tutorials::Follow>("wpb_home_follow/start");   //连接跟随开始的服务
    follow_stop = n.serviceClient<wpb_home_tutorials::Follow>("wpb_home_follow/stop");      //连接跟随停止的服务
    follow_resume = n.serviceClient<wpb_home_tutorials::Follow>("wpb_home_follow/resume");   //连接跟随继续的服务

    ROS_INFO("[main] wpb_home_follow");
    nState = STATE_HAND;
    ros::Rate r(1);         //while函数的循环周期,这里为1Hz
    while(ros::ok())        //程序主循环
    {
        ros::spinOnce();        //短时间挂起,让回调函数得以调用
        r.sleep();          //控制循环周期的sleep函数,这里会暂停1秒(因为r在构造时参赛为1Hz)
    }
    
    return 0;
}
