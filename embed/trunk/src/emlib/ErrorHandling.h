#ifndef _ERRORHANDLING_H
#define _ERRORHANDLING_H

#include "EM_config.h"
#include <string>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cstring>


//! Exception to throw when a variable has a wrong value
class EMDLLEXPORT EMBED_WrongValue: public std::exception
{
public:
  virtual ~EMBED_WrongValue() throw() {};

  //! Constructor
  /** \param[in] var string with the name of the variable that failed
      \param[in] value value of the variable that failed
   */
  EMBED_WrongValue(std::string variable, float val);

  //! Error message
  virtual const char* what() const throw() {
    std::ostringstream msg;
    msg << "Wrong value with variable: " << var_ << " " << value_;
    return msg.str().c_str();
  }

protected:
  //! Stores the name of the variable that failed
  std::string var_;
  //! Stores the value that made it fail
  float value_;
};


//! Exception to throw when there are I/O problems
class EMDLLEXPORT EMBED_IOException: public std::exception
{
public:
  virtual ~EMBED_IOException() throw() {};

  //! Constructor
  /** \param[in] s string used as text when function what() is called. 
   */
  EMBED_IOException(std::string s);

  //! Error message
  /** \return the error message
   */
  virtual const char* what() const throw() {
    std::ostringstream msg;
    msg << txt_;
    return msg.str().c_str();
  }

protected:
  //! Text to be printed when the what() function is called
  std::string txt_;
};

#endif //_ERRORHANDLING_H
