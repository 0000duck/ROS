

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
#include <actionlib/client/simple_action_client.h>
#include <control_msgs/GripperCommandAction.h>

int main (int argc, char **argv)
{
	ros::init(argc, argv, "test_client_gripper_action_server");

	ros::NodeHandle pnh("~");

	std::string gripper_name;
	pnh.param<std::string>("gripper_name", gripper_name, "gripper");

	// Define the action client (true: we want to spin a thread)
	actionlib::SimpleActionClient< control_msgs::GripperCommandAction > ac(gripper_name , true);

	// Wait for the action server to come up
	while(!ac.waitForServer(ros::Duration(5.0))) {
		ROS_INFO("Waiting for turn action server to come up");
	}

	// Set the goal
	control_msgs::GripperCommandGoal goal;
	goal.command.position = 0.0;
	goal.command.max_effort = 100.0;
	
	// Send the goal
	ac.sendGoal(goal);
	return 0;
}