#ifndef _ERRORHANDELING_H
#define _ERRORHANDELING_H

#include "EM_config.h"
#include <string>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cstring>


//! Exception to throw when a variable has a wrong value
class EMDLLEXPORT EMBED_WrongValue: public std::exception {
// Using EMDLLEXPORT is necessary to make the object public when compiled in a 
// dynamic share object in SWIG
  public:
  //! Stores the name of the variable that failed
  std::string var;
  //! Stores the value that make it fail
  float value;

  virtual ~EMBED_WrongValue() throw() {};

  //! Constructor
  /**
     \param[in] var string with the name of the variable that failed
     \param[in] value value of the variable that failed
  */
  EMBED_WrongValue(std::string variable,float val);

   //! Error message
    virtual const char* what() const throw() {
      std::ostringstream msg;
      msg << "Wrong value with variable: " << var << " " << value;
      return msg.str().c_str();
    }
 };

//! Exception to throw when there are I/O problems
class EMDLLEXPORT EMBED_IOException: public std::exception {
  public:
  //! Text to be printed when the what() function is called
  std::string txt;
  virtual ~EMBED_IOException() throw() {};

  //! Constructor
  /**
     \param[in] s string used as text when function what() is called. 
                  Is stored in txt().
  */
  EMBED_IOException(std::string s);
  //! Error message
  /**
     \return the error message
  */
  virtual const char* what() const throw();
};

#endif //_ERRORHANDELING_H
