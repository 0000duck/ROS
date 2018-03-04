
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

#include "nist_robotsnc/RvizMarker.h"
int CRvizMarker::_id = 1;

CRvizMarker::CRvizMarker(ros::NodeHandle & nh) : n(nh) {
    scalex = scaley = scalez = 0.005;
    // Set our initial shape type to be a cube
    shape = visualization_msgs::Marker::SPHERE;
    r = 1.0;
    g = 0.0;
    b = 0.0;
    a = 1.0;
}
int CRvizMarker::Scale(double x, double y, double z){
    scalex = x; scaley = y; scalez = z;
    return 0;
}
void CRvizMarker::Init(std::string frameid) {
    marker_pub = n.advertise<visualization_msgs::Marker>("visualization_marker", 10000);
    _frameid=frameid; // "world";
}

void CRvizMarker::Clear() {
    visualization_msgs::Marker reset_marker_;
    reset_marker_.header.frame_id = "world";
    reset_marker_.header.stamp = ros::Time();
    reset_marker_.ns = "deleteAllMarkers"; // helps during debugging
    reset_marker_.action = 3;
    marker_pub.publish(reset_marker_);
    
}
int CRvizMarker::Send(tf::Pose p, std::string frame) {
    visualization_msgs::Marker marker;
    // Set the frame ID and timestamp.  See the TF tutorials for information on these.
    marker.header.frame_id = frame;
    marker.header.stamp = ros::Time::now();

    // Set the namespace and id for this marker.  This serves to create a unique ID
    // Any marker sent with the same namespace and id will overwrite the old one
    marker.ns = "nist_fanuc";
    marker.id = _id++;

    // Set the marker type.  Initially this is CUBE, and cycles between that and SPHERE, ARROW, and CYLINDER
    marker.type = shape;

    // Set the marker action.  Options are ADD and DELETE
    marker.action = visualization_msgs::Marker::ADD;

    // Set the pose of the marker.  This is a full 6DOF pose relative to the frame/time specified in the header
#if 0
    marker.pose.position.x = 0;
    marker.pose.position.y = 0;
    marker.pose.position.z = 0;
    marker.pose.orientation.x = 0.0;
    marker.pose.orientation.y = 0.0;
    marker.pose.orientation.z = 0.0;
    marker.pose.orientation.w = 1.0;
#endif
    marker.pose.position.x = p.getOrigin().x();
    marker.pose.position.y = p.getOrigin().y();
    marker.pose.position.z = p.getOrigin().z();
    marker.pose.orientation.x = p.getRotation().x();
    marker.pose.orientation.y = p.getRotation().y();
    marker.pose.orientation.z = p.getRotation().z();
    marker.pose.orientation.w = p.getRotation().w();

    // Set the scale of the marker -- 1x1x1 here means 1m on a side!!!
    marker.scale.x = scalex;
    marker.scale.y = scaley;
    marker.scale.z = scalez;

    // Set the color -- be sure to set alpha to something non-zero!
    marker.color.r = r;
    marker.color.g = g;
    marker.color.b = b;
    marker.color.a = a;

    marker.lifetime = ros::Duration(0.0);

    // Publish the marker
    marker_pub.publish(marker);
    ros::spinOnce();
    ros::spinOnce();

    return 0;
}

void CRvizMarker::SetColor(double r, double g, double b, double a) {
    this->r = r;
    this->g = g;
    this->b = b;
    this->a = a;
}

uint32_t CRvizMarker::SetShape(uint32_t shape) {
    uint32_t oldshape = this->shape;
    // Cycle between different shapes
    switch (shape) {
        case visualization_msgs::Marker::CUBE:
            this->shape = visualization_msgs::Marker::CUBE;
            break;
        case visualization_msgs::Marker::SPHERE:
            this->shape = visualization_msgs::Marker::SPHERE;
            break;
        case visualization_msgs::Marker::ARROW:
            this->shape = visualization_msgs::Marker::ARROW;
            break;
        case visualization_msgs::Marker::CYLINDER:
            this->shape = visualization_msgs::Marker::CYLINDER;
            break;
    }
    return oldshape;
}
void CRvizMarker::SetShape(std::string shape){
    std::transform(shape.begin(), shape.end(), shape.begin(), ::tolower);
    if(shape=="cube")
        SetShape(visualization_msgs::Marker::CUBE);
    if(shape=="sphere")
        SetShape(visualization_msgs::Marker::SPHERE);
    if(shape=="arrow")
        SetShape(visualization_msgs::Marker::ARROW);
    if(shape=="cylinder")
        SetShape(visualization_msgs::Marker::CYLINDER);
}
#include "Conversions.h"
using namespace Conversion;

void CRvizMarker::publishLine(const tf::Vector3 &point1,
        const tf::Vector3 &point2,
        double radius,
        double scale) {
    visualization_msgs::Marker generic_marker_;
    // Set the timestamp
    generic_marker_.header.stamp = ros::Time::now();
    generic_marker_.lifetime = ros::Duration(0.0);

    generic_marker_.type = visualization_msgs::Marker::LINE_STRIP;
    generic_marker_.color.r = r;
    generic_marker_.color.g = g;
    generic_marker_.color.b = b;
    generic_marker_.color.a = a;

    geometry_msgs::Vector3 vscale;
    vscale.x = radius;
    vscale.y = radius;
    vscale.z = radius;

    generic_marker_.id = _id++;
    generic_marker_.ns = "line";
     generic_marker_.scale = vscale;

    generic_marker_.points.clear();
    generic_marker_.points.push_back(Convert<tf::Vector3, geometry_msgs::Point>(point1));
    generic_marker_.points.push_back(Convert<tf::Vector3, geometry_msgs::Point>(point2));
    generic_marker_.action = visualization_msgs::Marker::ADD;
    generic_marker_.header.frame_id = _frameid;
    generic_marker_.pose.orientation.w=1.0;
    // Helper for publishing rviz markers
    marker_pub.publish(generic_marker_); // line_strip_marker_);
    ros::spinOnce();
    ros::spinOnce();
}

int CRvizMarker::publishMesh(const tf::Pose &pose,
        const std::string &file_name,
        double scale,
        std::size_t id) {
    visualization_msgs::Marker generic_marker_;
    // Set the timestamp
    generic_marker_.action = visualization_msgs::Marker::ADD;
    generic_marker_.header.frame_id = _frameid;
    generic_marker_.header.stamp = ros::Time::now();
    generic_marker_.lifetime = ros::Duration(0.0);
    generic_marker_.ns = "mesh";
    if (id == 0)
        generic_marker_.id = _id++;
    else
        generic_marker_.id = id;
    generic_marker_.type = visualization_msgs::Marker::MESH_RESOURCE;

    // Set the mesh
    generic_marker_.mesh_resource = file_name;
    generic_marker_.mesh_use_embedded_materials = true;

    // Set the pose
    generic_marker_.pose = Convert<tf::Pose, geometry_msgs::Pose >(pose);

    // Set marker size
    generic_marker_.scale.x = scale;
    generic_marker_.scale.y = scale;
    generic_marker_.scale.z = scale;

    generic_marker_.color.r = r;
    generic_marker_.color.g = g;
    generic_marker_.color.b = b;
    generic_marker_.color.a = a;

    marker_pub.publish(generic_marker_);
    ros::spinOnce();
    ros::spinOnce();    
    return generic_marker_.id;
}
