#include <ros/ros.h>
#include <std_msgs/String.h>
#include <sound_play/SoundRequest.h>
#include "xfyun_waterplus/IATSwitch.h"
#include "qrcodedetect/qrcodedetect.h"
#include <waterplus_map_tools/GetWaypointByName.h>
#include <string.h>

#define STATE_START   0
#define STATE_QR      1
#define STATE_END     3

using namespace std;

static int nState = STATE_START;
string strlisten;

static ros::Publisher detect_pub;
static ros::Publisher spk_pub;
static std_msgs::String strSpeak;
static xfyun_waterplus::IATSwitch srvIAT;
static ros::ServiceClient clientIAT;
static std_msgs::String strWatch;


//TTS
static void speak(const std::string inStr)
{
    strSpeak.data = inStr;
    spk_pub.publish(strSpeak);
}

void KeywordCB(const std_msgs::String::ConstPtr & msg)
{
//十个物体是否要每个都要判断
    int nFindIndex = 0;
    nFindIndex = msg->data.find("ow many objects");
    if(nFindIndex >= 0 )
    {
        strlisten = "ow many objects";
        //发布对应的物体名       
        std::string strwa = "amoxicillin";
        strWatch.data = strwa;
        detect_pub.publish(strWatch);
        //speak("The number is ");
    }
    nFindIndex = msg->data.find("here is the object");
    if(nFindIndex >= 0 )
    {
         //发布对应的物体名
        strlisten = "here is object";       
        std::string strwa = "objects";
        strWatch.data = strwa;
        detect_pub.publish(strWatch);
        //speak("If you want to know the place of object,please watch the screen.");
    }
}

void KeyCB(const std_msgs::String::ConstPtr & msg)
{
    if(strlisten == "ow many objects")
    {
        string a = "the number is " + msg->data;
        speak(a);
    }
    if(strlisten == "here is object")
    {
        if (msg->data == "one")
        {
            speak("If you want to know the place of object,please watch the screen.");
        }   
        if (msg->data == "zero")
        {
            speak("SOrry, i can't find the obeject");
        }
    }
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "IJCAI_03");
    
    ros::NodeHandle n;
    
    //ros::Subscriber pc_sub = n.subscribe("/kinect2/hd/points", 1 , ProcCloudCB);
    //ros::Subscriber rgb_sub = n.subscribe("/kinect2/hd/image_color",1 ,ProcColorCB);
    //ros::Subscriber rgb_sub = n.subscribe("webcam/image_raw",1 ,ProcColorCB);
    
    ros::Subscriber rgb_sub = n.subscribe("/kinect2/detect2",1 , KeyCB);
    ros::Subscriber sub_sr = n.subscribe("/xfyun/iat", 10, KeywordCB);  //订阅讯飞语音识别结果主题
    detect_pub = n.advertise<std_msgs::String>("/ijcai/detect2", 10); //开辟一个主题

    //语音与识别
    spk_pub = n.advertise<std_msgs::String>("/xfyun/tts", 10);
    clientIAT = n.serviceClient<xfyun_waterplus::IATSwitch>("xfyun_waterplus/IATSwitch");
    
    srvIAT.request.active = true;
    srvIAT.request.duration = 10;
    clientIAT.call(srvIAT);
   	
    ROS_INFO("[IJCAI - 03] mian function");
    ros::Rate r(10);
    while(ros::ok())
    {
        ros::spinOnce();
        r.sleep();
    }

    return 0;
}

