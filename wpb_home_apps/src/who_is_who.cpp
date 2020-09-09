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
#include <vector>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include "xfyun_waterplus/IATSwitch.h"
#include <waterplus_map_tools/GetWaypointByName.h>
#include <sensor_msgs/Image.h>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;
static ros::Publisher spk_pub;
static ros::Publisher facereco_pub;
static ros::ServiceClient clientIAT;
static xfyun_waterplus::IATSwitch srvIAT;
static ros::ServiceClient cliGetWPName;
static waterplus_map_tools::GetWaypointByName srvName;

//有限状态机
#define STATE_READY     0
#define STATE_WAIT_ENTR 1
#define STATE_GOTO_RECO 2
#define STATE_WAIT_RECO 3
#define STATE_CONFIRM   4
#define STATE_GOTO_ROOM 5
#define STATE_FIND_OBJ  6
#define STATE_GOTO_CMD  7
#define STATE_FIND_FACE 8
#define STATE_GOTO_EXIT 9

//定义需要识别的人数(需要比赛前修改)
#define NUM_OF_PERSON 3

static int nState = STATE_WAIT_ENTR;  //程序启动时初始状态
//调试
//static int nState = STATE_FIND_FACE;

static int nOpenCount = 0;

static int nPersonCount = 0;

//识别关键词
static vector<string> arKWPerson;
static vector<string> arKWConfirm;
static vector<string> arKWObject;

//识别到的人名与物品名
static vector<string> Persons;
static vector<string> Objects;

//语音传递的消息
static std_msgs::String strSpeak;

//开启人脸识别传递的消息
static std_msgs::String strFaceReco;

//在当前状态下只执行一次任务，所需要进行判断的变量
static bool bGotoGoal = false;
static bool bTakePhoto = false;
static bool bOpenFace = true;

//当前的人物名字
static string currentName;

static void Init_keywords()
{
    //人名关键词(根据比赛前一天提供的人名列表进行修改)
    arKWPerson.push_back("jack");
    arKWPerson.push_back("tom");
    arKWPerson.push_back("lucy");
    arKWPerson.push_back("david");
    arKWPerson.push_back("james");
    arKWPerson.push_back("alex");
    arKWPerson.push_back("ryan");
    arKWPerson.push_back("john");
    arKWPerson.push_back("eric");
    arKWPerson.push_back("adam");
    arKWPerson.push_back("carter");
    arKWPerson.push_back("tyler");
    arKWPerson.push_back("lily");
    arKWPerson.push_back("mary");
    arKWPerson.push_back("anna");
    arKWPerson.push_back("zoe");
    arKWPerson.push_back("sofia");
    arKWPerson.push_back("sophia");//
    arKWPerson.push_back("faith");
    arKWPerson.push_back("julia");
    arKWPerson.push_back("paige");
    arKWPerson.push_back("jessica");

    //物品关键词(根据比赛前一天提供的物品名列表进行修改)
    arKWObject.push_back("milk");
    arKWObject.push_back("chips");
    arKWObject.push_back("safeguard");
    arKWObject.push_back("lao gan ma");//????
    arKWObject.push_back("cola");
    arKWObject.push_back("ice tea");
    arKWObject.push_back("water");
    arKWObject.push_back("porridge");
    arKWObject.push_back("napkin");
    arKWObject.push_back("sprite");

    //yes or no
    arKWConfirm.push_back("yes");
    arKWConfirm.push_back("Yes");
    arKWConfirm.push_back("Yeah");
    arKWConfirm.push_back("no");
    arKWConfirm.push_back("No");
}

//容错
//需要在关键字中加入错误的，然后在这边改正
static string correctKeyWord(string word)
{
    if (word == "sophia")
    {
        word = "sofia";
    }

    return word;
}

static string FindWord(string inSentence, vector<string> & arWord)
{
    string strRes = "";
	int nNum = arWord.size();
	for (int i = 0; i < nNum; i++)
	{
		int tmpIndex = inSentence.find(arWord[i]);
		if (tmpIndex >= 0)
		{
			strRes = arWord[i];
			break;
		}
	}
	return strRes;
}

//语音说话函数,参数为说话内容字符串
static void Speak(std::string inStr)
{
    strSpeak.data = inStr;
    spk_pub.publish(strSpeak);
    ROS_INFO("[Speak] - %s", inStr.c_str());
    sleep(inStr.size() / 8);
}

static void startFaceReco(std::string inStr)
{
    strFaceReco.data = inStr;
    facereco_pub.publish(strFaceReco);
}

static bool Goto(string inStr)
{
    string strGoto = inStr;     
    srvName.request.name = strGoto;
    if (cliGetWPName.call(srvName))
    {
        std::string name = srvName.response.name;
        float x = srvName.response.pose.position.x;
        float y = srvName.response.pose.position.y;
        ROS_INFO("Get_wp_name: name = %s (%.2f,%.2f)", strGoto.c_str(),x,y);

        MoveBaseClient ac("move_base", true);
        if(!ac.waitForServer(ros::Duration(5.0)))
        {
            ROS_INFO("The move_base action server is no running. action abort...");
            return false;
        }
        else
        {
            move_base_msgs::MoveBaseGoal goal;
            goal.target_pose.header.frame_id = "map";
            goal.target_pose.header.stamp = ros::Time::now();
            goal.target_pose.pose = srvName.response.pose;
            ac.sendGoal(goal);
            ac.waitForResult();
            if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED)
            {
                ROS_INFO("Arrived at %s!",strGoto.c_str());
                return true;
            }
            else
            {
                ROS_INFO("Failed to get to %s ...",strGoto.c_str() );
                return false;
            }
        }
        
    }
    else
    {
        ROS_ERROR("Failed to call service GetWaypointByName");
        return false;
    }
}

void EntranceCB(const std_msgs::String::ConstPtr & msg)
{
    //ROS_WARN("[WhoIsWho EntranceCB] - %s",msg->data.c_str());
    string strDoor = msg->data;
    if(strDoor == "door open")
    {
        nOpenCount ++;
    }
    else
    {
        nOpenCount = 0;
    }
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
    ROS_WARN("[stage4 KeywordCB] - %s", strListen.c_str());

    if(nState == STATE_WAIT_RECO)
    {
        ROS_WARN("STATE_WAIT_RECO");
        //判断是否跳过本次回调，如果句子中有判断词，也不会confirm阶段，必须等下一次语音识别的回调
        bool bFindObj = false;
        bTakePhoto = false;

        //确认的内容
        string strRepeat;

        //从听到的句子里找人名
        string person = FindWord(strListen,arKWPerson);

        //对难以识别的人名加以处理
        person = correctKeyWord(person);

        if(person.length() > 0)
        {
            printf("句子里包含人名 - %s \n",person.c_str());
            currentName = person;

            bTakePhoto = true;

            strRepeat = "your name is " + person + ".";
        }

        //从听到的句子里找物品名
        string object = FindWord(strListen,arKWObject);

        //对难以识别的物品名加以处理
        object = correctKeyWord(object);

        if(object.length() > 0)
        {
            printf("句子里包含物品名 - %s \n", object.c_str());

            //识别到了物品名关键字
            bFindObj = true;

            strRepeat += "and you want " + object;
        }

        //如果识别到了人名与物品，确认
        if (bTakePhoto == true && bFindObj == true)
        {
            bFindObj = false;
            Persons.push_back(person);
            Objects.push_back(object);
            nState = STATE_CONFIRM;

            Speak(strRepeat);
            return;
        }
    }

    if(nState == STATE_CONFIRM)
    {
        ROS_WARN("STATE_CONFIRM");

        string confirm = FindWord(strListen,arKWConfirm);
        if(confirm == "yes" || confirm == "Yes" || confirm == "Yeah")
        {
            nPersonCount ++;
            if(nPersonCount >= NUM_OF_PERSON)   //要识别的人名个数
            {
                bGotoGoal = true;
                nState = STATE_GOTO_ROOM;
            }
            else
            {
                Speak("ok,I have memory you. Next one, please");
                nState = STATE_WAIT_RECO;
            }
        }
        if(confirm == "no " || confirm == "No ")
        {
            Speak("ok,Repeat your name");
            nState = STATE_WAIT_RECO;
        }
    }
}

void ProcColorCB(const sensor_msgs::ImageConstPtr& msg)
{
    cv_bridge::CvImagePtr cv_ptr;

    try
    {
        cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
    }
    catch (cv_bridge::Exception& e)
    {
        ROS_ERROR("cv_bridge exception: %s", e.what());
        return;
    }

    if (nState == STATE_CONFIRM && bTakePhoto == true)
    {
        bTakePhoto = false;
        string filename = "/home/robot/photos/person/" + currentName + ".png";
        ROS_INFO("saving the picture %s", filename.c_str());
        imwrite(filename, cv_ptr->image);
    }

    if (nState == STATE_FIND_OBJ)
    {
        string filename = "/home/robot/photos/object/objects.png";
        ROS_INFO("saving the picture %s", filename.c_str());
        imwrite(filename, cv_ptr->image);

        sleep(3);

        bGotoGoal = true;
        nState = STATE_GOTO_CMD;
    }
}

void FaceEndCB(const std_msgs::String::ConstPtr & msg)
{
    ROS_WARN("[WhoIsWho FaceEndCB] - %s",msg->data.c_str());
    if (msg->data == "finish")
    {
        bGotoGoal = true;
        nState = STATE_GOTO_EXIT;
    }
    else
    {
        int i = 0;
        for (i = 0; i < Persons.size(); i ++)
        {
            if (Persons[i] == msg->data)
            {
                break;
            }
        }
        string strTakeObj = Persons[i] + ", get here to take your " + Objects[i];
        Speak(strTakeObj);
        nPersonCount ++;
    }
    if (nPersonCount >= NUM_OF_PERSON)
    {
        bGotoGoal = true;
        nState = STATE_GOTO_EXIT;
    }
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "wpb_home_WhoIsWho");
    Init_keywords();

    ros::NodeHandle n;
    ros::Subscriber sub_ent = n.subscribe("/wpb_home/entrance_detect", 10, EntranceCB);
    cliGetWPName = n.serviceClient<waterplus_map_tools::GetWaypointByName>("/waterplus/get_waypoint_name");
    ros::Subscriber rgb_sub = n.subscribe("/kinect2/qhd/image_color", 1, ProcColorCB);
    ros::Subscriber sub_sr = n.subscribe("/xfyun/iat", 10, KeywordCB);
    spk_pub = n.advertise<std_msgs::String>("/xfyun/tts", 10);
    clientIAT = n.serviceClient<xfyun_waterplus::IATSwitch>("xfyun_waterplus/IATSwitch");
    facereco_pub = n.advertise<std_msgs::String>("/whoiswho/start", 10);
    ros::Subscriber sub_end = n.subscribe("/whoiswho/end", 10, FaceEndCB);

    //可以输出中文
    setlocale(LC_ALL, "");

    ROS_INFO("[main] wpb_home_WhoIsWho");
    ROS_WARN("[main] 记得清除人脸存放目录下的已经存在的人脸");
    ROS_WARN("[main] 必须要有cmd room exit这三个航点");

    ros::Rate r(10);
    while(ros::ok())
    {
        //等待开门
        if(nState == STATE_WAIT_ENTR)
        {
            //等待开门,一旦检测到开门,便去往发令地点
            if(nOpenCount > 20)
            {
                bool bArrived = Goto("cmd");
                if(bArrived == true)
                {
                    Speak("My name is robot,Tell me your name and look at me");
                    nState = STATE_WAIT_RECO;
                    srvIAT.request.active = true;
                    srvIAT.request.duration = 6;
                    clientIAT.call(srvIAT);
                }
            }
        }
        //去房间
        if(nState == STATE_GOTO_ROOM && bGotoGoal == true)
        {
            bGotoGoal = false;
            
            //识别完毕,关闭语音识别
            srvIAT.request.active = false;
            clientIAT.call(srvIAT);

            Speak("ok,I have memory you. I am leaving.");
            bool bArrived = Goto("room");
            if(bArrived == true)
            {
                //识别、抓取物品
                nState = STATE_FIND_OBJ;
            }
        }

        //返回cmd点
        if(nState == STATE_GOTO_CMD && bGotoGoal == true)
        {
            bGotoGoal = false;

            bool bArrived = Goto("cmd");
            if(bArrived == true)
            {
                //人脸识别
                nState = STATE_FIND_FACE;
            }
        }

        //人脸识别开启（whoiswho.py 进入识别状态）
        if (nState == STATE_FIND_FACE && bOpenFace == true)
        {
            bOpenFace = false;
            /*
            //调试
            Persons.push_back("jack");
            Persons.push_back("alex");
            Persons.push_back("tom");

            Objects.push_back("milk");
            Objects.push_back("chips");
            Objects.push_back("cola");

            //识别完毕,关闭语音识别
            srvIAT.request.active = false;
            clientIAT.call(srvIAT);
            //调试完毕
            */

            startFaceReco("start");
            nPersonCount = 0;
            //等待订阅到识别成功的消息，然后去出口
        }

        //去出口
        if(nState == STATE_GOTO_EXIT && bGotoGoal == true)
        {
            bGotoGoal = false;

            sleep(5);
            Speak("I am leaving.");
            Goto("exit");
        }
        ros::spinOnce();
        r.sleep();
    }

    return 0;
}
