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

// Once query is processed -rename original file to end with
// this phrase.
//
#define CUSTOM_QUERY_OUTPUT_FILE_NAME_TERMINATION   "_output"

// Time the thread should sleep before checking in the folders again.
//
#define CUSTOM_QUERY_THREAD_SLEEP_TIME_SECONDS      2


// This method is the function that the custom query thread starts
// with and helps implement the custom query support.
//
void CheckForCustomQueries();