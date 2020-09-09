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
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include "posedetect/posedetect.h"

using namespace std; 
using namespace cv;



#define STATE_IAT         0
#define STATE_DETECT      1
#define STATE_END         2

static int nState = STATE_IAT;

//检测的服务
static ros::ServiceClient pose_detect;


static ros::Publisher spk_pub;
static std_msgs::String strSpeak;
static xfyun_waterplus::IATSwitch srvIAT;
static ros::ServiceClient clientIAT;


void KeywordCB(const std_msgs::String::ConstPtr & msg)
{
    ROS_INFO("KeywordCB");
    if (nState == STATE_IAT)
    {
        //string strListen = msg->data;
	    ROS_WARN("[KeywordCB] - %s",msg->data.c_str());
	    int nFindIndex = 1;
	    ROS_INFO("======%s======",msg->data.c_str());
	    nFindIndex = msg->data.find("开始");
            if(nFindIndex >= 0 )

            {
                srvIAT.request.active = false;
                clientIAT.call(srvIAT);
               
	            nState = STATE_DETECT;
            }
	    //关闭语音识别
	    
    }
    else
    {
        return;
	}
	ROS_INFO("[KeywordCB]nState = %d",nState);
}



void ProcColorCB(const sensor_msgs::ImageConstPtr& msg)
{
    ros::NodeHandle nh;
    //ROS_INFO("ProcColorCB");
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
        ROS_INFO("ProcColorCB");
		ROS_INFO("saving the pic");
		string filename = format("/home/robot/photos/sample.png");
		imwrite(filename, cv_ptr->image);
		posedetect::posedetect detect;
		detect.request.begin = 1;
		if (pose_detect.call(detect))
		{
			// 注意我们的response部分中的内容只包含一个变量response，另，注意将其转变成字符串
			if (detect.response.end == 1)
			{
			    ROS_INFO("[肢体检测]检测到了人脸");
			}
			else
			{
			    ROS_WARN("[肢体检测]没有检测到人脸");
			}
		}
        else
        {
            ROS_ERROR("Failed to call service PoseDetect");
        }
        nState = STATE_END;
    }
}


int main(int argc, char** argv)
{
    ros::init(argc, argv, "wpb_home_path_plan");
    
    ros::NodeHandle n;
    //检测的服务创建
    pose_detect = n.serviceClient<posedetect::posedetect>("posedetect");
    
    //ros::Subscriber pc_sub = n.subscribe("/kinect2/hd/points", 1 , ProcCloudCB);
    ros::Subscriber rgb_sub = n.subscribe("/kinect2/hd/image_color",1 ,ProcColorCB);
    //ros::Subscriber rgb_sub = n.subscribe("webcam/image_raw",1 ,ProcColorCB);
    //语音与识别
    ros::Subscriber sub_sr = n.subscribe("/xfyun/iat", 10, KeywordCB);
    spk_pub = n.advertise<std_msgs::String>("/xfyun/tts", 10);
    clientIAT = n.serviceClient<xfyun_waterplus::IATSwitch>("xfyun_waterplus/IATSwitch");
    
    srvIAT.request.active = true;
    srvIAT.request.duration = 8;
    clientIAT.call(srvIAT);
   	
    ROS_INFO("[main] wpb_home_path_plan");
    ros::Rate r(10);
    while(ros::ok())
    {
        ros::spinOnce();
        r.sleep();
    }

    return 0;
}

















