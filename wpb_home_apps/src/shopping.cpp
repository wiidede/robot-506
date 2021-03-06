#include <ros/ros.h>
#include <std_msgs/String.h>
#include "wpb_home_tutorials/Follow.h"
#include <geometry_msgs/Twist.h>
#include "xfyun_waterplus/IATSwitch.h"
#include <sound_play/SoundRequest.h>
#include "wpb_home_tutorials/Follow.h"
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include <waterplus_map_tools/Waypoint.h>
#include <waterplus_map_tools/GetWaypointByName.h>
#include <tf/transform_listener.h>
#include <geometry_msgs/PoseStamped.h>
#include <vector>
using namespace std;

#define STATE_READY     0
#define STATE_FOLLOW    1
#define STATE_ASK       2
#define STATE_GOTO      3
#define STATE_GRAB      4
#define STATE_COMEBACK  5
#define STATE_PASS      6


typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;
static string strGoto;
static sound_play::SoundRequest spk_msg;
static ros::Publisher spk_pub;
static ros::Publisher vel_pub;
static string strToSpeak = "";
static string strKeyWord = "";
static ros::ServiceClient clientIAT;
static xfyun_waterplus::IATSwitch srvIAT;
static ros::ServiceClient cliGetWPName;
static waterplus_map_tools::GetWaypointByName srvName;
static ros::Publisher add_waypoint_pub;
static ros::ServiceClient follow_start;
static ros::ServiceClient follow_stop;
static ros::ServiceClient follow_resume;
static wpb_home_tutorials::Follow srvFlw;
static ros::Publisher behaviors_pub;
static std_msgs::String behavior_msg;

static ros::Subscriber grab_result_sub;
static ros::Subscriber pass_result_sub;
static bool bGrabDone;
static bool bPassDone;

//语音传递的消息
static std_msgs::String strSpeak;

static int nState = STATE_READY;
static int nDelay = 0;
static int N = 0;
static vector<string> arKeyword;
static vector<string> svec;
// 添加航点关键词
void InitKeyword()
{
    arKeyword.push_back("start");   //机器人开始启动的地点,最后要回去
    arKeyword.push_back("milk");
    arKeyword.push_back("chips");
    arKeyword.push_back("safeguard");
    arKeyword.push_back("lao gan ma");
    arKeyword.push_back("cola");
    arKeyword.push_back("ice tea");
    arKeyword.push_back("water");
    arKeyword.push_back("porridge");
    arKeyword.push_back("napkin");
    arKeyword.push_back("sprite");
}

// 从句子里找arKeyword里存在的关键词
static string FindKeyword(string inSentence)
{
    string res = "";
    int nSize = arKeyword.size();
    for(int i=0;i<nSize;i++)
    {
        int nFindIndex = inSentence.find(arKeyword[i]);
        if(nFindIndex >= 0)
        {
            res = arKeyword[i];
            break;
        }
    }
    return res;
}
static void FindWord(string inSentence)
{
    int nSize = arKeyword.size();
    //ROS_INFO(""+nSize);
    for(int i=0;i<nSize;i++)
    {
        int nFindIndex = inSentence.find(arKeyword[i]);
        if(nFindIndex >= 0)
        {
            svec.push_back(arKeyword[i]);
        }
    }
    // ROS_INFO(""+arKeyword.size());
    //ROS_INFO(""+svec.size());
}

// 将机器人当前位置保存为新航点
void AddNewWaypoint(string inStr)
{
    tf::TransformListener listener;
    tf::StampedTransform transform;
    try
    {
        listener.waitForTransform("/map","/base_footprint",  ros::Time(0), ros::Duration(10.0) );
        listener.lookupTransform("/map","/base_footprint", ros::Time(0), transform);
    }
    catch (tf::TransformException &ex)
    {
        ROS_ERROR("[lookupTransform] %s",ex.what());
        return;
    }

    float tx = transform.getOrigin().x();
    float ty = transform.getOrigin().y();
    tf::Stamped<tf::Pose> p = tf::Stamped<tf::Pose>(tf::Pose(transform.getRotation() , tf::Point(tx, ty, 0.0)), ros::Time::now(), "map");
    geometry_msgs::PoseStamped new_pos;
    tf::poseStampedTFToMsg(p, new_pos);

    waterplus_map_tools::Waypoint new_waypoint;
    new_waypoint.name = inStr;
    new_waypoint.pose = new_pos.pose;
    add_waypoint_pub.publish(new_waypoint);

    ROS_WARN("[New Waypoint] %s ( %.2f , %.2f )" , new_waypoint.name.c_str(), tx, ty);
}

// 语音说话
static void Speak(std::string inStr)
{
    strSpeak.data = inStr;
    spk_pub.publish(strSpeak);
    ROS_INFO("[Speak] - %s", inStr.c_str());
    sleep(inStr.size() / 8);
}

// 跟随模式开关
static void FollowSwitch(bool inActive, float inDist)
{
    if(inActive == true)
    {
        srvFlw.request.thredhold = inDist;
        if (!follow_start.call(srvFlw))
        {
            ROS_WARN("[CActionManager] - follow start failed...");
        }
    }
    else
    {
        if (!follow_stop.call(srvFlw))
        {
            ROS_WARN("[CActionManager] - failed to stop following...");
        }
    }
}

// 物品抓取模式开关
static void GrabSwitch(bool inActive)
{
    if(inActive == true)
    {
        behavior_msg.data = "grab start";
        behaviors_pub.publish(behavior_msg);
    }
    else
    {
        behavior_msg.data = "grab stop";
        behaviors_pub.publish(behavior_msg);
    }
}

// 物品递给开关
static void PassSwitch(bool inActive)
{
    if(inActive == true)
    {
        behavior_msg.data = "pass start";
        behaviors_pub.publish(behavior_msg);
    }
    else
    {
        behavior_msg.data = "pass stop";
        behaviors_pub.publish(behavior_msg);
    }
}

// 语音识别结果处理函数
void KeywordCB(const std_msgs::String::ConstPtr & msg)
{
    ROS_WARN("------ Keyword = %s ------",msg->data.c_str());
    if(nState == STATE_FOLLOW)
    {
        // 从识别结果句子中查找物品（航点）关键词
        string strKeyword = FindKeyword(msg->data);
        int nLenOfKW = strlen(strKeyword.c_str());
        if(nLenOfKW > 0)
        {
            // 发现物品（航点）关键词
            AddNewWaypoint(strKeyword);
            string strSpeak = strKeyword + " . OK. I have memoried. Next one , please";
            Speak(strSpeak);
        }

        // 停止跟随的指令
        int nFindIndex = msg->data.find("top");
        if(nFindIndex >= 0)
        {
            FollowSwitch(false, 0);
            AddNewWaypoint("master");
            //ROS_INFO("1");
            nState = STATE_ASK;
            //ROS_INFO("2");
            nDelay = 0;
            //ROS_INFO("3");
            Speak("OK. What do you want me to fetch?");
            //ROS_INFO("4");
        }
    }

    if(nState == STATE_ASK)
    {
        // ROS_INFO("5");
        // 从识别结果句子中查找物品（航点）关键词
        svec.clear();
        FindWord(msg->data);
        //ROS_INFO("6");
        //ROS_INFO(""+svec.size());
        //必须识别到两个点才去
        //比赛前需要修改
        /////////////////////////////////////////////////////////////////////
        if(svec.size() == 2)
        {
            int nLenOfKW = strlen(svec[0].c_str());
            if(nLenOfKW > 0)
            {
                // 发现物品（航点）关键词
                strGoto = svec[0];
                string strSpeak;
                strSpeak = "OK. You want ";
                for (int i = 0; i < svec.size(); i ++)
                {
                    strSpeak += svec[i] + " ,";
                }
                strSpeak += ". I will go to get it for you.";
                Speak(strSpeak);
                nState = STATE_GOTO;
            }   
        }
        else if (svec.size() == 1)
        {
            Speak("please speak again.");
        }
    }
}

// 物品抓取状态
void GrabResultCallback(const std_msgs::String::ConstPtr& res)
{
    int nFindIndex = 0;
    nFindIndex = res->data.find("done");
    if( nFindIndex >= 0 )
    {
        bGrabDone = true;
    }
}

// 物品递给状态
void PassResultCallback(const std_msgs::String::ConstPtr& res)
{
    int nFindIndex = 0;
    nFindIndex = res->data.find("done");
    if( nFindIndex >= 0 )
    {
        bPassDone = true;
    }
}


int main(int argc, char** argv)
{
    ros::init(argc, argv, "wpb_home_shopping");

    ros::NodeHandle n;
    ros::Subscriber sub_sr = n.subscribe("/xfyun/iat", 10, KeywordCB);
    follow_start = n.serviceClient<wpb_home_tutorials::Follow>("wpb_home_follow/start");
    follow_stop = n.serviceClient<wpb_home_tutorials::Follow>("wpb_home_follow/stop");
    follow_resume = n.serviceClient<wpb_home_tutorials::Follow>("wpb_home_follow/resume");
    cliGetWPName = n.serviceClient<waterplus_map_tools::GetWaypointByName>("/waterplus/get_waypoint_name");
    add_waypoint_pub = n.advertise<waterplus_map_tools::Waypoint>( "/waterplus/add_waypoint", 1);
    spk_pub = n.advertise<std_msgs::String>("/xfyun/tts", 10);
    spk_msg.sound = sound_play::SoundRequest::SAY;
    spk_msg.command = sound_play::SoundRequest::PLAY_ONCE;
    vel_pub = n.advertise<geometry_msgs::Twist>("/cmd_vel", 10);
    clientIAT = n.serviceClient<xfyun_waterplus::IATSwitch>("xfyun_waterplus/IATSwitch");
    behaviors_pub = n.advertise<std_msgs::String>("/wpb_home/behaviors", 30);
    grab_result_sub = n.subscribe<std_msgs::String>("/wpb_home/grab_result",30,&GrabResultCallback);
    pass_result_sub = n.subscribe<std_msgs::String>("/wpb_home/pass_result",30,&PassResultCallback);

    InitKeyword();

    ROS_WARN("[main] wpb_home_shopping");
    ros::Rate r(30);
    while(ros::ok())
    {
        // 1、刚启动，准备
        if(nState == STATE_READY)
        {
            // 启动后延迟一段时间然后开始跟随
            nDelay ++;
            // ROS_WARN("[STATE_READY] - nDelay = %d", nDelay);
            if(nDelay > 100)
            {
                AddNewWaypoint("start");
                nDelay = 0;
                nState = STATE_FOLLOW;
                srvIAT.request.active = true;
                srvIAT.request.duration = 8;
                clientIAT.call(srvIAT);
            }
        }

        // 2、跟随阶段
        if(nState == STATE_FOLLOW)
        {
            if(nDelay == 0)
            {
                FollowSwitch(true, 0.7);
            }
            nDelay ++;
        }

        // 3、询问要去哪个航点
        /*   if(nState == STATE_ASK)
        {
            ROS_INFO("10");   
        }
        */

        // 4、导航去指定航点
        if(nState == STATE_GOTO)
        {
            srvName.request.name = strGoto;
            if (cliGetWPName.call(srvName))
            {
                std::string name = srvName.response.name;
                float x = srvName.response.pose.position.x;
                float y = srvName.response.pose.position.y;
                ROS_INFO("[STATE_GOTO] Get_wp_name = %s (%.2f,%.2f)", strGoto.c_str(),x,y);

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
                        Speak("OK. I am taking it.");
                        nState = STATE_GRAB;
                        nDelay = 0;
                    }
                    else
                    {
                        ROS_INFO("Failed to get to %s ...",strGoto.c_str() );
                        Speak("Failed to go to the waypoint.");
                        nState = STATE_ASK;
                    }
                }

            }
            else
            {
                ROS_ERROR("Failed to call service GetWaypointByName");
                Speak("There is no this waypoint.");
                nState = STATE_ASK;
            }
        }

        // 5、抓取物品
        if (nState == STATE_GRAB)
        {
            if (nDelay == 0)
            {
                N++;
                if (N < svec.size())               
                {
                    bGrabDone = false;
                    strGoto = svec[N];
                    nState = STATE_GOTO;
                }
                else
                {
                   svec.clear();
                   N = 0;
                   bGrabDone = true;
                }
            //GrabSwitch(true);
            }
            nDelay++;

            if (bGrabDone == true)
            {
                //GrabSwitch(false);
                Speak("I got it. I am coming back.");
                nState = STATE_COMEBACK;
            }
            
        }

        // 6、抓完物品回来
        if(nState == STATE_COMEBACK)
        {
            srvName.request.name = "master";
            if (cliGetWPName.call(srvName))
            {
                std::string name = srvName.response.name;
                float x = srvName.response.pose.position.x;
                float y = srvName.response.pose.position.y;
                ROS_INFO("[STATE_COMEBACK] Get_wp_name = %s (%.2f,%.2f)", strGoto.c_str(),x,y);

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
                        Speak("Hi,master. This is what you wanted.");
                        nState = STATE_PASS;
                        nDelay = 0;
                    }
                    else
                    {
                        ROS_INFO("Failed to get to %s ...",strGoto.c_str() );
                        Speak("Failed to go to the master.");
                        nState = STATE_ASK;
                    }
                }

            }
            else
            {
                ROS_ERROR("Failed to call service GetWaypointByName");
                Speak("There is no waypoint named master.");
                nState = STATE_ASK;
            }
        }

        // 7、将物品给主人
        if(nState == STATE_PASS)
        {
            if(nDelay == 0)
            {
                bPassDone = true;
                //PassSwitch(true);
            }
            nDelay ++;
            if(bPassDone == true)
            {
                //PassSwitch(false);
                Speak("OK. What do you want next?");
                nState = STATE_ASK;
            }
        }

        ros::spinOnce();
        r.sleep();
    }

    return 0;
}
