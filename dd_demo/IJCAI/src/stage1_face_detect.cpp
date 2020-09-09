#include <string>
#include <vector>
#include <ros/ros.h>
#include <std_msgs/String.h>
#include <sound_play/SoundRequest.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include "xfyun_waterplus/IATSwitch.h"
#include "qrcodedetect/qrcodedetect.h"
#include <waterplus_map_tools/GetWaypointByName.h>

#define STATE_START     0
#define STATE_QR        1
#define STATE_GO        2
#define STATE_DETECT    3
#define STATE_EXIST     4
#define STATE_NOTEXIST  5
#define STATE_EATING    6
#define STATE_END       7

using namespace std;

static int nState = STATE_START;
static int flag = 0;    //the first time. is there any person
                        //0 - no person
                        //1 - has person
typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;
static ros::Publisher spk_pub;
static std_msgs::String strSpeak;
static xfyun_waterplus::IATSwitch srvIAT;
static ros::ServiceClient clientIAT;
static ros::ServiceClient cliGetWPName;
static waterplus_map_tools::GetWaypointByName srvName;

vector<string> medicine;

void SplitString(const string& s, vector<string>& v, const string& c)
{
    string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while(string::npos != pos2)
    {
        v.push_back(s.substr(pos1, pos2-pos1));
         
        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }
    if(pos1 != s.length())
        v.push_back(s.substr(pos1));
}

//TTS
static void speak(std::string inStr)
{
    strSpeak.data = inStr;
    spk_pub.publish(strSpeak);
}

//Answer the question
void KeywordCB(const std_msgs::String::ConstPtr & msg)
{
    if(nState == STATE_EXIST || nState == STATE_NOTEXIST || nState == STATE_EATING || nState == STATE_END)
    {
        ROS_INFO("[KeywordCB] - %s", msg->data.c_str());
        
        int nFindIndex = 0;
        vector<string>::size_type i;
        
        nFindIndex = msg->data.find("rescription");
        if(nFindIndex >= 0 )
        {
            speak("the prescription is");
            sleep(2);
            ROS_INFO("[KeywordCB] - the prescription is");
            for(i = 0; i != medicine.size(); ++i)
            {
                speak(medicine[i]);
                ROS_INFO("[KeywordCB] - %s", medicine[i].c_str());
                sleep(5);
            }
            ROS_INFO("[KeywordCB] - the prescription is %s", msg->data.c_str());
        }
        nFindIndex = msg->data.find("first");
        if(nFindIndex >= 0 )
        {
            i = 0;
            speak(medicine[i]);
            ROS_INFO("[KeywordCB] - %s", medicine[i].c_str());
            sleep(3);
        }
        nFindIndex = msg->data.find("second");
        if(nFindIndex >= 0 )
        {
            i = 1;
            speak(medicine[i]);
            ROS_INFO("[KeywordCB] - %s", medicine[i].c_str());
            sleep(3);
        }
    }
}


//QRCode 检测
void QRCodeCB(const std_msgs::String::ConstPtr & msg)
{
    //clientIAT.call(srvIAT);
    if (nState == STATE_QR)
    {
        ROS_INFO("[QRCodeCB] - %s", msg->data.c_str());
        speak("Prescription received.");
        ROS_INFO("[QRCodeCB] - Prescription received.The prescription is");
        sleep(3);
        speak("The prescription is");
        sleep(2);
        //字符串分割
        SplitString(msg->data, medicine,","); //可按多个字符来分隔;
        for(vector<string>::size_type i = 0; i != medicine.size(); ++i)
        {
            speak(medicine[i]);
            ROS_INFO("[QRCodeCB] - %s", medicine[i].c_str());
            sleep(3);
        }
            
        //分割结束
        //speak(msg->data);
        sleep(6);
        nState = STATE_GO;
    }
    if (nState == STATE_DETECT)
    {
        ROS_INFO("exist %s", msg->data.c_str());
        if (msg->data == "person exist")
        {
            flag = 1;
            nState = STATE_EXIST;
        }
        if(msg->data == "person not exist")
        {
            flag = 0;
            nState = STATE_NOTEXIST;
            speak("please go here to eat medicine");
        }
    }
    if (nState == STATE_NOTEXIST)
    {
        ROS_INFO("not - exist %s", msg->data.c_str());
        if (msg->data == "person exist")
        {
            nState = STATE_EATING;
            //sleep(3);
            speak("please eat the medicine on the table");
            sleep(3);
        }
    }
}


int main(int argc, char** argv)
{
    ros::init(argc, argv, "IJCAI_01");
    
    ros::NodeHandle n;
    
    //ros::Subscriber pc_sub = n.subscribe("/kinect2/hd/points", 1 , ProcCloudCB);
    //ros::Subscriber rgb_sub = n.subscribe("/kinect2/hd/image_color",1 ,ProcColorCB);
    //ros::Subscriber rgb_sub = n.subscribe("webcam/image_raw",1 ,ProcColorCB);
    
    ros::Subscriber sub_qr = n.subscribe("/kinect2/qrcode", 10, QRCodeCB);
    //语音与识别
    ros::Subscriber sub_sr = n.subscribe("/xfyun/iat", 10, KeywordCB);
    spk_pub = n.advertise<std_msgs::String>("/xfyun/tts", 10);
    clientIAT = n.serviceClient<xfyun_waterplus::IATSwitch>("xfyun_waterplus/IATSwitch");
   	cliGetWPName = n.serviceClient<waterplus_map_tools::GetWaypointByName>("/waterplus/get_waypoint_name");
    ROS_INFO("[IJCAI - 01] mian function");
    
    nState = STATE_QR;
    
    ros::Rate r(10);
    while(ros::ok())
    {
        if(nState == STATE_GO)
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
                        nState = STATE_DETECT;
                        //开启语音识别
                        srvIAT.request.active = true;
                        srvIAT.request.duration = 10;
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
        if (nState == STATE_EXIST)
        {
            ROS_INFO("检测你吃没吃药");
            speak("检测你吃没吃药detecting what what hahaha");
            nState = STATE_EATING;
            sleep(3);
        }
        if (nState == STATE_EATING)
        {
            sleep(3);
            speak("the old man is eating the medicine");
            nState = STATE_END;
            //关闭语音识别
            //srvIAT.request.active = false;
            //clientIAT.call(srvIAT);
        }
        ros::spinOnce();
        r.sleep();
    }

    return 0;
}

