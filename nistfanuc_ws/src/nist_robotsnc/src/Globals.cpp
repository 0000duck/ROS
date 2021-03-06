// Globals.cpp

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
//#pragma message "Compiling " __FILE__ 

#include "Globals.h"
#include <map>
#include <iostream>
CGlobals Globals;
std::ofstream    ofsRobotURDF;
std::ofstream    ofsRobotMoveJoint;
std::ofstream    ofsRobotMoveTo;
std::ofstream    ofsScene;
std::ofstream    ofsMotionTrace;
std::ofstream    ofsRobotExercise;
std::ofstream    ofsIkFast;
void DebugBreak() {
    assert(0);
}

CGlobals::CGlobals()
{
    SocketPort = "64444";
    srand((unsigned int) time(NULL)); //activates the simple randome number generator

}

CGlobals::~CGlobals() {
    ofsRobotURDF.close();
    ofsRobotMoveJoint.close();
    ofsRobotMoveTo.close();
    ofsScene.close();
    ofsMotionTrace.close();
    ofsRobotExercise.close();
    ofsIkFast.close();
}
void CGlobals::DebugSetup()
{
    ofsRobotURDF.open(this->ExeDirectory+"RobotUrdf.log", std::ofstream::out);
    ofsRobotMoveJoint.open(this->ExeDirectory+"MoveJoint.log", std::ofstream::out);
    ofsScene.open(this->ExeDirectory+"Scene.log", std::ofstream::out);
    ofsMotionTrace.open(this->ExeDirectory + "Trace.log", std::ofstream::out);
    //ofsRobotMoveTo.open(this->ExeDirectory + "Moveto.log", std::ofstream::out);
    ofsRobotMoveTo.copyfmt(ofsMotionTrace); //1
    ofsRobotMoveTo.clear(ofsMotionTrace.rdstate()); //2
    ofsRobotMoveTo.basic_ios<char>::rdbuf(ofsMotionTrace.rdbuf());           //3
    ofsRobotExercise.open(this->ExeDirectory+"Exercise.log", std::ofstream::out);
    ofsIkFast.copyfmt(ofsRobotExercise);
    ofsIkFast.clear(ofsRobotExercise.rdstate()); //2
    ofsIkFast.basic_ios<char>::rdbuf(ofsRobotExercise.rdbuf());           //3
   
}
void CGlobals::AssignOfs(std::ostream *inOfs, std::ostream *replacementOfs)
{

    (*inOfs).copyfmt((*replacementOfs));
    (*inOfs).clear((*replacementOfs).rdstate()); //2
    (*inOfs).basic_ios<char>::rdbuf((*replacementOfs).rdbuf());           //3

}
static std::string  LeftTrim (std::string  str)
{
    size_t startpos = str.find_first_not_of(" \t\r\n");

    if ( std::string::npos != startpos )
    {
        str = str.substr(startpos);
    }
    return str;
}
// trim from end
static std::string  RightTrim (std::string  str, std::string trim = " \t\r\n")
{
    size_t endpos = str.find_last_not_of(trim);

    if ( std::string::npos != endpos )
    {
        str = str.substr(0, endpos + 1);
    }
    return str;
}
// trim from both ends
std::string  CGlobals::Trim (std::string  s)
{
    return LeftTrim(RightTrim(s) );
}
bool CGlobals::ReadFile (std::string filename, std::string & contents)
{
    std::ifstream     in(filename.c_str( ) );
    std::stringstream buffer;

    buffer << in.rdbuf( );
    contents = buffer.str( );
    return true;
}
void CGlobals::WriteFile (std::string filename, std::string & contents)
{
    std::ofstream outFile(filename.c_str( ) );

    outFile << contents.c_str( );
}
void CGlobals::AppendFile (std::string filename, std::string  contents)
{
    std::ofstream outFile;

    outFile.open(filename.c_str( ), std::ofstream::out | std::ofstream::app);
    outFile << contents.c_str( );
}
#ifdef _WINDOWS
#include "targetver.h"
#include "Windows.h"

// #include "StdStringFcn.h"
#include <Lmcons.h>

#define SECURITY_WIN32
#include "security.h"
#pragma comment(lib, "Secur32.lib")

#if 0
unsigned int CGlobals::ErrorMessage (std::string errmsg)
{
    OutputDebugString(errmsg.c_str( ) );
    std::cout << errmsg;
    return E_FAIL;
}
unsigned int CGlobals::DebugMessage (std::string errmsg)
{
    OutputDebugString(errmsg.c_str( ) );
    return E_FAIL;
}
#endif
// std::string CGlobals::ExeDirectory()
// { unsigned int CGlobals::DebugStrFormat(const char *fmt, ...) {
    va_list argptr;

    va_start(argptr, fmt);
    std::string str = FormatString(fmt, argptr);
    va_end(argptr);
    return -1; // FIXME: return DebugMessage( str);
}
//	TCHAR buf[1000];
//	GetModuleFileName(NULL, buf, 1000);
//	std::string path(buf);
//	path=path.substr( 0, path.find_last_of( '\\' ) +1 );
//	return path;
// }

#if 0
std::string CGlobals::GetUserName ( )
{
    TCHAR username[UNLEN + 1];
    DWORD size = UNLEN + 1;

    ::GetUserName( (TCHAR *) username, &size);
    return username;
}
std::string CGlobals::GetUserDomain ( )
{
    TCHAR username[UNLEN + 1];
    DWORD size = UNLEN + 1;

    // NameDnsDomain campus.nist.gov
    if ( GetUserNameEx(NameSamCompatible, (TCHAR *) username, &size) )
    {
        std::string domain = username;
        domain = domain.substr(0, domain.find_first_of('\\') );
        return domain;
    }
    return "";
}
#endif
std::string CGlobals::GetTimeStamp (TimeFormat format)
{ unsigned int CGlobals::DebugStrFormat(const char *fmt, ...) {
    va_list argptr;

    va_start(argptr, fmt);
    std::string str = FormatString(fmt, argptr);
    va_end(argptr);
    return -1; // FIXME: return DebugMessage( str);
}
    SYSTEMTIME st;
    char       timestamp[64];

    GetSystemTime(&st);
    sprintf(timestamp, "%4d-%02d-%02dT%02d:%02d:%02d", st.wYear, st.wMonth,
        st.wDay, st.wHour, st.wMinute, st.wSecond);

    if ( format == GMT_UV_SEC )
    {
        sprintf(timestamp + strlen(timestamp), ".%04dZ", st.wMilliseconds);
    }
    else
    {
        strcat(timestamp, "Z");
    }

    return timestamp;
}
#else
#if 0
unsigned int CGlobals::ErrorMessage (std::string errmsg)
{
    std::cout << errmsg;
    return -1;
}
unsigned int CGlobals::DebugMessage (std::string errmsg)
{
    std::cout << errmsg;
    return -1;
}
#endif
static inline std::string FormatString(const char *fmt, va_list ap) {
    int m, n = (int) strlen(fmt) + 1028;
    std::string tmp(n, '0');

    while ((m = vsnprintf(&tmp[0], n - 1, fmt, ap)) < 0) {
        n = n + 1028;
        tmp.resize(n, '0');
    }

    return tmp.substr(0, m);
}


std::string CGlobals::GetTimeStamp (TimeFormat format)
{
    char            timeBuffer[50];
    struct tm *     timeinfo;
    struct timeval  tv;
    struct timezone tz;

    gettimeofday(&tv, &tz);
    timeinfo = ( format == LOCAL ) ? localtime(&tv.tv_sec) : gmtime(&tv.tv_sec);

    switch ( format )
    {
    case HUM_READ:
        {
            strftime(timeBuffer, 50, "%a, %d %b %Y %H:%M:%S %Z", timeinfo);
        }
        break;

    case GMT:
        {
            strftime(timeBuffer, 50, "%Y-%m-%dT%H:%M:%SZ", timeinfo);
        }
        break;

    case GMT_UV_SEC:
        {
            strftime(timeBuffer, 50, "%Y-%m-%dT%H:%M:%S", timeinfo);
        }
        break;

    case LOCAL:
        {
            strftime(timeBuffer, 50, "%Y-%m-%dT%H:%M:%S%z", timeinfo);
        }
        break;
    }

//    if ( format == GMT_UV_SEC )
//    {
//        sprintf(timeBuffer + strlen(timeBuffer), ".%06dZ", tv.tv_usec);
//    }

    return std::string(timeBuffer);

}
#endif
