//****************************************************************************
//      Copyright (c) Microsoft Corporation. All rights reserved.
//      Licensed under the MIT license.
//
// File: SQLQuery.h
//
// Purpose:
//   This file contains the backend function declarations used to query
//   the SQL server.
//

#pragma once

#ifndef _SQL__QUERY_H_
#define _SQL__QUERY_H_

#define progName                        "sqlserverFS"
#define dbName                          "master"
#define MAX_COLUMN_ENTRY_LEN            32
#define SQLFS_MAX_LOGIN_TIMEOUT_SEC     3
#define SQLFS_MAX_RESPONSE_WAIT_SEC     5


// Current format of outputs supported from SQL Query
//
enum FileFormat 
{
    TYPE_TSV,
    TYPE_JSON
};

// This method executes the provided SQL query on the given server.
//
int ExecuteQuery(
    const string& query,
    string& output,
    const string& dbServer,
    const string& username,
    const string& password,
    const FileFormat type);

// This method checks if DB-Lib is able to connect with the given 
// credentials of the given IP address.
//
bool
VerifyServerInfo(
    string hostname,
    string username,
    string password);

#endif
