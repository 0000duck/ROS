
// RvizMarker.h
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
#pragma once
#include <ros/ros.h>
#include <visualization_msgs/Marker.h>
#include <tf/transform_datatypes.h>

/*
 Sample code:
        boost::shared_ptr<CRvizMarker> pRvizMarker;
        pRvizMarker = boost::shared_ptr<CRvizMarker>(new CRvizMarker(nh));
        pRvizMarker->Init();
. . .
                     if (bMarker()) {
                        RCS::Pose goalpose = Kinematics()->FK(_newcc.joints.position);
                         LOG_DEBUG << "Marker Pose " << DumpPose(goalpose).c_str();
                        RvizMarker()->Send(goalpose);
                    }
 
 */
/**
 * \brief The CRvizMarker provides a C++ class to send markers to rviz.
 * Note you must manually add the rviz subscribe to marker messages. Read here for
 * explanation
 *  http://answers.ros.org/question/11135/plotting-a-markerarray-of-spheres-with-rviz/
 * 
 * 
 * In the ROS Electric version of rviz (the latest version), Marker is a proper display type.
 * To show the data from the program you posted, in rviz do:
 *   Click the "Add" button in the "Displays" area.
 *   Choose "Marker" in the type list.
 *   Click "OK"

 *If it doesn't show immediately (it hung for me the first time), do:

 *   Click on the "Marker Array Topic" line in the new display entry. This should make an elipsis ("...") button appear.
 *  Click the "..." button.
 *  A dialog with currently-published MarkerArray topics should appear.
 *  Click on "visualization_marker_array" (or whatever you have named it).
 *  Click "OK"
 */


class CRvizMarker {
public:
    /*!
     *\brief Constructor for "Marker" which sets default marker values...
     */
    CRvizMarker(ros::NodeHandle & nh);
    /*!
     *\brief Initialization routine which subsribes to the "Marker" topic..
     */
    void Init();
    /*!
     *\brief Publish a visualization marker message to the Marker topic.
     * \param pose p is where the marker is to be placed relative to the base link.
     * \param frame is the world fixed frame name or base
     */
    int Send(tf::Pose p, std::string frame);
    
    int Scale(double x= 0.005, double y= 0.005, double z= 0.005);
    
    /*!
     *\brief Set the marker to be displayed.
     * \param shape as string:  cube, arrow, cylinder, sphere. 
     */
    void SetShape(std::string shape);
     /*!
     *\brief Set the color of the maker to be displayed.
     * \param red, green. blue and alpha are the values used herein.
     */
    void SetColor(double r, double g, double b, double a);
    /*!
     *\brief Clear all markers that have been created.
     */   
    void Clear();
    /////////////////////////////////
    ros::Publisher marker_pub;
    ros::NodeHandle & n;
    uint32_t shape;
    static int _id;
    double scalex, scaley, scalez;
    double r, g, b, a;
        /*!
     *\brief Set the marker to be displayed.
     * \param shape is the enumeration of the marker type. 
     */
    uint32_t SetShape(uint32_t shape);
};