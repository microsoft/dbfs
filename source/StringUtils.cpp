//****************************************************************************
//      Copyright (c) Microsoft Corporation. All rights reserved.
//      Licensed under the MIT license.
//
// File: StringUtils.cpp
//
// Purpose:
//   String utility functions for functionality not available in the standard
//   classes.
//
#include "UtilsPrivate.h"

// ---------------------------------------------------------------------------
// Static string values used in the trim functions.
//
static const char* WHITESPACE = " \n\r\t";
static const char* EMPTYSTRING = "";

// ---------------------------------------------------------------------------
// Function: TrimLeft
//
// Description:
//    Function to trim whitespace off the beginning of a string.
//
// Returns:
//    Trimmed string.
//
string
TrimLeft(
    const string& s) // The string to trim.
{
    string::size_type startpos = s.find_first_not_of(WHITESPACE);
    return (startpos == string::npos) ? EMPTYSTRING : s.substr(startpos);
}

// ---------------------------------------------------------------------------
// Function: TrimRight
//
// Description:
//    Function to trim whitespace off the end of a string.
//
// Returns:
//    Trimmed string.
//
string
TrimRight(
    const string& s) // The string to trim.
{
    string::size_type endpos = s.find_last_not_of(WHITESPACE);
    return (endpos == string::npos) ? EMPTYSTRING : s.substr(0, endpos + 1);
}

// ---------------------------------------------------------------------------
// Function: Trim
//
// Description:
//    Function to trim whitespace off the beginning and end of a string.
//
// Returns:
//    Trimmed string.
//
string
Trim(
    const string& s) // The string to trim.
{
    return TrimRight(TrimLeft(s));
}

// ---------------------------------------------------------------------------
// Function: Split
//
// Description:
//    Function that splits the given string into a vector of values on
//    the delimiter character.  Note this function does not trim whitespace
//    off the values returned.
//
// Returns:
//    Vector of values between the delimiter character in the given string.
//
vector<string>
Split(
    const string& s,     // String to split
    char          delim) // Delimiter character
{
    string item;
    stringstream ss(s);
    vector<string> result;
    while (getline(ss, item, delim))
    {
        if (!item.empty())
        {
            result.push_back(item);
        }
    }

    if (result.size() == 0)
    {
        // Couldn't be split.  Send back the input string.
        //
        result.push_back(s);
    }

    return result;
}

// ---------------------------------------------------------------------------
// Function: SplitFirst
//
// Description:
//    Function that splits the given string on the first occurrence of a
//    delimiter.  It puts the results in the given pair.  Note this function
//    does not trim whitespace off the values returned.
//
// Returns:
//    The values before and after the delimiter.
//
vector<string>
SplitFirst(
    const string& s,     // The string to split
    char          delim) // The delimiter character
{
    vector<string> result;
    size_t delimLocation = s.find_first_of(delim);

    if (delimLocation == s.npos)
    {
        // The delimiter wasn't found.
        //
        result.push_back(s);
    }
    else
    {
        result.push_back(s.substr(0, delimLocation));
        result.push_back(s.substr(delimLocation + 1));
    }
    return result;
}

// ---------------------------------------------------------------------------
// Function: SplitLast
//
// Description:
//    Function that splits the given string on the last occurrence of a
//    delimiter.  It puts the results in the given pair.  Note this function
//    does not trim whitespace off the values returned.
//
// Returns:
//    The values before and after the delimiter.
//
vector<string>
SplitLast(
    const string& s,     // The string to split
    char          delim) // The delimiter character
{
    vector<string> result;
    size_t delimLocation = s.find_last_of(delim);

    if (delimLocation == s.npos)
    {
        // The delimiter wasn't found.
        //
        result.push_back(s);
    }
    else
    {
        result.push_back(s.substr(0, delimLocation));
        result.push_back(s.substr(delimLocation + 1));
    }
    return result;
}

// ---------------------------------------------------------------------------
// Function: StringFormat
//
// Description:
//    Function that provides sprintf like functionality for string types.
//    This method is used instead of stringstream pipe because this will
//    allow the format string to be localized.
//
// Returns:
//    Formatted string.
//
string
StringFormat(
    const string fmt, // The format string
    ...)              // Vararg list of parameters to the format string
{
    va_list vl, vl_size;
    va_start(vl, fmt);
    va_copy(vl_size, vl);

    // Clang is warning about using non string-literals as arguments to vsnprintf.
    //
    // In our use case the format string is a string literal, usually boxed to a
    // string type. However the compiler doesn't seem to be able to deduce that.
    // Changing the signature to take a "const char* fmt" doesn't help either.
    //

    // Figure out the size of the final string.
#ifdef _WINDLL
    int size = _vscprintf(fmt.c_str(), vl_size);
#else
    int size = vsnprintf(NULL, 0, fmt.c_str(), vl_size);
#endif

    size += sizeof(char); // Add a space for null terminator
    unique_ptr<char[]> buffer = make_unique<char[]>(static_cast<unsigned long>(size));
#ifdef _WINDLL
    vsprintf_s(buffer.get(), size, fmt.c_str(), vl);
#else
    vsnprintf(buffer.get(), sizeof(char) * size, fmt.c_str(), vl);
#endif

    va_end(vl);
    va_end(vl_size);

    string str(buffer.get());

    return str;
}

// ---------------------------------------------------------------------------
// Function: StringReplace
//
// Description:
//    Function that will replace all occurrences of the given substring with
//    another.
//
// Returns:
//    Formatted string.
//
string
StringReplace(
    string       source, // Target string for the replacement
    const string from,   // String to search for in the target string
    const string to)     // String to replace in the target string
{
    string result;

    string::size_type lastPos = 0;
    string::size_type findPos;

    while (string::npos != (findPos = source.find(from, lastPos)))
    {
        result.append(source, lastPos, findPos - lastPos);
        result += to;
        lastPos = findPos + from.length();
    }

    result += source.substr(lastPos);
    return result;
}

// ---------------------------------------------------------------------------
// Function: StringReplace
//
// Description:
//    Function that will replace all occurrences of the given character with
//    another.
//
// Returns:
//    Formatted string.
//
string
StringReplace(
    string     source, // Target string for the replacement
    const char from,   // Character to search for in the target string
    const char to)     // Character to replace in the target string
{
    string fromStr, toStr;
    fromStr += from;
    toStr += to;

    return StringReplace(source, fromStr, toStr);
}

// ---------------------------------------------------------------------------
// Function: IsPrefix
//
// Description:
//    Function that will determine if the given string starts with the prefix.
//
// Returns:
//    0 if the string starts with the prefix.
//
int
IsPrefix(string prefix, string value)
{
    if (prefix.length() > value.length())
    {
        return 2;
    }
    else if (equal(prefix.begin(), prefix.end(), value.begin()))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

// ---------------------------------------------------------------------------
// Function: ConvertU8ToU16
//
// Description:
//    This function converts an eight bit character string to a 16 bit
//    character string.
//
// Returns:
//    A new 16 bit character string with wide char equivalent string as the
//    input value.
//
u16string
ConvertU8ToU16(
    const string& inputValue) // eight bit character string to convert
{
    wstring_convert<codecvt_utf8<char16_t>, char16_t> converter;
    u16string result = converter.from_bytes(inputValue);
    return result;
}

// ---------------------------------------------------------------------------
// Function: ConvertU16ToU8
//
// Description:
//    This function converts a 16 bit character string to an eight bit
//    character string.
//
// Returns:
//    A new 8 bit character string with equivalent string as the
//    input value.
//
string
ConvertU16ToU8(
    const u16string& inputValue) // eight bit character string to convert
{
    wstring_convert<codecvt_utf8<char16_t>, char16_t> converter;
    string result = converter.to_bytes(inputValue);
    return result;
}

// ---------------------------------------------------------------------------
// Function: StringToUpper
//
// Description:
//    This function takes a string and converts the characters to upper case.
//
// Returns:
//    A new string containing only upper case characters.
//
string
StringToUpper(
    string stringToConvert)
{
    transform(stringToConvert.begin(), stringToConvert.end(), stringToConvert.begin(), ::toupper);
    return stringToConvert;
}

// ---------------------------------------------------------------------------
// Function: StringToLower
//
// Description:
//    This function takes a string and converts the characters to lower case.
//
// Returns:
//    A new string containing only lower case characters.
//
string
StringToLower(
    string stringToConvert)
{
    transform(stringToConvert.begin(), stringToConvert.end(), stringToConvert.begin(), ::tolower);
    return stringToConvert;
}

// ---------------------------------------------------------------------------
// InsensitiveCompare Constructor
//
// Description:
//    This is the constructor that takes the value to search for as a
//    parameter.
//
// Returns:
//    none
//
InsensitiveCompare::InsensitiveCompare(
    std::string const& input // Value to search for
    ) : comparison(input)
{
}

// ---------------------------------------------------------------------------
// InsensitiveCompare Operator()
//
// Description:
//    This is the operation() overload that is used to compare two strings
//    during at STL find_if operation.
//
// Returns:
//    TRUE if the strings are a case insensitive match.
//
bool
InsensitiveCompare::operator()(
    std::string const& test) const // The value to search for
{
    bool result = false;
    if (StringToLower(comparison) == StringToLower(test))
    {
        result = true;
    }

    return result;
}

