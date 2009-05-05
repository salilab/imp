// Typemaps to allow Python director class methods to write to C++ std::ostream
// objects as if they were Python file-like objects; should only be included
// directly by the kernel so that the class is only defined there
%import "IMP_streams_kernel.i"

// Typemaps to allow Python file-like objects to be used for C++ code that
// expects a std::ostream

%typemap(in) std::ostream& (PyOutFileAdapter *tmp=NULL) {
  tmp = new PyOutFileAdapter($input);
  $1 = new std::ostream(tmp);
  $1->exceptions(std::ostream::badbit);
}
%typemap(freearg) std::ostream& {
  if ($1) delete $1;
  if (tmp$argnum) delete tmp$argnum;
}

// Something of an abuse of argout: force anything in our streambuf adapter to
// be flushed out to the file, and catch any exceptions raised. (Ideally the
// call to pubsync would come immediately after the method call, but the only
// way to handle that would be to use %exception or %feature("action") for
// each method using ostreams.)
// Potential memory leak here if there are other argout arguments that come
// after the ostream.
%typemap(argout) std::ostream& {
  try {
    tmp$argnum->pubsync();
  } catch (...) {
    Py_DECREF($result);
    if (!PyErr_Occurred()) {
      handle_imp_exception();
    }
    SWIG_fail;
  }
}

%typemap(typecheck) std::ostream& = PyObject *;

%{
// Adapter class that acts like a std::streambuf but delegates to a Python
// file-like object, p
class PyOutFileAdapter : public std::streambuf
{
  std::vector<char> buffer_;
  PyObject *p_;
public:
  PyOutFileAdapter(PyObject *p) : buffer_(1024), p_(p) {
    setp(&buffer_.front(), &buffer_.front() + buffer_.size());
    Py_XINCREF(p_);
  }

  virtual ~PyOutFileAdapter() { Py_XDECREF(p_); }

protected:
  virtual int_type overflow(int_type c) {
    if (c != EOF) {
      sync();
      *pptr() = c;
      pbump(1);
    }
    return c;
  }

  virtual int_type sync() {
    // Python API uses char* arguments rather than const char*, so create
    // here to quell the compiler warning
    static char method[] = "write";
    static char fmt[] = "(s#)";
    int num = pptr() - pbase();
    if (num <= 0) {
      return 0;
    }
    PyObject *result = PyObject_CallMethod(p_, method, fmt, pbase(), num);
    if (!result) {
      // Python exception will be reraised when SWIG method finishes
      throw std::ostream::failure("Python error on write");
    } else {
      pbump(-num);
      Py_DECREF(result);
      return 0;
    }
  }

  virtual std::streamsize xsputn(const char *s, std::streamsize n) {
    if (n > buffer_.size() * 2) {
      // Only take this route for large buffers, since two Python calls will
      // result per call (one here, potentially one in sync) rather than one per
      // buffer_.size() characters via the regular buffering
      sync();
      static char method[] = "write";
      static char fmt[] = "(s#)";
      PyObject *result = PyObject_CallMethod(p_, method, fmt, s, n);
      if (!result) {
        throw std::ostream::failure("Python error on write");
      } else {
        Py_DECREF(result);
      }
      return n;
    } else {
      // Use the regular buffering mechanism
      for (std::streamsize i = 0; i < n; ++i) {
        if (sputc(s[i]) == EOF) {
          return i;
        }
      }
      return n;
    }
  }
};
%}


// Typemaps to allow Python file objects to be used for C++ code that
// expects a std::istream
// It is rather expensive to call the Python 'read' method for every character
// (and we cannot read multiple bytes from the Python file, since there is no
// way to put them back if we read too many; even if the stream is seekable
// there is no guarantee we can restore the file position unless it is opened
// in binary mode). Thus, we try to use the underlying FILE pointer (only
// available for real files, not for-like objects) if possible. This may fail
// on Windows where different C runtimes can make FILE pointers unusable:
// http://www.python.org/doc/faq/windows/#pyrun-simplefile-crashes-on-windows-but-not-on-unix-why

%typemap(in) std::istream& (std::streambuf *tmp=NULL) {
  bool real_file;
  try {
    real_file = (PyFile_Check($input) && ftell(PyFile_AsFile($input)) != -1);
  } catch(...) {
    real_file = false;
  }
  if (real_file) {
    tmp = new PyInFileAdapter(PyFile_AsFile($input));
  } else {
    tmp = new PyInFilelikeAdapter($input);
  }
  $1 = new std::istream(tmp);
  $1->exceptions(std::istream::badbit);
}

%typemap(freearg) std::istream& {
  if ($1) delete $1;
  if (tmp$argnum) delete tmp$argnum;
}

%typemap(typecheck) std::istream& = PyObject *;

// Adapter class that acts like a std::streambuf but delegates to C-style
// stdio via a FILE pointer
%{
class PyInFileAdapter : public std::streambuf
{
  FILE *fh_;
public:
  PyInFileAdapter(FILE *fh) : fh_(fh) {}

  virtual ~PyInFileAdapter() {}

protected:
  virtual int_type uflow() {
    return getc(fh_);
  }

  virtual int_type underflow() {
    int c = getc(fh_);
    if (c != EOF) {
      ungetc(c, fh_);
    }
    return c;
  }

  virtual std::streamsize xsgetn(char *s, std::streamsize n) {
    return fread(s, 1, n, fh_);
  }

  virtual int_type pbackfail(int c) {
    return c == EOF ? EOF : ungetc(c, fh_);
  }

  virtual int_type sync() {
    return fflush(fh_);
  }
};
%}

// Adapter class that acts like a std::streambuf but delegates to a Python
// file-like object
%{
class PyInFilelikeAdapter : public std::streambuf
{
  PyObject *p_;
  // Last character peeked from the stream by underflow(), or -1
  int peeked_;
public:
  PyInFilelikeAdapter(PyObject *p) : p_(p), peeked_(-1) {}

  virtual ~PyInFilelikeAdapter() {
    if (peeked_ != -1) {
      IMP_WARN("One excess character read from Python stream - "
               "cannot be put back.")
    }
  }

protected:
  virtual int_type uflow() {
    int c;
    c = (peeked_ == -1 ? underflow() : peeked_);
    peeked_ = -1;
    return c;
  }

  virtual int_type underflow() {
    static char method[] = "read";
    static char fmt[] = "(i)";
    peeked_ = -1;
    PyObject *result = PyObject_CallMethod(p_, method, fmt, 1);
    if (!result) {
      // Python exception will be reraised when SWIG method finishes
      throw std::ostream::failure("Python error on read");
    } else {
      if (PyString_Check(result)) {
        if (PyString_Size(result) == 1) {
          int c = peeked_ = PyString_AsString(result)[0];
          Py_DECREF(result);
          return c;
        } else {
          Py_DECREF(result);
          return EOF;
        }
      } else {
        Py_DECREF(result);
        PyErr_SetString(PyExc_TypeError, "Python file-like object read method "
                        "should return a string");
        throw std::ostream::failure("Python error on read");
      }
    }
  }

  virtual std::streamsize xsgetn(char *s, std::streamsize n) {
    static char method[] = "read";
    static char fmt[] = "(i)";
    PyObject *result = PyObject_CallMethod(p_, method, fmt, n);
    if (!result) {
      throw std::ostream::failure("Python error on read");
    } else {
      if (PyString_Check(result)) {
        int len = PyString_Size(result);
        char *str = PyString_AsString(result);
        if (len > n) {
          Py_DECREF(result);
          PyErr_SetString(PyExc_IOError, "Python file-like object read method "
                          "returned data longer than the input buffer");
          throw std::ostream::failure("Python error on read");
        } else {
          memcpy(s, str, len);
          Py_DECREF(result);
          return len;
        }
      } else {
        Py_DECREF(result);
        PyErr_SetString(PyExc_TypeError, "Python file-like object read method "
                        "should return a string");
        throw std::ostream::failure("Python error on read");
      }
    }
  }
};
%}
