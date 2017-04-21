//****************************************************************************
//      Copyright (c) Microsoft Corporation. All rights reserved.
//      Licensed under the MIT license.
//
// File: sqlfs.h
//
// Purpose:
//   This file contains the declarations of various structures
//   used by the SQL FS to get server information and paths.
//

#pragma once

#ifndef _SQL_FS_H_
#define _SQL_FS_H_

#define MAX_ARGS                8

// Structure to track entries of various paths and configuration file
//
struct SQLFsPaths 
{
    string m_mountPath;
    string m_dumpPath;
    string m_confPath;
    string m_logfilePath;
};

// Structure used to track information for a server.
//
class ServerInfo 
{

public:
    string m_hostname;
    string m_username;
    string m_password;
    int m_version;
};

int StartFuse(char* ProgramName);

#endif
