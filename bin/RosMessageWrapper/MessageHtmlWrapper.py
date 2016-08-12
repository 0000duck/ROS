#!/usr/bin/env python
# -*- coding: utf-8 -*-


'''

Running in Spyder
    Set ROS env package path
    e.g., '    os.environ["ROS_PACKAGE_PATH"]="/usr/local/michalos/nistcrcl_ws/src/fanuc_lrmate200id_support:/usr/local/michalos/nistcrcl_ws/src/nistcrcl:/usr/local/michalos/nistcrcl_ws/src/rawcrcl:/usr/local/michalos/nistcrcl_ws/src/testcrcl:/usr/local/michalos/nistcrcl_ws/src/testkin:/opt/ros/indigo/share:/opt/ros/indigo/stacks"
h'
F6
Put in command line options (e.g., -pnistcrcl)
Run (">||")
'''
import subprocess
import os
import sys
import inspect
import getopt
#import RosEnv

packagelist = {}
def GetCurrentPath():
    return os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))


class Msg(object):
    def __init__(self, typename=None, name=None, comment=None):
        self.typename = typename
        self.name = name
        self.comment = comment
    def Dump(self):
        print "TypeName=",self.typename, "Name=",self.name, "Comment=",self.comment,

#def ScriptSheet():
#    mystr = ""
#    mystr = ""
#    mystr+="<!-- Javascript goes in the document HEAD -->\n"
#    mystr+="<script type=\"text/javascript\">\n"
#    mystr+="function altRows(id){\n"
#    mystr+="               if(document.getElementsByTagName){ \n"
#    mystr+="                            \n"
#    mystr+="                              var table = document.getElementById(id); \n"
#    mystr+="                              var rows = table.getElementsByTagName(\"tr\");\n"
#    mystr+="                               \n"
#    mystr+="                             for(i = 0; i < rows.length; i++){         \n"
#    mystr+="                                             if(i % 2 == 0){\n"
#    mystr+="                                                            rows[i].className = \"evenrowcolor\";\n"
#    mystr+="                                            }else{\n"
#    mystr+="               import sys                                             rows[i].className = \"oddrowcolor\";\n"
#    mystr+="                                             }    \n" 
#    mystr+="                              }\n"
#    mystr+="               }\n"
#    mystr+="}\n"
#    return mystr
#
#def ScriptSheet():
#    mystr = ""
#    mystr+="window.onload=function(){\n"
#    mystr+="               altRows('alternatecolor');\n"
#    mystr+="}\n"
#    mystr+="</script>\n"
#    return mystr

def StyleSheet():
    mystr = ""
    mystr+="<!-- CSS goes in the document HEAD or added to your external stylesheet -->\n"
    mystr+="<style type=\"text/css\">\n"
    mystr+="table.altrowstable {\n"
    mystr+="               font-family: verdana,arial,sans-serif;\n"
    mystr+="               font-size:11px;\n"
    mystr+="               color:#333333;\n"
    mystr+="               border-width: 1px;\n"
    mystr+="               border-color: #a9c6c9;\n"
    mystr+="               border-collapse: collapse;\n"
    mystr+="}\n"
    mystr+="table.altrowstable th {\n"
    mystr+="               border-width: 1px;\n"
    mystr+="               padding: 0px;\n"
    mystr+="               border-style: solid;\n"
    mystr+="               border-color: #a9c6c9;\n"
    mystr+="}\n"
    mystr+="table.altrowstable td {\n"
    mystr+="               border-width: 1px;\n"
    mystr+="               padding: 0px;\n"
    mystr+="               border-style: solid;\n"
    mystr+="               border-color: #a9c6c9;\n"
    mystr+="}\n"
    mystr+=".oddrowcolor{\n"
    mystr+="               background-color:#d4e3e5;\n"
    mystr+="}\n"
    mystr+=".evenrowcolor{\n"
    mystr+="               background-color:#c3dde0;\n"
    mystr+="}\n"
    mystr+="@page {\n"
    mystr+="    @bottom {\n"
    mystr+="        content: \"Page \" counter(page) \" of \" counter(pages)\n"
    mystr+="    }\n"
    mystr+="}\n"
    mystr+="</style>\n"
    return mystr

def run_command(cmd):
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE)
    return proc.communicate()[0]

def GetRosMsgDepends():
    p = subprocess.Popen(['rospack', 'list'], stdout=subprocess.PIPE,  stderr=subprocess.PIPE, stdin=subprocess.PIPE)
    out, err = p.communicate()
    lines = out.splitlines(True)
        os.environ["ROS_PACKAGE_PATH"]="/usr/local/michalos/nistcrcl_ws/src/fanuc_lrmate200id_support:/usr/local/michalos/nistcrcl_ws/src/nistcrcl:/usr/local/michalos/nistcrcl_ws/src/rawcrcl:/usr/local/michalos/nistcrcl_ws/src/testcrcl:/usr/local/michalos/nistcrcl_ws/src/testkin:/opt/ros/indigo/share:/opt/ros/indigo/stacks"
rosdepends = []
    for line in lines:
        words = line.split()
        if(words[0].find("_msgs") != -1): 
            rosdepends.append(words[0])
            packagelist[words[0]] = words[1]
    return  rosdepends   

def BuildHtmlMsgTable(package):
    mymystr = "<h1>" + package + "</h1>\n"
    msgs = run_command(['rosmsg','package', package])
    lines = msgs.split('\n') 
    for line in lines:
        if(line == ""): continue
        mymystr+="<h2>" + line + "</h2>\n"
        mymystr+="<table class=\"altrowstable\"\">\n"
        out = run_command(['rosmsg','show', line])
        fields = out.split('\n') 
        j = 0
        for field in fields:
            words = field.split()
            if(len(words) < 2): continue
            mymystr+="<tr><td>" + words[0] + "</td><td>" + words[1] + "</td></tr>\n"
            #if(j%2 == 0): mymystr+="<tr class = \"evenrowcolor\"><td>"+
            #words[0]+"</td><td>"+ words[1] +"</td></tr>\n"
            #if(j%2 == 1): mymystr+="<tr class = \"oddrowcolor\"><td>"+
            #words[0]+"</td><td>"+ words[1] +"</td></tr>\n"
            j = j + 1
        mymystr+="</table>\n"
    return mymystr

def MsgFilename(mystr):
    names = mystr.split("/")
    if(len(names) == 1): return names[0]
    if(len(names) > 1): return names[len(names) - 1]
    return mystr

def ReadFile(filename):
    with open(filename, "r") as myfile:
        data = myfile.read()
    return data

def RemoveComments(contents):
    cleaned = ""
    lines = contents.split('\n') 
    for line in lines:
        li = line.strip()
        if not li.startswith("#"):
            cleaned+= line

def ParseMessages(contents):
    msgs = []
    comment = ""
    lines = contents.split('\n') 
    bAutoGen = False
    n = 0
    while (len(lines) > n and (lines[n].strip().startswith("#") or lines[n].find("DO NOT MODIFY! AUTOGENERATED FROM AN ACTION DEFINITION") != -1)): 
        if(lines[n].find("DO NOT MODIFY! AUTOGENERATED FROM AN ACTION DEFINITION") != -1): bAutoGen = True
        comment+=lines[n].replace("#"," ")
        n = n + 1

    if(lines[n].strip() == "" or bAutoGen):
        msgs+=[Msg("Header", "", comment)]
        comment = "" 
    else:
        msgs+=[Msg("Header", "", "")]

    for i in range(n, len(lines)):
        li = lines[i].strip()
        if(li == ""): continue
        if li.startswith("#"):
            comment+=li.replace("#"," ")
        if not li.startswith("#"):
            words = li.split()
            if(len(words) < 2): continue
            msgs+=[Msg(words[0], words[1], comment)]
            comment = ""
    return msgs

def BuildHtmlRawMsgTable(package):
    mystr = "<h1>" + package + "</h1>\n"
    msgs = run_command(['rosmsg','package', package])
#    folder = packagelist[package] + "/msg/"
    lines = msgs.split('\n') 
    for line in lines:
        if(line == ""): continue
        ref = line.replace("/","_")
        mystr+="<h2><A NAME=\"" + ref + "\">" + line + "</A></h2>\n" 
        mystr+="<table class=\"altrowstable\"\">\n"
        contents = run_command(['rosmsg','show', line]) # ReadFile(folder + MsgFilename(line) + ".msg")
        msgs = ParseMessages(contents)
        if(len(msgs) > 0): mystr+="<p>" + msgs[0].comment + "</p>"
        if(len(msgs) > 1):
            mystr+="<tr><th>Type</th><th> Name </th><th> Comment </th></tr>\n"            
        for i in range(1,len(msgs)):
            msg = msgs[i]
            localtypename = package + "/" + msg.typename
            localtypename = localtypename.replace("[","").replace("]","")
            if(msg.typename.find("/") != -1):
        os.environ["ROS_PACKAGE_PATH"]="/usr/local/michalos/nistcrcl_ws/src/fanuc_lrmate200id_support:/usr/local/michalos/nistcrcl_ws/src/nistcrcl:/usr/local/michalos/nistcrcl_ws/src/rawcrcl:/usr/local/michalos/nistcrcl_ws/src/testcrcl:/usr/local/michalos/nistcrcl_ws/src/testkin:/opt/ros/indigo/share:/opt/ros/indigo/stacks"
            ref = msg.typename.replace("/","_")
                ref = ref.replace("[","").replace("]","")
                mystr+="<tr><td><A HREF=\"#" + ref + "\">" + msg.typename + "</A></td><td>" + msg.name + "</td><td>" + msg.comment + "</td></tr>\n"
            elif localtypename in lines:
                localtypename = localtypename.replace("/","_")
                mystr+="<tr><td><A HREF=\"#" + localtypename + "\">" + msg.typename + "</A></td><td>" + msg.name + "</td><td>" + msg.comment + "</td></tr>\n"
            else:
                mystr+="<tr><td>" + msg.typename + "</td><td>" + msg.name + "</td><td>" + msg.comment + "</td></tr>\n"
        mystr+="</table>\n"
    return mystr

def BuildHtmlMsg(packages):
    mymystr = "<html><head>\n"
    mymystr+=StyleSheet()
    mymystr+="</head>\n<body>\n"
    for package in packages:
        print package
        mymystr+=BuildHtmlRawMsgTable(package)
    mymystr+="</body></html>"
    return mymystr
    
def BuildRawHtmlMsg(package):
    mymystr = "<html><head>\n"
    mymystr+=StyleSheet()
    mymystr+="</head>\n<body>\n"
    mymystr+=BuildHtmlRawMsgTable(package)
    mymystr+="</body></html>"
    return mymystr
    
# source /usr/local/michalos/nistcrcl_ws/devel/setup.bash
if __name__ == '__main__':
    print '============ Starting ros msg depends'
    #RosEnv.RosEnvironment('source /usr/local/dict[key]michalos/nistcrcl_ws/devel/setup.bash ; /bin/bash -i /usr/local/michalos/nistcrcl_ws/src/nistcrcl/scripts/setup.bash|sort')
    os.environ["ROS_PACKAGE_PATH"]="/usr/local/michalos/nistcrcl_ws/src/fanuc_lrmate200id_support:/usr/local/michalos/nistcrcl_ws/src/nistcrcl:/usr/local/michalos/nistcrcl_ws/src/rawcrcl:/usr/local/michalos/nistcrcl_ws/src/testcrcl:/usr/local/michalos/nistcrcl_ws/src/testkin:/opt/ros/indigo/share:/opt/ros/indigo/stacks"
    outfile = "Output.html"
    mystr=""
    try:
      opts, args = getopt.getopt(sys.argv[1:],"hp:aoi:",["package=","all","outfile="])
    except getopt.GetoptError:
        print 'usage: MessageHtmlWrapper.py [-p <packagename> | -a]'
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print 'usage: MessageHtmlWrapper.py {-p <packagename> | -a]'
            sys.exit()
        elif opt in ("-p", "--package"):
            print "-p=", arg
            #mystr= BuildHtmlMsg(["actionlib_msgs"])
            mystr = BuildRawHtmlMsg(arg)
        elif opt in ("-a", "--all"):
            mystr = BuildHtmlMsg(GetRosMsgDepends())
        elif opt in ("-o", "--outfile"):
            outfile = arg
    print "Argv ", sys.argv
    if(len(sys.argv)==1):  # no options
        mystr = BuildHtmlMsg(GetRosMsgDepends())

    print  GetRosMsgDepends()
    '''ls
    'actionlib_msgs', 'control_msgs', 'controller_manager_msgs', 'diagnostic_msgs', 'gazebo_msgs', 'geometry_msgs', 'household_objects_database_msgs', 'industrial_msgs', 'manipulation_msgs', 'map_msgs', 'moveit_msgs', 'nav_msgs', 'object_recognition_msgs', 'octomap_msgs', 'pcl_msgs', 'rosgraph_msgs', 'sensor_msgs', 'shape_msgs', 'smach_msgs', 'std_msgs', 'stereo_msgs', 'tf2_geometry_msgs', 'tf2_msgs', 'tf2_sensor_msgs', 'trajectory_msgs', 'visualization_msgs'
    '''
    if(len(mystr)>0):
        text_file = open(outfile, "w")
        text_file.write(mystr)
        text_file.close()