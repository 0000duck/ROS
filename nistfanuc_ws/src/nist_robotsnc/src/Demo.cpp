

/*
DISCLAIMER:
This software was produced by the National Institute of Standards
and Technology (NIST), an agency of the U.S. government, and by statute is
not subject to copyright in the United States.  Recipients of this software
assume all responsibility associated with its operation, modification,
maintenance, and subsequent redistribution.

See NIST Administration Manual 4.09.07 b and Appendix I.
 */
//#pragma message "Compiling " __FILE__ 


#include "Demo.h"
#include "Controller.h"
#include "Globals.h"
#include "Scene.h"
#include "Shape.h"
#include "nist_robotsnc/MotionException.h"

using namespace RCS;
using namespace Conversion;

//#ifndef PI_2
//#define PI_2 1.5707963268
//#endif

boost::mutex RvizDemo::_flag_mutex;

// Simplistic Testing code
int InlineRobotCommands::crclcommandnum = 1;
RCS::Pose retract = RCS::Pose(tf::QIdentity(), tf::Vector3(0, 0, 0.1));

void InlineRobotCommands::SetGripper(double ee) {
    // set gripper to 0..1
    RCS::CanonCmd cmd;
    cmd.crclcommand = CanonCmdType::CANON_SET_GRIPPER;
    cmd.crclcommandnum = crclcommandnum++;
    cmd.eepercent = ee;
    _cnc->crclcmds.AddMsgQueue(cmd);
}

void InlineRobotCommands::CloseGripper() {
    SetGripper(0.75);
}

void InlineRobotCommands::OpenGripper() {
    SetGripper(0.45);
}

void InlineRobotCommands::DoDwell(double dwelltime) {

    RCS::CanonCmd cmd;
    cmd.crclcommand = CanonCmdType::CANON_DWELL;
    cmd.crclcommandnum = crclcommandnum++;
    cmd.dwell_seconds = dwelltime;
    _cnc->crclcmds.AddMsgQueue(cmd);
}

void InlineRobotCommands::MoveTo(RCS::Pose pose, std::string objname) {
    RCS::CanonCmd cmd;
    cmd.crclcommandnum = crclcommandnum++;
    cmd.crclcommand = CanonCmdType::CANON_MOVE_TO;

    //cmd.hint = _hints.FindClosest(pose);
    cmd.finalpose = Convert<tf::Pose, geometry_msgs::Pose> (pose);
    if (!objname.empty()) {
        SceneObject & obj = pScene->Find(objname);
        cmd.partname = objname;
        //cmd.partcolor = (int) pScene->MARKERCOLOR(obj.color);
        cmd.partcolor = obj.rawcolor;
        
        // lookup gripper close amount from object
    }
    _cnc->crclcmds.AddMsgQueue(cmd);
}

void InlineRobotCommands::EraseObject(std::string objname) {
    RCS::CanonCmd cmd;
    cmd.crclcommand = CanonCmdType::CANON_ERASE_OBJECT;
    cmd.crclcommandnum = crclcommandnum++;
    cmd.partname = objname;
    _cnc->crclcmds.AddMsgQueue(cmd);
}
void InlineRobotCommands::GraspObject(std::string objname) {
    RCS::CanonCmd cmd;
    cmd.crclcommand = CanonCmdType::CANON_GRASP_OBJECT;
    cmd.crclcommandnum = crclcommandnum++;
    cmd.partname = objname;
    _cnc->crclcmds.AddMsgQueue(cmd);
}
void InlineRobotCommands::ReleaseObject(std::string objname) {
    RCS::CanonCmd cmd;
    cmd.crclcommand = CanonCmdType::CANON_RELEASE_OBJECT;
    cmd.crclcommandnum = crclcommandnum++;
    cmd.partname = objname;
    _cnc->crclcmds.AddMsgQueue(cmd);
}
void InlineRobotCommands::MoveObject(std::string objname, RCS::Pose pose, std::string color) {
    RCS::CanonCmd cmd;
    cmd.crclcommand = CanonCmdType::CANON_DRAW_OBJECT;
    cmd.crclcommandnum = crclcommandnum++;
    cmd.partname = objname;
    cmd.partcolor = Scene::GetColor(color);
    cmd.finalpose = Convert<tf::Pose, geometry_msgs::Pose>(pose);
    _cnc->crclcmds.AddMsgQueue(cmd);
}

void InlineRobotCommands::Pick(RCS::Pose pose, std::string objname) {

    tf::Vector3 offset = pose.getOrigin();

    // Retract
    MoveTo(retract * RCS::Pose(QBend, offset));
    DoDwell(mydwell);
    MoveTo(RCS::Pose(QBend, offset));
    DoDwell(mydwell);
    CloseGripper();
    DoDwell(mydwell);
    GraspObject(objname);
    MoveTo(retract * RCS::Pose(QBend, offset), objname);
}

void InlineRobotCommands::MoveJoints(std::vector<long unsigned int> jointnum,
        std::vector<double> positions) {
    RCS::CanonCmd cmd;
    cmd.crclcommandnum = crclcommandnum++;
    cmd.crclcommand = CanonCmdType::CANON_MOVE_JOINT;
    cmd.joints = RCS::ZeroJointState(jointnum.size());
    cmd.joints.position = positions;
    cmd.jointnum = jointnum; // ToVector<long unsigned int>(6, 0L, 1L, 2L, 3L, 4L, 5L);
    cmd.bCoordinated = true;
    _cnc->crclcmds.AddMsgQueue(cmd);

}
// fixme: object has to be richer description

void InlineRobotCommands::Place(RCS::Pose pose, std::string objname) {

    tf::Vector3 offset = pose.getOrigin();
    // Retract
    MoveTo(retract * RCS::Pose(QBend, offset), objname);
    DoDwell(mydwell);
    MoveTo(RCS::Pose(QBend, offset), objname);
    ReleaseObject(objname);
    DoDwell(mydwell);
    OpenGripper();
    DoDwell(mydwell);
    MoveTo(retract * RCS::Pose(QBend, offset));

}
/////////////////////////////////////////////////////////////////////////

GearDemo::GearDemo(ros::NodeHandle & nh, std::string pkgpath, tf::Pose offset) :
_nh(nh), _path(pkgpath), _baseoffset(offset), rvizdemo(nh) {

    _jsonpath = pkgpath + "/config/shapes.json";
    _shapes.ParseJsonFile(_jsonpath);

}

bool GearDemo::IssueRobotCommands(InlineRobotCommands & r, bool bSafe) {
    // Finish queuing commands before handling them....
    boost::mutex::scoped_lock lock(cncmutex);
    static double dwelltime = 1.0;
    std::vector<long unsigned int> vjointnum = ToVector<long unsigned int>(6, 0L, 1L, 2L, 3L, 4L, 5L);
    //RCS::Pose retract = RCS::Pose(tf::Quaternion(0, 0, 0, 1), tf::Vector3(0, 0, 0.2));

    RCS::CanonCmd cmd;
    RCS::Pose pickpose;

    boost::shared_ptr< ShapeModel::Instance> instance;
    if ((instance = FindFreeGear()) == NULL)
        return false;

    std::string gearname = instance->name;

    // Ok we will move this gear - mark as no longer free standing
    instance->properties["state"] = "stored";

    SceneObject &obj = pScene->Find(instance->name);
    //NC_ASSERT(obj != NULL);

    tf::Pose affpose = obj.pose; // Convert<Eigen::Vector3d, Eigen::Affine3d>(obj.pose.translation()); 
    // The object gripper offset is where on the object it is to be gripped
    tf::Pose gripperoffset = obj.gripperoffset; // Convert<Eigen::Affine3d, tf::Pose>(obj.gripperoffset);
    // THe gripperoffset is the robot gripper offset back to the 0T6 equivalent
    pickpose = RCS::Pose(r.QBend, affpose.getOrigin())* gripperoffset; // /*Convert<Eigen::Vector3d,tf::Vector3>(affpose.translation()))*gripperoffset;

    tf::Vector3 offset = pickpose.getOrigin();

    // Retract
    r.MoveTo(retract * RCS::Pose(r.QBend, offset));
    r.DoDwell(r.mydwell);
    r.MoveTo(RCS::Pose(r.QBend, offset)*gripperoffset);
    r.DoDwell(r.mydwell);
    r.CloseGripper();
    r.DoDwell(r.mydwell);
    r.MoveTo(retract * RCS::Pose(r.QBend, offset), gearname);

    tf::Pose slotpose;

    try {
        GearDemo::OpenHolderSlot holderslot = FindFreeGearHolder(instance->metatype);
        slotpose = holderslot.second;
        boost::shared_ptr< ShapeModel::Instance> vinstance = holderslot.first;
        tf::Pose gearpose = _baseoffset * _shapes.GetInstancePose(vinstance);
        slotpose = gearpose * slotpose;
    } catch (...) {
        return false;
    }

    RCS::Pose placepose = RCS::Pose(r.QBend, slotpose.getOrigin()); // fixme: what if gear rotated
    r.Place(placepose , gearname);
    r.DoDwell(r.mydwell);

    if (bSafe) {
        std::vector<unsigned long> jointnum(r.cnc()->Kinematics()->NumJoints());
        std::iota(jointnum.begin(), jointnum.end(), 0); // adjusted already to 0..n-1
        r.MoveJoints(jointnum, r.cnc()->NamedJointMove["Safe"]);
    }
    return true;
}

boost::shared_ptr< ShapeModel::Instance> GearDemo::FindFreeGear() {
    std::string type = "gear";
    std::vector<boost::shared_ptr< ShapeModel::Instance> > instances = _shapes.TypeInstances(type);
    for (size_t i = 0; i < instances.size(); i++) {

        if (instances[i]->properties["state"] == "free")
            return instances[i];
    }
    return NULL;
}

GearDemo::OpenHolderSlot GearDemo::FindFreeGearHolder(std::string geartype, bool bFill) {

    std::string vesseltype = "holder";
    std::vector<boost::shared_ptr< ShapeModel::Instance> > vinstances = _shapes.TypeInstances(vesseltype);
    for (size_t i = 0; i < vinstances.size(); i++) {
        std::vector<std::string> slots = _shapes.GetChildrenNames("parts." + vinstances[i]->metatype + ".contains");
        std::vector<std::string> slotnames = _shapes.GetShapeChildrenNames(vinstances[i]->metatype, "contains");
        for (size_t j = 0; j < slotnames.size(); j++) {
            // Check that type == holder and metatype == metatype
            std::string type = _shapes.GetChildValue<std::string>(slotnames[j] + ".type");
            std::string metatype = _shapes.GetChildValue<std::string>(slotnames[j] + ".metatype");
            if (type != "holder" || metatype != geartype)
                continue;
            std::string statechild = vinstances[i]->propname + ".contains." + slots[j] + ".state";
            std::string state = _shapes.GetChildValue<std::string>(statechild);
            if (state == "full")
                continue;
            if (bFill) {
                _shapes.PutChildValue<std::string>(statechild, "full");
            }
            return std::make_pair(vinstances[i], _shapes.GetChildPose(slotnames[j]));
        }
    }
    throw MotionException(1030, "");
}

void GearDemo::Setup() {

    // Find and make scene objects of all free gears
    std::string type = "gear";
    std::vector<boost::shared_ptr< ShapeModel::Instance> > instances = _shapes.TypeInstances(type);
    for (size_t i = 0; i < instances.size(); i++) {

        //        if (instances[i]->properties["state"] != "free")
        //            continue;

        std::string gearname = instances[i]->name; // Globals.StrFormat("%s%d", sku.c_str(), i + 1);
        tf::Pose gearpose = _baseoffset * _shapes.GetInstancePose(instances[i]);

        ShapeModel::Shape shape = _shapes.GetInstanceShape(instances[i]);
        SceneObject &obj = pScene->CreateMesh(gearname, "gear",
                pScene->gid++,
               gearpose,
                shape.meshfile,
                Scene::GetColor(shape.color),
                shape.scale);
        std::vector<double> graspoffset = _shapes.GetChildValues<double>("parts." + shape.name + ".gripper.offset");
        obj.gripperoffset = Conversion::CreateRPYPose(graspoffset);
        obj.instance = instances[i];
        //        pScene->CreateMarker("maker",
        //                Convert<tf::Pose, Eigen::Affine3d>(gearpose * Conversion::CreatePose(tf::Vector3(0.0, 0.0, 0.04))
        //                ),
        //                "GREEN");
    }

    // Find and make scene objects of all gear holders (kits or vessel)  
    type = "holder";
    instances = _shapes.TypeInstances(type);
    for (size_t i = 0; i < instances.size(); i++) {

        std::string holdername = instances[i]->name;

        // adjust pose into robot coordinates (baseoffset) 
        tf::Pose gearpose = _baseoffset * _shapes.GetInstancePose(instances[i]);

        ShapeModel::Shape shape = _shapes.GetInstanceShape(instances[i]);
        //tf::Vector3 pos = shapes.GetInstancePosition(instances[i]); // +tf::Vector3(0.0455, 0.0356,0.0);
        //        pos.rotate(tf::Vector3(0.0,0.0,1.0), Deg2Rad(-90.0));
        SceneObject &obj = pScene->CreateMesh(holdername,
                "gearholder",
                pScene->gid++,
                gearpose,
                shape.meshfile,
                Scene::GetColor(shape.color),
                shape.scale);
        obj.instance = instances[i];

    }
    // Debug: LOG_DEBUG << SceneObject::DumpDB();   
}

void GearDemo::Reset() {
    //    pScene->ClearScene(); // erases all markers
    //    Setup(); // recreates all markers
    //    pScene->DrawScene(); // draws rviz scene with markers
}

void GearDemo::Cycle(boost::shared_ptr<RCS::CController> nc, InlineRobotCommands&robot) {
    ros::Rate r(50);
    while (IssueRobotCommands(robot) != NULL) {
        while (nc->IsBusy()) {
            if (rvizdemo.Clicked()) {
                RCS::Thread::SuspendAll();
                while (!rvizdemo.Clicked())
                    r.sleep();
                RCS::Thread::ResumeAll();
            }
            ros::spinOnce();
            ros::spinOnce();
            r.sleep();
        }
    }
}
//////////////////////////////////////////////////////////////////////////
#include "RvizMarker.h" 


CheckersGame::CheckersGame(ros::NodeHandle & nh) : _nh(nh) {
    rvizgame = boost::shared_ptr<RvizCheckers> (new RvizCheckers(nh));

}
void CheckersGame::PhysicalMove(InlineRobotCommands &robot,
        int player,
        int i, int j,
        Checkers::Move m,
        bool doublejump) {
    
    std::string typemove("move");
    
    try {
        // If blank space there can be no actual checkername2 object
        std::string checkername1 = Globals.StrFormat("Checker[%d:%d]", i, j);
        std::string checkername2 = Globals.StrFormat("Checker[%d:%d]", m.row, m.col);
        tf::Pose frompose = rvizgame->GetPose(i, j);
        tf::Pose topose = rvizgame->GetPose(m.row, m.col);
        SceneObject & obj = pScene->Find(checkername1);
        if(Scene::IsNull(obj))
            throw std::runtime_error(Globals.StrFormat("PhysicalMove null scene object Checker[%d:%d]", i, j));

#ifdef DEBUG
//        LOG_DEBUG << "Move From " << Globals.StrFormat("[%d,%d]=%f,%f", i, j, frompose(0, 3), frompose(1, 3));
//        LOG_DEBUG << "Move To   " << Globals.StrFormat("[%d,%d]=%f,%f", m.row, m.col, topose(0, 3), topose(1, 3));
        LOG_DEBUG << "Move From " << Globals.StrFormat("[%d,%d]=%f,%f", i, j, frompose.getOrigin().x(), frompose.getOrigin().y());
        LOG_DEBUG << "Move To   " << Globals.StrFormat("[%d,%d]=%f,%f", m.row, m.col, topose.getOrigin().x(), topose.getOrigin().y());
        LOG_DEBUG << "tf    Checkerboard1 pose" << RCS::DumpPoseSimple(obj.pose);
 #endif

        robot.Pick(obj.pose, checkername1);
        robot.Place(topose, checkername1);

        while (robot.cnc()->IsBusy())
            ros::Duration(0.01).sleep();

        if (m.bJump) {
            typemove = "jump";
            Checkers::Move jumped = m.Diff(Checkers::Move(i, j));
            std::string jumpedcheckername = Globals.StrFormat("Checker[%d:%d]", jumped.row, jumped.col);
            pScene->DeleteObject(jumpedcheckername);
        }
        // change checker name from old checkerboard spot to new one
        obj.name = checkername2;
        obj.pose = topose;

        if (rvizgame->IsKing(m)) {
            // Double checker height - for now 
            // Only 2* level for king
            if (obj.height == rvizgame->HEIGHT) {
                obj.height *= 2;
                obj.pose.setOrigin(  obj.pose.getOrigin() + tf::Vector3(0, 0, 0.01));
                ros::spinOnce();
                ros::spinOnce();
                ros::spinOnce();
            }
        }
        ros::spinOnce();

        if (m.doublejumps.size() > 0) {
            PhysicalMove(robot, player, m.row, m.col, m.doublejumps[0], true);
        }

        // Done - move to safe non-collision position - skip if double jump
        if (!doublejump) {
            std::vector<unsigned long> jointnum(robot.cnc()->Kinematics()->NumJoints());
            std::iota(jointnum.begin(), jointnum.end(), 0); // adjusted already to 0..n-1
            robot.MoveJoints(jointnum, robot.cnc()->NamedJointMove["Safe"]);
            while (robot.cnc()->IsBusy()) {
                ros::spinOnce();
                ros::Duration(0.01).sleep();
            }
        }
    } catch (std::runtime_error &e) {
        std::cout << e.what() << "\n";
        throw;
    }

}

void CheckersGame::Setup() {
    rvizgame->RvizBoardSetup();
    rvizgame->RvizPiecesSetup();
    pScene->DrawScene(); // Debug: LOG_DEBUG << SceneObject::DumpDB();
}

void CheckersGame::Play(InlineRobotCommands * red, InlineRobotCommands * black) {
    //InlineRobotCommands * Ncs[2]={&nccmds[0], &nccmds[1]};
    //InlineRobotCommands * Ncs[2]={&fanucrobot, &fanucrobot};
    //InlineRobotCommands * Ncs[2]={&motomanrobot, &motomanrobot};

    // Play checkers - only move markers, no robot interaction
    Checkers::Move from, to;
    int player;
    for (size_t i = 0; i < 40; i++) {
        bool bQuit = rvizgame->CheckersMove(player, from, to);
           
        rvizgame->Game().printDisplayFancy(rvizgame->Game().Board());
        if (player == Checkers::RED) {
            LOG_DEBUG << "RED Move " << red->cnc()->Name().c_str();
            PhysicalMove(*red, player, from.row, from.col, to);
        } else {
            LOG_DEBUG << "BLACK Move " << black->cnc()->Name().c_str();
            PhysicalMove(*black, player, from.row, from.col, to);
        }
        if(bQuit)
            break;

#if 0
        // Synchronize with rviz let PublishPoint pause execution
        if (rvizdemo.Clicked()) {
            for (size_t j = 0; j < Ncs.size(); j++)
                Ncs[0]->cnc()->Suspend();
            while (1) {
                ros::spinOnce();
                ros::Duration(0.2).sleep();
                if (rvizdemo.Clicked())
                    break;
            }
            for (size_t j = 0; j < Ncs.size(); j++)
                Ncs[0]->cnc()->Resume();

        }
#endif
        ros::spinOnce();
        ros::spinOnce();
        ros::Duration(0.2).sleep();
    }
}

void ExerciseDemo::MarkPose(int flag, tf::Pose pose) {
    pose = Robot()->cnc()->basePose() * pose;
    if (flag == 1)
        RvizMarker()->SetColor(GoodColor()[0], GoodColor()[1], GoodColor()[2], 1.0);
    else if (flag == 0)
        RvizMarker()->SetColor(BadColor()[0], BadColor()[1], BadColor()[2], 1.0);
    else if (flag == -1)
        RvizMarker()->SetColor(TrapColor()[0], TrapColor()[1], TrapColor()[2], 1.0);
        //RvizMarker()->SetColor(0.597,0.0,0.597, 1.0);  // purple
    RvizMarker()->Send(pose);
}
#include <boost/bind.hpp>

void ExerciseDemo::Exercise(InlineRobotCommands *robot) {
    pScene->ClearScene();
    RvizMarker()->Clear();
    ros::spinOnce();
    ros::spinOnce();
    ros::spinOnce();
    ros::spinOnce();
    Globals.Sleep(1000);
    Robot() = robot;
    Robot()->cnc()->Kinematics()->ENormalize(- M_PI,  M_PI);
    std::vector<size_t>  scorecard = Robot()->cnc()->Kinematics()->VerifyKinematics(boost::bind(&ExerciseDemo::MarkPose, this, _1, _2));
    int total = std::accumulate(scorecard.begin(), scorecard.end(), 0);
    //int fail = std::accumulate(scorecard.begin()+1, scorecard.end(), 0);
    LOG_DEBUG <<  "Tests:" << total << ": good=" << scorecard[0]
           << ": bad="  << scorecard[1]
           << ": singular=" << scorecard[2] <<  "\n";
}

