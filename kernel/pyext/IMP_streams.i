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

%typemap(typecheck) std::ostream& = PyObject *;

%{
// Adapter class that acts like a std::streambuf but delegates to a Python
// file-like object, p
class PyOutFileAdapter : public std::streambuf
{
  PyObject *p_;
public:
  PyOutFileAdapter(PyObject *p) : p_(p) { Py_XINCREF(p_); }

  virtual ~PyOutFileAdapter() { Py_XDECREF(p_); }

protected:
  virtual int_type overflow(int_type c) {
    if (c != EOF) {
      char buf[2];
      buf[0] = static_cast<char>(c); buf[1] = '\0';
      xsputn(buf, 1);
    }
    return c;
  }

  virtual std::streamsize xsputn(const char *s, std::streamsize num) {
    // Python API uses char* arguments rather than const char*, so create
    // here to quell the compiler warning
    static char method[] = "write";
    static char fmt[] = "(s#)";
    PyObject *result = PyObject_CallMethod(p_, method, fmt, s, num);
    if (!result) {
      // Python exception will be reraised when SWIG method finishes
      throw std::ostream::failure("Python error on write");
    } else {
      Py_DECREF(result);
    }
    return num;
  }
};
%}


// Typemaps to allow Python file objects to be used for C++ code that
// expects a std::istream
// Note: only 'real' file objects can be used (not file-like objects), i.e.
// the objects must contain a FILE pointer, not simply a 'read' method. This
// is because file-like objects do not support operations such as
// ungetc, and the overhead of a Python method call for every getc is
// probably too great.

%typemap(in) std::istream& (PyInFileAdapter *tmp=NULL) {
  if (!PyFile_Check($input)) {
    SWIG_exception(SWIG_TypeError,
               "Can only use Python file objects here (not file-like objects), "
               "in method $symname, argument number $argnum");
  }
  tmp = new PyInFileAdapter(PyFile_AsFile($input));
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

  virtual int_type pbackfail(int c) {
    return c == EOF ? EOF : ungetc(c, fh_);
  }

  virtual int_type sync() {
    return fflush(fh_);
  }
};
%}
