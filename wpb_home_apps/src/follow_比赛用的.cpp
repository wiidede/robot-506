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
 arKWPlacement.push_back("living room");
 arKWPlacement.push_back("bedroom");
 arKWPlacement.push_back("dining room");
 
// arKWPlacement.push_back("help");
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
    nFindIndex = msg->data.find("跟");   //开始跟随的语音指令关键词,可以替换成其他关键词
    if( nFindIndex >= 0 )
    {
        //从语音识别结果中提取到了"Follow me"字符串,说明已经接收到跟随开始的语音指令
        //ROS_WARN("[KeywordCB] - 开始跟随");
        Speak("OK, Let's go.'");    //语音回应发令者
        wpb_home_tutorials::Follow srv_start;
        srv_start.request.thredhold = 0.8;  //目标跟随的距离值,单位为米
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

    nFindIndex = msg->data.find("停"); //等10秒的语音指令关键词,可以替换成其他关键词
    if( nFindIndex >= 0 )
    {
        //ROS_WARN("[KeywordCB] - 停止跟随");
		strKeyword = "stop";
		AddNewWaypoint(strKeyword);//添加车子位置
       // Speak("OK, I will stay here for 10 seconds.'");   
 //语音回应发令者
        wpb_home_tutorials::Follow srv_stop;
        if (follow_stop.call(srv_stop))                 //调用启智ROS的跟随停止服务
        {
            ROS_WARN("[KeywordCB] - stop following!");          //调用服务成功
            sleep(4);
           //nWaitCnt = 10;
           // nState = STATE_WAIT;          //调用成功了,改变状态机的状态值到停止等待状态
	nState = STATE_IAT;  
sleep(3);
	Speak("Where do i need to send these things?");
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
    //spk_pub = n.advertise<sound_play::SoundRequest>("/robotsound", 20); //开辟一个主题,用来语音发音
    spk_pub = n.advertise<std_msgs::String>("/xfyun/tts", 10);
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
//修改寻找帮助的地方
		/*string strGoto = arKWPlacement[3]; 
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
                }*/
             //找人 旋转180度 等会写
	    Speak("Please help me carry something and follow me");
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
