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
#include <vector>
#include "action_manager.h"
#include <sound_play/SoundRequest.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include "xfyun_waterplus/IATSwitch.h"
#include <waterplus_map_tools/GetWaypointByName.h>

using namespace std;

typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;
static ros::Publisher spk_pub;
static CActionManager action_manager;
static ros::ServiceClient clientIAT;
static xfyun_waterplus::IATSwitch srvIAT;
static ros::ServiceClient cliGetWPName;
static waterplus_map_tools::GetWaypointByName srvName;

static std_msgs::String strSpeak;

//有限状态机
#define STATE_READY     0
#define STATE_WAIT_ENTR 1
#define STATE_GOTO_CMD  2
#define STATE_WAIT_CMD  3
#define STATE_CONFIRM   4
#define STATE_ACTION    5

static int nState = STATE_WAIT_ENTR;  //程序启动时初始状态;

//识别结果
static string result_placement;     //去往地点
static string result_object;        //抓取物品
static string result_person;        //交互对象人
static string result_action;        //其他行为

//识别关键词
static vector<string> arKWPlacement;
static vector<string> arKWObjece;
static vector<string> arKWPerson;
static vector<string> arKWAction;
static vector<string> arKWConfirm;
static void Init_keywords()
{
    //地点关键词
    arKWPlacement.push_back("liveing room");
    arKWPlacement.push_back("kitchen");
    arKWPlacement.push_back("bedroom");

    //物品关键词
    arKWObjece.push_back("water");
    arKWObjece.push_back("chips");
    arKWObjece.push_back("milk");

    //人名关键词
    arKWPerson.push_back("jack");
    arKWPerson.push_back("tom");

    //其他行为
    arKWAction.push_back("team name");
    arKWAction.push_back("your name");
arKWAction.push_back("most handsome");
arKWAction.push_back("in Canada");
arKWAction.push_back("in the world");
arKWAction.push_back("smartphone");
arKWAction.push_back("largest coin");
arKWAction.push_back("the first time");
arKWAction.push_back("the second time");
arKWAction.push_back("the winter");
arKWAction.push_back("police formed");
arKWAction.push_back("the royal canadian");
arKWAction.push_back("is canada's only desert");
arKWAction.push_back("big is Canada's only desert");
arKWAction.push_back("nanobot");
arKWAction.push_back("small");
arKWAction.push_back("the first compute");
arKWAction.push_back("hard disk");
arKWAction.push_back("big was");
arKWAction.push_back("computer bug");
arKWAction.push_back("mechanical knight");
arKWAction.push_back("knowledge engineering");
arKWAction.push_back("is a chatbot");
arKWAction.push_back("cars safe");
arKWAction.push_back("invented the");
arKWAction.push_back("programming language");
arKWAction.push_back("python");
arKWAction.push_back("a robot");
arKWAction.push_back("of the apple");
arKWAction.push_back("considered to");
arKWAction.push_back("program do");
arKWAction.push_back("the shelf");
arKWAction.push_back("the plant");
arKWAction.push_back("in the dining room");
arKWAction.push_back("smallest food");
arKWAction.push_back("lightest drink");
arKWAction.push_back("is today");
arKWAction.push_back("which year");
arKWAction.push_back("the sofa");
arKWAction.push_back("the chair");
arKWAction.push_back("the table");
arKWAction.push_back("the tv table");
arKWAction.push_back("the drink");
arKWAction.push_back("the food");





arKWAction.push_back("count invented");
arKWAction.push_back("first president");
arKWAction.push_back("for the Emperor");
arKWAction.push_back("which city");
arKWAction.push_back("many children");
arKWAction.push_back("was called");
arKWAction.push_back("the northern");
arKWAction.push_back("the spaceship");
arKWAction.push_back("first king");
arKWAction.push_back("name of");
arKWAction.push_back("capital of");
arKWAction.push_back("another name");
arKWAction.push_back("prefer to");
arKWAction.push_back("jump higher");
arKWAction.push_back("name of");
arKWAction.push_back("fish with");//

arKWAction.push_back("flag contain");

arKWAction.push_back("the tiger");
arKWAction.push_back("she was born");
arKWAction.push_back("number of");
arKWAction.push_back("the leading");

arKWAction.push_back("the country");
arKWAction.push_back("the sound");
arKWAction.push_back("around the earth");
arKWAction.push_back("the most");
arKWAction.push_back("color is");
arKWAction.push_back("rubber");

arKWAction.push_back("we use to");
arKWAction.push_back("the light");
arKWAction.push_back("invented the");

arKWAction.push_back("in space");


arKWAction.push_back("hot air");
arKWAction.push_back("existing metal");
arKWAction.push_back("inventor of");
arKWAction.push_back("steam engine");
arKWAction.push_back("invented by");
arKWAction.push_back("lightest existing");
arKWAction.push_back("three primary");
arKWAction.push_back("nearest the sun");
arKWAction.push_back("the Great Wall");
arKWAction.push_back("human bone");
arKWAction.push_back("is named after");
arKWAction.push_back("on the flag");
arKWAction.push_back("do you get");
arKWAction.push_back("largest number");



    //yes or no
    arKWConfirm.push_back("yes");
    arKWConfirm.push_back("Yes");
    arKWConfirm.push_back("Yeah");
    arKWConfirm.push_back("no");
    arKWConfirm.push_back("No");
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

static void Speak(string inStr)
{
    strSpeak.data = inStr;
    spk_pub.publish(strSpeak);
}

static int nOpenCount = 0;
void EntranceCB(const std_msgs::String::ConstPtr & msg)
{
    //ROS_WARN("[GPSR EntranceCB] - %s",msg->data.c_str());
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

void KeywordCB(const std_msgs::String::ConstPtr & msg)
{
    //ROS_WARN("[GPSR KeywordCB] - %s",msg->data.c_str());
    string strListen = msg->data;

    if(nState == STATE_WAIT_CMD)
    {
        bool bAction = false;
        //[1]从听到的句子里找地点
        string placement = FindWord(strListen,arKWPlacement);
        if(placement.length() > 0)
        {
            printf("句子里包含地点 - %s \n",placement.c_str());
            stAct newAct;
            newAct.nAct = ACT_GOTO;
            newAct.strTarget = placement;
            action_manager.arAct.push_back(newAct);
            bAction = true;
        }

        //[2]从听到的句子里找物品
        string object = FindWord(strListen,arKWObjece);
        if(object.length() > 0)
        {
            printf("句子里包含物品 - %s \n",object.c_str());
            stAct newAct;
            newAct.nAct = ACT_GRAB;
            newAct.strTarget = object;
            action_manager.arAct.push_back(newAct);
            bAction = true;
        }

        //[3]从听到的句子里找人名
        string person = FindWord(strListen,arKWPerson);
        if(person.length() > 0)
        {
            printf("句子里包含人名 - %s \n",person.c_str());
            stAct newAct;
            newAct.nAct = ACT_PASS;
            newAct.strTarget = person;
            action_manager.arAct.push_back(newAct);
            bAction = true;
        }

        //[4]从听到的句子里找其他行为
        string action = FindWord(strListen,arKWAction);
        if(action.length() > 0)
        {
            printf("句子里包含其他行为 - %s \n",action.c_str());
            stAct newAct;
            newAct.nAct = ACT_SPEAK;
            //根据行为来确定说话内容
            if(action == "team name")
            {
                newAct.strTarget = "The team name is Shanghai University";
            }
            if(action == "your name")
            {
                newAct.strTarget = "My name is Robot";
            }
if(action == "most handsome")
            {
                newAct.strTarget = "I that Justin Trudeau is very handsome";
            }
if(action == "in Canada")
            {
                newAct.strTarget = "Canada spans almost 10 million square km and comprises 6 time zones";
            }
if(action == "in the world")
            {
                newAct.strTarget = "Yonge Street in Ontario is the longest street in the world";
            }
if(action == "smartphone")
            {
                newAct.strTarget = "It was developed in Ontario, at Research In Motion's Waterloo offices";
            }
if(action == "largest coin")
            {
                newAct.strTarget = "The Big Nickel in Sudbury, Ontario. It is nine meters in diameter";
            }
if(action == "the first time")
            {
                newAct.strTarget = "The first time that the USA invaded Canada was in 1775";
            }
if(action == "the second time")
            {
                newAct.strTarget = "The USA invaded Canada a second time in 1812";
            }
if(action == "the winter")
            {
                newAct.strTarget = "Canada does! With 14 Golds at the 2010 Vancouver Winter Olympics";
            }
if(action == "the royal canadian")
            {
                newAct.strTarget = "In 1920, when The Mounted Police merged with the Dominion Police";
            }
if(action == "police formed")
            {
                newAct.strTarget = "The Mounted Police was formed in 1873";
            }
if(action == "is canada's only desert")
            {
                newAct.strTarget = "Canada's only desert is British Columbia";
            }
if(action == "big is canada's only desert")
            {
                newAct.strTarget = "The British Columbia desert is only 15 miles long";
            }
if(action == "nanobot")
            {
                newAct.strTarget = "The smallest robot possible is called a nanobot";
            }
if(action == "small")
            {
                newAct.strTarget = "A nanobot can be less than one-thousandth of a millimeter";
            }
if(action == "the first compute")
            {
                newAct.strTarget = "The IBM 305 RAMAC";
            }
if(action == "hard disk")
            {
                newAct.strTarget = "The IBM 305 RAMAC was launched in 1956";
            }
if(action == "big was")
            {
                newAct.strTarget = "The IBM 305 RAMAC hard disk weighed over a ton and stored 5 MB of data";
            }
if(action == "computer bug")
            {
                newAct.strTarget = "The first actual computer bug was a dead moth stuck in a Harvard Mark II";
            }
if(action == "mechanical knight")
            {
                newAct.strTarget = "A robot sketch made by Leonardo DaVinci";
            }
if(action == "knowledge engineering")
            {
                newAct.strTarget = "It is when you need to load an AI with enough knowledge to start learning";
            }
if(action == "is a chatbot")
            {
                newAct.strTarget = "A chatbot is an A.I. you put in customer service to avoid paying salaries";
            }
if(action == "cars safe")
            {
                newAct.strTarget = "Yes. Car accidents are product of human misconduct";
            }
if(action == "invented the")
            {
                newAct.strTarget = "Grace Hoper. She wrote it in her spare time";
            }
if(action == "programming language")
            {
                newAct.strTarget = "C was invented by Dennis MacAlistair Ritchie";
            }
if(action == "python")
            {
                newAct.strTarget = "Python was invented by Guido van Rossum";
            }
if(action == "a robot")
            {
                newAct.strTarget = "Sure. I've never seen him drink water";
            }
if(action == "of the apple")
            {
                newAct.strTarget = "My lord and master Steve Wozniak";
            }
if(action == "considered to")
            {
                newAct.strTarget = "Ada Lovelace";
            }
if(action == "program do")
            {
                newAct.strTarget = "Adobe Wan Kenobi";
            }
if(action == "the shelf")
            {
                newAct.strTarget = "The shelf is in the kitchen";
            }
if(action == "the plant")
            {
                newAct.strTarget = "The plant is in the living room";
            }
if(action == "in the dining room")
            {
                newAct.strTarget = "There is no chair in the dining room";
            }
if(action == "smallest food")
            {
                newAct.strTarget = "The bread is the smallest in the food category";
            }
if(action == "lightest drink")
            {
                newAct.strTarget = "The Coke Zero, is lighter than water";
            }
if(action == "is today")
            {
                newAct.strTarget = "Today is Friday";
            }
if(action == "which year")
            {
                newAct.strTarget = "RoboCup@Home was founded in 2006";
            }
if(action == "the sofa")
            {
                newAct.strTarget = "Near the table";
            }
if(action == "the chair")
            {
                newAct.strTarget = "Near the tv table";
            }
if(action == "the tv table")
            {
                newAct.strTarget = "Near the sofa";
            }
if(action == "the table")
            {
                newAct.strTarget = "Between the chairs";
            }
if(action == "the drink")
            {
                newAct.strTarget = "In the kitchen";
            }
if(action == "the food")
            {
                newAct.strTarget = "In the dining room";
            }
if(action == "the bed")
            {
                newAct.strTarget = "In the bedroom";
            }





if(action == "do you get")
            {
                newAct.strTarget = "Pink";
            }
if(action == "on the flag")
            {
                newAct.strTarget = "Four stars";
            }
if(action == "is named after")
            {
                newAct.strTarget = "Venezuela";
            }
if(action == "human bone")
            {
                newAct.strTarget = "Clavicle";
            }
if(action == "largest number")
            {
                newAct.strTarget = "99999";
            }
if(action == "the Great Wall")
            {
                newAct.strTarget = "6259 kilometers";
            }
if(action == "nearest the sun")
            {
                newAct.strTarget = "Mercury";
            }
if(action == "three primary")
            {
                newAct.strTarget = "Blue, yellow and red";
            }
if(action == "lightest existing")
            {
                newAct.strTarget = "Aluminium";
            }
if(action == "invented by")
            {
                newAct.strTarget = "The typewriter";
            }
if(action == "steam engine")
            {
                newAct.strTarget = "James Watt";
            }
if(action == "in space")
            {
                newAct.strTarget = "Alan Shepard";
            }
if(action == "hot air")
            {
                newAct.strTarget = "Montgolfier";
            }
if(action == "invented the")
            {
                newAct.strTarget = "Telescope";
            }
if(action == "the food")
            {
                newAct.strTarget = "Goodyear";
            }
if(action == "the light")
            {
                newAct.strTarget = "Blue";
            }

if(action == "we use to")
            {
                newAct.strTarget = "The northern hemisphere";
            }
if(action == "rubber")
            {
                newAct.strTarget = "Yuri Gagarin";
            }
if(action == "short for")
            {
                newAct.strTarget = "Random Access Memory";
            }
if(action == "the sound")
            {
                newAct.strTarget = "Kbps";
            }

if(action == "domain of")
            {
                newAct.strTarget = "The .be domain";
            }
if(action == "the leading")
            {
                newAct.strTarget = "Whoopi Goldberg";
            }
if(action == "the name")
            {
                newAct.strTarget = "Captain Picard";
            }
if(action == "best James Bond")
            {
                newAct.strTarget = "Austin Powers";
	    }
if(action == "around the earth")
            {
                newAct.strTarget = "Fifty-three";
            }

if(action == "the sound")
            {
                newAct.strTarget = "Quentin Tarantino";
            }
if(action == "the leading")
            {
                newAct.strTarget = "Pekinese";
            }
if(action == "number of")
            {
                newAct.strTarget = "Number 742";
            }
if(action == "she was born")
            {
                newAct.strTarget = "Six toes";
            }
if(action == "the tiger")
            {
                newAct.strTarget = "Siberian tiger";
            }
if(action == "flag contain")
            {
                newAct.strTarget = "Maple";
            }
if(action == "fish with")
            {
                newAct.strTarget = "Eel fish";
            }
if(action == "name of")
            {
                newAct.strTarget = "Wisent";
            }
if(action == "jump higher")
            {
                newAct.strTarget = "Fleas";
            }
if(action == "prefer to")
            {
                newAct.strTarget = "Mosquitoes";
            }
if(action == "another name")
            {
                newAct.strTarget = "Paleontology";
            }
if(action == "the food")
            {
                newAct.strTarget = "Melbourne";
            }
if(action == "name of Paris")
            {
                newAct.strTarget = "Lutetia";
            }
if(action == "name of")
            {
                newAct.strTarget = "New Amsterdam";
            }
if(action == "first king")
            {
                newAct.strTarget = "Leopold I";
            }
if(action == "the spaceship")
            {
                newAct.strTarget = "Alien";
            }
if(action == "the northern")
            {
                newAct.strTarget = "Hadrians wall";
            }
if(action == "was called")
            {
                newAct.strTarget = "Louis XIV";
            }
if(action == "many children")
            {
                newAct.strTarget = "Nine children";
            }
if(action == "which city")
            {
                newAct.strTarget = "Belfast";
            }
if(action == "for the Emperor")
            {
                newAct.strTarget = "Pork";
            }
if(action == "first president")
            {
                newAct.strTarget = "George Washington";
            }
if(action == "count invented")
            {
                newAct.strTarget = "Count von Zeppelin";
            }

            action_manager.arAct.push_back(newAct);
            bAction = true;
        }

        if(bAction == true)
        {
            Speak(strListen);
            nState = STATE_CONFIRM;
        }
    }

    if(nState == STATE_CONFIRM)
    {
        string confirm = FindWord(strListen,arKWConfirm);
        if(confirm == "yes" || confirm == "Yes" ||confirm == "Yeah")
        {
            Speak("ok,I will do it");
            action_manager.ShowActs();
            nState = STATE_ACTION;
            //识别完毕,关闭语音识别
            srvIAT.request.active = false;
            clientIAT.call(srvIAT);
        }
        if(confirm == "no" || confirm == "No")
        {
            action_manager.Reset();
            Speak("ok,Repeat the command");
            nState = STATE_WAIT_CMD;
        }
    }

    if(nState == STATE_ACTION && action_manager.nCurActCode == ACT_LISTEN)
    {
        action_manager.strListen = strListen;
    }
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "wpb_home_gpsr");
    Init_keywords();
    action_manager.Init();

    ros::NodeHandle n;
    ros::Subscriber sub_ent = n.subscribe("/wpb_home/entrance_detect", 10, EntranceCB);

    ros::Subscriber sub_sr = n.subscribe("/xfyun/iat", 10, KeywordCB);
    spk_pub = n.advertise<std_msgs::String>("/xfyun/tts", 10);
    clientIAT = n.serviceClient<xfyun_waterplus::IATSwitch>("xfyun_waterplus/IATSwitch");

    cliGetWPName = n.serviceClient<waterplus_map_tools::GetWaypointByName>("/waterplus/get_waypoint_name");

    ROS_INFO("[main] wpb_home_gpsr");
    ros::Rate r(10);
    while(ros::ok())
    {
        if(nState == STATE_WAIT_ENTR)
        {
            //等待开门,一旦检测到开门,便去往发令地点
            if(nOpenCount > 20)
            {
                string strGoto = "cmd";     //cmd是发令地点名称,请在地图里设置这个航点
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
                            ros::spinOnce();
                            nState = STATE_WAIT_CMD;
                            srvIAT.request.active = true;
                            srvIAT.request.duration = 8;
                            clientIAT.call(srvIAT);
                        }
                        else
                            ROS_INFO("Failed to get to %s ...",strGoto.c_str() );
                    }
                    
                }
                else
                {
                    ROS_ERROR("Failed to call service GetWaypointByName");
                }
            }
        }
        if(nState == STATE_ACTION)
        {
            action_manager.Main();
        }
        ros::spinOnce();
        r.sleep();
    }

    return 0;
}
