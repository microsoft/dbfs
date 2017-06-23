//****************************************************************************
//      Copyright (c) Microsoft Corporation. All rights reserved.
//      Licensed under the MIT license.
//
// File: SQLQuery.cpp
//
// Purpose:
//   This file contains the backend function definitions used to query
//   the SQL server.
//
#include "UtilsPrivate.h"

// ---------------------------------------------------------------------------
// Method: DBErrorHandler
//
// Description:
//    This method is invoked whenever DB-Library determines that an 
//    error has occurred.
//
// Returns:
//    int
//
int DBErrorHandler(
    DBPROCESS* dbproc,
    int severity,
    int dberr,
    int oserr,
    char* dberrstr,
    char* oserrstr)
{
    if ((dbproc == NULL) || (DBDEAD(dbproc)))
    {
        fprintf(stderr, "DB process structure failed to initialize.\n");
        return(INT_CANCEL);
    }
    else
    {
        fprintf(stderr, "DB-Library error:\n\t%s\n", dberrstr);

        if (oserr != DBNOERR)
        {
            fprintf(stderr, "Operating-system \error:\n\t%s\n", oserrstr);
        }

        return(INT_CANCEL);
    }
}

// ---------------------------------------------------------------------------
// Method: InstallDBHandlers
//
// Description:
//    This method installs the error and message handlers.
//
// Returns:
//    none.
//
void InstallDBHandlers()
{
    dberrhandle(DBErrorHandler);
}

// ---------------------------------------------------------------------------
// Method: UninstallDBHandlers
//
// Description:
//    This method uninstalls the error and message handlers.
//
// Returns:
//    none.
//
void UninstallDBHandlers()
{
    dberrhandle(NULL);
}

// ---------------------------------------------------------------------------
// Method: CreateConnectionAndRunQuery
//
// Description:
//    This method opens up a DB connection to the provided server
//    and executes the query given.
//
// Returns:
//    Query repsonse connection pointer (DBPROCESS *) on success
//    NULL on error.
//
static RETCODE
CreateConnectionAndRunQuery(
    const string& query,
    const string& dbServer,
    const string& username,
    const string& password,
    DBPROCESS*& dbConn)
{
    LOGINREC*   login;
    RETCODE     status = SUCCEED;
    char        hostname[MAXHOSTNAMELEN];
    int         maxLen = MAXHOSTNAMELEN;
    dbConn = NULL;

    // Init the DB library.
    //
    if (dbinit() == FAIL)
    {
        PrintMsg("Could not init db.\n");
        status = FAIL;
    }

    // Install error-handler and message-handlers.
    //
    InstallDBHandlers();

    // Set the login timeout.
    //
    if (status == SUCCEED)
    {
        status = dbsetlogintime(SQLFS_MAX_LOGIN_TIMEOUT_SEC);
        if (status == FAIL)
        {
            PrintMsg("Could not set the login timeout.\n");
        }
    }

    // Allocate a login params structure.
    //
    if (status == SUCCEED)
    {
        login = dblogin();
        if (!login)
        {
            PrintMsg("Could not initialize dblogin() structure.\n");
            status = FAIL;
        }
    }

    // Initialize the login params in the structure.
    //
    if (status == SUCCEED)
    {
        DBSETLUSER(login, username.c_str());
        DBSETLPWD(login, password.c_str());
        DBSETLAPP(login, progName);
        if (gethostname(hostname, maxLen) == 0)
        {
            DBSETLHOST(login, hostname);
        }

        dbConn = dbopen(login, dbServer.c_str());
        if (!dbConn)
        {
            PrintMsg("Could not connect to DB Server: %s\n", dbServer.c_str());
            status = FAIL;
        }

        // login structure no longer needed after logging in.
        //
        dbloginfree(login);
    }
    
    if (status == SUCCEED)
    {
        status = dbuse(dbConn, dbName);
        if (status == FAIL)
        {
            PrintMsg("Could not switch to database %s on DB Server %s\n",
                dbName, dbServer.c_str());
        }
    }

    // Set the timeout to get response for SQL command.
    //
    if (status == SUCCEED)
    {
        status = dbsettime(SQLFS_MAX_RESPONSE_WAIT_SEC);
        if (status == FAIL)
        {
            PrintMsg("Could not set the timeout for sql server response\n");
        }
    }

    // Now prepare a SQL statement.
    //
    if (status == SUCCEED)
    {
        dbcmd(dbConn, query.c_str());

        // Now execute the SQL statement.
        //
        status = dbsqlexec(dbConn);
        if (status == FAIL)
        {
            PrintMsg("Could not execute the sql statement\n");
        }
    }

    if (status == SUCCEED)
    {
        dbresults(dbConn);
    }

    return status;
}

// ---------------------------------------------------------------------------
// Method: BindStringVector
//
// Description:
//    This method resizes the vector to hold the required number of
//    entries (equal to number of columns). Also resizes each string
//    taking into account the maximum size of entry in the column.
//
// Returns:
//    VOID
//
static void
    BindStringVector(
    DBPROCESS* dbConn,
    int numColumns,
    vector<string>& stringVector)
{
    int     maxColumnEntryLen;
    int     columnLen;
    char*   stringPtr;

    // Extend vector to hold appropriate number of strings.
    //
    stringVector.resize(numColumns);

    for (int i = 0; i < numColumns; i++)
    {
        maxColumnEntryLen = MAX_COLUMN_ENTRY_LEN;
        columnLen = 0;

        // Calculate column entry length.
        //
        if ((columnLen = dbcollen(dbConn, i + 1)) > maxColumnEntryLen)
        {
            maxColumnEntryLen = columnLen;
        }

        // Resize string.
        //
        stringVector[i].resize(maxColumnEntryLen);

        // Now bind the returned columns to the variables
        // Column numbers start from 1
        // The fourth argument is to specify the maximum size(bytes)
        // that can be copied into the buffer. This avoids memory corruption.
        //
        stringPtr = (char*)stringVector[i].c_str();
        dbbind(dbConn, i + 1, NTBSTRINGBIND, maxColumnEntryLen,
            (BYTE*)stringPtr);
    }
}

// ---------------------------------------------------------------------------
// Method: CopyColumnNames
//
// Description:
//    This methods copies the names of columns (tab seperated) into the 
//    given string. 
//
// Returns:
//    VOID
//
static void
    CopyColumnNames(
    DBPROCESS* dbConn,
    int numColumns,
    ostringstream& stream)
{

    // Copy name of columns.
    //
    for (int i = 0; i < numColumns; i++)
    {
        // Insert a tab between two column names
        // This is not needed for the first entry.
        //
        if (i != 0)
        {
            stream << '\t';
        }

        // Copy columnn name into output string
        // Column numbering starts from 1 (hence i+1).
        //
        stream << dbcolname(dbConn, i + 1);
    }
    stream << '\n';
}

// ---------------------------------------------------------------------------
// Method: CopyAllRowData
//
// Description:
//    This methods copies the contents from all the rows into the 
//    given string. This is done for all the columns.
//
// Returns:
//    VOID
//
static void
    CopyAllRowData(
    DBPROCESS* dbConn,
    int numColumns,
    ostringstream& stream,
    vector<string>& stringVector)
{
    size_t dataLen;
    const char* data;

    // Loop thru the result set.
    //
    while (dbnextrow(dbConn) != NO_MORE_ROWS)
    {
        // copy out the data for each row.
        //
        for (int i = 0; i < numColumns; i++)
        {
            // Insert a tab between two column names
            // This is not needed for the first entry.
            //
            if (i != 0)
            {
                stream << '\t';
            }

            data = stringVector[i].c_str();

            // Calculate length of data for this row.
            //
            dataLen = strlen(data);

            // Copy columnn name into output string.
            //
            stream.write(data, dataLen);
        }
        stream << '\n';
    }
}

// ---------------------------------------------------------------------------
// Method: ExecuteQuery
//
// Description:
//    This method executes the provided SQL query on the given server.
//    If JSON is requested, the function does not copy the column name into
//    provided buffer because that is not a part of the JSON object.
//
// Returns:
//    0 on success and -1 on error.
//
int
ExecuteQuery(
    const string& query,
    string& output,
    const string& dbServer,
    const string& username,
    const string& password,
    const FileFormat type)
{
    DBPROCESS*      dbConn;
    RETCODE         status = FAIL;
    int             numColumns;
    vector<string>  stringVector;
    int             result = -1;
    ostringstream   stream;

    status = CreateConnectionAndRunQuery(query, dbServer, username, password, dbConn);

    if (status == SUCCEED)
    {
        // Getting number of columns to allocate memory accordingly.
        //
        numColumns = dbnumcols(dbConn);

        BindStringVector(dbConn, numColumns, stringVector);

        // In JSON there is just one row and the row name is a weird
        // string - basically not the JSON object.
        //
        if (type != TYPE_JSON)
        {
            CopyColumnNames(dbConn, numColumns, stream);
        }

        // Copy row data.
        //
        CopyAllRowData(dbConn, numColumns, stream, stringVector);

        // Clean up.
        //
        dbfreebuf(dbConn);
        dbclose(dbConn);
        dbexit();

        result = 0;
    }

    // Uninstall error-handler and message-handlers.
    //
    UninstallDBHandlers();

    output = stream.str();

    return result;
}

// ---------------------------------------------------------------------------
// Method: VerifyServerInfo
//
// Description:
//    This method checks if DB-Lib is able to connect with the given 
//    credentials of the given IP address. Also implicitly checks if the IP
//    address is reachable.
//
// Returns:
//    bool.
//
bool
VerifyServerInfo(
    string hostname,
    string username,
    string password)
{
    DBPROCESS* dbConn;
    RETCODE result = FAIL;
    string  query;
    bool status = true;

    // Just a basic query to test connection with server.
    //
    query = "SELECT @@version";

    result = CreateConnectionAndRunQuery(query, 
                                        hostname,
                                        username, 
                                        password, 
                                        dbConn);

    if (result == SUCCEED)
    {
        // Clean up.
        //
        dbfreebuf(dbConn);
        dbclose(dbConn);
        dbexit();
    }
    else
    {
        PrintMsg("Provided combination of hostname, username and password don't work. "
                 "This section would be ignored.\n");
        status = false;
    }

    // Un-install error-handler and message-handlers.
    //
    UninstallDBHandlers();

    return status;
}
