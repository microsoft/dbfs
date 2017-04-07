//****************************************************************************
//               Copyright (c) Microsoft Corporation.
//
// File: INIFile.h
//
// Author: Scott Konersmann
//
// Purpose:
//  INI file parser class.  An INI file is a UTF8 (no Byte Order Mark)
//  or ASCII file that contains one or more sections with each section
//  containing a list of name/value pairs separated by an equals '='
//  character.
//
//****************************************************************************
#pragma once

// ---------------------------------------------------------------------------
// Type definitions
//

// The name value pair type is used to represent an equal ('=') delimited
// line that follows a section header.  Example: 'setting=10'.
//
typedef unordered_multimap<
        string, // The key for the name=value pair
        string> // The value on the right hand side of the '='
    SectionNameValuePair;

// The section list type represents a section header plus all the following
// key/value pairs in an INI file.
//
typedef map<
        string,               // The section name
        SectionNameValuePair> // a list of key/value pairs contained in the section.
    SectionList;

//--------------------------------------------------------------------
// Class: INIFile
//
// Description:
//  INI file parser class.  An INI file is a UTF8 (no Byte Order Mark)
//  or ASCII file that contains one or more sections with each section
//  containing a list of name/value pairs separated by an equals '='
//  character.
//
//  A section is put on a line with a name surrounded by square brackets.
//  Below the section header is a list of name value pairs.  Comments
//  can be added by using a semicolon ';'.
//
//    Example: '; This is a test section.'
//             '[Test Section]'
//             'setting1=true'
//             'setting2=1000'
//
// Dev notes:
//  There are several things added to support various issues found in existing
//  code.  First the PAL INI files use a '#' to indicate a comment while
//  almost all other formats use ';'.  Second both Windows and Linux end of
//  line are supported ('\n' and '\r\n').  And third the PAL INI files use
//  duplicate keys within a section.
//
//  This class has the ability to either allow duplicate keys or fail if
//  a duplicate is inserted.  There is a flag that can be passed into the
//  constructor that sets this behavior.
//
//  Duplicate keys are supported within a single section because the
//  PAL code currently uses duplicate keys for arrays of items like
//  registry keys.
//
//  Usage:
//     There are two methods to creating the class and parsing an INI file.
//     First you can just create the class using default constructor and call
//     LoadFile.
//             INIFile ini;
//             ini.LoadFile("somefile.ini");
//
//     You can also just pass in a string to a constructor.
//             INIFile ini("[SectionName]\nname=value");
//
//     To access the contents there is the Sections property.  If the section
//     or key does not exist an empty section or name will be returned.
//
//     To pull out a specific section you can look it up by name.
//             SectionNameValuePair values = ini.GetSections()["SectionName"];
//
//     To get a specific property in a given section.
//             std::string value = ini.GetSections()["SectionName"].find("name")->second;
//
//     To determine if a section exists use:
//             if (ini.GetSections().find("TestSection1") == ini.Sections.end())
//
//     To determine if a name exists use:
//             if (ini.GetSections()["SectionName"].find("name")->second.empty());
//
//      If duplicate keys are enabled then to retreive the list of keys
//      that match a search you can get back an iterator using the
//      equal_range function.
//             SectionNameValuePair::iterator iter =
//                  ini.Sections["SectionName"].equal_range("name");
//
class INIFile
{
public:
    // Default Constructor
    //
    INIFile();

    // Loads the INI file.
    //
    void LoadFile(
        string fileName,                            // Path of file name to load
        bool   allowDuplicateValueKeys = false);    // allow dup value keys

    // Destructor
    //
    ~INIFile();

    // Sections is a data structure which is a map of section name and
    // section content.  The section conent is a list of name=value pairs.
    //
    SectionList& GetSections();

private:
    // Parses the INI file into the class state.
    //
    void ParseFile();

    // Helper method for determining if an INI file line is empty or
    // contains a comment.
    //
    bool IsEmptyOrComment(
        string line); // Line to check for comment or whitespace

    // Helper method to determine if a INI file line contains a section
    // header.
    //
    bool IsSectionHeader(
        string line); // Line to check for section header

    // Helper method to extract a section name from a line containing
    // a section header.
    //
    string ExtractSectionName(
        string line); // Line to extract section name from.

    // Helper method to parse a INI file line containing a
    // name=value line.
    //
    tuple<string, string> GetNameValuePair(
        string line); // Line to extract name value pair from.

    // This methods reads the next line from the internal open
    // file stream and fills in the string provided with it.
    //
    bool GetNextLine(
        string& fileLine);

    ifstream m_fileStream;       // Filestream to the open
                                 // file that the class is 
                                 //loaded with
    bool m_allowDuplicateValues; // Determines whether or not
                                 // duplicate keys can exist
                                 // within a section header.
    unsigned int m_lineno;       // The current INI file line
                                 // number.
    bool m_isLoaded;             // Determines if the INI file
                                 // has been loaded or not.
    SectionList m_sections;      // List of sections and content
};
