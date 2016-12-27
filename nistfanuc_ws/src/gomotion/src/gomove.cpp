

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

#include "gomotion/gotypes.h"
#include "gomotion/gointerp.h"
#include "gomotion/gotraj.h"
#include "gomotion/gomotion.h"

#include "gomotion/gomove.h"
#include "gomotion/gomath.h"
#include <stdlib.h>

#define _E(str) #str,__FILE__,__LINE__
#ifndef Deg2Rad
#define Deg2Rad(Ang)    ( (double) ( Ang * M_PI / 180.0 ) )
#define Rad2Deg(Ang)    ( (double) ( Ang * 180.0 / M_PI ) )
#define MM2Meter(d)     ( (double) ( d / 1000.00 ) )
#define Meter2MM(d)     ( (double) ( d * 1000.00 ) )
#endif
namespace gomotion {
    
 

    inline std::string DumpPoseSimple(tf::Pose pose) {
        std::stringstream s;

        s << boost::format("%7.4f") % pose.getOrigin().x() << ":" <<
                boost::format("%7.4f") % pose.getOrigin().y() << ":" <<
                boost::format("%7.4f") % pose.getOrigin().z() << "|";
        tf::Quaternion q = pose.getRotation();
        s << boost::format("%7.4f") % q.x() << ":" <<
                boost::format("%7.4f") % q.y() << ":" <<
                boost::format("%7.4f") % q.z() << ":" <<
                boost::format("%7.4f") % q.w();
        return s.str();
    }
   
    
    // the class that throws, if an HRESULT failed

    class E {
        std::string error_str;
        std::string file_name;
        std::string line_number;

        std::string ExtractFilename(const std::string& path) {
            return path.substr(path.find_last_of('/') + 1);
        }

    public:

        E(char* error, char* file_name, int line_number) {
            char buffer[128];
            error_str = error;
            error_str += "@";
            this->file_name = file_name;
            error_str += ExtractFilename(this->file_name);
            sprintf(buffer, ":%d", line_number);
            this->line_number = buffer;
            error_str += this->line_number;
        }

        void operator=(go_result hr) {
            if (hr != GO_RESULT_OK)
                throw std::runtime_error(error_str.c_str());
        }
    };
#define EF(X) E(#X,__FILE__,__LINE__) = X


    struct go_motion_interface {
        int _type;
        double _deltat;
        go_motion_spec _gms;
        std::vector<go_motion_spec> _space;
        go_motion_queue _gmq;
        go_position _position;
        size_t _queuesize;
        static size_t _id;
    };
    size_t go_motion_interface::_id = 0;

    static go_pose ConvertTfPose(tf::Pose pose) {
        tf::Quaternion q = pose.getRotation();
        return go_pose_this(pose.getOrigin().x(), pose.getOrigin().y(), pose.getOrigin().z(),
                q.w(), q.x(), q.y(), q.z());

    }

    static tf::Pose ConvertGoPose(go_pose p) {
        return tf::Pose(tf::Quaternion(p.rot.x, p.rot.y, p.rot.z, p.rot.s),
                tf::Vector3(p.tran.x, p.tran.y, p.tran.z));
    }

    GoMotion::GoMotion() {
        pgm = boost::shared_ptr<go_motion_interface>(new go_motion_interface());
    }

    int GoMotion::Init(JointState here, double cycletime) {
        try {
            pgm->_gmq.reset();
            pgm->_queuesize = 100;
            pgm->_deltat = cycletime;
            pgm->_space.resize(pgm->_queuesize);
            if (GO_RESULT_OK != pgm->_gmq.init(&pgm->_space[0], pgm->_queuesize, pgm->_deltat)) {
                return 1;
            }
            EF(pgm->_gmq.set_type(GO_MOTION_JOINT));
            go_position position;
            num_joints = here.position.size();
            EF(pgm->_gmq.set_joint_number(num_joints));
            std::copy(here.position.begin(), here.position.end(), position.u.joint);
            EF(pgm->_gmq.set_here(&position));
        } catch (std::runtime_error & e) {
            std::cout << "GoMotion::Init Exception" << e.what() << "\n";
        }
        return GO_RESULT_OK;
    }

    int GoMotion::InitPose(tf::Pose here, tf::Pose there, double deltat,
            gomotion::GoMotionParams tparams,
            gomotion::GoMotionParams rparams) {
        go_position position;
        try {
            //Init(deltat);
            if (pgm->_gmq.get_type() != GO_MOTION_WORLD) {
                EF(pgm->_gmq.reset());
                EF(pgm->_gmq.set_type(GO_MOTION_WORLD));
                position.u.pose = ConvertTfPose(here);
                EF(pgm->_gmq.set_here(&position)); // position copied	
#if defined(DEBUG) && defined (GODEBUG)
                std::cout << "    HERE    = " << DumpPoseSimple(here).c_str() << "\n";
                std::cout << "    HERE    = " << ArrayDump<double>((double *) &position.u.pose, 7) << "\n";
#endif
            }
            EF(pgm->_gms.init());
            EF(pgm->_gms.set_type(GO_MOTION_LINEAR));
            EF(pgm->_gms.set_id(1));
            EF(pgm->_gms.set_tpar(tparams.vel, tparams.acc, tparams.jerk)); // set translation parameters
            EF(pgm->_gms.set_rpar(1.0, 10.0, 100.0)); // set rotation parameters
            position.u.pose = ConvertTfPose(there);
            AppendPose(there);
#if defined(DEBUG) && defined (GODEBUG)
            std::cout << "    THERE   = " << DumpPoseSimple(there).c_str() << "\n";
            std::cout << "    THERE   = " << ArrayDump<double>((double *) &position.u.pose, 7) << "\n";
#endif

        } catch (std::runtime_error & e) {
            std::cout << "GoMotion::InitPose Exception" << e.what() << "\n";
        }
        return GO_RESULT_OK;
    }

    void GoMotion::AppendPose(tf::Pose where) {
        go_position position;
        try {
            if (pgm->_gms.get_type() != GO_MOTION_LINEAR)
                throw std::runtime_error("NextPose when not setup for GO_MOTION_LINEAR");
            // Use pgm->_gms again
            position.u.pose = ConvertTfPose(where);
            EF(pgm->_gms.set_end_position(&position));
            EF(pgm->_gmq.append(&(pgm->_gms)));
        } catch (std::runtime_error & e) {
            std::cout << "GoMotion::InitPose Exception" << e.what() << "\n";
        }
    }

    tf::Pose GoMotion::NextPose() {
        go_position position;
        try {
            if (pgm->_gms.get_type() != GO_MOTION_LINEAR)
                throw std::runtime_error("NextPose when not setup for GO_MOTION_LINEAR");
            EF(pgm->_gmq.interp(&position));
        } catch (std::exception &e) {
            std::cout << "GoMotion::NextPose Exception" << e.what() << "\n";
            throw e;
        }
        return ConvertGoPose(position.u.pose);
    }

    int GoMotion::InitUJoints(JointState here, JointState there, double deltat, gomotion::GoMotionParams jparams) {
        try {
            EF(InitJoints(here, there, deltat, jparams));
            EF(pgm->_gms.set_type(GO_MOTION_UJOINT));
            EF(pgm->_gmq.set_type(GO_MOTION_JOINT));
        } catch (std::exception e) {
            std::cout << "GoMotion::InitUJoints Exception" << e.what() << "\n";
        }
    }

    int GoMotion::InitJoints(JointState here, JointState there, double deltat, gomotion::GoMotionParams jparams) {
        JointState nextjoints;
        go_position position;
        try {
            if (pgm->_gmq.get_type() != GO_MOTION_JOINT) {
                // Set type must go BEFORE set_here!
                EF(pgm->_gmq.reset());
                EF(pgm->_gmq.set_type(GO_MOTION_JOINT));
                std::copy(here.position.begin(), here.position.end(), position.u.joint);
                EF(pgm->_gmq.set_here(&position)); // position copied
            }
            //Init(deltat);
            EF(pgm->_gms.init());
            EF(pgm->_gms.set_type(GO_MOTION_JOINT));
            for (size_t i = 0; i < num_joints; i++)
                EF(pgm->_gms.set_jpar(i, jparams.vel, jparams.acc, jparams.jerk));

            std::copy(there.position.begin(), there.position.end(), position.u.joint);
            EF(pgm->_gms.set_end_position(&position));


            EF(pgm->_gmq.append(&(pgm->_gms)));
        } catch (std::exception &e) {
            std::cout << "GoMotion::InitJoints Exception" << e.what() << "\n";
        }
    }

    void GoMotion::InitStop() {
        try {
            EF(pgm->_gmq.stop());
        } catch (std::exception e) {
            std::cout << "GoMotion::Stop Exception" << e.what() << "\n";
        }
    }

    bool GoMotion::IsDone() {
        return pgm->_gmq.is_empty();
    }

    JointState GoMotion::NextJoints() {
        JointState nextjoints;
        go_position position;
        try {
            if (pgm->_gms.get_type() != GO_MOTION_JOINT)
                throw std::runtime_error("NextJoints when not setup for GO_MOTION_JOINT");
            EF(pgm->_gmq.interp(&position));
            nextjoints.position = std::vector<double>(&position.u.joint[0], &position.u.joint[ num_joints]);
            nextjoints.velocity.resize(num_joints, 0.0);
        } catch (std::exception &e) {
            std::cout << "GoMotion::NextJoints Exception" << e.what() << "\n";
            throw e;
        }
        return nextjoints;
    }

};


