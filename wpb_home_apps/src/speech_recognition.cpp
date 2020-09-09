/*********************************************************************
* Software License Agreement (BSD License)
* 
*  Copyright (c) 2017-2020, Waterplus http://www.6-robot.com
*  All rights reserved.
* 
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
* 
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of the WaterPlus nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
* 
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  FOOTPRINTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*********************************************************************/
/* @author Zhang Wanjie                                             */
#include <ros/ros.h>
#include <std_msgs/String.h>
#include <geometry_msgs/Twist.h>
#include <sound_play/SoundRequest.h>

using namespace std;

#define STATE_SR    0
#define STATE_SPEAK 1

static ros::Publisher vel_pub;
static ros::Publisher spk_pub;
static int nState = STATE_SR;
static int nSpeakCount = 0;

//语音
static std_msgs::String strSpeak;
//语音说话函数,参数为说话内容字符串
static void Speak(std::string inStr)
{
    strSpeak.data = inStr;
    spk_pub.publish(strSpeak);
}


void KeywordCB(const std_msgs::String::ConstPtr & msg)
{
    if(nState != STATE_SR)
    {
        return;
    }
    //ROS_WARN("[KeywordCB] - %s",msg->data.c_str());
    string strSpeak;
    int nFindIndex = 0;
    nFindIndex = msg->data.find("capital of china");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Beijing ";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
    nFindIndex = msg->data.find("capital of japan");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Tokyo ";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
    nFindIndex = msg->data.find("most handsome");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "I think Justin Trudeau is very handsome";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("many time");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Canada spans almost 10 million square km and comprises 6 time zones";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("in the world");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Yonge Street in Ontario is the longest street in the world";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("smartphone");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "It was developed in Ontario, at Research In Motion's Waterloo offices";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("largest coin");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "The Big Nickel in Sudbury, Ontario. It is nine meters in diameter";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("the first time");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "The first time that the USA invaded Canada was in 1775";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("the second time");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "The USA invaded Canada a second time in 1812";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("the winter");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Canada does! With 14 Golds at the 2010 Vancouver Winter Olympics";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("police formed");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "The Mounted Police was formed in 1873";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("the royal canadian");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "In 1920, when The Mounted Police merged with the Dominion Police";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("is canada's only desert");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "The British Columbia desert is only 15 miles long";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("nanobot");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "The smallest robot possible is called a nanobot";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("small");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "A nanobot can be less than one-thousandth of a millimeter";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("the first computer");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "The IBM 305 RAMAC";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("hard disk");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "The IBM 305 RAMAC was launched in 1956";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("big was");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "The IBM 305 RAMAC hard disk weighed over a ton and stored 5 MB of data";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("computer bug");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "The first actual computer bug was a dead moth stuck in a Harvard Mark II";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("mechanical knight");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "A robot sketch made by Leonardo DaVinci";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("knowledge engineering");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "t is when you need to load an AI with enough knowledge to start learning";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("is a chatbot");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "A chatbot is an A.I. you put in customer service to avoid paying salaries";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("cars safe");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Yes. Car accidents are product of human misconduct";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("invented the");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Grace Hoper. She wrote it in her spare time";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("programming language");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "C was invented by Dennis MacAlistair Ritchie";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("python");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Python was invented by Guido van Rossum";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("a robot");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Sure. I've never seen him drink water";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("of the apple");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "My lord and master Steve Wozniak";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("considered to");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Ada Lovelace";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("program do");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Adobe Wan Kenobi";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("the shelf");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "The shelf is in the kitchen";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("the plant");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "The plant is in the living room";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("in the dining room");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "There is no chair in the dining room";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("smallest food");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "The bread is the smallest in the food category";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("lightest drink");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "The Coke Zero, is lighter than water";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("is today");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Today is Friday";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("which year");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "RoboCup@Home was founded in 2006";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("the sofa");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Near the table";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("the chair");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Near the tv table";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("the tv table");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Near the sofa";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("the table");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Between the chairs";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("the drink");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "In the kitchen";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("the food");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "In the dining room";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("the bed");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "In the bedroom";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("do you get");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Pink";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("on the flag");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Four stars";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("is named after");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Venezuela";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("human bone");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Clavicle";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("largest number");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "99999";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("the Great Wall");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "6259 kilometers";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("nearest the sun");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Mercury";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("three primary");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Blue, yellow and red";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("lightest existing");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Aluminium";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("invented by");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "The typewriter";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("steam engine");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "James Watt";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("in space");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Alan Shepard";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("hot air");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Montgolfier";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("invented the");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Telescope";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("the food");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Goodyear";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("the light");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Blue";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("we use to");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "The northern hemisphere";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("rubber");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Yuri Gagarin";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("the sound");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Kbps";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }



nFindIndex = msg->data.find("domain of");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Whoopi Goldberg";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("the leading");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Random Access Memory";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("the name");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Captain Picard";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("best James Bond");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Austin Powers";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("around the earth");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Fifty-three";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("the sound");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Quentin Tarantino";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("the leading");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Pekinese";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("number of");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Number 742";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("she was born");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Six toes";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("the tiger");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Siberian tiger";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("flag contain");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Maple";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("fish with");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Eel fish";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("name of");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Wisent";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("jump higher");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Fleas";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("prefer to");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Mosquitoes";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("another name");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Paleontology";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("the food");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Melbourne";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("name of Paris");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Lutetia";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("name of");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "New Amsterdam";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("first king");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Leopold I";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("the spaceship");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Alien";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("the northern");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Hadrians wall";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("was called");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Louis XIV";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("many children");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Nine children";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("which city");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Belfast";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("for the Emperor");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Pork";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("first president");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "George Washington";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
nFindIndex = msg->data.find("count invented");
    if( nFindIndex >= 0 )
    {
        strSpeak = "The quetion is " + msg->data;
        strSpeak += ".The answer is ";
        strSpeak += "Count von Zeppelin";
        Speak(strSpeak);
        nSpeakCount = 10;
        nState = STATE_SPEAK;
    }
    ROS_INFO("[QA] - %s",strSpeak.c_str());
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "speech_recognition");

    ros::NodeHandle n;
    ros::Subscriber sub_sr = n.subscribe("/xfyun/iat", 10, KeywordCB);
    spk_pub = n.advertise<std_msgs::String>("/xfyun/tts", 10);
    vel_pub = n.advertise<geometry_msgs::Twist>("/cmd_vel", 10);

    ROS_INFO("[main] speech_recognition");
    ros::Rate r(1);
    while(ros::ok())
    {
        if(nState == STATE_SPEAK)
        {
            //在STATE_SPEAK期间,语音识别关键词会被忽略,以避免误识别
            nSpeakCount --;
            if(nSpeakCount <= 0)
            {
                nState = STATE_SR;
                ROS_INFO("[main] back to speech recognition");
            }
        }
        ros::spinOnce();
        r.sleep();
    }

    return 0;
}
