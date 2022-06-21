/**
 *  \file RMF/exceptions.h
 *  \brief Declarations of the various exception types.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_EXCEPTIONS_H
#define RMF_EXCEPTIONS_H

#include <boost/exception/exception.hpp>  // IWYU pragma: export
#include <exception>
#include <string>

#include "RMF/config.h"
#include "compiler_macros.h"

RMF_ENABLE_WARNINGS
namespace RMF {
/** The base class for RMF exceptions. Use the what() method
    to get back a string describing the exception.

    Use get_message() to get a nice message describing the
    exception.
 */
class RMFEXPORT Exception : public virtual std::exception,
                            public virtual boost::exception {
  mutable std::string message_;

 public:
  RMF_CXX11_DEFAULT_COPY_CONSTRUCTOR(Exception);
  Exception();
  const char* what() const RMF_NOEXCEPT override;
  virtual ~Exception() RMF_NOEXCEPT;
};

/** Use this instead of the more standard what() to get the
    message as what() presents issues for memory management
    with dynamically generated messages like. */
RMFEXPORT std::string get_message(const Exception& e);

/** Usage exceptions are thrown when the library is misused in some way,
    e.g., an out of bounds element is requested from a collection. In general
    when these are throw, the failed operation should have been cleanly
    aborted without changing the file.
 */
class RMFEXPORT UsageException : public Exception {
 public:
  RMF_CXX11_DEFAULT_COPY_CONSTRUCTOR(UsageException);
  UsageException();
  ~UsageException() RMF_NOEXCEPT;
};

/** IOExceptions are thrown when some operation on a disk file fails.
 */
class RMFEXPORT IOException : public Exception {
 public:
  RMF_CXX11_DEFAULT_COPY_CONSTRUCTOR(IOException);
  IOException();
  ~IOException() RMF_NOEXCEPT;
};

/** Internal exceptions are thrown when the library discovers that some
    internal invariant no longer holds. Since they represent bugs in the
    library, one can not necessarily recover when they occur..
 */
class RMFEXPORT InternalException : public Exception {
 public:
  RMF_CXX11_DEFAULT_COPY_CONSTRUCTOR(InternalException);
  InternalException();
  ~InternalException() RMF_NOEXCEPT;
};

/** IndexExceptions are thrown when you walk off the end of something.
 */
class RMFEXPORT IndexException : public Exception {
 public:
  RMF_CXX11_DEFAULT_COPY_CONSTRUCTOR(IndexException);
  IndexException();
  ~IndexException() RMF_NOEXCEPT;
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

#define RMF_USAGE_CHECK(check, message)                                    \
  do {                                                                     \
    if (!(check)) {                                                        \
      RMF_THROW(Message(message) << Type("Usage"), ::RMF::UsageException); \
    }                                                                      \
  } while (false)

#define RMF_INDEX_CHECK(value, end)                                      \
  RMF_USAGE_CHECK(                                                       \
      static_cast<unsigned int>(value) < static_cast<unsigned int>(end), \
      "Out of range index");

#define RMF_PATH_CHECK(path)                                                 \
  if (!boost::filesystem::exists(path)) {                                    \
    RMF_THROW(Message("File does not exist") << File(path) << Type("Usage"), \
              IOException);                                                  \
  }

#ifndef RMF_NDEBUG
#define RMF_INTERNAL_CHECK(check, message)                                   \
  do {                                                                       \
    if (!(check)) {                                                          \
      RMF_THROW(Message(message) << Type("Internal") << SourceFile(__FILE__) \
                                 << SourceLine(__LINE__)                     \
                                 << Function(BOOST_CURRENT_FUNCTION),        \
                ::RMF::InternalException);                                   \
    }                                                                        \
  } while (false)

#define RMF_IF_CHECK if (true)

#else  // NDEBUG
#define RMF_INTERNAL_CHECK(check, message)
#define RMF_IF_CHECK

#endif

#define RMF_NOT_IMPLEMENTED                                                \
  RMF_THROW(Message("Not implemented") << Function(BOOST_CURRENT_FUNCTION) \
                                       << SourceFile(__FILE__)             \
                                       << SourceLine(__LINE__)             \
                                       << Type("NotImplemented"),          \
            ::RMF::InternalException)

RMF_DISABLE_WARNINGS

#include "RMF/internal/errors.h"  // IWYU pragma: export

#endif /* RMF_EXCEPTIONS_H */
