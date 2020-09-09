/******************
*name = 猜谜呀
*author = dd王个帅b
*******************/

#include <ros/ros.h>
#include <std_msgs/String.h>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/Pose2D.h>
#include "opencv2/objdetect.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <sensor_msgs/Image.h>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/stitching.hpp>

using namespace std;
using namespace cv;
bool try_use_gpu = false;
vector<Mat> imgs;

#define STATE_SPEAK       0
#define STATE_WAIT        1
#define STATE_TURN        2
#define STATE_END         3
#define STATE_DETECT      4

static int nState = STATE_SPEAK;
static int cnt = 9;
static int flag = 0;
static ros::Publisher spk_pub;
static ros::Publisher vel_pub;
static std_msgs::String strSpeak;

String faceCascadePath;
CascadeClassifier faceCascade;

static void Speak(string inStr)
{
    strSpeak.data = inStr.c_str();
    ROS_INFO("[Speak] -- %s", inStr.c_str());
    spk_pub.publish(strSpeak);
}

int detectFaceOpenCVHaar(CascadeClassifier faceCascade, Mat &frameOpenCVHaar, int inHeight=300, int inWidth=0)
{
    int frameHeight = frameOpenCVHaar.rows;
    int frameWidth = frameOpenCVHaar.cols;
    if (!inWidth)
        inWidth = (int)((frameWidth / (float)frameHeight) * inHeight);

    float scaleHeight = frameHeight / (float)inHeight;
    float scaleWidth = frameWidth / (float)inWidth;

    Mat frameOpenCVHaarSmall, frameGray;
    resize(frameOpenCVHaar, frameOpenCVHaarSmall, Size(inWidth, inHeight));
    cvtColor(frameOpenCVHaarSmall, frameGray, COLOR_BGR2GRAY);

    std::vector<Rect> faces;
    faceCascade.detectMultiScale(frameGray, faces);
    


    for ( size_t i = 0; i < faces.size(); i++ )
    {
      int x1 = (int)(faces[i].x * scaleWidth);
      int y1 = (int)(faces[i].y * scaleHeight);
      int x2 = (int)((faces[i].x + faces[i].width) * scaleWidth);
      int y2 = (int)((faces[i].y + faces[i].height) * scaleHeight);
      rectangle(frameOpenCVHaar, Point(x1, y1), Point(x2, y2), Scalar(0,255,0), (int)(frameHeight/150.0), 4);
    }

    return faces.size();
}

void ProcColorCB(const sensor_msgs::ImageConstPtr& msg)
{
    //ROS_INFO("ProcColorCB %d", nState);
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
        imgs.push_back(cv_ptr->image);
        //string filename = format("/home/robot/photos/%d.png", rj);
        //imwrite(filename, cv_ptr->image);
        nState = STATE_TURN;
    
    }
    
}



int main(int argc, char** argv)
{
    ros::init(argc, argv, "riddle");
    
    ros::NodeHandle n;
    
    spk_pub = n.advertise<std_msgs::String>("/xfyun/tts", 10);
    vel_pub = n.advertise<geometry_msgs::Twist>("/cmd_vel", 10);
    ros::Subscriber rgb_sub = n.subscribe("/kinect2/qhd/image_color", 1 , ProcColorCB);
    
    faceCascadePath = "/opt/ros/kinetic/share/OpenCV-3.3.1-dev/haarcascades/haarcascade_frontalface_alt.xml";

    if( !faceCascade.load( faceCascadePath ) ){ printf("--(!)Error loading face cascade\n"); return -1; };
    Mat frame;

    double tt_opencvHaar = 0;
    double fpsOpencvHaar = 0;
    
    
    
    ros::Rate r(1);
    while(ros::ok())
    {
        if (nState == STATE_SPEAK)
        {
            //ros::spinOnce();
            //Speak("任智慧");
            ros::spinOnce();
            sleep(3);
            Speak("I want to play riddles");
            ros::spinOnce();
            sleep(3);
            nState = STATE_WAIT;
           
        }
        if (nState == STATE_WAIT)
        {
            if (cnt >= 0)
            {
                std::ostringstream stringStream;
                stringStream << cnt;
                std::string retStr = stringStream.str();
                Speak(retStr);
                ROS_INFO("[Countdown] - %s",retStr.c_str());
                cnt --;
                sleep(1);
            }
            else
            {
                nState = STATE_TURN;
                cnt = 5;
            }
        }
        if (nState == STATE_TURN)
        {
            if (cnt > 0)
            {
                geometry_msgs::Twist vel_cmd;
                vel_cmd.linear.x = 0;
                vel_cmd.linear.y = 0;
                vel_cmd.linear.z = 0;
                vel_cmd.angular.x = 0;
                vel_cmd.angular.y = 0;
                vel_cmd.angular.z = 0.4;    //旋转速度,如果转身不理想,可以修改这个值
                vel_pub.publish(vel_cmd);
                cnt --;
                sleep(2);
                ROS_INFO("[Turn] -- cnt = %d", cnt);
                nState = STATE_DETECT;
            }
            //ROS_INFO("[Turn] count=%d  z= %.2f",nTurnCount, pose_diff.theta);
            else
            {
                flag = 1;
                nState = STATE_END;
                geometry_msgs::Twist vel_cmd;
                vel_cmd.linear.x = 0;
                vel_cmd.linear.y = 0;
                vel_cmd.linear.z = 0;
                vel_cmd.angular.x = 0;
                vel_cmd.angular.y = 0;
                vel_cmd.angular.z = 0;    //旋转速度,如果转身不理想,可以修改这个值
                vel_pub.publish(vel_cmd);
                
                
                Stitcher stitcher = Stitcher::createDefault(try_use_gpu);
                // 使用stitch函数进行拼接
                Mat pano;
                Stitcher::Status status = stitcher.stitch(imgs, pano);
                // 显示源图像，和结果图像
                //imshow("全景图像", pano);n
                
                int count;
                count = detectFaceOpenCVHaar ( faceCascade, pano );
                imwrite("/home/robot/photos/res.png", pano);
                ROS_INFO("The number of people is %d  ...", count);
		ROS_INFO(" sit : 2" );
 		ROS_INFO(" stand : %d  ...", count-2);
            }
        }
        ros::spinOnce();
    }
    
    return 0;
}




/*#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/stitching.hpp>
//#include <opencv2/stitching/stitcher.hpp>
using namespace std;
using namespace cv;
bool try_use_gpu = false;
vector<Mat> imgs;
string result_name = "result.jpg";
int main(int argc, char * argv[])
{
    Mat img1 = imread("/home/robot/photos/1.jpg");
    Mat img2 = imread("/home/robot/photos/2.jpg");
    Mat img3 = imread("/home/robot/photos/3.jpg");    
    Mat img4 = imread("/home/robot/photos/4.jpg");    
    if (img1.empty() || img2.empty())
    {
        cout << "Can't read image"<< endl;
        return -1;
    }
    imgs.push_back(img1);
    imgs.push_back(img2);
    imgs.push_back(img3);
    imgs.push_back(img4);
    Stitcher stitcher = Stitcher::createDefault(try_use_gpu);
    // 使用stitch函数进行拼接
    Mat pano;
    Stitcher::Status status = stitcher.stitch(imgs, pano);
    imwrite(result_name, pano);
    Mat pano2=pano.clone();
    // 显示源图像，和结果图像
    //imshow("全景图像", pano);n
    imwrite("/home/robot/photos/res.png", pano);
    //if(waitKey()==27)
        return 0;
}*/
