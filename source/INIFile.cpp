//****************************************************************************
//               Copyright (c) Microsoft Corporation.
//
// File: INIFile.cpp
//
// Author: Scott Konersmann
//
// Purpose:
//  INI file parser class.
//
//  There are several things added to support various issues found in existing
//  code.  First the PAL INI files use a '#' to indicate a comment while
//  almost all other formats use ';'.  Second both Windows and Linux end of
//  line are supported ('\n' and '\r\n').
//
// Notes:
//     BNF for INI file parsing
//
//     <file>         ::= <commentsorws>* <section>+
//     <commentsorws> ::= <comment>* | <whitespace>*
//     <whitespace>   ::= [ \t]
//     <section>      ::= '[' <identifier> ']' <whitespace>* <eol> <body>
//     <body>         ::= <statement>*
//     <statement>    ::= <comment> | <equality> | <emptyline>
//     <comment>      ::= <whitespace>* <commentmarker> <string> <eol>
//     <commentmarker>::= ';' | '#'
//     <equality>     ::= <whitespace>* <identifier> <whitespace>* '=' <whitespace>* <value> <eol>
//     <identifier>   ::= [a-zA-Z], [a-zA-Z0-9_]*
//     <value>        ::= <text>
//     <eol>          ::= [\t\n] | [\n]
//
//****************************************************************************
#include "UtilsPrivate.h"

// ---------------------------------------------------------------------------
// Enumeration: IniStateMap
//
// Description:
//   This enumeration is used to define the states possible when parsing an
//   INI file.
//
//   Start State - can only have empty lines and comments in this state.
//   SectionHeader State - Indicates the beginning of a new section.
//   Values State - Indicates the name value portion of a section.  Ends
//       when a new section header is found.
//   End State - the end of file has been reached.
//
enum class IniStateMap { Start, SectionHeader, Values, End };

// ---------------------------------------------------------------------------
// Token values for parsing INI file.
//
const char COMMENT_DELIM1           = ';';
const char COMMENT_DELIM2           = '#';
const char SECTION_START_DELIM      = '[';
const char SECTION_END_DELIM        = ']';
const char NAME_VALUE_PAIR_DELIM    = '=';

// ---------------------------------------------------------------------------
// Method: Default contructor
//
// Description:
//    Use this when loading a INI file from a file path.  After creating the
//    class call the LoadFile method.
//
// Returns:
//    none
//
INIFile::INIFile() :
    m_allowDuplicateValues(false),  // Allow duplicate values by default
    m_lineno(0),                    // Line number of 0
    m_isLoaded(false)               // Loaded is false
{
};

// ---------------------------------------------------------------------------
// Method: LoadFile
//
// Description:
//    This method takes a fully qualified file path and name, opens the file
//    for reading and parses it.
//
// Returns:
//    none
//
void
INIFile::LoadFile(
    string fileName,                // Fully qualified file path and name
    bool   allowDuplicateValueKeys) // allow dup value keys
{
    if (m_isLoaded)
    {
        throw runtime_error(
            StringFormat("INI file class cannot be loaded twice.").c_str());
    }

    // Opening a read filestream to the file given
    //
    m_fileStream.open(fileName.c_str(), ios::in | ios::binary);
    if (m_fileStream.fail())
    {
        throw runtime_error(
            StringFormat("File cannot be opened. Filename = %s", 
                            fileName.c_str()).c_str());
    }

    m_allowDuplicateValues = allowDuplicateValueKeys;
    m_isLoaded = true;

    // Parsing the provided file.
    //
    ParseFile();
};

// ---------------------------------------------------------------------------
// Method: GetSections
//
// Description:
//    This method returns the sections data structure.
//
// Returns:
//    none
//
SectionList&
INIFile::GetSections()
{
    return m_sections;
}

// ---------------------------------------------------------------------------
// Method: Destructor
//
// Description:
//    Currently no cleanup is required.
//
INIFile::~INIFile()
{
    m_fileStream.close();
};

// ---------------------------------------------------------------------------
// Method: ParseFile
//
// Description:
//    This method takes a file containing the content of the INI file and
//    parses the content.  Filling in the Sections class property in the
//    process.  Any errors will result in exceptions being thrown.
//
// Returns:
//    none
//
void
INIFile::ParseFile()
{
    string fileLine;
    string sectionName;
    IniStateMap iniState = IniStateMap::Start;

    while (iniState != IniStateMap::End)
    {
        switch (iniState)
        {
        case IniStateMap::Start:
        {
            if (GetNextLine(fileLine))
            {
                m_lineno++;
                if (!IsEmptyOrComment(fileLine))
                {
                    if (IsSectionHeader(fileLine))
                    {
                        iniState = IniStateMap::SectionHeader;
                    }
                    else
                    {
                        throw ParseException(
                                  StringFormat("Line %u: The INI file is formatted incorrectly."
                                               "  File must start with whitespace, comments, or a section header.",
                                               m_lineno).c_str());
                    }
                }
                else
                {
                    // Continue processing until the first section is found
                }
            }
            else
            {
                // No more file to read.
                //
                iniState = IniStateMap::End;
            }
        }
        break;

        case IniStateMap::SectionHeader:
        {
            // Note: Segment headers are found while in other states.  No need to
            // read the line.  fileLine already has the section header in it.
            //
            sectionName = ExtractSectionName(fileLine);

            if (m_sections.count(sectionName) > 0)
            {
                throw ParseException(
                          StringFormat("Line %u: The INI file is formatted incorrectly."
                                       "  The section name [%s] is a duplicate.", m_lineno,
                                       sectionName.c_str()).c_str());
            }

            m_sections[sectionName] = SectionNameValuePair();
            iniState = IniStateMap::Values;
        }
        break;

        case IniStateMap::Values:
        {
            tuple<string, string> value;

            if (GetNextLine(fileLine))
            {
                m_lineno++;
                if (!IsEmptyOrComment(fileLine))
                {
                    if (IsSectionHeader(fileLine))
                    {
                        iniState = IniStateMap::SectionHeader;
                    }
                    else
                    {
                        value = GetNameValuePair(fileLine);
                        if (m_sections[sectionName].count(get<0>(value)) == 0 ||
                            m_allowDuplicateValues)
                        {
                            m_sections[sectionName].emplace(get<0>(value), get<1>(value));
                        }
                        else
                        {
                            throw ParseException(
                                      StringFormat("Line %u: The INI file is formatted incorrectly."
                                                   "  The section [%s] has a duplicate value name [%s].",
                                                   m_lineno, sectionName.c_str(),
                                                   get<0>(value).c_str()).c_str());
                        }
                    }
                }
                else
                {
                    // Skip empty or comment lines
                }
            }
            else
            {
                // No more file to read.
                //
                iniState = IniStateMap::End;
            }
        }
        break;

        case IniStateMap::End:
            // Do nothing but satisfy the compiler.
            //
            break;

        default:
            // TO DO - ResetState at this stage:
            //
            assert(0);
        }
    }

    if (m_sections.size() == 0)
    {
        throw ParseException(
                  StringFormat("Line %u: The INI file is formatted incorrectly."
                               "  File must contain at least one section.", m_lineno).c_str());
    }
};

// ---------------------------------------------------------------------------
// Method: IsEmptyOrComment
//
// Description:
//    This determines if the passed in string is empty, whitespace only, or
//    contains a INI file comment or not.
//
//    Note: Everything after a comment marker is comment until the end of line.
//
// Returns:
//    TRUE if the passed in string is empty, whitespace, or a comment.
//    FALSE if the passed in string contains other information.
//
bool
INIFile::IsEmptyOrComment(
    string line) // The string to interrogate.
{
    string trimmedLine = Trim(line);
    size_t commentPosition = min(line.find(COMMENT_DELIM1, 0), line.find(COMMENT_DELIM2, 0));
    size_t sectionStartPosition = line.find(SECTION_START_DELIM, 0);
    size_t nvpPosition = line.find(NAME_VALUE_PAIR_DELIM, 0);

    if (trimmedLine.empty())
    {
        return true;
    }
    if (commentPosition == 0)
    {
        // The first thing on the line is the comment delimiter.
        //
        return true;
    }
    else if ((sectionStartPosition != string::npos && sectionStartPosition > commentPosition) ||
             (nvpPosition != string::npos && nvpPosition > commentPosition))
    {
        // There are section start or equal operators on this line but they are after the comment marker.
        //
        return true;
    }
    else
    {
        return false;
    }
}

// ---------------------------------------------------------------------------
// Method: IsSectionHeader
//
// Description:
//    This method determines if a line contains a section header.
//
// Returns:
//    TRUE if the passed in string contains a section header.
//    FALSE if the passed in string is not a section header.
//
bool
INIFile::IsSectionHeader(
    string line) // The string to be interrogated.
{
    // Note this check assumes that you have already checked and the line is not a comment.
    //
    size_t sectionStartPosition = line.find(SECTION_START_DELIM, 0);
    size_t sectionEndPosition = line.find(SECTION_END_DELIM, 0);
    size_t commentPosition = min(line.find(COMMENT_DELIM1, 0), line.find(COMMENT_DELIM2, 0));
    size_t nvpPosition = line.find(NAME_VALUE_PAIR_DELIM, 0);

    return sectionStartPosition != string::npos && sectionEndPosition != string::npos &&
           sectionStartPosition < commentPosition && sectionStartPosition < nvpPosition;
}

// ---------------------------------------------------------------------------
// Method: ExtractSectionName
//
// Description:
//    This method takes a line containing a section header and extracts the
//    section name from it.
//
// Returns:
//    String containing the section name.
//
string
INIFile::ExtractSectionName(
    string line) // INI File line containing section name
{
    size_t sectionStartPosition = line.find(SECTION_START_DELIM, 0);
    size_t sectionEndPosition = line.find(SECTION_END_DELIM, 0);
    string result = Trim(line.substr(sectionStartPosition + 1,
                                     sectionEndPosition - sectionStartPosition - 1));
    string extraText = Trim(line.substr(sectionEndPosition + 1));

    if (sectionEndPosition <= sectionStartPosition)
    {
        throw ParseException(
                  StringFormat("Line %u: The INI file is formatted incorrectly."
                               "  An invalid section header was found.", m_lineno).c_str());
    }

    if (result.length() == 0)
    {
        throw ParseException(
                  StringFormat("Line %u: The INI file is formatted incorrectly."
                               "  An empty section header was found.", m_lineno).c_str());
    }

    // Only whitespace is allowed on the line after a section header. Support
    // for comments could be added by changing this to a call to
    // IsEmptyOrComment.
    //
    if (extraText.length() > 0)
    {
        throw ParseException(
                  StringFormat("Line %u: The INI file is formatted incorrectly."
                               "  Only whitespace can follow a section header.", m_lineno).c_str());
    }

    return result;
}

// ---------------------------------------------------------------------------
// Method: GetNameValuePair
//
// Description:
//    This method takes a line containing a name value pair and splits it
//    into the name and value parts.
//
//    Note that the value can have NVP separators ('=') because this method
//    only splits at the first separator.
//
// Returns:
//    A pair containing the separated name and value.
//
tuple<string, string>
INIFile::GetNameValuePair(
    string line) // The string to Split into name and value.
{
    size_t nvpSeparatorPosition = line.find(NAME_VALUE_PAIR_DELIM, 0);
    vector<string> values = SplitFirst(line, NAME_VALUE_PAIR_DELIM);

    if (nvpSeparatorPosition == string::npos ||
        values.size() != 2)
    {
        throw ParseException(
                  StringFormat("Line %u: The INI file is formatted incorrectly."
                               "  An name value pair was found without an equality operator or malformed section header.",
                               m_lineno).c_str());
    }

    return tuple<string, string>(Trim(values[0]), Trim(values[1]));
}

// ---------------------------------------------------------------------------
// Method: GetNextLine
//
// Description:
//    This methods reads the next line from the internal open
//    file stream and fills in the string provided with it.
//
// Returns:
//    bool
//
bool
INIFile::GetNextLine(
    string& fileLine)
{
    bool status;

    status = true;

    // Read line.
    //
    getline(m_fileStream, fileLine);
    if (m_fileStream.fail())
    {
        // Clear the string in case no other line left.
        //
        fileLine.clear();
        status = false;
    }

    return status;
}

