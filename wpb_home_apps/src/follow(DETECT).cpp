/*#include <ros/ros.h>
#include <vector>
#include <std_msgs/String.h>
#include <sound_play/SoundRequest.h>
#include "xfyun_waterplus/IATSwitch.h"
#include "wpb_home_tutorials/Follow.h"
#include <waterplus_map_tools/GetWaypointByName.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include "action_manager.h"

//有限状态机的三个状态
#define STATE_READY     0   //有限状态机里的准备状态
#define STATE_FOLLOW    1   //有限状态机里的跟随状态
#define STATE_WAIT      2   //有限状态机里的等待状态
#define STATE_PAUSE     3   //有限状态机里的不计时等待状态
#define STATE_GO        4   
#define STATE_IAT       5
#define STATE_FIND      6   
#define STATE_GOBACK    7

//订阅主题和服务的变量和结构体
typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;
static ros::Publisher spk_pub;
static ros::ServiceClient follow_start;
static ros::ServiceClient follow_stop;
static ros::ServiceClient follow_resume;
static ros::ServiceClient clientIAT;
static waterplus_map_tools::GetWaypointByName srvName;
static ros::ServiceClient cliGetWPName;
static xfyun_waterplus::IATSwitch srvIAT;
static ros::Publisher add_waypoint_pub;
static int nState = STATE_READY;    //有限状态机的初始状态
static int nWaitCnt = 10;           //倒计时时间
static vector<string> arKWPlacement; //地点关键词
static string strKeyword;  //stop位置
//语音
static string strSpeak;
static string keyPlace;


//语音说话函数,参数为说话内容字符串
static void Speak(std::string inStr)
{
   // sound_play::SoundRequest sp;
   // sp.sound = sound_play::SoundRequest::SAY;
   // sp.command = sound_play::SoundRequest::PLAY_ONCE;
   // sp.arg = inStr;
    spk_pub.publish(inStr);
}
static void Init_keywords()
{
 arKWPlacement.push_back("kitchen");
 arKWPlacement.push_back("卧室");
 arKWPlacement.push_back("客厅");
 arKWPlacement.push_back("help");
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
//进行语音识别
void KeyWordCB(const std_msgs::String::ConstPtr & msg)
{
    if(nState == STATE_IAT)
	{
	//srvIAT.request.active = true;
        //srvIAT.request.duration = 10;
        //clientIAT.call(srvIAT);
	 strSpeak = msg->data;
	 keyPlace = FindWord(strSpeak,arKWPlacement);
	 nState = STATE_GO;
	}
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

//语音识别结果的回调函数
void KeywordCB(const std_msgs::String::ConstPtr & msg)
{
    //ROS_WARN("[KeywordCB] - %s",msg->data.c_str());
    int nFindIndex = 0;
    nFindIndex = msg->data.find("跟随我");   //开始跟随的语音指令关键词,可以替换成其他关键词
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

    nFindIndex = msg->data.find("车来了"); //等10秒的语音指令关键词,可以替换成其他关键词
    if( nFindIndex >= 0 )
    {
        //ROS_WARN("[KeywordCB] - 停止跟随");
		strKeyword = "stop";
		AddNewWaypoint(strKeyword);//添加车子位置
       // Speak("OK, I will stay here for 10 seconds.'");    //语音回应发令者
        wpb_home_tutorials::Follow srv_stop;
        if (follow_stop.call(srv_stop))                 //调用启智ROS的跟随停止服务
        {
            ROS_WARN("[KeywordCB] - stop following!");          //调用服务成功
            sleep(4);
           nWaitCnt = 10;
          nState = STATE_WAIT;          //调用成功了,改变状态机的状态值到停止等待状态
		
	}
        else
        {
            ROS_WARN("[KeywordCB] - failed to stop following...");   //调用服务失败
        }
    }

    nFindIndex = msg->data.find("wait"); //不计时等待的语音指令关键词,可以替换成其他关键词
    if( nFindIndex >= 0 )
    {
         Speak("OK, I will go there for next command.'");    //语音回应发令者
        wpb_home_tutorials::Follow srv_stop;
        if (follow_stop.call(srv_stop))                 //调用启智ROS的跟随停止服务
        {
            
            nState = STATE_PAUSE;        //调用成功了,改变状态机的状态值到无限等待状态
        }
        else
        {
            ROS_WARN("[KeywordCB] - failed to Wait...");   //调用服务失败
        }
    }
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "wpb_home_follow");  //程序初始化
    Init_keywords();
    ros::NodeHandle n;
    ros::Subscriber sub_sr = n.subscribe("/xfyun/iat", 10, KeywordCB);  //订阅讯飞语音识别结果主题
    ros::Subscriber sub__sr = n.subscribe("/xfyun/iat", 10, KeyWordCB);
    spk_pub = n.advertise<sound_play::SoundRequest>("/robotsound", 20); //开辟一个主题,用来语音发音
    
    clientIAT = n.serviceClient<xfyun_waterplus::IATSwitch>("xfyun_waterplus/IATSwitch");   //连接语音识别开关服务
    follow_start = n.serviceClient<wpb_home_tutorials::Follow>("wpb_home_follow/start");   //连接跟随开始的服务
    follow_stop = n.serviceClient<wpb_home_tutorials::Follow>("wpb_home_follow/stop");      //连接跟随停止的服务
    follow_resume = n.serviceClient<wpb_home_tutorials::Follow>("wpb_home_follow/resume");   //连接跟随继续的服务
    cliGetWPName = n.serviceClient<waterplus_map_tools::GetWaypointByName>("/waterplus/get_waypoint_name");
	add_waypoint_pub = n.advertise<waterplus_map_tools::Waypoint>( "/waterplus/add_waypoint", 1);

    ROS_INFO("[main] wpb_home_follow");
    ros::Rate r(1);         //while函数的循环周期,这里为1Hz
    while(ros::ok())        //程序主循环
    {
        ros::spinOnce();        //短时间挂起,让回调函数得以调用
        r.sleep();          //控制循环周期的sleep函数,这里会暂停1秒(因为r在构造时参赛为1Hz)
        if(nState == STATE_WAIT)    //有限状态机里的等待状态
        {
           wpb_home_tutorials::Follow srv_stop;
        if (follow_stop.call(srv_stop))                 //调用启智ROS的跟随停止服务
        {
            
             nState = STATE_IAT;     
        }
        else
        {
            ROS_WARN("[KeywordCB] - failed to Wait...");   //调用服务失败
        }
                 //调用成功了,改变状态机的状态值到跟随状态
               
        }
                if(nState==STATE_GO)
		{
		ROS_INFO("1");
                string strGoto = keyPlace;     //发令地点名称,请在地图里设置这个航点
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
                            sleep(3);
			  nState=STATE_FIND;  							
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
         if(nState == STATE_FIND)
	{// 去空房间
                ROS_INFO("2");
		string strGoto = arKWPlacement[3]; 
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
                            sleep(3); 
							
                        }
                        else
                            ROS_INFO("Failed to get to %s ...",strGoto.c_str() );
                    }
                    
                }
                else
                {
                    ROS_ERROR("Failed to call service GetWaypointByName");
                }
             //找人 旋转180度 等会写
	    Speak("Please help me carry thing.");
	    nState=STATE_GOBACK;	
	}
        if(nState == STATE_GOBACK)
	{ 
               string strGoto = strKeyword;
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
                            sleep(3); 
							
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

    return 0;
}
*/



#include <ros/ros.h>
#include <vector>
#include <std_msgs/String.h>
#include <sound_play/SoundRequest.h>
#include "xfyun_waterplus/IATSwitch.h"
#include "wpb_home_tutorials/Follow.h"
#include <waterplus_map_tools/GetWaypointByName.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include "action_manager.h"
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/Pose2D.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/objdetect/objdetect.hpp"
#include <visualization_msgs/Marker.h>
#include <cv_bridge/cv_bridge.h>
#include <tf/transform_listener.h>

//有限状态机的三个状态
#define STATE_READY     0   //有限状态机里的准备状态
#define STATE_FOLLOW    1   //有限状态机里的跟随状态
#define STATE_WAIT      2   //有限状态机里的等待状态
#define STATE_PAUSE     3   //有限状态机里的不计时等待状态
#define STATE_GO        4   
#define STATE_IAT       5
#define STATE_FIND      6   
#define STATE_GOBACK    7
#define STATE_TURN      8
#define STATE_CROWD     9
#define STATE_DETECT    10
#define STATE_REPORT    11
#define STATE_COUNTDOWN 12
#define STATE_OPERATOR  13

using namespace cv;

//人脸检测
static std::string rgb_topic;
static std::string pc_topic;
static std::string face_cascade_name;
static CascadeClassifier face_cascade;

static Mat frame_gray;
static ros::Publisher image_pub;
static std::vector<Rect> faces;
static std::vector<cv::Rect>::const_iterator face_iter;
static ros::Publisher ctrl_pub;
static std_msgs::String ctrl_msg;
static geometry_msgs::Pose2D pose_diff;

static ros::Publisher pc_pub;
static tf::TransformListener *tf_listener; 
static ros::Publisher marker_pub;
static visualization_msgs::Marker line_face;
static visualization_msgs::Marker pos_face;
static visualization_msgs::Marker text_marker;
static ros::Publisher vel_pub;

//订阅主题和服务的变量和结构体
typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;
static ros::Publisher spk_pub;
static ros::ServiceClient follow_start;
static ros::ServiceClient follow_stop;
static ros::ServiceClient follow_resume;
static ros::ServiceClient clientIAT;
static waterplus_map_tools::GetWaypointByName srvName;
static ros::ServiceClient cliGetWPName;
static xfyun_waterplus::IATSwitch srvIAT;
static ros::Publisher add_waypoint_pub;
static int nState = STATE_READY;    //有限状态机的初始状态
static int nWaitCnt = 10;           //倒计时时间
static vector<string> arKWPlacement; //地点关键词
static std::string strKeyword;  //stop位置
//语音
static std::string strSpeak;
static std::string keyPlace;
static int nCountDown = 300;
static int nTurnCount = 6;
static int dd = 0; 


//语音说话函数,参数为说话内容字符串
static void Speak(std::string inStr)
{
    //sound_play::SoundRequest sp;
    //sp.sound = sound_play::SoundRequest::SAY;
    //sp.command = sound_play::SoundRequest::PLAY_ONCE;
   // sp.arg = inStr;
    spk_pub.publish(inStr);
}
static void Init_keywords()
{
 arKWPlacement.push_back("kitchen");
 arKWPlacement.push_back("bedroom");
 arKWPlacement.push_back("客厅");
 arKWPlacement.push_back("help");
}


static std::string FindWord(std::string inSentence, vector<std::string> & arWord)
{
    std::string strRes = "";
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
//进行语音识别
void KeyWordCB(const std_msgs::String::ConstPtr & msg)
{
    if(nState == STATE_IAT)
	{
	srvIAT.request.active = true;
        srvIAT.request.duration = 10;
        clientIAT.call(srvIAT);
	    strSpeak = msg->data;
	    keyPlace = FindWord(strSpeak,arKWPlacement);
        if (keyPlace.length() == 0 )
        {
            ;
        }
        else
        {
            nState = STATE_GO;
        }        
        ROS_INFO("[keyplace] = %s", keyPlace.c_str());
	}
}
// 将机器人当前位置保存为新航点
void AddNewWaypoint(std::string inStr)
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

//语音识别结果的回调函数
void KeywordCB(const std_msgs::String::ConstPtr & msg)
{
    //ROS_WARN("[KeywordCB] - %s",msg->data.c_str());
    int nFindIndex = 0;
    nFindIndex = msg->data.find("跟随我");   //开始跟随的语音指令关键词,可以替换成其他关键词
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

    nFindIndex = msg->data.find("车来了"); //等10秒的语音指令关键词,可以替换成其他关键词
    if( nFindIndex >= 0 )
    {
        //ROS_WARN("[KeywordCB] - 停止跟随");
		strKeyword = "stop";
		AddNewWaypoint(strKeyword);//添加车子位置
        Speak("OK, I will stay here for 10 seconds.'");    //语音回应发令者
        wpb_home_tutorials::Follow srv_stop;
        if (follow_stop.call(srv_stop))                 //调用启智ROS的跟随停止服务
        {
            ROS_WARN("[KeywordCB] - stop following!");          //调用服务成功
            sleep(4);
           //nWaitCnt = 10;
           // nState = STATE_WAIT;          //调用成功了,改变状态机的状态值到停止等待状态
	nState = STATE_IAT;  
        }
        else
        {
            ROS_WARN("[KeywordCB] - failed to stop following...");   //调用服务失败
        }
    }

    nFindIndex = msg->data.find("Wait"); //不计时等待的语音指令关键词,可以替换成其他关键词
    if( nFindIndex >= 0 )
    {
         Speak("OK, I will wait here for next command.'");    //语音回应发令者
        wpb_home_tutorials::Follow srv_stop;
        if (follow_stop.call(srv_stop))                 //调用启智ROS的跟随停止服务
        {
            ROS_WARN("[KeywordCB] - Wait!");          //调用服务成功
            nState = STATE_PAUSE;          //调用成功了,改变状态机的状态值到无限等待状态
        }
        else
        {
            ROS_WARN("[KeywordCB] - failed to Wait...");   //调用服务失败
        }
    }
}

//人脸检测 GOBACK
//框选出人脸
cv::Mat drawFacesRGB( cv::Mat inImage) 
{
    std::vector<cv::Rect>::const_iterator i;
   // sp.arg = inStr;
    //spk_pub.publish(inStr);
    for (face_iter = faces.begin(); face_iter != faces.end(); ++face_iter) 
    {
        cv::rectangle(
            inImage,
            cv::Point(face_iter->x , face_iter->y),
            cv::Point(face_iter->x + face_iter->width, face_iter->y + face_iter->height),
            CV_RGB(255, 0 , 255),
            10);
    }
    return inImage;
}

void DrawTextMarker(const std::string inText, int inID, float inScale, float inX, float inY, float inZ, float inR, float inG, float inB)
{
    text_marker.header.frame_id = "base_footprint";
    text_marker.ns = "line_obj";
    text_marker.action = visualization_msgs::Marker::ADD;
    text_marker.id = inID;
    text_marker.type = visualization_msgs::Marker::TEXT_VIEW_FACING;
    text_marker.scale.z = inScale;
    text_marker.color.r = inR;
    text_marker.color.g = inG;
    text_marker.color.b = inB;
    text_marker.color.a = 1.0;

    text_marker.pose.position.x = inX;
    text_marker.pose.position.y = inY;
    text_marker.pose.position.z = inZ;
    
    text_marker.pose.orientation=tf::createQuaternionMsgFromYaw(1.0);

    text_marker.text = inText;

    marker_pub.publish(text_marker);
}

void callbackRGB(const sensor_msgs::ImageConstPtr& msg)
{
    if(nState != STATE_OPERATOR && nState != STATE_CROWD)
    {
        return;
    }
    //ROS_INFO("callbackRGB");
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
    // change contrast: 0.5 = half  ; 2.0 = double
    cv_ptr->image.convertTo(frame_gray, -1, 1.5, 0);

    // create B&W image
    cvtColor( frame_gray, frame_gray, CV_BGR2GRAY );

	equalizeHist( frame_gray, frame_gray );
    //-- Detect faces
	face_cascade.detectMultiScale( frame_gray, faces, 1.1, 9, 0|CV_HAAR_SCALE_IMAGE, Size(30, 30) );

    //ROS_INFO("face = %d",faces.size());
    if(faces.size() > 0)
    {
        cv_ptr->image = drawFacesRGB(cv_ptr->image);
    }
    imshow("frame",cv_ptr->image);
    image_pub.publish(cv_ptr->toImageMsg());
    ROS_INFO("检测人脸中:%d", dd ++);

    int nNumFace = faces.size();
    if(nNumFace > 0)
    {
       /* if( nState == STATE_OPERATOR )
        {
            imwrite("/home/robot/operator.jpg",cv_ptr->image);
            ROS_INFO("Save the image of operator!!");
            ROS_INFO("Ready to turn!");
            //Speak("OK,I have memoried you.Please go to the crowd.I will find you out.");
            usleep(7*1000*1000);
            nState = STATE_COUNTDOWN;
        }
        if(nState == STATE_CROWD)
        {
            imwrite("/home/robot/crowd.jpg",cv_ptr->image);
            ROS_INFO("Save the image of crowd!!");
            //Speak("OK,I have recognize the faces of crowd.");
            nState = STATE_REPORT;
        }*/
        nState = STATE_GOBACK;
    }
}

void callbackPointCloud(const sensor_msgs::PointCloud2 &input)
{
    if(nState != STATE_OPERATOR && nState != STATE_CROWD)
    {
        return;
    }
    //to footprint
    sensor_msgs::PointCloud2 pc_footprint;
    bool res = tf_listener->waitForTransform("/base_footprint", input.header.frame_id, input.header.stamp, ros::Duration(5.0)); 
    if(res == false)
    {
        return;
    }
    pcl_ros::transformPointCloud("/base_footprint", input, pc_footprint, *tf_listener);

    //source cloud
    pcl::PointCloud<pcl::PointXYZRGB> cloud_src;
    pcl::fromROSMsg(pc_footprint , cloud_src);
    //ROS_WARN("cloud_src size = %d  width = %d",cloud_src.size(),input.width); 

    ////////////////////////////////
    // Draw Face Boxes
    line_face.points.clear();
    line_face.header.frame_id = "base_footprint";
    line_face.ns = "line_face";
    line_face.action = visualization_msgs::Marker::ADD;
    line_face.id = 1;
    line_face.type = visualization_msgs::Marker::LINE_LIST;
    line_face.scale.x = 0.01;
    line_face.color.r = 1.0;
    line_face.color.g = 0;
    line_face.color.b = 1.0;
    line_face.color.a = 1.0;

    pos_face.points.clear();
    pos_face.header.frame_id = "base_footprint";
    pos_face.ns = "pos_face";
    pos_face.action = visualization_msgs::Marker::ADD;
    pos_face.id = 1;
    pos_face.type = visualization_msgs::Marker::CUBE_LIST;
    pos_face.scale.x = 0.5;
    pos_face.scale.y = 0.5;
    pos_face.scale.z = 0.001;
    pos_face.color.r = 1.0;
    pos_face.color.g = 0;
    pos_face.color.b = 1.0;
    pos_face.color.a = 1.0;

    geometry_msgs::Point p;
    int nFaceIndex = 1;
    std::vector<cv::Rect>::const_iterator i;
    for (face_iter = faces.begin(); face_iter != faces.end(); ++face_iter) 
    {
        int rgb_face_x = face_iter->x  + face_iter->width/2;
        int rgb_face_y = face_iter->y + face_iter->height/2;
        int index_pc = rgb_face_y*input.width + rgb_face_x;
        float face_x = cloud_src.points[index_pc].x;
        float face_y = cloud_src.points[index_pc].y;
        float face_z = cloud_src.points[index_pc].z;
       
        p.x = 0.2; p.y = 0; p.z = 1.37; line_face.points.push_back(p);
        //p.x = -0.1; p.y = 0; p.z = 1.25; line_face.points.push_back(p);
        p.x = face_x; p.y = face_y; p.z = face_z; line_face.points.push_back(p);
        p.z = 0;pos_face.points.push_back(p);

        std::ostringstream stringStream;
        stringStream << "Face_" << nFaceIndex;
        std::string face_id = stringStream.str();
        DrawTextMarker(face_id,nFaceIndex,0.1,face_x,face_y,face_z+0.2,0,1.0,0);
        nFaceIndex ++;

        ROS_WARN("face (%d,%d) - (%.2f %.2f %.2f)",rgb_face_x,rgb_face_y,face_x,face_y,face_z); 
    }
    marker_pub.publish(line_face);
    marker_pub.publish(pos_face);

    for(int y=0; y< 300; y++)
    {
        for(int x=0; x< 200; x++)
        {
            int index_pc = y*input.width + x;
            cloud_src.points[index_pc].r = 1.0f;
            cloud_src.points[index_pc].g = 0.0f;
            cloud_src.points[index_pc].b = 1.0f;
        }
    }
    sensor_msgs::PointCloud2 output;
    pcl::toROSMsg(cloud_src, output);
    output.header.frame_id = pc_footprint.header.frame_id;
    pc_pub.publish(output);

}

void PoseDiffCallback(const geometry_msgs::Pose2D::ConstPtr& msg)
{
    pose_diff.x = msg->x;
    pose_diff.y = msg->y;
    pose_diff.theta = msg->theta;
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "wpb_home_follow");  //程序初始化
	Init_keywords();
    ros::NodeHandle n;
    ros::Subscriber sub_sr = n.subscribe("/xfyun/iat", 10, KeywordCB);  //订阅讯飞语音识别结果主题
    ros::Subscriber sub__sr = n.subscribe("/xfyun/iat", 10, KeyWordCB);
    spk_pub = n.advertise<sound_play::SoundRequest>("/robotsound", 20); //开辟一个主题,用来语音发音
    
    clientIAT = n.serviceClient<xfyun_waterplus::IATSwitch>("xfyun_waterplus/IATSwitch");   //连接语音识别开关服务
    follow_start = n.serviceClient<wpb_home_tutorials::Follow>("wpb_home_follow/start");   //连接跟随开始的服务
    follow_stop = n.serviceClient<wpb_home_tutorials::Follow>("wpb_home_follow/stop");      //连接跟随停止的服务
    follow_resume = n.serviceClient<wpb_home_tutorials::Follow>("wpb_home_follow/resume");   //连接跟随继续的服务
    cliGetWPName = n.serviceClient<waterplus_map_tools::GetWaypointByName>("/waterplus/get_waypoint_name"); 	
    add_waypoint_pub = n.advertise<waterplus_map_tools::Waypoint>( "/waterplus/add_waypoint", 1);


//人脸检测
    ros::NodeHandle nh_param("~");
    nh_param.param<std::string>("rgb_topic", rgb_topic, "/kinect2/hd/image_color");
    nh_param.param<std::string>("topic", pc_topic, "/kinect2/hd/points");
    nh_param.param<std::string>("face_cascade_name", face_cascade_name, "haarcascade_frontalface_alt.xml");
    ros::Subscriber rgb_sub = n.subscribe(rgb_topic, 1 , callbackRGB);
    ros::Subscriber pc_sub = n.subscribe(pc_topic, 1 , callbackPointCloud);
    image_pub = n.advertise<sensor_msgs::Image>("/face/image", 2);
    marker_pub = n.advertise<visualization_msgs::Marker>("face_marker", 2);
    pc_pub = n.advertise<sensor_msgs::PointCloud2>("face_pointcloud",1);
    vel_pub = n.advertise<geometry_msgs::Twist>("/cmd_vel", 10);
    ctrl_pub = n.advertise<std_msgs::String>("/wpb_home/ctrl", 30);
    ros::Subscriber pose_diff_sub = n.subscribe("/wpb_home/pose_diff", 1, PoseDiffCallback);

    ROS_INFO("[main] wpb_home_follow");
    ros::Rate r(1);         //while函数的循环周期,这里为1Hz
    while(ros::ok())        //程序主循环
    {
        ros::spinOnce();        //短时间挂起,让回调函数得以调用
        r.sleep();          //控制循环周期的sleep函数,这里会暂停1秒(因为r在构造时参赛为1Hz)
        if(nState == STATE_WAIT)    //有限状态机里的等待状态
        {
            ROS_INFO("[main] waiting downcount ... %d ", nWaitCnt);     //在终端里显示倒计时数值
            std::ostringstream stringStream;            //后面四行是把倒计时数值转换成字符,然后机器人用语音念出来
            stringStream << nWaitCnt;
            std::string retStr = stringStream.str();
            Speak(retStr);
            nWaitCnt --;
            if(nWaitCnt <= 0)       //检查倒记时是否完毕
            {
                //倒计时完毕,继续进行跟随
                Speak("OK, Move on.'");
                wpb_home_tutorials::Follow srv_resume;
                if (follow_resume.call(srv_resume))     //调用跟随继续的服务
                {
                    ROS_WARN("[main] - continue!");
                    nWaitCnt = 0;
                    nState = STATE_FOLLOW;     //调用成功了,改变状态机的状态值到跟随状态
                }
                else
                {
                    ROS_WARN("[main] - failed to continue...");     //调用失败
                    nWaitCnt = 0;
                }
            }
        }
                if(nState==STATE_GO)
		        {
                     string strGoto = keyPlace;     //发令地点名称,请在地图里设置这个航点
                srvName.request.name = strGoto;
                ROS_INFO("[STATE_GO]strGoto = %s", strGoto.c_str());
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
                            sleep(3);
			  nState=STATE_FIND;  							
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
         if(nState == STATE_FIND)
	{// 去空房间
		std::string strGoto = arKWPlacement[3]; 
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
                            sleep(3); 
							
                        }
                        else
                            ROS_INFO("Failed to get to %s ...",strGoto.c_str() );
                    }
                    
                }
                else
                {
                    ROS_ERROR("Failed to call service GetWaypointByName");
                }
	   
	    nState=STATE_TURN;	
	}
//找人 旋转180度
        if(nState == STATE_TURN)
	{
	//ROS_INFO("[Turn] count=%d  z= %.2f",nTurnCount, pose_diff.theta);
            geometry_msgs::Twist vel_cmd;
            vel_cmd.linear.x = 0;
            vel_cmd.linear.y = 0;
            vel_cmd.linear.z = 0;
            vel_cmd.angular.x = 0;
            vel_cmd.angular.y = 0;
            vel_cmd.angular.z = 0.2;    //旋转速度,如果转身不理想,可以修改这个值
            nTurnCount --;
            if(pose_diff.theta >= 3.13)
            {
                vel_cmd.angular.z = 0;
                nCountDown ++;
                if(nCountDown > 100)
                {
                    nState = STATE_CROWD;
                }
            }
        vel_pub.publish(vel_cmd); 	
	
	}
	if(nState == STATE_CROWD)
	{
	    geometry_msgs::Twist vel_cmd;
            vel_cmd.linear.x = 0;
            vel_cmd.linear.y = 0;
            vel_cmd.linear.z = 0;
            vel_cmd.angular.x = 0;
            vel_cmd.angular.y = 0;
            vel_cmd.angular.z = 0;
            vel_pub.publish(vel_cmd);
	    //Speak("Please help me carry something.");
	    //nState = STATE_GOBACK;
            ros::spinOnce();
	}	

        if(nState == STATE_GOBACK)
	{ 
               std::string strGoto = strKeyword;
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
                            sleep(3); 
							
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

    return 0;
}
