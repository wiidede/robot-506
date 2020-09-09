#include <ros/ros.h>
#include <std_msgs/String.h>
#include <sensor_msgs/Image.h>
#include <sound_play/SoundRequest.h>
#include "xfyun_waterplus/IATSwitch.h"
#include "wpb_home_tutorials/Follow.h"
#include <cv_bridge/cv_bridge.h>
#include <image_transport/image_transport.h>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"

//有限状态机的三个状态
#define STATE_READY          0   //有限状态机里的准备状态
#define STATE_FOLLOW         1   //有限状态机里的跟随状态
#define STATE_OVERPHOTO      2   //老人摔倒状态
#define STATE_END	         3   //结束状态


//订阅主题和服务的变量和结构体
static ros::Publisher spk_pub;
static ros::ServiceClient follow_start;
static ros::ServiceClient follow_stop;
static ros::ServiceClient follow_resume;
static ros::ServiceClient clientIAT;
static xfyun_waterplus::IATSwitch srvIAT;
static int nState = STATE_READY;    //有限状态机的初始状态

//语音
static std_msgs::String strSpeak;

//语音说话函数,参数为说话内容字符串
static void Speak(std::string inStr)
{
    strSpeak.data = inStr;
    spk_pub.publish(strSpeak);
}
//保存照片
void ProcColorCB(const sensor_msgs::ImageConstPtr & msg)
{
    //ROS_INFO("ProcColorCB");
	if(nState == STATE_OVERPHOTO)
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
		// 保存图片
		imwrite("/home/robot/catkin_ws/photos",cv_ptr->image);
		ROS_INFO("[callbackRGB] Save the image of the random  event!");
		nState = STATE_END;
	}
}
//语音识别结果的回调函数
void KeywordCB(const std_msgs::String::ConstPtr & msg)
{
    //ROS_WARN("[KeywordCB] - %s",msg->data.c_str());
    int nFindIndex = 0;
    nFindIndex = msg->data.find("Follow me");   //开始跟随的语音指令关键词,可以替换成其他关键词
    if( nFindIndex >= 0 )
    {
        //从语音识别结果中提取到了"Follow me"字符串,说明已经接收到跟随开始的语音指令
        //ROS_WARN("[KeywordCB] - 开始跟随");
        Speak("OK, Let's go.'");    //语音回应发令者
        wpb_home_tutorials::Follow srv_start;
        srv_start.request.thredhold = 0.7;  //目标跟随的距离值,单位为米
        if (follow_start.call(srv_start))   //调用启智ROS的跟随服务
        {
            ROS_WARN("[KeywordCB] - follow start !");           //调用服务成功
            nState = STATE_FOLLOW;          //调用成功了,改变状态机的状态值到跟随状态
        }
        else
        {
            ROS_WARN("[KeywordCB] - follow start failed...");   //调用服务失败
        }
    }
}
//检测到老人摔倒
void poseDetect(const std_msgs::String::ConstPtr & msg)
{

    if(msg->data =="fall down")
    {
        //停止跟随
        wpb_home_tutorials::Follow srv_stop;
        follow_stop.call(srv_stop);
        //拍照
        nState = STATE_OVERPHOTO;
        //拨打电话
        Speak("12345");
        //Speak("The old man fell down.");
        //生成事故报告
        ROS_INFO("The old man fell down.");
    }
    else  if(msg->data =="no watch")
    {
		Speak("Please take the watch.");
    }
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "wpb_home_follow");  //程序初始化

    ros::NodeHandle n;
    ros::Subscriber sub_sr = n.subscribe("/xfyun/iat", 10, KeywordCB);  //订阅讯飞语音识别结果主题
	ros::Subscriber rgb_sub = n.subscribe("/kinect2/qhd/image_color", 1 , ProcColorCB);//订阅摄像头消息
	//ros::Subscriber pose_sub = n.subscribe("/kinect2/qrcode", 10, poseDetect);
    ros::Subscriber pose_sub = n.subscribe("/kinect2/pose_detect", 10, poseDetect);  //订阅肢体检测结果主题
    //spk_pub = n.advertise<sound_play::SoundRequest>("/robotsound", 20); //开辟一个主题,用来语音发音

    spk_pub = n.advertise<std_msgs::String>("/xfyun/tts", 10); 
    clientIAT = n.serviceClient<xfyun_waterplus::IATSwitch>("xfyun_waterplus/IATSwitch");   //连接语音识别开关服务
    follow_start = n.serviceClient<wpb_home_tutorials::Follow>("wpb_home_follow/start");   //连接跟随开始的服务
    follow_stop = n.serviceClient<wpb_home_tutorials::Follow>("wpb_home_follow/stop");      //连接跟随停止的服务
    follow_resume = n.serviceClient<wpb_home_tutorials::Follow>("wpb_home_follow/resume");   //连接跟随继续的服务

    ROS_INFO("[main] wpb_home_follow");
    ros::Rate r(1);         	 //while函数的循环周期,这里为1Hz
    while(ros::ok())        	 //程序主循环
    {
        ros::spinOnce();        //短时间挂起,让回调函数得以调用
        r.sleep();		            //控制循环周期的sleep函数,这里会暂停1秒(因为r在构造时参赛为1Hz
    }

    return 0;
}

