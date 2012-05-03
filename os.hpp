/*
 MurkMUD++ - A Windows compatible, C++ compatible Merc 2.2 Mud.

 \author Jon A. Lambert
 \date 08/30/2006
 \version 1.4
 \remarks
  This source code copyright (C) 2005, 2006 by Jon A. Lambert
  All rights reserved.

  Use governed by the MurkMUD++ public license found in license.murk++
*/

#ifndef OS_HPP
#define OS_HPP

/*-----------------------------------------------------------------------*/
/* OS DEPENDENT INCLUDES AND DEFINITIONS                                 */
/*-----------------------------------------------------------------------*/
// c++ standard library
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cctype>
#include <cerrno>
#include <csignal>
#include <ctime>

#include <fcntl.h>
#include <sys/types.h>
#include "sqlite3/sqlite3.h"

#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <list>
#include <map>
//#include "boost/format.hpp"

// replacement or substitute for itoa
std::string itoa(int value, int base);

/*-----------------------------------------------------------------------*/
/* WINDOWS DEFINITIONS SECTION                                           */
/*-----------------------------------------------------------------------*/
#ifdef WIN32                    /* Windows portability */

#ifdef __BORLANDC__
using std::snprintf;
using std::vsnprintf;
#endif

#if defined _MSC_VER || defined __DMC__
#define NOMINMAX
#endif
#define FD_SETSIZE 1024
#include <winsock2.h>

#define GETERROR     WSAGetLastError()
#define WIN32STARTUP \
    { \
      WSADATA wsaData; \
      int err = WSAStartup(0x202,&wsaData); \
      if (err) \
        std::cerr << "Error(WSAStartup):" << err << std::endl; \
    }
#define WIN32CLEANUP WSACleanup();

#undef EWOULDBLOCK
#define EWOULDBLOCK       WSAEWOULDBLOCK

#define OS_RAND std::rand
#define OS_SRAND std::srand

#if defined _MSC_VER
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#endif

#if defined __DMC__
#define snprintf _snprintf
#endif

void gettimeofday (struct timeval *tp, struct timezone *tzp);
char *crypt (char *pw, const char *salt);

/*-----------------------------------------------------------------------*/
/* UNIX DEFINITION SECTION                                               */
/*-----------------------------------------------------------------------*/
#else /* Unix portability - some a consequence of above */

#include <sys/time.h>           /* Redhat and BSD need this */
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#ifndef __FreeBSD__
#include <crypt.h>
#endif

#define GETERROR  errno
#define INVALID_SOCKET -1       /* 0 on Windows */
#define SOCKET_ERROR -1
#define SOCKET int
#define WIN32STARTUP
#define WIN32CLEANUP
#define OS_RAND rand
#define OS_SRAND srand
#define closesocket(X) close(X)
#endif

#endif // OS_HPP

