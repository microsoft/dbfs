//****************************************************************************
//      Copyright (c) Microsoft Corporation. All rights reserved.
//      Licensed under the MIT license.
//
// File: helper.c
//
// Purpose:
//   This file contains the definitions of helper functions used by
//   sqlfs and SQLQuery.
//

#include "UtilsPrivate.h"

extern struct SQLFsPaths g_UserPaths;
extern bool g_InVerbose;
extern unordered_map<string, class ServerInfo*> g_ServerInfoMap;
extern bool g_UseLogFile;

// ---------------------------------------------------------------------------
// Method: CalculateDumpPath
//
// Description:
//    This method concatenates the dump directory path to the provided 
//    relative path.
//
//    FUSE always gets paths relative to the mount directory.
//
// Returns:
//    VOID
//
std::string
CalculateDumpPath(
    string path)
{
    return(g_UserPaths.m_dumpPath + path);
}

// ---------------------------------------------------------------------------
// Method: ReturnErrnoAndPrintError
//
// Description:
//    This method prints the ERRNO error string along with the function that 
//    caused the error and a custom string that is passed.
//
// Returns:
//    -errno. Fuse always returns -errno.
//
int
ReturnErrnoAndPrintError(
    const char* func,
    std::string error_str)
{
    int     result;
    int     status;
    FILE*   outFile;

    result = -errno;
    (void)outFile;      //To suppress warning
    
    if (g_InVerbose)
    {
        outFile = stderr;
        status = SUCCESS;

        if (g_UseLogFile == true)
        {
            outFile = fopen(g_UserPaths.m_logfilePath.c_str(), "a");
            if (!outFile)
            {
                status = FAILURE;
            }
        }

        if (status == SUCCESS)
        {
            fprintf(outFile, "SQLFS Error in %s :: Reason - %s, Details - %s\n",
                func, error_str.c_str(), strerror(errno));

            if (outFile != stderr)
            {
                fclose(outFile);
            }
        }
    }

    return result;
}

// ---------------------------------------------------------------------------
// Method: PrintError
//
// Description:
//    This method prints error message provided if verbose mode is enabled 
//    either on STDERR or the log file depending on whether the log file
//    paramater was passed at startup.
//
// Returns:
//    VOID
//
void
PrintMsg(const char* format, ...)
{
    FILE*   outFile;
    va_list argptr;
    int     status;

    (void)outFile;      //To suppress warning

    if (g_InVerbose)
    {
        outFile = stderr;
        status = SUCCESS;

        if (g_UseLogFile == true)
        {
            outFile = fopen(g_UserPaths.m_logfilePath.c_str(), "a");
            if (!outFile)
            {
                status = FAILURE;
            }
        }

        if (status == SUCCESS)
        {
            va_start(argptr, format);
            vfprintf(outFile, format, argptr);
            va_end(argptr);

            if (outFile != stderr)
            {
                fclose(outFile);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Method: GetServerDetails
//
// Description:
//    This method is a helper function used to get the details like 
//    hostname/IP, username and password for a given server name.
//
//    It searches the in-memory struct ServerInfo for this information.
//
//    The list is constant post initialization and read opeations will not
//    require synchronization primitives.
//
// Returns:
//    VOID
//
void
GetServerDetails(
    string servername,
    string& hostname,
    string& username,
    string& password)
{
    auto search = g_ServerInfoMap.find(servername);
    
    if (search != g_ServerInfoMap.end())
    {
        hostname = search->second->m_hostname;
        username = search->second->m_username;
        password = search->second->m_password;
    }
    else
    {
        PrintMsg("Unknown server %s\n", servername.c_str());
        KillSelf();
    }
}


// ---------------------------------------------------------------------------
// Method: CreateFile
//
// Description:
//    This method creates a file with write permissions. Or truncates to zero
//    if one already exists.
//
//    Note - Absolute path need to be provided to the function.
//
// Returns:
//    VOID
//
void
CreateFile(
    const char* path)
{
    FILE*   fp;

    fp = fopen(path, "w+");
    if (!fp) 
    {
        PrintMsg("Error creating file %s %s\n", path, strerror(errno));
        KillSelf();
    }
    else
    {
        fclose(fp);
    }
}

// ---------------------------------------------------------------------------
// Method: CreateDMVFiles
//
// Description:
//    This method creates the empty DMV files for a given server.
//    The location of the files (as seen) is <MOUNT DIR>/<SERVER NAME>/. 
//    Of course the files are actually getting created in the dump directory.
//
//    The method requests the server for the list of DMV's and based on the
//    version of the server - may or may not create the .json files. 
//    Only for SQL Server 2016 (version 16) does the method create the .json.
//
//    This only happens at startup so no issue with synchronization.
//
// Returns:
//    VOID
//
void
CreateDMVFiles(
    string servername,
    string hostname,
    string username,
    string password,
    int version)
{
    string          dmvQuery;
    string          fpath;
    string          responseString;
    int             result;
    vector<string>  filenames;
    int             numEntries;

    fpath = CalculateDumpPath(servername);

    // Creating folder for this server's data.
    //
    result = mkdir(fpath.c_str(), DEFAULT_PERMISSIONS);
    if (result)
    {
        PrintMsg("mkdir failed for %s- %s\n", fpath.c_str(), 
            strerror(errno));
    }

    if (!result)
    {
        // Query SQL server for all the DMV files to be created.
        //
        dmvQuery = "SELECT name from sys.system_views";
        result = ExecuteQuery(dmvQuery.c_str(), responseString, hostname,
            username, password, TYPE_TSV);
        if (result)
        {
            PrintMsg("ExecuteQuery failed\n");
        }
    }

    if (!result)
    {
        // Tokenising response to extract DMV names.
        //
        filenames = Split(responseString, '\n');

        // On success, it will have at least two entries.
        numEntries = filenames.size();
        assert(numEntries > 1);

        // We need to skip the first name because the result of the SQL Query
        // includes the name of the column as well in the output.
        // We do not want to create a file corresponding to the column name.
        //
        for (int i = 0; i < numEntries; i++)
        {
            if (i == 0)
            {
                continue;
            }

            // Create the regular file - TSV.
            //
            fpath = StringFormat("%s/%s/%s", g_UserPaths.m_dumpPath.c_str(), servername.c_str(), filenames[i].c_str());
            CreateFile(fpath.c_str());

            if (version >= 16)
            {
                // Creating the json file.
                //
                fpath = StringFormat("%s.json", fpath.c_str());
                CreateFile(fpath.c_str());
            }
        }
    }
    else
    {
        PrintMsg("There was an error creating the folders to hold the server DMV files. Exiting.\n");

        // Abort in case of any error.
        //
        KillSelf();
    }
}

// ---------------------------------------------------------------------------
// Method: KillSelf
//
// Description:
//    This method exits the program and in doing so the function DestroySQLFs
//    is called. This ensures a graceful shutdown of the program which
//    also ensures that the mount directory is unmounted at exit.
//
//    We are basically trying to leave the system back in the same state as
//    before the SQL Fs was stated. 
//
// Returns:
//    -1.
//
void
KillSelf()
{
    // Close SQLFs
    //
    kill(getpid(), SIGHUP);
}

