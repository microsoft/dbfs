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

    // This is a canonicalized path to user custom queries directory
    // that was specified in the config file.
    //
    string m_customQueriesPath;

    // Sql server version. The version number is used to determine
    // if JSON output is supported. Value for SQL Server 2016 is [16].
    // [16] is the minimum version required for JSON output.
    //
    int m_version;
};

int StartFuse(char* ProgramName);
