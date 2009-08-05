// Typemaps to allow Python director class methods to write to C++ std::ostream
// objects as if they were Python file-like objects; should only be included
// directly by the kernel so that the class is only defined there
%import "IMP_streams_kernel.i"

// Typemaps to allow Python file-like objects to be used for C++ code that
// expects a std::ostream

%typemap(in) std::ostream& (PyOutFileAdapter tmp) {
  $1 = tmp.set_python_file($input);
  if (!$1) {
    SWIG_fail;
  }
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
    tmp$argnum.pubsync();
  } catch (...) {
    Py_DECREF($result);
    if (!PyErr_Occurred()) {
      handle_imp_exception();
    }
    SWIG_fail;
  }
}

%typemap(typecheck) std::ostream& = PyObject *;

// Typemaps to allow Python file objects to be used for C++ code that
// expects a std::istream
%typemap(in) std::istream& (PyInFileAdapter tmp) {
  $1 = tmp.set_python_file($input);
  if (!$1) {
    SWIG_fail;
  }
}

%typemap(typecheck) std::istream& = PyObject *;


%{
// Adapter class that acts like an output std::streambuf but delegates to
// a Python file-like object, p
class PyOutFileAdapter : public std::streambuf
{
  std::vector<char> buffer_;
  std::ostream *ostr_;
  PyObject *write_method_;
public:
  PyOutFileAdapter() : buffer_(1024), ostr_(NULL), write_method_(NULL) {
    setp(&buffer_.front(), &buffer_.front() + buffer_.size());
  }

  virtual ~PyOutFileAdapter() { Py_XDECREF(write_method_); delete ostr_; }

  // Given a Python file object, return an ostream that will write to this
  // object, or NULL if the object is not suitable.
  std::ostream* set_python_file(PyObject *p) {
    if (!(write_method_ = PyObject_GetAttrString(p, "write"))) {
      return NULL;
    }

    ostr_ = new std::ostream(this);
    ostr_->exceptions(std::ostream::badbit);
    return ostr_;
  }

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
    static char fmt[] = "(s#)";
    int num = pptr() - pbase();
    if (num <= 0) {
      return 0;
    }
    PyObject *result = PyObject_CallFunction(write_method_, fmt, pbase(), num);
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
    if (static_cast<std::size_t>(n) > buffer_.size() * 2) {
      // Only take this route for large buffers, since two Python calls will
      // result per call (one here, potentially one in sync) rather than one per
      // buffer_.size() characters via the regular buffering
      sync();
      static char fmt[] = "(s#)";
      PyObject *result = PyObject_CallFunction(write_method_, fmt, s, n);
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

%{
// Base for input adapters
class InAdapter : public std::streambuf
{
};

// Adapter class that acts like an input std::streambuf but delegates to
// C-style stdio via a FILE pointer
class PyInCFileAdapter : public InAdapter
{
  FILE *fh_;
public:
  PyInCFileAdapter(FILE *fh) : fh_(fh) {}

  virtual ~PyInCFileAdapter() {}

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

// Adapter class that acts like an input std::streambuf but delegates to
// a Python file-like object
class PyInFilelikeAdapter : public InAdapter
{
  PyObject *read_method_;
  // Last character peeked from the stream by underflow(), or -1
  int peeked_;
public:
  PyInFilelikeAdapter(PyObject *read_method)
            : read_method_(read_method), peeked_(-1) {}

  virtual ~PyInFilelikeAdapter() {
    Py_DECREF(read_method_);
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
    static char fmt[] = "(i)";
    peeked_ = -1;
    PyObject *result = PyObject_CallFunction(read_method_, fmt, 1);
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
    static char fmt[] = "(i)";
    PyObject *result = PyObject_CallFunction(read_method_, fmt, n);
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

// Adapter class that delegates C++ stream input to a Python object.

// It is rather expensive to call the Python 'read' method for every character
// (and we cannot read multiple bytes from the Python file, since there is no
// way to put them back if we read too many; even if the stream is seekable
// there is no guarantee we can restore the file position unless it is opened
// in binary mode). Thus, we try to use the underlying FILE pointer (only
// available for real files, not for file-like objects) if possible. This may
// fail on Windows where different C runtimes can make FILE pointers unusable:
// http://www.python.org/doc/faq/windows/#pyrun-simplefile-crashes-on-windows-but-not-on-unix-why

// Note that this is still not optimal, since the streambuf is not buffered;
// uflow() or underflow() virtual methods will be called for each character
// (unless xsgetn can be used). This could be alleviated (at the expense of
// making the classes rather more complex) by buffering if the underlying file
// is seekable:
//   populate_buffer() {
//     pos_ = (fgetpos() or Python tell());
//     fill buffer with fread(buffer_size) or Python read(buffer_size);
//   }
//   sync() {
//     fsetpos(pos_) or Python seek(pos_);
//     chars_consumed = gptr() - eback();
//     read and discard char array with
//                      fread(chars_consumed) or Python read(chars_consumed);
//     empty buffer;
//   }
//   uflow() and underflow() call populate_buffer(); sync ensures that future
//   reads from the underlying file will reread characters currently between
//   gptr() and egptr(). Note that we cannot simply do
//   fseek(fh, gptr() - egptr(), SEEK_CUR) in sync since this doesn't work with
//   text files on Windows, or with Python file-like objects, where the only
//   inputs to seek() that have defined behavior are offsets previously
//   returned by tell().

class PyInFileAdapter
{
  InAdapter *streambuf_;
  std::istream *istr_;
public:
  PyInFileAdapter() : streambuf_(NULL), istr_(NULL) {}
  ~PyInFileAdapter() { delete streambuf_; delete istr_; }

  // Given a Python file object, return an istream that will read from this
  // object, or NULL if the object is not suitable.
  std::istream* set_python_file(PyObject *p) {
    // Is the object a 'real' C-style FILE ?
    bool real_file;
    try {
      real_file = (PyFile_Check(p) && ftell(PyFile_AsFile(p)) != -1);
    } catch(...) {
      real_file = false;
    }

    if (real_file) {
      streambuf_ = new PyInCFileAdapter(PyFile_AsFile(p));
    } else {
      PyObject *read_method;
      if (!(read_method = PyObject_GetAttrString(p, "read"))) {
        return NULL;
      }
      streambuf_ = new PyInFilelikeAdapter(read_method);
    }
    istr_ = new std::istream(streambuf_);
    istr_->exceptions(std::istream::badbit);
    return istr_;
  }
};
%}
