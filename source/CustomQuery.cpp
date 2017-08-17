//****************************************************************************
//      Copyright (c) Microsoft Corporation. All rights reserved.
//      Licensed under the MIT license.
//
// File: CustomQuery.cpp
//
// Purpose:
//   This file contains definitions of functions used to enable custom
//   query support.
//
#include "UtilsPrivate.h"

// ---------------------------------------------------------------------------
// Method: ExecuteCustomQuery
//
// Description:
//  This method reads the query from queryFilePath and puts the output 
//  in queryResultPath.
//
//  queryFilePath - absolute path to a file that contains query.
//  queryResultPath - absolute path ot a file that is used to store query output.
//
// Returns:
//    none.
//
void
ExecuteCustomQuery(
    const string& queryFilePath,
    const string& queryResultPath,
    const string& hostname,
    const string& username,
    const string& password)
{
    string responseString;

    // Read the query
    //
    ifstream ifs(queryFilePath);
    string query((std::istreambuf_iterator<char>(ifs)),
                        (std::istreambuf_iterator<char>()));
    // Execute the query.
    //
    // We want the column names as well so use type as TYPE_TSV.
    //
    int result = ExecuteQuery(query, responseString, hostname,
                              username, password, TYPE_TSV);

    if (!result)
    {
        // Open file to write response.
        //
        int fd = open(queryResultPath.c_str(), O_RDWR);
        if (fd == -1)
        {
            ReturnErrnoAndPrintError(__FUNCTION__, "open failed");
        }
        else
        {
            // Write in the response.
            //
            result = pwrite(fd, responseString.c_str(), responseString.length(), 0);
            if (result == -1)
            {
                result = ReturnErrnoAndPrintError(__FUNCTION__, "pwrite failed");
            }

            // Close fd after write.
            //
            result = close(fd);
            if (result == -1)
            {
                result = ReturnErrnoAndPrintError(__FUNCTION__, "close failed");
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Method: RemoveCustomQueriesOutputFiles
//
// Description:
//  Remove all the files under custom query dump directory.
//
//  dumpPath - absolute path to a custom query dump directory.
//
// Returns:
//    none.
//
void 
RemoveCustomQueriesOutputFiles(
    DIR* dp,
    const string& dumpPath)
{
    struct dirent*  de;
    string          filepath;

    // Remove old files from custom query dump directory
    //
    while ((de = readdir(dp)) != NULL)
    {
        if (de->d_type == DT_REG)
        {
            filepath = StringFormat("%s/%s", dumpPath.c_str(), de->d_name);

            // Do the best to remove the file
            //
            (void) remove(filepath.c_str());
        }
    }
}

// ---------------------------------------------------------------------------
// Method: CreateCustomQueriesOutputFiles
//
// Description:
//  Create 
//
//  dumpPath - absolute path to a custom query dump directory.
//
// Returns:
//    none.
//
void 
CreateCustomQueriesOutputFiles(
    const string& servername,
    const string& dumpPath)
{
    DIR*            userQueriesDir;
    string          userQueriesPath;
    string          filepath;
    struct dirent*  de;

    userQueriesPath = GetUserCustomQueryPath(servername);
    if (!userQueriesPath.empty())
    {
        // Open user customQueries dir 
        //
        userQueriesDir = opendir(userQueriesPath.c_str());
        if (userQueriesDir)
        {
            // Read all files in customQueries dir
            //
            while((de = readdir(userQueriesDir)) != NULL)
            {
                if (de->d_type == DT_REG)
                {
                    // Create new files with the same name in
                    // the dump directory for storing results
                    //
                    filepath = StringFormat("%s/%s", dumpPath.c_str(), de->d_name);
                    CreateFile(filepath.c_str());
                }
            }
            closedir(userQueriesDir);
        }
    }
}
