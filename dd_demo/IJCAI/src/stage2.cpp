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
#include <stdlib.h>
#include <time.h>

#define STATE_READY     0   //有限状态机里的准备状态
#define STATE_WATCH     1   //有限状态机里的腕表状态
#define STATE_FOLLOW    2   //有限状态机里的跟随状态
#define STATE_DOWN      3   //有限状态机里的摔倒状态
#define STATE_END       4   //有限状态机里的结束状态

//订阅主题和服务的变量和结构体
static ros::Publisher spk_pub;
static ros::Publisher watch_pub;
static std_msgs::String strSpeak;
static std_msgs::String strWatch;
static ros::ServiceClient follow_start;
static ros::ServiceClient follow_stop;
static ros::ServiceClient follow_resume;
static ros::ServiceClient clientIAT;
static xfyun_waterplus::IATSwitch srvIAT;
static int nState = STATE_READY;    //有限状态机的初始状态
static int nWaitCnt = 10;           //倒计时时间

static int watch = 0;

//TTS
static void speak(std::string inStr)
{
    strSpeak.data = inStr;
    spk_pub.publish(strSpeak);
}

//语音识别结果的回调函数
void KeywordCB(const std_msgs::String::ConstPtr & msg)
{
    int nFindIndex = 0;
    nFindIndex = msg->data.find("ollow me");   //开始跟随的语音指令关键词,可以替换成其他关键词
    if( nFindIndex >= 0 )
    {
        std::string strwa = "watch";
        strWatch.data = strwa;
        watch_pub.publish(strWatch);
        
        if (watch == 0)
        {
            speak("you don't wear a smart wristband,  please wear your smart wristband");
            ROS_WARN("[KeywordCB] - The old man did't wear a smart wristband"); 
            sleep(10);
        }
        else
        {
            ROS_WARN("[KeywordCB] - The old man wore a smart wristband");
        }
        speak("OK, Let's go.");    //语音回应发令者
        ROS_WARN("[KeywordCB] - OK, Let's go.");
        //从语音识别结果中提取到了"Follow me"字符串,说明已经接收到跟随开始的语音指令
        //ROS_WARN("[KeywordCB] - 开始跟随");
        wpb_home_tutorials::Follow srv_start;
        srv_start.request.thredhold = 0.7;  //目标跟随的距离值,单位为米
        if (follow_start.call(srv_start))   //调用启智ROS的跟随服务
        {
            ROS_WARN("[KeywordCB] - follow start !");           //调用服务成功
            nState = STATE_FOLLOW;         //调用成功了,改变状态机的状态值到跟随状态
        }
        else
        {
            ROS_WARN("[KeywordCB] - follow start failed...");   //调用服务失败
        }
    }
}

//openpose detect callback function
void PoseCB(const std_msgs::String::ConstPtr & msg)
{
    ROS_INFO("[PoseCB] - %s", msg->data.c_str());
    if (msg->data == "has watch")
    {
        watch = 1;
    }
    if (msg->data == "fall down")
    {
        if (nState == STATE_FOLLOW)
        {
            nState = STATE_DOWN;
            speak("call the phone number 0 8 6 1 8 8 6 2 2 8 2 9 7 5 : the old man fell down");
            //停止跟随 
            wpb_home_tutorials::Follow srv_stop;
            follow_stop.call(srv_stop);
            //关闭语音识别
            srvIAT.request.active = false;
            clientIAT.call(srvIAT);
            //生成报告
            ROS_WARN("**********accident report***********");
            if (watch)
            {
                ;
            }
            else
            {
                ROS_WARN("* The old man forget to wear the   *");
                ROS_WARN("* smart wristband before he walk.  *");
            }
            //获取系统时间  
            time_t now_time=time(NULL);  
            //获取本地时间  
            tm*  t_tm = localtime(&now_time);  
            ROS_WARN("*The old man fallen down          *");
            ROS_WARN("*Falling time : %s", asctime(t_tm));
            ROS_WARN("***********************************");
        }
    }
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "wpb_home_follow");  //程序初始化

    ros::NodeHandle n;
    ros::Subscriber sub_sr = n.subscribe("/xfyun/iat", 10, KeywordCB);  //订阅讯飞语音识别结果主题
    ros::Subscriber sub_pose = n.subscribe("/kinect2/openpose", 10, PoseCB);  //订阅openpose结果主题
    spk_pub = n.advertise<std_msgs::String>("/xfyun/tts", 10); //开辟一个主题,用来语音发音
    watch_pub = n.advertise<std_msgs::String>("/ijcai/watch", 10); //开辟一个主题,用来语音发音
	
    clientIAT = n.serviceClient<xfyun_waterplus::IATSwitch>("xfyun_waterplus/IATSwitch");   //连接语音识别开关服务
    follow_start = n.serviceClient<wpb_home_tutorials::Follow>("wpb_home_follow/start");   //连接跟随开始的服务
    follow_stop = n.serviceClient<wpb_home_tutorials::Follow>("wpb_home_follow/stop");      //连接跟随停止的服务
    follow_resume = n.serviceClient<wpb_home_tutorials::Follow>("wpb_home_follow/resume");   //连接跟随继续的服务

    ROS_INFO("[main] wpb_home_follow");
    nState = STATE_WATCH;
    speak("If you want to let me follow, please speak   follow me.");
    ROS_WARN("-----If you want to let me follow, please speak   follow me");
    sleep(3);
    int cnt = 0;
    //开启语音识别
    srvIAT.request.active = true;
    srvIAT.request.duration = 10;
    clientIAT.call(srvIAT);
    ros::Rate r(1);         //while函数的循环周期,这里为1Hz
    while(ros::ok())        //程序主循环
    {
        cnt ++;
        if (nState == STATE_WATCH)
        {
            if (cnt > 20)
            {
                cnt = 0;
                //关闭语音识别
                srvIAT.request.active = false;
                clientIAT.call(srvIAT);
                speak("If you want to let me follow, please speak \"follow me.\"");
                ROS_WARN("###If you want to let me follow, please speak   follow me");
                sleep(6);
                //开启语音识别
                srvIAT.request.active = true;
                srvIAT.request.duration = 10;
                clientIAT.call(srvIAT);
            }
        }
        ros::spinOnce();        //短时间挂起,让回调函数得以调用
        r.sleep();          //控制循环周期的sleep函数,这里会暂停1秒(因为r在构造时参赛为1Hz)
    }
    
    return 0;
}
