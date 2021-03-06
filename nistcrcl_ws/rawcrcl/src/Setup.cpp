
#include "Setup.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <sstream>

#include <boost/algorithm/string.hpp>

#include <ros/ros.h>

#include "Globals.h"

template<typename T>
static std::string MapDump(std::map<std::string, T> m) {
    std::stringstream s;

    for (typename std::map<std::string, T>::iterator it = m.begin(); it != m.end(); it++) {
        s << (*it).first << "=" << (*it).second << std::endl;
    }
    s << std::endl;
    return s.str();
}

static void SetupRosEnv(std::string envname, std::string envval, int overwrite = 1) {
    if (0 != setenv(envname.c_str(), envval.c_str(), overwrite))
        throw std::runtime_error(Globals.StrFormat("SetupRosEnv Failed %s=%s", envname.c_str(), envval.c_str()));
}

static std::vector<std::string> Split(const std::string &s, char delim, bool bKeepEmpty = false) {
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        if (item.empty() && !bKeepEmpty)
            continue;
        elems.push_back(item);
    }
    return elems;
}

static std::map<std::string, std::string> ParseIniString(std::string s) {
    std::map<std::string, std::string> symbols;
    std::vector<std::string> lines = Split(s, '\n');
    for (size_t i = 0; i < lines.size(); i++) {
        size_t found;
        if ((found = lines[i].find("=")) == std::string::npos)
            continue;
#if 0
        std::vector<std::string> tokens = Split(lines[i], '=');
        if (tokens.size() < 2)
            continue;
        if (tokens[0].empty())
            continue;
        if (tokens.size() > 2) // multiple equals - skip for now
            continue;
        symbols[Globals.Trim(tokens[0])] = Globals.Trim(tokens[1]);
#endif
        // FIXME: 1st token must not be empty
        std::string key = lines[i].substr(0, found);
        std::string value = lines[i].substr(found + 1);
        //        std::cout << key.c_str() << "=" << value.c_str() << std::endl;
        symbols[Globals.Trim(key)] = Globals.Trim(value);

    }
    return symbols;
}

std::string ExecuteShellCommand(std::string command) {
    std::string result;
    FILE *fpipe;
    char line[512];
    if (!(fpipe = (FILE*) popen((char *) command.c_str(), "r"))) {
        // If fpipe is NULL
        std::cerr << "Problems with pipe";
        return result;
    }
    while (fgets(line, sizeof line, fpipe)) {
        result += line; // does it have line feed at end?
    }
    pclose(fpipe);
    return result;
}

        
bool SetupAppEnvironment() {
    std::string user = getenv("USER");
    std::string shostname = ExecuteShellCommand("echo `hostname`");
    shostname = Globals.Trim(shostname);
       // Save properties in setup
    Globals._appproperties["user"]= user;
    Globals._appproperties["hostname"]= shostname;
	return true;
}  
bool SetupRosEnvironment(std::string pkgpath) {
#if 1
    //std::string rosdistro =  getenv ("ROS_DISTRO");
    // suppose we could just copy all the environment variables
    //std::string user = Globals._appproperties["user"];
    //std::cout << "user=" << user.c_str() << std::endl;
    std::string cmd = Globals.StrFormat("/bin/bash -i /usr/local/michalos/nistfanuc_ws/devel/lib/nist_fanuc/rossetup.bash  ");
    std::cout << "cmd=" << cmd.c_str() << std::endl;
#else
    //   pkgpath=pkgpath+"/scripts/rossetup.bash"
    pkgpath = pkgpath + "/../../devel/setup.bash";
    std::cout << "pkgpath=" << pkgpath.c_str() << std::endl;
    std::string cmd = Globals.StrFormat("/bin/bash -i %s && env\n", pkgpath.c_str());
    std::cout << "cmd=" << cmd.c_str() << std::endl;
#endif
    std::string envs = ExecuteShellCommand(cmd);
    std::cout << "envs=" << envs.c_str() << std::endl;
    std::map<std::string, std::string> envvars = ParseIniString(envs);
    for (std::map<std::string, std::string>::iterator it = envvars.begin(); it != envvars.end(); it++) {
        // std::cout << (*it).first.c_str() << "="  <<(*it).second.c_str()<< std::endl;
        SetupRosEnv((*it).first, (*it).second);
    }
    return true;
}

std::string ReadRosParam(ros::NodeHandle &nh, std::string paramkey) {

    std::stringstream  result;
    std::string s;
    double d;
    int i;
    bool b;
    std::vector< std::string > svec;
    std::vector< double > dvec;
    std::vector< float > fvec;
    std::vector< int > ivec;
    std::vector< bool > bvec;
    std::map< std::string, std::string > smap;
    std::map< std::string, double > dmap;
    std::map< std::string, float > fmap;
    std::map< std::string, int > imap;
    std::map < std::string, bool > bmap;
    XmlRpc::XmlRpcValue  v;
    if (nh.getParam(paramkey, s)) {
        result << s;
    } else if (nh.getParam(paramkey, d)) {
        result << d;
    } else if (nh.getParam(paramkey, i)) {
        result << i;
    } else if (nh.getParam(paramkey, b)) {
        result << b;
    } else if (nh.getParam(paramkey, svec)) {
        result << VectorDump<std::string> (svec);
    } else if (nh.getParam(paramkey, dvec)) {
        result << VectorDump<double> (dvec);
    } else if (nh.getParam(paramkey, fvec)) {
        result << VectorDump<float> (fvec);
    } else if (nh.getParam(paramkey, ivec)) {
        result << VectorDump<int> (ivec);
    } else if (nh.getParam(paramkey, bvec)) {
        result << VectorDump<bool> (bvec);
    } else if (nh.getParam(paramkey, smap)) {
        result << MapDump<std::string> (smap);
    } else if (nh.getParam(paramkey, dmap)) {
        result << MapDump<double> (dmap);
    } else if (nh.getParam(paramkey, fmap)) {
        result << MapDump<float> (fmap);
    } else if (nh.getParam(paramkey, imap)) {
        result << MapDump<int> (imap);
    } else if (nh.getParam(paramkey, bmap)) {
        result << MapDump<bool> (bmap);
    } else if (nh.getParam(paramkey,  v)) {

    }
    return result.str();
}

std::string ReadRosParams(ros::NodeHandle &nh) {
    std::stringstream  result;
    std::string cmd = "/opt/ros/indigo/bin/rosparam list  ";
    std::string params = ExecuteShellCommand(cmd);
   // std::string rvizid = RvizId(params, Globals._appproperties["hostname"]);
   // Globals._appproperties["rvizid"]= rvizid;

    std::vector<std::string> lines = Split(params, '\n');
    for (size_t i = 0; i < lines.size(); i++) {
        lines[i] = Globals.Trim(lines[i]);
        if (lines[i].empty())
            continue;
        std::string param = ReadRosParam(nh, lines[i]);
        if (!param.empty()) {
            result << lines[i] << "=" << param.c_str() << std::endl;
        }

    }
    return result.str();
}

