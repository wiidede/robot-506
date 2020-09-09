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

using namespace std;

static ros::Publisher spk_pub;
static ros::ServiceClient clientIAT;
static xfyun_waterplus::IATSwitch srvIAT;


//语音传递的消息
static std_msgs::String strSpeak;


//当前的人物名字
static string currentName;


//语音说话函数,参数为说话内容字符串
static void Speak(std::string inStr)
{
    strSpeak.data = inStr;
    spk_pub.publish(strSpeak);
    ROS_INFO("[Speak] - %s", inStr.c_str());
    sleep(inStr.size() / 10);
}

string LowerCase(string s)
{
    int dif='a'-'A';
    for(int i=0;i<s.length();i++)
    {
        if((s[i]>='A')&&(s[i]<='Z'))
            s[i]+=dif;
    }
   return s;
}

void KeywordCB(const std_msgs::String::ConstPtr & msg)
{
    ROS_WARN("[stage4 KeywordCB] - %s", msg->data.c_str());
    string strListen = LowerCase(msg->data);
    if(msg->data == "no " || msg->data == "No ")
    {
        Speak("ok,Repeat your name");
    }
    ROS_WARN("[stage4 KeywordCB] - %s", strListen.c_str());
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "ijcai04");

    ros::NodeHandle n;
    ros::Subscriber sub_sr = n.subscribe("/xfyun/iat", 10, KeywordCB);
    spk_pub = n.advertise<std_msgs::String>("/xfyun/tts", 10);
    clientIAT = n.serviceClient<xfyun_waterplus::IATSwitch>("xfyun_waterplus/IATSwitch");


    //可以输出中文
    setlocale(LC_ALL, "");

    ROS_INFO("[main] wpb_home_WhoIsWho");
    ROS_WARN("[main] 记得清除人脸存放目录下的已经存在的人脸");
    ROS_WARN("[main] 必须要有cmd room exit这三个航点");

    sleep(3);
    srvIAT.request.active = true;
    srvIAT.request.duration = 6;
    clientIAT.call(srvIAT);

    ros::Rate r(10);
    while(ros::ok())
    {
        ros::spinOnce();
        r.sleep();
    }

    return 0;
}

