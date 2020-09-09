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
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>        
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <time.h>

#define STATE_START     0
#define STATE_QR        1
#define STATE_GO        2
#define STATE_DETECT    3
#define STATE_EXIST     4
#define STATE_NOTEXIST  5
#define STATE_EATING    6
#define STATE_END       7

using namespace std;
using namespace cv;

static int nState = STATE_START;
static int flag = 0;    //the first time. is there any person
                        //0 - no person
                        //1 - has person
static ros::Publisher detect_pub;

//第几个药
static int N = 0;

typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;
static ros::Publisher spk_pub;
static std_msgs::String strSpeak;
static xfyun_waterplus::IATSwitch srvIAT;
static ros::ServiceClient clientIAT;
static ros::ServiceClient cliGetWPName;
static waterplus_map_tools::GetWaypointByName srvName;
static std_msgs::String strWatch;

vector<string> medicine;
vector<string> eatingTime;

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
static void speak(const std::string inStr)
{
    strSpeak.data = inStr;
    spk_pub.publish(strSpeak);
}

//Answer the question
void KeywordCB(const std_msgs::String::ConstPtr & msg)
{
    if(nState == STATE_EXIST || nState == STATE_NOTEXIST || nState == STATE_EATING)
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
                string medicineName = medicine[i];
                string medicinePath = "/home/robot/photos/medicine/" + medicineName + ".png";
                speak(medicineName);
                ROS_INFO("[KeywordCB] - %s", medicineName.c_str());
                Mat medicineImg = imread(medicinePath);
                imshow("medicine", medicineImg);
                waitKey(100);
                sleep(3);
                destroyWindow("medicine");
            }
            ROS_INFO("[KeywordCB] - the prescription is %s", msg->data.c_str());
        }
        nFindIndex = msg->data.find("hich");
        if(nFindIndex >= 0 )
        {
            i = N;
            if (i < medicine.size())
            {
                speak("the medicine is");
                sleep(2);
                string medicineName = medicine[i];
                string medicinePath = "/home/robot/photos/medicine/" + medicineName + ".png";
                speak(medicineName);
                ROS_INFO("[KeywordCB] - %s", medicineName.c_str());
                Mat medicineImg = imread(medicinePath);
                imshow("medicine", medicineImg);
                waitKey(100);
                sleep(3);
                destroyWindow("medicine");
            }
        }
        nFindIndex = msg->data.find("ext");
        if(nFindIndex >= 0 )
        {
            i = N;
            if (i < medicine.size())
            {
                speak("the medicine is");
                sleep(2);
                string medicineName = medicine[i];
                string medicinePath = "/home/robot/photos/medicine/" + medicineName + ".png";
                speak(medicineName);
                ROS_INFO("[KeywordCB] - %s", medicineName.c_str());
                Mat medicineImg = imread(medicinePath);
                imshow("medicine", medicineImg);
                waitKey(100);
                sleep(3);
                destroyWindow("medicine");
            }
        }
        nFindIndex = msg->data.find("hat");
        if(nFindIndex >= 0 )
        {
            
            if (N > 0)
            {
                i = N - 1;
            }
            else
            {
                i = N;
            }
            
            if (i < medicine.size())
            {
                speak("the medicine is");
                sleep(2);
                string medicineName = medicine[i];
                string medicinePath = "/home/robot/photos/medicine/" + medicineName + ".png";
                speak(medicineName);
                ROS_INFO("[KeywordCB] - %s", medicineName.c_str());
                Mat medicineImg = imread(medicinePath);
                imshow("medicine", medicineImg);
                waitKey(100);
                sleep(3);
                destroyWindow("medicine");
            }
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
        SplitString(msg->data, medicine,";"); //可按多个字符来分隔;
        for(vector<string>::size_type i = 0; i != medicine.size(); ++i)
        {
            string medicineName = medicine[i];
            string medicinePath = "/home/robot/photos/medicine/" + medicineName + ".png";
            Mat medicineImg = imread(medicinePath);
            
            speak(medicineName);
            ROS_INFO("[QRCodeCB] - %s", medicineName.c_str());
            
            namedWindow("medicine", CV_WINDOW_AUTOSIZE);
            moveWindow("medicine", 0, 0);
            resizeWindow("medicine", 512, 512);
            imshow("medicine", medicineImg);
            waitKey(100);
            sleep(3);
            destroyWindow("medicine");
        }
        //分割结束
        //speak(msg->data);
        sleep(4);
        nState = STATE_EXIST;
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


void detectCB(const std_msgs::String::ConstPtr & msg)
{
    for(vector<string>::size_type i = N; i < medicine.size(); ++i)
    {
        if (medicine[N] == msg->data)
        {
            N++;
            time_t now_time=time(NULL);  
            tm*  t_tm = localtime(&now_time);
            eatingTime.push_back(asctime(t_tm));
            break;
        }
    }
    ROS_WARN("****** medicine taking report\t ******");
    ROS_WARN("********** prescription  \t***********");
    for(vector<string>::size_type i = N; i < medicine.size(); ++i)
    {
        ROS_WARN("****** %s\t ******", medicine[i].c_str());
    }
    ROS_WARN("**********  taking time  \t***********");
    for(vector<string>::size_type i = N; i < eatingTime.size(); ++i)
    {
        ROS_WARN("****** %s\t ******", eatingTime[i].c_str());
    }
    if (N >= medicine.size())
    {
        nState = STATE_END;
    }
    ROS_INFO("[detectCB] - %s - %d", msg->data.c_str(), N);
    std::string strwa = "watch";
    strWatch.data = strwa;
    detect_pub.publish(strWatch);
}
int main(int argc, char** argv)
{
    ros::init(argc, argv, "IJCAI_01");
    
    ros::NodeHandle n;
    
    //ros::Subscriber pc_sub = n.subscribe("/kinect2/hd/points", 1 , ProcCloudCB);
    //ros::Subscriber rgb_sub = n.subscribe("/kinect2/hd/image_color",1 ,ProcColorCB);
    ros::Subscriber rgb_sub = n.subscribe("/kinect2/detect", 10, detectCB);
    
    ros::Subscriber sub_qr = n.subscribe("/kinect2/qrcode", 10, QRCodeCB);
    //语音与识别
    ros::Subscriber sub_sr = n.subscribe("/xfyun/iat", 10, KeywordCB);
    spk_pub = n.advertise<std_msgs::String>("/xfyun/tts", 10);
    clientIAT = n.serviceClient<xfyun_waterplus::IATSwitch>("xfyun_waterplus/IATSwitch");
    cliGetWPName = n.serviceClient<waterplus_map_tools::GetWaypointByName>("/waterplus/get_waypoint_name");
    detect_pub = n.advertise<std_msgs::String>("/ijcai/detect", 10); //开辟一个主题

    ROS_INFO("[IJCAI - 01] mian function");
    
    nState = STATE_QR;
    
    ros::Rate r(10);
    while(ros::ok())
    {
        if (nState == STATE_EXIST)
        {
            //开启语音识别
            srvIAT.request.active = true;
            srvIAT.request.duration = 10;
            clientIAT.call(srvIAT);
            std::string strwa = "watch";
            strWatch.data = strwa;
	        detect_pub.publish(strWatch);
            //ROS_INFO("检测你吃没吃药");
            speak("please take the medicine");
            nState = STATE_EATING;
            sleep(3);
        }
        if (nState == STATE_EATING)
        {
            sleep(3);
            //speak("the old man is eating the medicine");
            //nState = STATE_END;
            //关闭语音识别
            //srvIAT.request.active = false;
            //clientIAT.call(srvIAT);
        }
        if (nState == STATE_END)
        {
            srvIAT.request.active = false;
            clientIAT.call(srvIAT);
        }
        ros::spinOnce();
        r.sleep();
    }

    return 0;
}

