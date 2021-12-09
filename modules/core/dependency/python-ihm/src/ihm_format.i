%module _format

%{
#include <stdlib.h>
#include "ihm_format.h"
%}

/* Get simple return values */
%apply int *OUTPUT { int * };

%ignore ihm_keyword;
%ignore ihm_error_set;

/* Convert ihm_error to a Python exception */

%init {
  file_format_error = PyErr_NewException("_format.FileFormatError", NULL, NULL);
  Py_INCREF(file_format_error);
  PyModule_AddObject(m, "FileFormatError", file_format_error);
}

%{
static PyObject *file_format_error;

static void handle_error(struct ihm_error *err)
{
  PyObject *py_err_type = PyExc_IOError;
  switch(err->code) {
  case IHM_ERROR_FILE_FORMAT:
    py_err_type = file_format_error;
    break;
  case IHM_ERROR_VALUE:
    py_err_type = PyExc_ValueError;
    break;
  case IHM_ERROR_IO:
    py_err_type = PyExc_IOError;
    break;
  }
  /* Don't overwrite a Python exception already raised (e.g. by a callback) */
  if (!PyErr_Occurred()) {
    PyErr_SetString(py_err_type, err->msg);
  }
  ihm_error_free(err);
}
%}

%typemap(in, numinputs=0) struct ihm_error **err (struct ihm_error *temp) {
  temp = NULL;
  $1 = &temp;
}
%typemap(argout) struct ihm_error **err {
  if (*$1) {
    handle_error(*$1);
    Py_DECREF(resultobj);
    SWIG_fail;
  }
}

%{

/* Read data from a Python filelike object */
static ssize_t pyfile_read_callback(char *buffer, size_t buffer_len,
                                    void *data, struct ihm_error **err)
{
  Py_ssize_t read_len;
  char *read_str;
  static char fmt[] = "(n)";
  PyObject *bytes = NULL;
  PyObject *read_method = data;
  /* Note that we can read up to `buffer_len` *bytes*, but Python's read()
     can return Unicode *characters*. One Unicode character can require up to
     4 bytes to be represented with UTF-8, so limit the read accordingly.
     (mmCIF files are supposed to be ASCII but we should be liberal in what
     we accept.) */
  PyObject *result = PyObject_CallFunction(read_method, fmt, buffer_len / 4);
  if (!result) {
    ihm_error_set(err, IHM_ERROR_VALUE, "read failed");
    return -1;
  }

#if PY_VERSION_HEX < 0x03000000
  if (PyString_Check(result)) {
    if (PyString_AsStringAndSize(result, &read_str, &read_len) < 0) {
      ihm_error_set(err, IHM_ERROR_VALUE, "string creation failed");
      Py_DECREF(result);
      return -1;
    }
  } else if (PyUnicode_Check(result)) {
    if (!(bytes = PyUnicode_AsUTF8String(result))) {
      ihm_error_set(err, IHM_ERROR_VALUE, "string to bytes failed");
      Py_DECREF(result);
      return -1;
    }
    if (PyBytes_AsStringAndSize(bytes, &read_str, &read_len) < 0) {
      ihm_error_set(err, IHM_ERROR_VALUE, "string creation failed");
      Py_DECREF(result);
      return -1;
    }
#else
  if (PyUnicode_Check(result)) {
    /* This returns const char * on Python 3.7 or later */
    if (!(read_str = (char *)PyUnicode_AsUTF8AndSize(result, &read_len))) {
      ihm_error_set(err, IHM_ERROR_VALUE, "string creation failed");
      Py_DECREF(result);
      return -1;
    }
  } else if (PyBytes_Check(result)) {
    char *bytes_buffer;
    Py_ssize_t bytes_length;
    bytes = result;
    /* Convert to Unicode. Since we don't know the encoding, choose something
       permissive (latin-1). mmCIF files are supposed to be ASCII anyway. */
    if (PyBytes_AsStringAndSize(bytes, &bytes_buffer, &bytes_length) < 0) {
      Py_DECREF(bytes);
      ihm_error_set(err, IHM_ERROR_VALUE, "PyBytes_AsStringAndSize failed");
      return -1;
    }
    result = PyUnicode_DecodeLatin1(bytes_buffer, bytes_length, NULL);
    Py_DECREF(bytes);
    if (!result) {
      ihm_error_set(err, IHM_ERROR_VALUE, "latin1 string creation failed");
      return -1;
    }
    /* This returns const char * on Python 3.7 or later */
    if (!(read_str = (char *)PyUnicode_AsUTF8AndSize(result, &read_len))) {
      ihm_error_set(err, IHM_ERROR_VALUE, "string creation failed");
      Py_DECREF(result);
      return -1;
    }
#endif
  } else {
    ihm_error_set(err, IHM_ERROR_VALUE, "read method should return a string");
    Py_DECREF(result);
    return -1;
  }

  if (read_len > buffer_len) {
    ihm_error_set(err, IHM_ERROR_VALUE,
                  "Python read method returned too many bytes");
#if PY_VERSION_HEX < 0x03000000
    Py_XDECREF(bytes);
#endif
    Py_DECREF(result);
    return -1;
  }

  memcpy(buffer, read_str, read_len);
#if PY_VERSION_HEX < 0x03000000
  Py_XDECREF(bytes);
#endif
  Py_DECREF(result);
  return read_len;
}

static void pyfile_free(void *data)
{
  PyObject *read_method = data;
  Py_DECREF(read_method);
}

%}


%inline %{
/* Wrap a Python file object as an ihm_file */
struct ihm_file *ihm_file_new_from_python(PyObject *pyfile,
                                          struct ihm_error **err)
{
  PyObject *read_method;

#if !defined(_WIN32) && !defined(_WIN64) && PY_VERSION_HEX < 0x03000000
  /* Use the file descriptor directly if the Python file is a real file,
     except on Windows where we can't reliably tell if we and Python are
     using the same C runtime (if we're not, we can't pass file descriptors),
     or on Python 3 where the file descriptor may not be correct
     (e.g. PyObject_AsFileDescriptor() returns a valid descriptor for a
     file opened with gzip.open()) */
  if (PyFile_Check(pyfile)) {
    int fd = fileno(PyFile_AsFile(pyfile));
    return ihm_file_new_from_fd(fd);
  }
#endif

  /* Otherwise, look for a read() method and use that */
  if (!(read_method = PyObject_GetAttrString(pyfile, "read"))) {
    ihm_error_set(err, IHM_ERROR_VALUE, "no read method");
    return NULL;
  }

  return ihm_file_new(pyfile_read_callback, read_method, pyfile_free);
}

%}

%{
struct category_handler_data {
  /* The Python callable object that is given the data */
  PyObject *callable;
  /* Python value used for keywords not in the file (usually None) */
  PyObject *not_in_file;
  /* Python value used for keywords marked as omitted, '.' (usually None) */
  PyObject *omitted;
  /* Python value used for keywords marked as unknown, '?' (usually "?") */
  PyObject *unknown;
  /* The number of keywords in the category that we extract from the file */
  int num_keywords;
  /* Array of the keywords */
  struct ihm_keyword **keywords;
};

static void category_handler_data_free(void *data)
{
  struct category_handler_data *hd = data;
  Py_DECREF(hd->callable);
  Py_XDECREF(hd->not_in_file);
  Py_XDECREF(hd->omitted);
  Py_XDECREF(hd->unknown);
  /* Don't need to free each hd->keywords[i] as the ihm_reader owns
     these pointers */
  free(hd->keywords);
  free(hd);
}

/* Called for each category (or loop construct data line) with data */
static void handle_category_data(struct ihm_reader *reader, void *data,
                                 struct ihm_error **err)
{
  int i;
  struct category_handler_data *hd = data;
  struct ihm_keyword **keys;
  PyObject *ret, *tuple;

  /* make a tuple of the data */
  tuple = PyTuple_New(hd->num_keywords);
  if (!tuple) {
    ihm_error_set(err, IHM_ERROR_VALUE, "tuple creation failed");
    return;
  }

  for (i = 0, keys = hd->keywords; i < hd->num_keywords; ++i, ++keys) {
    PyObject *val;
    if (!(*keys)->in_file) {
      val = hd->not_in_file;
      Py_INCREF(val);
    } else if ((*keys)->omitted) {
      val = hd->omitted;
      Py_INCREF(val);
    } else if ((*keys)->unknown) {
      val = hd->unknown;
      Py_INCREF(val);
    } else {
#if PY_VERSION_HEX < 0x03000000
      val = PyString_FromString((*keys)->data);
#else
      val = PyUnicode_FromString((*keys)->data);
#endif
      if (!val) {
        ihm_error_set(err, IHM_ERROR_VALUE, "string creation failed");
        Py_DECREF(tuple);
        return;
      }
    }
    /* Steals ref to val */
    PyTuple_SET_ITEM(tuple, i, val);
  }

  /* pass the data to Python */
  ret = PyObject_CallObject(hd->callable, tuple);
  Py_DECREF(tuple);
  if (ret) {
    Py_DECREF(ret); /* discard return value */
  } else {
    /* Pass Python exception back to the original caller */
    ihm_error_set(err, IHM_ERROR_VALUE, "Python error");
  }
}

/* Called at the end of each save frame for each category */
static void end_frame_category(struct ihm_reader *reader, void *data,
                               struct ihm_error **err)
{
  PyObject *ret;
  struct category_handler_data *hd = data;

  ret = PyObject_CallMethod(hd->callable, "end_save_frame", NULL);
  if (ret) {
    Py_DECREF(ret); /* discard return value */
  } else {
    /* Pass Python exception back to the original caller */
    ihm_error_set(err, IHM_ERROR_VALUE, "Python error");
  }
}

static struct category_handler_data *do_add_handler(
                        struct ihm_reader *reader, char *name,
                        PyObject *keywords, PyObject *callable,
                        ihm_category_callback data_callback,
                        ihm_category_callback end_frame_callback,
                        ihm_category_callback finalize_callback,
                        struct ihm_error **err)
{
  Py_ssize_t seqlen, i;
  struct ihm_category *category;
  struct category_handler_data *hd;

  if (!PySequence_Check(keywords)) {
    ihm_error_set(err, IHM_ERROR_VALUE, "'keywords' should be a sequence");
    return NULL;
  }
  if (!PyCallable_Check(callable)) {
    ihm_error_set(err, IHM_ERROR_VALUE,
                  "'callable' should be a callable object");
    return NULL;
  }
  seqlen = PySequence_Length(keywords);
  hd = malloc(sizeof(struct category_handler_data));
  Py_INCREF(callable);
  hd->callable = callable;
  hd->not_in_file = NULL;
  hd->omitted = NULL;
  hd->unknown = NULL;
  hd->num_keywords = seqlen;
  hd->keywords = malloc(sizeof(struct ihm_keyword *) * seqlen);
  category = ihm_category_new(reader, name, data_callback, end_frame_callback,
                              finalize_callback, hd,
                              category_handler_data_free);
  if (!(hd->not_in_file = PyObject_GetAttrString(callable, "not_in_file"))
      || !(hd->omitted = PyObject_GetAttrString(callable, "omitted"))
      || !(hd->unknown = PyObject_GetAttrString(callable, "unknown"))) {
    ihm_error_set(err, IHM_ERROR_VALUE, "missing attribute");
    return NULL;
  }
  for (i = 0; i < seqlen; ++i) {
    PyObject *o = PySequence_GetItem(keywords, i);
#if PY_VERSION_HEX < 0x03000000
    if (PyString_Check(o)) {
      hd->keywords[i] = ihm_keyword_new(category, PyString_AsString(o));
#else
    if (PyUnicode_Check(o)) {
      hd->keywords[i] = ihm_keyword_new(category, PyUnicode_AsUTF8(o));
#endif
      Py_DECREF(o);
    } else {
      Py_XDECREF(o);
      ihm_error_set(err, IHM_ERROR_VALUE,
                    "keywords[%ld] should be a string", i);
      return NULL;
    }
  }
  return hd;
}

/* Pass unknown category info to a Python callable */
static void unknown_category_python(struct ihm_reader *reader,
                                    const char *category, int linenum,
                                    void *data, struct ihm_error **err)
{
  static char fmt[] = "(si)";
  PyObject *callable = data;
  PyObject *result = PyObject_CallFunction(callable, fmt, category, linenum);
  if (!result) {
    ihm_error_set(err, IHM_ERROR_VALUE, "Python error");
  } else {
    Py_DECREF(result);
  }
}

/* Pass unknown keyword info to a Python callable */
static void unknown_keyword_python(struct ihm_reader *reader,
                                   const char *category, const char *keyword,
                                   int linenum, void *data,
                                   struct ihm_error **err)
{
  static char fmt[] = "(ssi)";
  PyObject *callable = data;
  PyObject *result = PyObject_CallFunction(callable, fmt, category, keyword,
                                           linenum);
  if (!result) {
    ihm_error_set(err, IHM_ERROR_VALUE, "Python error");
  } else {
    Py_DECREF(result);
  }
}

/* Treat data as a Python object, and decrease its refcount */
static void free_python_callable(void *data)
{
  PyObject *obj = data;
  Py_DECREF(obj);
}

%}

%inline %{
/* Add a handler for unknown categories */
void add_unknown_category_handler(struct ihm_reader *reader,
                                  PyObject *callable, struct ihm_error **err)
{
  if (!PyCallable_Check(callable)) {
    ihm_error_set(err, IHM_ERROR_VALUE,
                  "'callable' should be a callable object");
    return;
  }
  Py_INCREF(callable);
  ihm_reader_unknown_category_callback_set(reader, unknown_category_python,
                                           callable, free_python_callable);
}

/* Add a handler for unknown keywords */
void add_unknown_keyword_handler(struct ihm_reader *reader,
                                 PyObject *callable, struct ihm_error **err)
{
  if (!PyCallable_Check(callable)) {
    ihm_error_set(err, IHM_ERROR_VALUE,
                  "'callable' should be a callable object");
    return;
  }
  Py_INCREF(callable);
  ihm_reader_unknown_keyword_callback_set(reader, unknown_keyword_python,
                                          callable, free_python_callable);
}

/* Add a generic category handler which collects all specified keywords for
   the given category and passes them to a Python callable */
void add_category_handler(struct ihm_reader *reader, char *name,
                          PyObject *keywords, PyObject *callable,
                          struct ihm_error **err)
{
  do_add_handler(reader, name, keywords, callable, handle_category_data,
                 end_frame_category, NULL, err);
}
%}

%{
/* Called for each _pdbx_poly_seq_scheme line */
static void handle_poly_seq_scheme_data(struct ihm_reader *reader,
                                        void *data, struct ihm_error **err)
{
  int i, seq_id, auth_seq_num;
  char *seq_id_endptr, *auth_seq_num_endptr;
  struct category_handler_data *hd = data;
  struct ihm_keyword **keys;

  /* If both asym_id (1st keyword) and pdb_strand_id (5th keyword) are
     present, but different, call the Python handler */
  if (hd->keywords[0]->in_file && hd->keywords[4]->in_file &&
      !hd->keywords[0]->omitted && !hd->keywords[4]->omitted &&
      !hd->keywords[0]->unknown && !hd->keywords[4]->unknown &&
      strcmp(hd->keywords[0]->data, hd->keywords[4]->data) != 0) {
    handle_category_data(reader, data, err);
    return;
  }

  for (i = 0, keys = hd->keywords; i < 3; ++i, ++keys) {
    /* Do nothing if seq_id or auth_seq_num is missing */
    if (!(*keys)->in_file || (*keys)->omitted || (*keys)->unknown) {
      return;
    }
  }

  /* If seq_id (2nd keyword) and auth_seq_num (3rd keyword) are identical
     integers, and pdb_ins_code (4th keyword) is blank or missing,
     nothing needs to be done */
  seq_id = strtol(hd->keywords[1]->data, &seq_id_endptr, 10);
  auth_seq_num = strtol(hd->keywords[2]->data, &auth_seq_num_endptr, 10);
  if (!*seq_id_endptr && !*auth_seq_num_endptr && seq_id == auth_seq_num
      && (!hd->keywords[3]->in_file || hd->keywords[3]->omitted
          || hd->keywords[3]->unknown)) {
    return;
  } else {
    /* Otherwise, call the normal handler */
    handle_category_data(reader, data, err);
  }
}
%}

%inline %{
/* Add a handler specifically for the _pdbx_poly_seq_scheme table.
   This speeds up processing by skipping the callback to Python in
   the common case where seq_id==auth_seq_num, asym_id==pdb_strand_id,
   and pdb_ins_code is blank */
void add_poly_seq_scheme_handler(struct ihm_reader *reader, char *name,
                                 PyObject *keywords, PyObject *callable,
                                 struct ihm_error **err)
{
  struct category_handler_data *hd;
  hd = do_add_handler(reader, name, keywords, callable,
                      handle_poly_seq_scheme_data, NULL, NULL, err);
  if (hd) {
    /* Make sure the Python handler and the C handler agree on the order
       of the keywords */
    assert(hd->num_keywords >= 5);
    assert(strcmp(hd->keywords[1]->name, "seq_id") == 0);
    assert(strcmp(hd->keywords[2]->name, "auth_seq_num") == 0);
    assert(strcmp(hd->keywords[3]->name, "pdb_ins_code") == 0);
    assert(strcmp(hd->keywords[4]->name, "pdb_strand_id") == 0);
  }
}

/* Test function so we can make sure finalize callbacks work */
void _test_finalize_callback(struct ihm_reader *reader, char *name,
                             PyObject *keywords, PyObject *callable,
                             struct ihm_error **err)
{
  do_add_handler(reader, name, keywords, callable,
                 handle_category_data, NULL, handle_category_data, err);
}

%}

%include "ihm_format.h"
