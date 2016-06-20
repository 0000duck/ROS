
//
//
//
//
// gripperactionserver.cpp
//

/*
* DISCLAIMER:
* This software was produced by the National Institute of Standards
* and Technology (NIST), an agency of the U.S. government, and by statute is
* not subject to copyright in the United States.  Recipients of this software
* assume all responsibility associated with its operation, modification,
* maintenance, and subsequent redistribution.
*
* See NIST Administration Manual 4.09.07 b and Appendix I.
*/

#include <ros/ros.h>
#include <actionlib/server/simple_action_server.h>
#include <control_msgs/GripperCommandAction.h>
#include <boost/bind.hpp>
#include <string>

// Commplete voodoo

class AGripperActionServer {
public:
    typedef actionlib::ActionServer<control_msgs::GripperCommandAction>  ActionServer;
    typedef boost::shared_ptr<ActionServer>                                                     ActionServerPtr;
    typedef ActionServer::GoalHandle                                                            GoalHandle;
     typedef boost::shared_ptr<GoalHandle>                                                       GoalHandlePtr;
protected:
    ros::NodeHandle nh;
    std::string action_name;
    ActionServer * gripper_server;
public:
    AGripperActionServer (std::string name) ;
    void goalCB(GoalHandle gh) {
        double position_ = gh.getGoal()->command.position;
        double  max_effort_ = gh.getGoal()->command.max_effort;
        ROS_INFO("AGripperActionServer callback for gripper: %s Position=%f", action_name.c_str(), position_, max_effort_);
    }
    void cancelCB(GoalHandle gh) {
        //double position_ = goal->command.position;
        //double  max_effort_ = goal->command.max_effort;
        ROS_INFO("AGripperActionServer callback for gripper: %s", action_name.c_str());
    }
};

AGripperActionServer::AGripperActionServer (std::string name) : action_name(name)
{
    gripper_server=new ActionServer(nh, "gripper",
                                    boost::bind(&AGripperActionServer::goalCB,   this, _1),
                                    boost::bind(&AGripperActionServer::cancelCB,   this, _1),
                                    false);
                   gripper_server->start();
 }

int main (int argc, char **argv)
{
	ros::init(argc, argv, "test_gripper_action_server");

	ros::NodeHandle pnh("~");

	std::string gripper_name;
	pnh.param<std::string>("gripper_name", gripper_name, "gripper");

	// The name of the gripper -> this server communicates over name/inputs and name/outputs
 	AGripperActionServer gripper (gripper_name);

	ROS_INFO("NIST simple action-server spinning for gripper: %s", gripper_name.c_str());
	ros::spin();
}