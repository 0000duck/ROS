// RCSInterpreter.cpp

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


#include <algorithm>  // max
#include <tf/transform_datatypes.h>
#include <boost/ref.hpp>

#include "RCSInterpreter.h"
#include "Controller.h"
#include "Globals.h"
#include "trajectoryMaker.h"
#include "Conversions.h"
#include "Debug.h"
using namespace RCS;
using namespace sensor_msgs;


/**
    uint8 crclcommand

    # https://github.com/ros/common_msgs 
    geometry_msgs/Pose  finalpose
    geometry_msgs/Pose[] waypoints
    # Below joint info could be  trajectory_msgs/JointTrajectoryPoint
    sensor_msgs/JointState joints
    bool bStraight
    float64   dwell_seconds
    string opmessage
    bool bCoordinated
    float64 eepercent
    CrclMaxProfileMsg[] profile # maximum profile 
 */
BangBangInterpreter::BangBangInterpreter(boost::shared_ptr<RCS::CController> nc, IKinematicsSharedPtr k ):
_nc(nc), _kinematics(k) 
{
}

void BangBangInterpreter::SetRange(std::vector<double> minrange, std::vector<double> maxrange) {
    this->minrange = minrange;
    this->maxrange = maxrange;
}

RCS::CanonCmd BangBangInterpreter::ParseCommand(RCS::CanonCmd cmd) {

    if (cmd.crclcommand == CanonCmdType::CANON_MOVE_JOINT) {
        // Move immediately to joint value - Fill in other joints with current values
        cmd.joints = _kinematics->UpdateJointState(cmd.jointnum, _nc->status.currentjoints, cmd.joints);
    } else if (cmd.crclcommand == CanonCmdType::CANON_MOVE_TO) {
        // FIXME: need to subtract off tool offset from tcp
        RCS::Pose finalpose = Conversion::GeomMsgPose2RcsPose(cmd.finalpose);
        //RCS::Pose goalpose =  Cnc.invGripperPose() * finalpose;
        RCS::Pose goalpose =  finalpose * _nc->invGripperPose() ;
        LOG_DEBUG << "Final Pose " << RCS::DumpPoseSimple(finalpose).c_str();
        LOG_DEBUG << "Minus Gripper Pose " << RCS::DumpPoseSimple(goalpose).c_str();

        if (cmd.hint.size() ==0){
            //cmd.joints.position = Cnc.Kinematics()->IK(goalpose, cmd.ConfigMin(), cmd.ConfigMax());
            cmd.joints.position = _kinematics->IK(goalpose, Subset(_nc->status.currentjoints.position, 6));
        }
        else
        {
            LOG_DEBUG << "Pose Hint " << VectorDump<double>(  cmd.hint).c_str();
            // KDL needs a hint or algorithm won't converge
            // http://www.orocos.org/forum/orocos/orocos-users/kdl-ik-position-solver
            cmd.joints.position = _kinematics->IK(goalpose, cmd.hint);
        }
           
        assert(cmd.joints.position.size() > 0);
        LOG_DEBUG << "KDL IK Joints " << VectorDump<double>(  cmd.joints.position).c_str();
        cmd.joints.name = _kinematics->JointNames();
        cmd.crclcommand = CanonCmdType::CANON_MOVE_JOINT;
    }

   _nc->robotcmds.AddMsgQueue(cmd);

    return cmd;
}
