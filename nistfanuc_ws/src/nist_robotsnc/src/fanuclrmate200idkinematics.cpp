

// DISCLAIMER:
// This software was developed by U.S. Government employees as part of
// their official duties and is not subject to copyright. No warranty implied 
// or intended.


#include "Kinematics.h"
#include <eigen_conversions/eigen_msg.h>
#include "RosConversions.h"
#include <iostream>
#include "Conversions.h"
#include "Globals.h"
#include "Debug.h"
#include <boost/format.hpp>
#include "Boost.h"
#include "nist_robotsnc/MotionException.h"
#include "nist_robotsnc/Controller.h"

using namespace RCS;

#define IKFAST_HAS_LIBRARY
#define IKFAST_NO_MAIN
#define IKFAST_NAMESPACE FanucLRMate200d

#include "Fanuc/ikfast.h"
#include <algorithm>
using namespace  ikfast;
using namespace FanucLRMate200d;


///////////////////////////////////////////////////////////////////////////////

static     void getClosestSolution(const IkSolutionList<double> &solutions, 
    const std::vector<double> &ik_seed_state, 
    std::vector<double> &solution)  ;
    
RCS::Pose FanucLRMate200idFastKinematics::FK(std::vector<double> joints) {
    // DO NOT Handle gearing of joints
    double eetrans[4];
    double eerot[9];

    // / Computes the end effector coordinates given the joint values using ikfast. This function is used to double check ik
    // Units? joint angles in radians or degree
    // Elsewhere: Returns the forward kinematic solution given the joint angles (in radians)
    ComputeFk(&joints[0], eetrans, eerot);

    tf::Pose pose;
    pose.getOrigin().setX(eetrans[0]);
    pose.getOrigin().setY(eetrans[1]);
    pose.getOrigin().setZ(eetrans[2]);
    //pose.setRotation(Convert2Rotation(eerot));
    tf::Matrix3x3 m(eerot[3 * 0 + 0], eerot[3 * 0 + 1], eerot[3 * 0 + 2],
            eerot[3 * 1 + 0], eerot[3 * 1 + 1], eerot[3 * 1 + 2],
    eerot[3 * 2 + 0], eerot[3 * 2 + 1], eerot[3 * 2 + 2]);
    pose.setBasis(m);
    return pose;
}
// http://docs.ros.org/jade/api/moveit_msgs/html/msg/PositionIKRequest.html
//http://docs.ros.org/hydro/api/ric_mc/html/GetPositionIK_8h_source.html

size_t FanucLRMate200idFastKinematics::AllIK(RCS::Pose & pose, std::vector<std::vector<double>> &joints) {

    // Inverse kinematics
    ikfast::IkSolutionList<double> solutions;

    // std::vector<double> vfree(GetNumFreeParameters());
    std::vector<double> vfree;

    double eetrans[3] = {pose.getOrigin().x(), pose.getOrigin().y(), pose.getOrigin().z()};
    double eerot[9];
     
 //    Convert2RotationMatrix(pose.getRotation(), eerot);
    tf::Matrix3x3 m3x3 = pose.getBasis ();
    eerot[3 * 0 + 0] = m3x3[0][0];
    eerot[3 * 0 + 1] = m3x3[0][1];
    eerot[3 * 0 + 2] = m3x3[0][2];
    eerot[3 * 1 + 0] = m3x3[1][0];
    eerot[3 * 1 + 1] = m3x3[1][1];
    eerot[3 * 1 + 2] = m3x3[1][2];
    eerot[3 * 2 + 0] = m3x3[2][0];
    eerot[3 * 2 + 1] = m3x3[2][1];
    eerot[3 * 2 + 2] = m3x3[2][2];  
    bool bSuccess = ComputeIk(eetrans, eerot, vfree.size() > 0 ? &vfree[0] : NULL, solutions);

    //TODO(Fix Singularity issue in FanucLRMate200idFastKinematics::AllPoseToJoints)
    if (!bSuccess) {
        LOG_DEBUG <<"Failed to get ik solution:"<<RCS::DumpPoseSimple(pose);
#if defined(IKFASTDEBUG)
        ofsIkFast <<"Failed to get ik solution:"<<RCS::DumpPoseSimple(pose)<<"\n";
#endif
        throw MotionException(10, _nc->Name().c_str());
        return -1;
    }
 
    // There are no redundant joints, so no free dof
    std::vector<double> solvalues(GetNumJoints());
#if defined(IKFASTDEBUG)
    ofsIkFast << "IKFAST IK Solve: " << RCS::DumpPoseSimple(pose).c_str()<<"\n";
#endif
    for (std::size_t i = 0; i < solutions.GetNumSolutions(); ++i) {
        const ikfast::IkSolutionBase<double> & sol = solutions.GetSolution(i);

        std::vector<double> vsolfree(sol.GetFree().size());
        sol.GetSolution(&solvalues[0], vsolfree.size() > 0 ? &vsolfree[0] : NULL);

        // No gearing done
        std::vector<double> jnts;
        for (std::size_t j = 0; j < solvalues.size(); ++j) {
            jnts.push_back(solvalues[j]);
        }
        joints.push_back(jnts);
#if defined(IKFASTDEBUG)
        ofsIkFast << "IK Solution[" <<i<<"]="<<VectorDump( solvalues)<<"\n";
#endif  
    }
    return solutions.GetNumSolutions();
}

Eigen::VectorXd FanucLRMate200idFastKinematics::ConvertJoints(std::vector<double> v) {
    Eigen::VectorXd p(v.size());
    for (size_t i = 0; i < v.size(); i++)
        p(i) = v[i];
    return p;
}

std::vector<double> FanucLRMate200idFastKinematics::ConvertJoints(Eigen::VectorXd ev) {
    std::vector<double> v;
    for (int i = 0; i < ev.size(); i++)
        v.push_back(ev(i));
    return v;
}

std::vector<double> FanucLRMate200idFastKinematics::NearestJoints(
        std::vector<double> oldjoints,
        std::vector<std::vector<double>> &newjoints) {
    std::vector<double> finaljoints;
    GetClosestSolution(newjoints,oldjoints, finaljoints);
    return finaljoints;
#if 0
     Eigen::VectorXd oldjointvec = ConvertJoints(oldjoints);
    double min = std::numeric_limits<double>::infinity();
    size_t index = 0;
    for (size_t i = 0; i < newjoints.size(); i++) {
        Eigen::VectorXd newjointvec = ConvertJoints(newjoints[i]);
        double diff = (oldjointvec - newjointvec).norm();
        if (diff < min) {
            min = diff;
            index = i;
        }
    }
    // save "best" solution - closset ignoring importance of wrist
    finaljoints.insert(finaljoints.begin(), newjoints[index].begin(), newjoints[index].end());
    return finaljoints;
#endif
}

std::vector<double> FanucLRMate200idFastKinematics::IK(RCS::Pose pose,
        std::vector<double> oldjoints) {
    std::vector<std::vector<double>> allsolutions;
    size_t bFlag = AllIK(pose, allsolutions);

    return NearestJoints(oldjoints, allsolutions);
    //response.error_code.val == response.error_code.SUCCES
    //return response.solution.joint_state.position;
}

bool FanucLRMate200idFastKinematics::IsSingular(RCS::Pose pose, double threshold) {
    return false;
}

void FanucLRMate200idFastKinematics::VerifyLimits(std::vector<double> joints) {
    for (size_t i = 0; i < joints.size(); i++)
        if (joints[i] < joint_min[i] || joints[i] > joint_max[i])
            ROS_ERROR_STREAM("Verify Joint Limits Joint" << joint_names[i] << "out of range");

}

#include "Fanuc/fanuc_lrmate200id_manipulator_ikfast_solver.cpp"

