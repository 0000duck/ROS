// Globals.h

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
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include <sstream>

#include <boost/thread.hpp>
#include <boost/preprocessor.hpp>

///#include "Logging.h"
//extern ALogger LogFile;

// #ifdef DUMPCANON_STATUSREPLYCRCLJOINTS
// #ifdef HEAVYDEBUG
//#ifdef DEBUGJOINTSTATUSCRCLUPDATE

#ifndef ROSPACKAGENAME
#define ROSPACKAGENAME "nist_robotsnc"
#endif

#ifndef TODO
#define DO_PRAGMA(x) _Pragma (#x)
          #define TODO(x) DO_PRAGMA(message ("TODO - " #x))
#endif      

/** \def TOOLS_DEPRECATED
    Macro that marks functions as deprecated -from ROS */

#ifdef __GNUC__
#define TOOLS_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define TOOLS_DEPRECATED __declspec(deprecated)
#elif defined(__clang__)
#define TOOLS_DEPRECATED __attribute__((deprecated("Use of this method is deprecated")))
#else
#define TOOLS_DEPRECATED /* Nothing */
#endif


#if defined DEBUG
#define IfDebug(arg)    arg
#else
#define IfDebug(arg)
#endif

#ifdef WIN32
#define strncasecmp(x, y, z)    _strnicmp(x, y, z)
#else
#define _strnicmp strncasecmp
#define S_OK 0
#define E_FAIL -1
#endif


// Adapter from  IKFAST
#ifndef NC_ASSERT
#include <stdexcept>
#include <sstream>
#include <iostream>
#ifndef __PRETTY_FUNCTION__
#define __PRETTY_FUNCTION__ __func__
#endif
#define NC_ASSERT(b) { if( !(b) ) { std::stringstream ss; ss << "nc exception: " << __FILE__ << ":" << __LINE__ << ": " <<__PRETTY_FUNCTION__ << ": Assertion '" << #b << "' failed"; throw std::runtime_error(ss.str()); } }
#endif



#ifndef CLEANSTORE
#define CLEANSTORE(Y, X, Z) \
	try{ Y = X; }           \
	catch ( ... ) { Y = Z; }
#define VALIDSTORE(Y, X) \
	try{ Y = X; }        \
	catch ( ... ) { }
#endif

// Up to user to mutex these accessor functions
#ifndef VAR
#define VAR(X, Y)    \
protected: Y _ ## X; \
public: Y & X( ) { return _ ## X; }

#define VARREF(X, Y)    \
protected: Y &_ ## X; \
public: Y & X( ) { return _ ## X; }

#define RVAR(X, Y)    \
protected: Y _ ## X; \
public: Y  X( ) { return _ ## X; }

#define RWVAR(X, Y)    \
protected: Y _ ## X; \
public: Y & X( ) { return _ ## X; }

#define NVAR(X, Y, Z) \
protected: Y Z;       \
public: Y & X( ) { return Z; }
#endif

#ifndef FOREACH
#define FOREACH(it, v) for(typeof((v).begin()) it = (v).begin(); it != (v).end(); it++)
#endif


#define X_DEFINE_ENUM_WITH_STRING_CONVERSIONS_TOSTRING_CASE(r, data, elem)    \
    case elem : return BOOST_PP_STRINGIZE(elem);

#define DEFINE_ENUM_WITH_STRING_CONVERSIONS(name, enumerators)                \
    enum name {                                                               \
        BOOST_PP_SEQ_ENUM(enumerators)                                        \
    };                                                                        \
                                                                              \
    inline const char* ToString(name v)                                       \
    {                                                                         \
        switch (v)                                                            \
        {                                                                     \
            BOOST_PP_SEQ_FOR_EACH(                                            \
                X_DEFINE_ENUM_WITH_STRING_CONVERSIONS_TOSTRING_CASE,          \
                name,                                                         \
                enumerators                                                   \
            )                                                                 \
            default: return "[Unknown " BOOST_PP_STRINGIZE(name) "]";         \
        }                                                                     \
    }

/**
 * \brief gives a random from 0 to 1
 * \return double
 */
inline double Random() {
    return ((double) rand() / (RAND_MAX));
}

inline int OneZero() {
    return ((double) rand() / (RAND_MAX));
}

/**
 * \brief gives a random between min and max
 * \param min minimum number as double
 * \param max maximum number as double
 * \return number as double between min and max
 */
inline double Random(double min, double max) {
    return min + Random() * (max - min); //gives a random from 0 to max ) +min
}

/**
 * \brief gives a random between min and max
 * \param min minimum number as integer
 * \param max maximum number as integer
 * \return number as integer between min and max
 */
inline int Random(int min, int max) {
    return min + (rand() % (max - min)); //gives a random from 0 to max ) +min
}

/**
 * \brief CGlobals is a catch-all data structure for collecting global functions, extensions, parameters, etc.
 *  Functions here usually vary between windows and linux, or
 * there is no easy mechanism in C++ to extend classes (e.g., string) like in C#
 */
class CGlobals {
public:

    enum TimeFormat {
        HUM_READ,
        GMT,
        GMT_UV_SEC,
        LOCAL
    };
    /*!
     * \brief Constructor for globals function. Functions here usually vary between windows and linux, or
     * there is no easy mechanism in C++ to extend classes (e.g., string) like in C#.
     */
    CGlobals();
    ~CGlobals();

    /*!
     * \brief StrFormat  accepts a traditional C format string and expects parameter to follow on calling stack and will
     * produce a string from it.
     * \param fmt is the C format string.
     */
    std::string StrFormat(const char *fmt, ...) {
        va_list argptr;
        va_start(argptr, fmt);
        int m;
        int n = (int) strlen(fmt) + 1028;
        std::string tmp(n, '0');
        while ((m = vsnprintf(&tmp[0], n - 1, fmt, argptr)) < 0) {
            n = n + 1028;
            tmp.resize(n, '0');
        }
        va_end(argptr);
        return tmp.substr(0, m);
    }

    /*!
     * \brief sleep milliseconds. Equivalent to Sleep in windows.
     * \param ms number of milliseconds to sleep
     */
    void Sleep(unsigned int ms) {
        boost::this_thread::sleep(boost::posix_time::milliseconds(ms));
    }
    /*!
     * \brief Reads a file all at once into a string. Include file open, read, close. If fails, empty string is only diagnostic.
     * \param filename is the name of the file to read from
     * \param contents is a reference to a string in which to store file contents.
     */
    bool ReadFile(std::string filename, std::string & contents);

    /*!
     * \brief Writes entire string contents to a file all at once. Include file open, write, close.
     * No error messages.
     * \param filename is the name of the file to write to
     * \param contents is a reference to a string in which to write string.
     */
    void WriteFile(std::string filename, std::string & contents);

    /*!
     * \brief Appends entire string contents to a file all at once. Include file open, write, close.
     * No error messages.
     * \param filename is the name of the file to write to
     * \param contents is a reference to a string in which to write string.
     */
    void AppendFile(std::string filename, std::string contents);

    /*!
     * \brief Trim cleans blank characters from the front and back of a string. 
     * Blank chars are white space, tab, carriage return.
     * \param str is the string to trim. Will trim a copy.
     * \return a new trimmed string 
     */
    std::string Trim(std::string s);
#if 0

    /*!
     * \brief Prints an diagnostic message to the debug reporting mechanism. (cout or OutputDebugString)
     * \param str errmsg is the error message that is posted to the debug reporting mechanism.
     * \return a error result integer. (e.g., E_FAIL or -1). 
     */
    unsigned int DebugMessage(std::string errmsg);
    /*!
     * \brief Prints an error message to the error reporting mechanism. 
     * \param str errmsg is the error message that is posted to the error reporting mechanism.
     * \return a error result integer. (e.g., E_FAIL or -1). 
     */
    unsigned int ErrorMessage(std::string errmsg);
#endif
    /*!
     * \brief Prints a format string and arguments as a diagnostic message to the debug reporting mechanism. (cout or OutputDebugString)
     * \param  fmt is the error format statement that uses parameters that follow and is posted to the debug reporting mechanism.
     * \return a error result integer. (e.g., E_FAIL or -1). 
     */
    //unsigned int  DebugStrFormat (const char *fmt, ...);

    /*!
     * \brief GetTimeStamp returns a timestamp string depending on the input format.
     * \param  format is one of an enumeration describing how to format timestamp.
     * \return a formated timestamp string. 
     */
    std::string GetTimeStamp(TimeFormat format = GMT_UV_SEC);

    /*!
     * \brief Open logging streams for debugging. Close on CGlobals destroctor.
     */
    void DebugSetup();
    /*!
     * \brief Assign a new output file stream to a file stream.
     */    
    void AssignOfs(std::ostream *inOfs, std::ostream *replacementOfs);
    // -----------------------------------------
    // Application specifie parameters
    std::map< std::string, std::string> _appproperties; /**<map of application properties, e.g., ["prop"]="value" */
    std::string ExeDirectory; /**< the path to directory where exe is located  */
    std::string inifile; /**< inifile path name  */
    std::string SocketPort; /**< socket port to listen for Crcl clients  */
};
extern CGlobals Globals; /**< global definition of globals  */
extern void DebugBreak(); /**< global definition of windows DebugBreak equivalent.  */


extern std::ofstream ofsRobotURDF;
extern std::ofstream ofsRobotMoveJoint;
extern std::ofstream ofsRobotMoveTo;
extern std::ofstream ofsScene;
extern std::ofstream ofsMotionTrace;
extern std::ofstream ofsRobotExercise;
extern std::ofstream ofsIkFast;
