/**
 *  \file RMF/exceptions.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_EXCEPTIONS_H
#define RMF_EXCEPTIONS_H

#include <RMF/config.h>
#include <boost/exception/exception.hpp>
#include "internal/errors.h"
#include <string>
#include <exception>

RMF_ENABLE_WARNINGS
namespace RMF {
/** The base class for RMF exceptions. Use the what() method
    to get back a string describing the exception.

    Use get_message() to get a nice message describing the
    exception.
 */
class RMFEXPORT Exception: public virtual std::exception,
                           public virtual boost::exception {
  mutable std::string message_;
public:
  Exception();
  const char * what() const throw();
  virtual ~Exception() throw();
};


/** Use this instead of the more standard what() to get the
    message as what() presents issues for memory management
    with dynamically generated messages like. */
RMFEXPORT std::string get_message(const Exception &e);


/** Usage exceptions are thrown when the library is misused in some way,
    eg, an out of bounds element is requested from a collection. In general
    when these are throw, the failed operation should have been cleanly
    aborted without changing the file.
 */
class RMFEXPORT UsageException: public Exception {
public:
  UsageException();
  ~UsageException() throw();
};

/** IOExceptions are thrown when some operation on a disk file fails.
 */
class RMFEXPORT IOException: public Exception {
public:
  IOException();
  ~IOException() throw();
};

/** Internal exceptions are thrown when the library discovers that some
    internal invariant no longer holds. Since they represent bugs in the
    library, one can not necessarily recover when they occur..
 */
class RMFEXPORT InternalException: public Exception {
public:
  InternalException();
  ~InternalException() throw();
};

}

#define RMF_THROW(m, e)                          \
  {                                              \
    using namespace ::RMF::internal::ErrorInfo;  \
    using ::RMF::internal::ErrorInfo::Type;      \
    using ::RMF::internal::ErrorInfo::Category;  \
    using ::RMF::internal::ErrorInfo::Key;       \
    using ::RMF::internal::ErrorInfo::Decorator; \
    using boost::operator<<;                     \
    throw e() << m;                              \
  }

#define RMF_RETHROW(m, e)                        \
  {                                              \
    using namespace ::RMF::internal::ErrorInfo;  \
    using ::RMF::internal::ErrorInfo::Type;      \
    using ::RMF::internal::ErrorInfo::Category;  \
    using ::RMF::internal::ErrorInfo::Key;       \
    using ::RMF::internal::ErrorInfo::Decorator; \
    using boost::operator<<;                     \
    e << m;                                      \
    throw;                                       \
  }

#define RMF_USAGE_CHECK(check, message)              \
  do {                                               \
    if (!(check)) {                                  \
      RMF_THROW(Message(message) << Type("Usage"),   \
                ::RMF::UsageException);              \
    }                                                \
  } while (false)

#define RMF_INDEX_CHECK(value, end) \
  RMF_USAGE_CHECK(value < end, "Out of range index");


#define RMF_PATH_CHECK(path)                                                   \
  if (!boost::filesystem::exists(path)) {                                      \
      RMF_THROW(Message("File does not exist") << File(path) << Type("Usage"), \
              IOException);                                                    \
  }



#ifndef RMF_NDEBUG
#  define RMF_INTERNAL_CHECK(check, message)                            \
  do {                                                                  \
    if (!(check)) {                                                     \
      RMF_THROW(Message(message) << Type("Internal")                    \
                << SourceFile(__FILE__) << SourceLine(__LINE__)         \
                << Function(BOOST_CURRENT_FUNCTION),                    \
                ::RMF::InternalException);                              \
    }                                                                   \
  } while (false)


#  define RMF_IF_CHECK \
  if (true)

#else // NDEBUG
#  define RMF_INTERNAL_CHECK(check, message)
#  define RMF_IF_CHECK

#endif

#define RMF_NOT_IMPLEMENTED                                 \
      RMF_THROW(Message("Not implemented")                  \
            << Function(BOOST_CURRENT_FUNCTION)             \
            << SourceFile(__FILE__) << SourceLine(__LINE__) \
            << Type("NotImplemented"),                      \
            ::RMF::InternalException)

RMF_DISABLE_WARNINGS

#endif  /* RMF_EXCEPTIONS_H */
