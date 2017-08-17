//****************************************************************************
//      Copyright (c) Microsoft Corporation. All rights reserved.
//      Licensed under the MIT license.
//
// File: CustomQuery.h
//
// Purpose:
//   This file contains declarations of functions used to enable custom
//   query support.
//
#pragma once

// Name of folder in which user needs to create custom query.
//
#define CUSTOM_QUERY_FOLDER_NAME                    "customQueries"

// Execute a user custom query
//
void
ExecuteCustomQuery(
    const string& queryFilePath,
    const string& queryResultPath,
    const string& hostname,
    const string& username,
    const string& password);

// Remove all the output files in custom query dump directory.
//
void RemoveCustomQueriesOutputFiles(
    DIR* dp,
    const string& dumpPath);

// Create output files in custom query dump directory.
//
void CreateCustomQueriesOutputFiles(
    const string& servername,
    const string& dumpPath);