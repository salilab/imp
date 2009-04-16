/**
 *  \file stream.h    \brief Streams for python .
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_STREAM_H
#define IMP_STREAM_H

#include "Object.h"
#include <fstream>
#include <iostream>

#if defined(IMP_DOXYGEN) || defined(SWIG)
namespace std {
  /** @name Streams for Python

      C++ uses streams as its standard abstraction for input and
      output. These cannot be trivially mapped onto the Python
      IO layer. Instead we export the C++ stream classes to python.

      \note The stream classes are in the std namespace in C++ but the
      \imp Python module.
      @{
  */
  //! Base class for all output streams
  /** \see istream
      \note The stream classes are in the std namespace in C++ but the
      \imp Python module.
  */
  class ostream {
    ostream();
  };
  //! Base class for all input streams
  /** \see istream
      \note The stream classes are in the std namespace in C++ but the
      \imp Python module.
  */
  class istream{
    istream();
  };
  //! An output stream to a file.
  struct ofstream: public ostream {
    //! Open the file with the given name.
    /** No exception is thrown on failure. You should check using
        is_open() instead.*/
    ofstream(char*name);
    bool is_open() const;
    void close();
    void open(char *name);
  };
  //! An input stream from a file
  struct ifstream: public istream {
    //! Open the file with the given name for input
    /** No exception is thrown on failure. You should check using
        is_open() instead.*/
    ifstream(char*name);
    bool is_open() const;
    void close();
    void open(char *name);
  };
  //! An output stream to a string
  struct ostringstream: public ostream {
    ostringstream();
    //! Get the string containing what has been output
    std::string str() const;
  };
  //! An input stream to read from a string
  struct istringstream: public istream {
    //! Read from the string str
    istringstream(std::string str);
  };


  /** @} */
}
#endif

#endif  /* IMP_STREAM_H */
