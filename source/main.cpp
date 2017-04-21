//****************************************************************************
//      Copyright (c) Microsoft Corporation. All rights reserved.
//      Licensed under the MIT license.
//
// File: main.cpp
//
// Purpose:
//   This file contains definitions of functions used to parse the user
//   input arguments and, upon validation, start the FUSE module.
//

#include "UtilsPrivate.h"

// Global variable used to track entries of various paths and 
//configuration file
//
struct SQLFsPaths g_UserPaths;

// Global variable used to determine if in verbose mode;
//
bool g_InVerbose;

// Global map used to track information for all the servers
//
std::unordered_map<std::string, class ServerInfo*> g_ServerInfoMap;

// Global variable used to determine if logfile path was givem;
//
bool g_UseLogFile;

// Global variable used to track if DBFS needs to be started in foreground
//
bool g_RunInForeground;

// ---------------------------------------------------------------------------
// Method: PrintUsageAndExit
//
// Description:
//    This method prints the intended usage of the program and exits.
//
// Returns:
//    exits with -EINVAL
//
static void
PrintUsageAndExit(
    char* command, 
    FILE* out)
{
    fprintf(out,
        "Usage: %s [Options]\n"
        "Options:\n"
        "   -m/--mount-path     :  The mount directory for SQL server(s) DMV files [REQUIRED]\n"
        "   -c/--conf-file      :  Location of .conf file. [REQUIED]\n"
        "   -d/--dump-path      :  The dump directory used. Default = \"/tmp/sqlserver\" [OPTIONAL]\n"
        "   -v/--verbose        :  Start in verbose mode [OPTIONAL]\n"
        "   -l/--log-file       :  Path to the log file (only used if in verbose mode) [OPTIONAL]\n"
        "   -f                  :  Run DBFS in foreground [OPTIONAL]\n"
        "   -h                  :  Print usage"
        "\n", command);
    exit(-EINVAL);
}

// ---------------------------------------------------------------------------
// Options that are accepted from the user at startup.
//
static struct option long_options[] =
{
    { "mount-path",         required_argument,          0,  'm' },
    { "conf-file",          required_argument,          0,  'c' },
    { "dump-path",          required_argument,          0,  'd' },
    { "verbose",            required_argument,          0,  'v' },
    { "log-file",           required_argument,          0,  'l' },
    { 0,                    0,                          0,   0 }
};

// ---------------------------------------------------------------------------
// Method: GenerateFileName
//
// Description:
//    This method creates a file named based on the current time. 
//    Format: sqfs_<time>
//    It then populates the string variable passed with the generated name.
//
// Returns:
//    bool
//
bool
GenerateFileName(
    string& fileName)
{
    bool status;
    time_t currentTime;

    status = true;

    // Get current time in epoch sec
    //
    currentTime = time(NULL);
    if (currentTime < 0)
    {
        fprintf(stderr, "ERROR - Internal error - time() call failed\n");
        status = false;
    }

    if (status)
    {
        fileName = "sqlfs_" + to_string(currentTime);
    }

    return status;
}

// ---------------------------------------------------------------------------
// Method: ParseArguments
//
// Description:
//    This method parses the required and optional arguments entered by the
//    user. 
//    
//    mount-point is mandatory. All others are optional.
//
// Returns:
//    VOID if no error.
//    On error, show usage and exits with -EINVAL.
//
static void
ParseArguments(
    int argc,
    char* argv[])
{
    int option;
    int idx;
    bool mountSet;
    bool confSet;
    bool status;
    string dumpDirPath;
    char* tempPtr;

    status = false;

    // Generate name of dump dir
    status = GenerateFileName(dumpDirPath);
    if (status)
    {
        // Default values
        g_UserPaths.m_dumpPath = "/tmp/" + dumpDirPath + "/";

        mountSet = false;
        confSet = false;
    }

    // Parsing
    while (status)
    {
        idx = 0;
        option = getopt_long(argc, argv, "m:c:d:hvfl:", long_options, &idx);

        if (option == -1)
        {
            // End of argument options
            break;
        }

        switch (option)
        {
        case 'h':
            status = false;
            break;

        case 'm':
            tempPtr = realpath(optarg, NULL);
            if (tempPtr)
            {
                g_UserPaths.m_mountPath = tempPtr;
                free(tempPtr);
                mountSet = true;
            }
            break;

        case 'c':
            tempPtr = realpath(optarg, NULL);
            if (tempPtr)
            {
                g_UserPaths.m_confPath = tempPtr;
                free(tempPtr);
                confSet = true;
            }
            break;

        case 'd':
            tempPtr = realpath(optarg, NULL);
            if (tempPtr)
            {
                g_UserPaths.m_dumpPath = tempPtr;
                free(tempPtr);
            }
            break;

        case 'v':
            g_InVerbose = true;
            break;
            
        case 'f':
            g_RunInForeground = true;
            break;

        case 'l':
            tempPtr = realpath(optarg, NULL);
            if (tempPtr)
            {
                g_UserPaths.m_logfilePath = tempPtr;
                free(tempPtr);
                g_UseLogFile = true;
            }
            break;

        default:
            fprintf(stderr, "ERROR - Unknown argument passed - %c\n", option);
            status = false;
        }
    }

    if (status)
    {
        // Mount path must be provided by the user.
        if (!mountSet)
        {
            fprintf(stderr, "###### Enter mount directory\n");
            status = false;
        }

        // Config file path must be provided by the user.
        if (!confSet)
        {
            fprintf(stderr, "###### Enter conf file path\n");
            status = false;
        }
    }

    if (!status)
    {
        PrintUsageAndExit(argv[0], stderr);
    }
}

// ---------------------------------------------------------------------------
// Method: CheckIfDirectoryExists
//
// Description:
//    This method if the provided path belong to an existing directory.
//
// Returns:
//    bool
//
static bool
CheckIfDirectoryExists(
    const char* path)
{
    struct stat statbuf;
    int result;
    bool status;

    status = false;

    result = stat(path, &statbuf);
    if (result)
    {
        if (errno != ENOENT)
        {
            fprintf(stderr, "STAT failed for %s\n", path);
        }
    }
    else if ((S_ISDIR(statbuf.st_mode)))
    {
        status = true;
    }
    else
    {
        fprintf(stderr, "%s exists but not a directory\n", path);
    }

    return status;
}

// ---------------------------------------------------------------------------
// Method: CheckIfFileExists
//
// Description:
//    This method checks if the provided path belong to an existing 
//    regular file.
//
// Returns:
//    bool.
//
static bool
CheckIfFileExists(
    const char* path)
{
    struct stat statbuf;
    int         result;
    bool        status;

    status = false;

    result = stat(path, &statbuf);
    if (result)
    {
        if (errno == ENOENT)
        {
            fprintf(stderr, "%s does not exist\n", path);
        }
        else
        {
            fprintf(stderr, "STAT failed for %s\n", path);
        }
    }
    else if ((S_ISREG(statbuf.st_mode)))
    {
        status = true;
    }
    else
    {
        fprintf(stderr, "%s exists but not a regular file\n", path);
    }

    return status;
}

// ---------------------------------------------------------------------------
// Method: CheckAllSetPaths
//
// Description:
//    Checks if the set mount_point and the configuration file exist.
//
// Returns:
//    VOID
//
static void
CheckAllSetPaths(void)
{
    bool    status;

    // Dump path should not exist
    //
    status = CheckIfDirectoryExists(g_UserPaths.m_dumpPath.c_str());
    if (status)
    {
        fprintf(stderr, "Dump direcotry '%s' should not exist at startup\n", 
            g_UserPaths.m_dumpPath.c_str());
        assert(0);
    }

    assert(CheckIfDirectoryExists(g_UserPaths.m_mountPath.c_str()));
    assert(CheckIfFileExists(g_UserPaths.m_confPath.c_str()));
}

// ---------------------------------------------------------------------------
// Method: ParseSectionEntry
//
// Description:
//    This method is used to find a given entryName (name,value) pair in the
//    provided map and fill in the provided string accordingly.
//
// Returns:
//    bool
//
static bool
ParseSectionEntry(
    map<std::string, SectionNameValuePair>::iterator sectionItr,
    string entryName,
    string& entryValue)
{
    bool status;
    auto&& entryItr = sectionItr->second.begin();

    status = true;

    entryItr = sectionItr->second.find(entryName.c_str());
    if (entryItr != sectionItr->second.end())
    {
        entryValue = entryItr->second;
        if (entryValue.length() == 0)
        {
            fprintf(stderr, "No value provided for \"%s\".\n",
                entryName.c_str());
            status = false;
        }
    }
    else
    {
        fprintf(stderr, "No \"%s\" entry for section %s.\n",
            entryName.c_str(),
            sectionItr->first.c_str());

        // Clear the output string since no such section was found
        //
        entryValue.clear();

        status = false;
    }

    return status;
}

// ---------------------------------------------------------------------------
// Method: convertToInt
//
// Description:
//    This method interprets the integer value of the provided string.
//
// Returns:
//    bool
//
static bool
convertToInt(
    string str,
    int& intVal)
{
    bool status = true;

    try
    {
        intVal = stoi(str);
    }
    // stoi may throw std::invalid_argument or std::out_of_range
    //
    catch (exception& e)
    {
        PrintMsg("Unable to convert string to int. Exception: %s\n", e.what());
        status = false;
    }

    return status;
}

// ---------------------------------------------------------------------------
// Method: QueryUserForPassword
//
// Description:
//    This method prompts the user for password and stores it in memory.
//
// Returns:
//    bool
//
static bool
QueryUserForPassword(
    string servername,
    string& password)
{
    termios oldt;
    termios newt;
    int result;
    int status = false;

    // Hide user input from terminal
    //
    result = tcgetattr(STDIN_FILENO, &oldt);

    if (!result)
    {
        newt = oldt;
        newt.c_lflag &= ~ECHO;
        result = tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    }

    if (!result)
    {
        fprintf(stdout, "Enter password for server %s:", servername.c_str());
        cin >> password;

        status = true;

        // To move cursor to next line
        //
        fprintf(stdout, "\n");
    }

    // Reset to original configuration
    //
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    if (result)
    {
        fprintf(stderr, "Error in setting/getting terminal attributes.\n");
    }

    return status;
}

// ---------------------------------------------------------------------------
// Method: ParseConfigFile
//
// Description:
//    This methods parses the configuration file and creates an in-memory
//    list of servers and it's information(IP, username, password, version).
//
//    Config file should have this format:
//    [server]
//    hostname=<>
//    username=<>
//    password=<>
//    version=<>
//
//    All entries must be under a [server] block
//
//    - Using the BOOST program_options library. 
//    - Given the above format, this library will read the file and segment it
//      into the following options:
//          - server.hostname
//          - server.username
//          - server.password
//          - server.version
//    - Only the set list of options is acceptable. Anything besides this would
//      throw an exception.
//
// Returns:
//    bool
//
static bool
ParseConfigFile(void)
{ 
    INIFile         ini;
    SectionList     sections;
    ServerInfo*     serverInfoEntry;
    string          serverName;
    string          hostname;
    string          username;
    string          password;
    string          version;
    int             versionInt;
    int             itrNum = 0;
    map<std::string, SectionNameValuePair>::iterator sectionItr;
    bool status;

    ini.LoadFile(g_UserPaths.m_confPath);

    // Fetch all the sections
    //
    sections = ini.GetSections();

    // Iterate over the sections
    //
    for (sectionItr = sections.begin(); sectionItr != sections.end(); ++sectionItr)
    {
        status = true;

        // server name would be the name of the folder inside which the DMV files
        // are created.
        //
        serverName = sectionItr->first;
        if (serverName.length())
        {
            PrintMsg("%d: Processing entry for section %s in configuration file:\n",
                ++itrNum, serverName.c_str());

            // Fetch the information needed to login to the server
            //
            if (status)
            {
                status = ParseSectionEntry(sectionItr, "hostname", hostname);
            }
            if (status)
            {
                status = ParseSectionEntry(sectionItr, "username", username);
            }
            if (status)
            {
                status = ParseSectionEntry(sectionItr, "version", version);
                if (status)
                {
                    status = convertToInt(version, versionInt);
                }
            }
            if (status)
            {
                status = ParseSectionEntry(sectionItr, "password", password);

                // Query user for password in case nothing in config file
                //
                if (!status)
                {
                    status = QueryUserForPassword(serverName, password);
                }
            }

            // Check if the credentials and/or IP are correct
            //
            if (status)
            {
                status = VerifyServerInfo(hostname, username, password);
            }

            // Record this entry
            //
            if (status)
            {
                serverInfoEntry = new ServerInfo();
                assert(serverInfoEntry);

                PrintMsg("SUCCESSFULLY added entry for server %s.\n", serverName.c_str());

                // Adding entry to the global server information map
                //
                g_ServerInfoMap.insert(make_pair(serverName, serverInfoEntry));

                // Fill in the global map with server information.
                //
                serverInfoEntry->m_hostname = hostname;
                serverInfoEntry->m_username = username;
                serverInfoEntry->m_password = password;
                serverInfoEntry->m_version = versionInt;
            }
            else
            {
                PrintMsg("FAILED to add entry for server %s. Ignoring it.\n", serverName.c_str());
            }
        }
    }

    // Return false only if there were no entries added to the global server information map
    //
    if (g_ServerInfoMap.size())
    {
        status = true;
    }
    else
    {
        status = false;
    }

    return status;
}

// ---------------------------------------------------------------------------
// Method: FatalSignalHandler
//
// Description:
//    This method dictates the action to take when a fatal signal like
//    SIGSEGV and SIGABRT is received.
//
// Returns:
//    none
//
static void
FatalSignalHandler(
    int signo)
{
    fprintf(stderr, "********** FATAL SIGNAL - %s (%d) **********\n"
        "Exiting\n",
        strsignal(signo), signo);

    // Also log in log file it its enabled
    //
    PrintMsg("********** FATAL SIGNAL - %s (%d) **********\n"
        "Exiting\n",
        strsignal(signo), signo);

    KillSelf();
}

// ---------------------------------------------------------------------------
// Method: InstallSignalHandlers
//
// Description:
//    This method installs the custom signal handler for a variety
//    of signals.
//
// Returns:
//    none
//
static void
InstallSignalHandlers()
{
    signal(SIGABRT, FatalSignalHandler);    // ABORT
    signal(SIGSEGV, FatalSignalHandler);    // SEGMENT VIOLATION
    signal(SIGILL, FatalSignalHandler);     // ILLEGAL INSTRUCTION
    signal(SIGBUS, FatalSignalHandler);     // BUS ERROR, SIGSEGV LIKE AT HARDWARE LEVEL
    signal(SIGFPE, FatalSignalHandler);     // FLOATING POINT EXCEPTION
    signal(SIGSYS, FatalSignalHandler);     // BAD SYS CALL
    signal(SIGXCPU, FatalSignalHandler);    // EXCEEDED CPU LIMIT
    signal(SIGXFSZ, FatalSignalHandler);    // EXCEEDED FILE LIMIT
    signal(SIGSTKFLT, FatalSignalHandler);  // CO-PROCESSOR STACK FAULT
}

// ---------------------------------------------------------------------------
// Method: main
//
// Description:
//    This is the entry point of the program. 
//    Parses arguments, does sanity checks and loads FUSE.
//
// Returns:
//    -EINVAL if setup failure.
//    Otherwise return status of StartFuse
//
int
main(
    int argc,
    char* argv[])
{
    int result = 0;
    FILE* filePtr;
    bool status;

    // Rejecting request if root is trying to run this
    if ((getuid() == 0) || (geteuid() == 0))
    {
        fprintf(stderr, "Running as root opens unwanted security holes\n");
        result = -1;
    }

    InstallSignalHandlers();

    if (!result)
    {
        ParseArguments(argc, argv);

        // Check if the set paths are valid
        CheckAllSetPaths();

        // Open the log-file path if given and in verbose mode
        if (g_InVerbose && g_UseLogFile)
        {
            filePtr = fopen(g_UserPaths.m_logfilePath.c_str(), "w");
            if (!filePtr)
            {
                fprintf(stderr, "Provided log path is incorrect. "
                    "Unable to create / open a file at that path\n"
                    "Exiting..\n");

                result = -1;
            }
            
            fclose(filePtr);
        }
    }

    if (!result)
    {
        status = ParseConfigFile();
        if (!status)
        {
            fprintf(stderr, "Error in the config file content.\n");
            result = -1;
        }
    }

    if (!result)
    {
        result = StartFuse(argv[0]);
    }

    return result;
}

