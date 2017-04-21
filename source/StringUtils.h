//****************************************************************************
//      Copyright (c) Microsoft Corporation. All rights reserved.
//      Licensed under the MIT license.
//
// File: StringUtils.h
//
// Purpose:
//  This is a collection of string utilities for things missing in the
//  standard classes.
//
#pragma once

#ifndef _STRING_UTILS_
#define _STRING_UTILS_ 

// ----------------------------------------------------------------------------
// Trim functions
//
// These are modeled after the proposed C++ extensions of trim_left, trim_right,
// and trim.  They are named camel case to match our coding standards.
//
string TrimLeft(const string& s);

string TrimRight(const string& s);

string Trim(const string& s);

// ----------------------------------------------------------------------------
// Split functions
//
// Note that there are normal split functions that take a delimiter and split
// once for each delimiter found.  There are also SplitFirst functions that
// only split on the first occurrence.  These are useful in cases where a
// delimiter has a separate context between the first and nth occurrence.
//
vector<string> Split(const string& s, char delim);

vector<string> SplitFirst(const string& s, char delim);

vector<string> SplitLast(const string& s, char delim);

// ----------------------------------------------------------------------------
// String format functions
//
// Provides printf like functionality given string classes.
//
string StringFormat(const string fmt, ...);

// ----------------------------------------------------------------------------
// String replacement functions
//
// Provides ability to replace part of a string with another character or string.
//
string StringReplace(string source, const string from, const string to);

string StringReplace(string source, const char from, const char to);

// ----------------------------------------------------------------------------
// String search functions
//
// Search a string to determine if it contains a substring.
//
int IsPrefix(string prefix, string value);

// ----------------------------------------------------------------------------
// String conversion functions
//
// These functions provide conversion between 8 bit and 16 bit string values.
//
u16string ConvertU8ToU16(const string& inputValue);

string ConvertU16ToU8(const u16string& inputValue);

// ----------------------------------------------------------------------------
// Character conversion to upper or lower case
//
string StringToUpper(string strToConvert);

string StringToLower(string strToConvert);

// ---------------------------------------------------------------------------
// Function: InsensitiveCompare
//
// Description:
//      This is a structure that contains a comparison function that is used
//      to do a case insensitive string match of STR sutrctures during find_if
//      operations.
//
// Returns:
//    TRUE if the strings match and FALSE otherwise.
//
struct InsensitiveCompare
{
    std::string comparison;

    // Constructor for creating the structure.  Takes the value to search
    // for.
    //
    InsensitiveCompare(std::string const& input);

    // Comparison function
    //
    bool operator()(std::string const& test) const;
};

#endif
