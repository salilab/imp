/**
 *  \file RMF/exceptions.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_EXCEPTIONS_H
#define RMF_EXCEPTIONS_H

#include <RMF/config.h>
#include <string>
#include <exception>

namespace RMF {
/** The base class for RMF exceptions. Use the what() method
    to get back a string describing the exception.*/
class RMFEXPORT Exception: public virtual std::exception {
  std::string message_, file_name_, operation_;
 public:
  Exception(const char *msg): message_(msg) {
  }
  void set_operation_name(const char *name) throw();
  void set_file_name(const char *name) throw();
  const char *what() const throw();
  virtual const char *get_type() const=0;
  virtual ~Exception() throw();
};


/** Usage exceptions are thrown when the library is misused in some way,
    eg, an out of bounds element is requested from a collection. In general
    when these are throw, the failed operation should have been cleanly
    aborted without changing the file.
*/
class RMFEXPORT UsageException: public virtual Exception {
 public:
  UsageException(const char *msg);
  const char *get_type() const;
  ~UsageException() throw();
};


/** IOExceptions are thrown when some operation on a disk file fails.
*/
class RMFEXPORT IOException: public virtual Exception {
 public:
  IOException(const char *msg);
  const char *get_type() const;
  ~IOException() throw();
};

/** Internal exceptions are thrown when the library discovers that some
    internal invariant no longer holds. Since they represent bugs in the
    library, one can not necessarily recover when they occur..
*/
class RMFEXPORT InternalException: public virtual Exception {
 public:
  InternalException(const char *msg);
  const char *get_type() const;
  ~InternalException() throw();
};

}

#endif  /* RMF_EXCEPTIONS_H */
