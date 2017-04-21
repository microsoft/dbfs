//****************************************************************************
//      Copyright (c) Microsoft Corporation. All rights reserved.
//      Licensed under the MIT license.
//
// File: ParseException.h
//
// Purpose:
//   An exception class thrown during parsing to indicate an error in the
//   input stream.
//
//   Note on Windows the std::exception class has a constructor that takes
//   a message that gets returned by what() but on Linux that constructor
//   is missing which means this class has to implement a constructor and
//   override the what() method.
//

#pragma once

//--------------------------------------------------------------------
// Class: ParseException
//
// Description:
//  This class is used to throw a typed exception when there is a
//  problem in parsing.
//
// Dev notes:
//
class ParseException : public exception
{
public:
    // Constructor to pass exception message
    //
    ParseException(const string& msg) : m_message(msg)
    {
    }

    // Copy constructor
    //
    ParseException(const ParseException& exception) : m_message(exception.m_message)
    {
    }

    // Destructor
    //
    virtual ~ParseException() noexcept {}

    // Method to retrieve the exception message
    //
    virtual const char* what() const noexcept {return m_message.c_str(); }

private:
    string m_message;
};

