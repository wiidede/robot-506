/******************
*name = 猜谜呀
*author = dd王个帅b
*******************/

#include <ros/ros.h>
#include <std_msgs/String.h>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/Pose2D.h>

using namespace std;

#define STATE_SPEAK       0
#define STATE_WAIT        1
#define STATE_TURN        2
#define STATE_END         3

static int nState = STATE_SPEAK;
static int cnt = 9;
static ros::Publisher spk_pub;
static ros::Publisher vel_pub;
static std_msgs::String strSpeak;

static void Speak(string inStr)
{
    strSpeak.data = inStr.c_str();
    ROS_INFO("[Speak] -- %s", inStr.c_str());
    spk_pub.publish(strSpeak);
}



int main(int argc, char** argv)
{
    ros::init(argc, argv, "riddle");
    
    ros::NodeHandle n;
    
    spk_pub = n.advertise<std_msgs::String>("/xfyun/tts", 10);
    vel_pub = n.advertise<geometry_msgs::Twist>("/cmd_vel", 10);
    
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
                cnt = 8;
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
            }
            //ROS_INFO("[Turn] count=%d  z= %.2f",nTurnCount, pose_diff.theta);
            else
            {
                nState = STATE_END;
                geometry_msgs::Twist vel_cmd;
                vel_cmd.linear.x = 0;
                vel_cmd.linear.y = 0;
                vel_cmd.linear.z = 0;
                vel_cmd.angular.x = 0;
                vel_cmd.angular.y = 0;
                vel_cmd.angular.z = 0;    //旋转速度,如果转身不理想,可以修改这个值
                vel_pub.publish(vel_cmd);
            }
        }
        ros::spinOnce();
    }
    
    return 0;
}
