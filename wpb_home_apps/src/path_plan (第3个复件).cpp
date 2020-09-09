#include <ros/ros.h>
#include <std_msgs/String.h>
#include <vector>
#include "action_manager.h"
#include <sound_play/SoundRequest.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include <waterplus_map_tools/GetWaypointByName.h>
#include <pcl_ros/point_cloud.h>
#include <pcl/point_types.h>
#include <boost/foreach.hpp>
#include <pcl/io/pcd_io.h>
#include <pcl/sample_consensus/method_types.h>
#include <pcl/sample_consensus/model_types.h>
#include <pcl/segmentation/sac_segmentation.h>
#include <pcl/filters/extract_indices.h>
#include <pcl/surface/convex_hull.h>
#include <pcl/segmentation/extract_polygonal_prism_data.h>
#include <pcl/visualization/cloud_viewer.h>
#include <pcl/segmentation/extract_clusters.h>
#include <image_geometry/pinhole_camera_model.h>
#include <cv_bridge/cv_bridge.h>
#include <image_transport/image_transport.h>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <sensor_msgs/Image.h>
#include <pcl_ros/transforms.h>
#include <sensor_msgs/PointCloud2.h>
#include <pcl_conversions/pcl_conversions.h>
#include <geometry_msgs/PointStamped.h>
#include <tf/transform_listener.h>
#include <visualization_msgs/Marker.h>
#include "highgui.h"
#include <stdlib.h>
#include <stdio.h>    
#include <math.h>
#include <iostream>
#include "xfyun_waterplus/IATSwitch.h"
#include"opencv2/opencv.hpp"
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/core/core.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include<opencv2/opencv.hpp>
# include "posedetect/posedetect.h"
using namespace std;
using namespace cv;

Mat g_srcImage, g_tempalteImage, g_resultImage;
int g_nMatchMethod;
int g_nMaxTrackbarNum = 5;

//有限状态机
#define STATE_READY       0
#define STATE_WAIT_ENTR   1
#define STATE_GOTO        2
#define STATE_DONE        3
#define STATE_DO          4
#define STATE_DETECT      5
#define STATE_RECO        6						
#define STATE_DONE1       7

static int nState = STATE_WAIT_ENTR;  //程序启动时初始状态;
int rj=0;

typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;
static CActionManager action_manager;
static ros::ServiceClient cliGetWPName;
static waterplus_map_tools::GetWaypointByName srvName;


typedef struct stBoxMarker
{
    float xMax;
    float xMin;
    float yMax;
    float yMin;
    float zMax;
    float zMin;
}stBoxMarker;

static stBoxMarker boxMarker;

static std::string pc_topic;
static ros::Publisher pc_pub;
static ros::Publisher marker_pub;
static tf::TransformListener *tf_listener; 
void DrawBox(float inMinX, float inMaxX, float inMinY, float inMaxY, float inMinZ, float inMaxZ, float inR, float inG, float inB);
void DrawText(std::string inText, float inScale, float inX, float inY, float inZ, float inR, float inG, float inB);
void DrawPath(float inX, float inY, float inZ);
void RemoveBoxes();
static visualization_msgs::Marker line_box;
static visualization_msgs::Marker line_follow;
static visualization_msgs::Marker text_marker;

typedef pcl::PointCloud<pcl::PointXYZRGB> PointCloud;
static ros::Publisher segmented_objects;
static ros::Publisher segmented_plane;
static ros::Publisher clustering0;
static ros::Publisher clustering1;
static ros::Publisher clustering2;
static ros::Publisher clustering3;
static ros::Publisher clustering4;
static ros::Publisher masking;
static ros::Publisher color;
static ros::Publisher image_pub;
static ros::Publisher spk_pub;
static ros::ServiceClient clientIAT;
static xfyun_waterplus::IATSwitch srvIAT;

static std::vector<Rect> arObj;
static string strListen;

cv::Mat rgb_image;
int img_counter = 0;
// 初始化航点遍历脚本 
static vector<string> arWaypoint;
static int nWaypointIndex = 0;
static void Init_waypoints()
{
    //arWaypoint.push_back("1");
    arWaypoint.push_back("2");
    arWaypoint.push_back("3");
   // arWaypoint.push_back("exit");
}

static void Speak(string inStr)
{
    sound_play::SoundRequest sp;
    sp.sound = sound_play::SoundRequest::SAY;
    sp.command = sound_play::SoundRequest::PLAY_ONCE;
    sp.arg = inStr;
    spk_pub.publish(sp);
}
void KeywordCB(const std_msgs::String::ConstPtr & msg)
{
	string strListen = msg->data;
}

static int nOpenCount = 0;
void EntranceCB(const std_msgs::String::ConstPtr & msg)
{
    //ROS_WARN("[EntranceCB] - %s",msg->data.c_str());
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

void ProcColorCB(const sensor_msgs::ImageConstPtr& msg)
{
	ros::NodeHandle nh;
    ROS_INFO("ProcColorCB");
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

    if(nState == STATE_DETECT)
    { 

ROS_INFO("6");
rj ++;
string filename = format("/home/robot/photos/%d.png", rj);
imwrite(filename, cv_ptr->image);
//find_star();作检测
//重复订单 和 饮料
/*const std_msgs::String::ConstPtr & msg；*/
 //spk_pub = nh.advertise<sound_play::SoundRequest>("/robotsound", 20);
Speak("Can I help you?");
clientIAT = nh.serviceClient<xfyun_waterplus::IATSwitch>("xfyun_waterplus/IATSwitch");
  clientIAT.call(srvIAT);
Speak(strListen);

//动作检测
	ros::ServiceClient client = nh.serviceClient<posedetect::posedetect>("posedetect");
	posedetect::posedetect srv;
	srv.request.begin = 1;
	if (client.call(srv))
	{
		// 注意我们的response部分中的内容只包含一个变量response，另，注意将其转变成字符串
		ROS_INFO("Response from server: %d", srv.response.end);
	}
	else
	{
		ROS_ERROR("Failed to call service Service_demo");
		return ;
	}
//动作检测结束

    nState = STATE_DONE1;
           // Speak("I have remembered it.");
            //bRemebered = true;
        
    }
}
static int nTextNum = 2;
int main(int argc, char** argv)
{
    ros::init(argc, argv, "wpb_home_path_plan");
    tf_listener = new tf::TransformListener(); 
    Init_waypoints();
    action_manager.Init();

    ros::NodeHandle n;
    ros::Subscriber sub_ent = n.subscribe("/wpb_home/entrance_detect", 10, EntranceCB);
    cliGetWPName = n.serviceClient<waterplus_map_tools::GetWaypointByName>("/waterplus/get_waypoint_name");
   
    //ros::Subscriber pc_sub = n.subscribe("/kinect2/hd/points", 1 , ProcCloudCB);
    ros::Subscriber rgb_sub = n.subscribe("/kinect2/hd/image_color",1 ,ProcColorCB);
    ros::Subscriber sub_sr = n.subscribe("/xfyun/iat", 10, KeywordCB);
    
  spk_pub = n.advertise<sound_play::SoundRequest>("/robotsound", 20);
    image_pub = n.advertise<sensor_msgs::Image>("/obj_reco/result", 2);
    string strGoto;
    ROS_INFO("[main] wpb_home_path_plan");
    ros::Rate r(10);
    while(ros::ok())
    {
        if(nState== STATE_WAIT_ENTR)
        {
            //等待开门,一旦检测到开门,便去往发令地点
            if(nOpenCount > 20)
            {
                strGoto = "start";     //start是场地内的起点,请在地图里设置这个航点
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
                            //到达"start"航点,开始执行航点遍历脚本
                          // Speak("I am ready.");
                            ros::spinOnce();
                       //     sleep(3);

                            nState = STATE_GOTO; 
                            strGoto = arWaypoint[nWaypointIndex];
                            nWaypointIndex ++;
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
		
        if(nState == STATE_GOTO)
        {
            // 正式的航点遍历脚本
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
                        //到达"start"航点,开始执行航点遍历脚本
                        string strSpeak = "I have got to waypoint " + strGoto;
                       // Speak(strSpeak);
						ROS_INFO("1");
                        ros::spinOnce();
	ROS_INFO("2");
                        sleep(3);
ROS_INFO("3");
                        nState= STATE_DO;
ROS_INFO("4");
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
        if(nState == STATE_DO)
        {
  
    nState = STATE_DETECT;
ROS_INFO("5");
}
if (nState ==STATE_DONE)
{//回酒保那并叙述订单

nState =STATE_DONE1;

}

 if(nState == STATE_DONE1)
{
  int nNumWayponts = arWaypoint.size();
                        if(nWaypointIndex < nNumWayponts)
                        {
                            // 航点未遍历完，继续下一个航点
                            strGoto = arWaypoint[nWaypointIndex];
                            nWaypointIndex ++;
                            nState = STATE_GOTO; 
                        }
                        else
                        {
                            // 航点遍历完成，结束
                            Speak("I am done.");
                            ros::spinOnce();
                           // nState = STATE_DONE;
                        }
                    }        
        ros::spinOnce();
        r.sleep();
    }

    return 0;
}
