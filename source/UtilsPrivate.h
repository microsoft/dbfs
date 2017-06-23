//****************************************************************************
//      Copyright (c) Microsoft Corporation. All rights reserved.
//      Licensed under the MIT license.
//
// File: UtilsPrivate.h
//
// Purpose:
//  This just includes the necessary header files needed by the utility
//  functions and the has the USING directives to include specific STL
//  types.
//
#pragma once

#ifndef _UTILS_PRIVATE_
#define _UTILS_PRIVATE_ 

// ---------------------------------------------------------------------------
// C++ STL Headers
//
#include <algorithm>
#include <atomic>
#include <chrono>
#include <codecvt>
#include <condition_variable>
#include <deque>
#include <exception>
#include <fstream>
#include <iostream>
#include <list>
#include <locale>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <iterator>

// ---------------------------------------------------------------------------
// Using directives to include specific STL types
//
using std::codecvt_utf8;
using std::exception;
using std::get;
using std::invalid_argument;
using std::ifstream;
using std::istream;
using std::list;
using std::make_tuple;
using std::make_unique;
using std::map;
using std::max;
using std::min;
using std::ostream_iterator;
using std::pair;
using std::runtime_error;
using std::set;
using std::shared_ptr;
using std::stack;
using std::string;
using std::stringstream;
using std::thread;
using std::tuple;
using std::u16string;
using std::unique_ptr;
using std::unordered_map;
using std::unordered_multimap;
using std::unordered_multiset;
using std::vector;
using std::wstring_convert;
using std::shared_ptr;
using std::make_shared;
using std::unique_ptr;
using std::make_unique;
using std::weak_ptr;
using std::enable_shared_from_this;
using std::static_pointer_cast;
using std::ostringstream;
using std::to_string;
using std::ios;
using std::cin;

// ---------------------------------------------------------------------------
// C Runtime Headers
//
#include <sys/syscall.h>
#include <aio.h>
#include <arpa/inet.h>
#include <asm/prctl.h>
#include <assert.h>
#include <cxxabi.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <libgen.h>
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/eventfd.h>
#include <sys/uio.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/signalfd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <ucontext.h>
#include <netdb.h>
#include <linux/aio_abi.h>
#include <fuse.h>
#include <attr/xattr.h>
#include <dirent.h>
#include <ctype.h>
#include <limits.h>
#include <unistd.h>
#include <sys/param.h>
#include <sybfront.h>
#include <sybdb.h>
#include <syberror.h>
#include <termios.h>
#include <cstddef>

// ---------------------------------------------------------------------------
// Local headers of utility files
//
#include "StringUtils.h"
#include "sqlfs.h"
#include "SQLQuery.h"
#include "helper.h"
#include "INIFile.h"
#include "ParseException.h"

// Common symbols needed by all files.
//
extern struct SQLFsPaths g_UserPaths;
extern bool g_InVerbose;
extern unordered_map<string, class ServerInfo*> g_ServerInfoMap;
extern bool g_UseLogFile;
extern bool g_RunInForeground;
extern char g_LocallyGeneratedFiles[];

#endif
