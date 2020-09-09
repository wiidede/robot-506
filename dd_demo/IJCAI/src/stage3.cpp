#include <ros/ros.h>
#include <std_msgs/String.h>
#include <sound_play/SoundRequest.h>
#include "xfyun_waterplus/IATSwitch.h"
#include "qrcodedetect/qrcodedetect.h"
#include <waterplus_map_tools/GetWaypointByName.h>
#include <string.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>        
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#define STATE_START   0
#define STATE_QR      1
#define STATE_END     3

int objects[10];

using namespace std;
using namespace cv;

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
    nFindIndex = msg->data.find("here");
    if(nFindIndex >= 0 )
    {
        nFindIndex = msg->data.find("tea");
        if(nFindIndex >= 0 )
        {
            nFindIndex = msg->data.find("lemon");
            if(nFindIndex >= 0 )
            {   
                if(objects[6])
                {
                    speak("please watch the screen to know the location of lemon tea");
                    string objPath = "/home/robot/photos/object/lemon_tea.jpg";
                    Mat objImg = imread(objPath);
                    namedWindow("object", CV_WINDOW_AUTOSIZE);
                    moveWindow("object", 0, 0);
                    imshow("object", objImg);
                    waitKey(100);
                    sleep(8);
                    destroyWindow("object");
                }
                else
                {
                    speak("there is no lemon tea");
                }
            }
            else
            {
                if(objects[0])
                {
                    speak("please watch the screen to know the location of vita tea");
                    string objPath = "/home/robot/photos/object/vita_tea.jpg";
                    Mat objImg = imread(objPath);
                    namedWindow("object", CV_WINDOW_AUTOSIZE);
                    moveWindow("object", 0, 0);
                    imshow("object", objImg);
                    waitKey(100);
                    sleep(8);
                    destroyWindow("object");
                }
                else
                {
                    speak("there is no vita tea");
                }
            }
        }
        nFindIndex = msg->data.find("biscuit");
        if(nFindIndex >= 0 )
        {   
            if(objects[1])
            {
                speak("please watch the screen to know the location of biscuit");
                string objPath = "/home/robot/photos/object/biscuit.jpg";
                Mat objImg = imread(objPath);
                namedWindow("object", CV_WINDOW_AUTOSIZE);
                moveWindow("object", 0, 0);
                imshow("object", objImg);
                waitKey(100);
                sleep(8);
                destroyWindow("object");
            }
            else
            {
                speak("there is no biscuit");
            }
        }
        nFindIndex = msg->data.find("party");
        if(nFindIndex >= 0 )
        {   
            if(objects[1])
            {
                speak("please watch the screen to know the location of vita tea");
                string objPath = "/home/robot/photos/object/vita_tea.jpg";
                Mat objImg = imread(objPath);
                namedWindow("object", CV_WINDOW_AUTOSIZE);
                moveWindow("object", 0, 0);
                imshow("object", objImg);
                waitKey(100);
                sleep(8);
                destroyWindow("object");
            }
            else
            {
                speak("there is no vita tea");
            }
        }
        nFindIndex = msg->data.find("chips");
        if(nFindIndex >= 0 )
        {   
            if(objects[2])
            {
                speak("please watch the screen to know the location of potato chips");
                string objPath = "/home/robot/photos/object/potato_chips.jpg";
                Mat objImg = imread(objPath);
                namedWindow("object", CV_WINDOW_AUTOSIZE);
                moveWindow("object", 0, 0);
                imshow("object", objImg);
                waitKey(100);
                sleep(8);
                destroyWindow("object");
            }
            else
            {
                speak("there is no photato chips");
            }
        }
        nFindIndex = msg->data.find("crisps");
        if(nFindIndex >= 0 )
        {   
            if(objects[3])
            {
                speak("please watch the screen to know the location of crisps");
                string objPath = "/home/robot/photos/object/crisps.jpg";
                Mat objImg = imread(objPath);
                namedWindow("object", CV_WINDOW_AUTOSIZE);
                moveWindow("object", 0, 0);
                imshow("object", objImg);
                waitKey(100);
                sleep(8);
                destroyWindow("object");
            }
            else
            {
                speak("there is no crisps");
            }
        }
        nFindIndex = msg->data.find("cake");
        if(nFindIndex >= 0 )
        {   
            if(objects[4])
            {
                speak("please watch the screen to know the location of cake");
                string objPath = "/home/robot/photos/object/cake.jpg";
                Mat objImg = imread(objPath);
                namedWindow("object", CV_WINDOW_AUTOSIZE);
                moveWindow("object", 0, 0);
                imshow("object", objImg);
                waitKey(100);
                sleep(8);
                destroyWindow("object");
            }
            else
            {
                speak("there is no cake");
            }
        }
        nFindIndex = msg->data.find("chocolate");
        if(nFindIndex >= 0 )
        {   
            if(objects[5])
            {
                speak("please watch the screen to know the location of chocolate");
                string objPath = "/home/robot/photos/object/chocolate.jpg";
                Mat objImg = imread(objPath);
                namedWindow("object", CV_WINDOW_AUTOSIZE);
                moveWindow("object", 0, 0);
                imshow("object", objImg);
                waitKey(100);
                sleep(8);
                destroyWindow("object");
            }
            else
            {
                speak("there is no chocolate");
            }
        }
        nFindIndex = msg->data.find("cola");
        if(nFindIndex >= 0 )
        {   
            if(objects[7])
            {
                speak("please watch the screen to know the location of coke cola");
                string objPath = "/home/robot/photos/object/coke_cola.jpg";
                Mat objImg = imread(objPath);
                namedWindow("object", CV_WINDOW_AUTOSIZE);
                moveWindow("object", 0, 0);
                imshow("object", objImg);
                waitKey(100);
                sleep(8);
                destroyWindow("object");
            }
            else
            {
                speak("there is no coke cola");
            }
        }
        nFindIndex = msg->data.find("snickers");
        if(nFindIndex >= 0 )
        {   
            if(objects[8])
            {
                speak("please watch the screen to know the location of snickers");
                string objPath = "/home/robot/photos/object/snickers.jpg";
                Mat objImg = imread(objPath);
                namedWindow("object", CV_WINDOW_AUTOSIZE);
                moveWindow("object", 0, 0);
                imshow("object", objImg);
                waitKey(100);
                sleep(8);
                destroyWindow("object");
            }
            else
            {
                speak("there is no snickers");
            }
        }
        nFindIndex = msg->data.find("gum");
        if(nFindIndex >= 0 )
        {   
            if(objects[9])
            {
                speak("please watch the screen to know the location of chew gum");
                string objPath = "/home/robot/photos/object/chew_gum.jpg";
                Mat objImg = imread(objPath);
                namedWindow("object", CV_WINDOW_AUTOSIZE);
                moveWindow("object", 0, 0);
                imshow("object", objImg);
                waitKey(100);
                sleep(8);
                destroyWindow("object");
            }
            else
            {
                speak("there is no chew gum");
            }
        }
    }
    nFindIndex = msg->data.find("many");
    if(nFindIndex >= 0 )
    {
        nFindIndex = msg->data.find("tea");
        if(nFindIndex >= 0 )
        {
            nFindIndex = msg->data.find("lemon");
            if(nFindIndex >= 0 )
            {   
                if(objects[6])
                {
                    speak("there is one.");
                    sleep(2);
                }
            }
            else
            {
                if(objects[0])
                {
                    speak("there is one.");
                    sleep(2);
                }
            }
        }
        nFindIndex = msg->data.find("party");
        if(nFindIndex >= 0 )
        {   
            if(objects[0])
            {
                speak("there is one.");
                sleep(2);
            }
        }
        nFindIndex = msg->data.find("biscuit");
        if(nFindIndex >= 0 )
        {   
            if(objects[1])
            {
                speak("there is one.");
                sleep(2);
            }
        }
        nFindIndex = msg->data.find("chips");
        if(nFindIndex >= 0 )
        {   
            if(objects[2])
            {
                speak("there is one.");
                sleep(2);
            }
        }
        nFindIndex = msg->data.find("crisps");
        if(nFindIndex >= 0 )
        {   
            if(objects[3])
            {
                speak("there is one.");
                sleep(2);
            }
        }
        nFindIndex = msg->data.find("cake");
        if(nFindIndex >= 0 )
        {   
            if(objects[4])
            {
                speak("there is one.");
                sleep(2);
            }
        }
        nFindIndex = msg->data.find("chocolate");
        if(nFindIndex >= 0 )
        {   
            if(objects[5])
            {
                speak("there is one.");
                sleep(2);
            }
        }
        nFindIndex = msg->data.find("cola");
        if(nFindIndex >= 0 )
        {   
            if(objects[7])
            {
                speak("there is one.");
                sleep(2);
            }
        }
        nFindIndex = msg->data.find("snickers");
        if(nFindIndex >= 0 )
        {   
            if(objects[8])
            {
                speak("there is one.");
                sleep(2);
            }
        }
        nFindIndex = msg->data.find("gum");
        if(nFindIndex >= 0 )
        {   
            if(objects[9])
            {
                speak("there is one.");
                sleep(2);
            }
        }
    }
    nFindIndex = msg->data.find("drink");
    if(nFindIndex >= 0 )
    {
        int nWaitCnt = objects[0] + objects[6] + objects[7];
        std::ostringstream stringStream;
        stringStream << nWaitCnt;
        std::string retStr = stringStream.str();
        speak("there is" + retStr);
        sleep(2);
    }
    nFindIndex = msg->data.find("snack");
    if(nFindIndex >= 0 )
    {   
        int nWaitCnt = objects[1] + objects[2] + objects[3] + objects[5] + objects[8] + objects[9];
        std::ostringstream stringStream;
        stringStream << nWaitCnt;
        std::string retStr = stringStream.str();
        speak("there is" + retStr);
        sleep(2);
    }
    nFindIndex = msg->data.find("food");
    if(nFindIndex >= 0 )
    {   
        if(objects[4])
        {
            speak("there is one.");
            sleep(2);
        }
        else
        {
            speak("there is zero.");
            sleep(2);
        }
    }
}

void KeyCB(const std_msgs::String::ConstPtr & msg)
{
    if (msg->data == "vita_tea")
    {
        objects[0] = 1;
    }
    if (msg->data == "biscuit")
    {
        objects[1] = 1;
    }
    if (msg->data == "potato_chips")
    {
        objects[2] = 1;
    }
    if (msg->data == "crisps")
    {
        objects[3] = 1;
    }
    if (msg->data == "cake")
    {
        objects[4] = 1;
    }
    if (msg->data == "chocolate")
    {
        objects[5] = 1;
    }
    if (msg->data == "lemon_tea")
    {
        objects[6] = 1;
    }
    if (msg->data == "coke_cola")
    {
        objects[7] = 1;
    }
    if (msg->data == "snickers")
    {
        objects[8] = 1;
    }
    if (msg->data == "chew_gum")
    {
        objects[9] = 1;
    }
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "IJCAI_03");
    
    ros::NodeHandle n;
    
    //ros::Subscriber pc_sub = n.subscribe("/kinect2/hd/points", 1 , ProcCloudCB);
    //ros::Subscriber rgb_sub = n.subscribe("/kinect2/hd/image_color",1 ,ProcColorCB);
    //ros::Subscriber rgb_sub = n.subscribe("webcam/image_raw",1 ,ProcColorCB);
    
    ros::Subscriber rgb_sub = n.subscribe("/kinect2/detect", 1, KeyCB);

    //语音与识别
    spk_pub = n.advertise<std_msgs::String>("/xfyun/tts", 10);
    clientIAT = n.serviceClient<xfyun_waterplus::IATSwitch>("xfyun_waterplus/IATSwitch");
    ros::Subscriber sub_sr = n.subscribe("/xfyun/iat", 10, KeywordCB);

    srvIAT.request.active = true;
    srvIAT.request.duration = 10;
    clientIAT.call(srvIAT);
    
    sleep(3);
    speak("I am ready");
   	
    ROS_INFO("[IJCAI - 03] mian function");
    ros::Rate r(10);
    while(ros::ok())
    {
        ros::spinOnce();
        r.sleep();
    }

    return 0;
}

