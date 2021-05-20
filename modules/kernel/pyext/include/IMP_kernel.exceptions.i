%{
  // make sure this is early enough
#include <IMP/exception.h>
%}

/* Get/set check level from Python */
#ifdef IMP_SWIG_KERNEL
namespace IMP
{
    enum CheckLevel {DEFAULT_CHECK=-1, NONE=IMP_NONE, USAGE=IMP_USAGE,
                     USAGE_AND_INTERNAL=IMP_INTERNAL};
    void set_check_level(CheckLevel tf);
    CheckLevel get_check_level();
}
#endif

/* Create Python exception classes at startup to mirror C++ classes, if we're
   building the kernel. If we're building a module, import these classes from
   the kernel. */
%define CREATE_EXCEPTION_CLASS(VAR, CNAME)
#if defined(IMP_SWIG_KERNEL)
VAR = PyErr_NewException((char *)"_IMP_kernel.CNAME", imp_exception, NULL);
Py_INCREF(VAR);
PyModule_AddObject(m, "CNAME", VAR)
#else
VAR = PyDict_GetItemString(base_dict, "CNAME")
#endif
%enddef

%define CREATE_EXCEPTION_CLASS_PYTHON(VAR, CNAME, PYNAME)
#if !defined(IMP_SWIG_KERNEL)
CREATE_EXCEPTION_CLASS(VAR, CNAME)
#else
do {
  PyObject *base_tuple = PyTuple_Pack(2, imp_exception, PyExc_##PYNAME);
  /* PyErr_NewException only takes a tuple in Python 2.5 or later. */
  VAR = PyErr_NewException((char *)"_IMP_kernel.CNAME", base_tuple, NULL);
  Py_INCREF(VAR);
  Py_DECREF(base_tuple);
  PyModule_AddObject(m, "CNAME", VAR);
} while(0)
#endif
%enddef

%init {
  {
    /* Create or load kernel exception class */
#ifdef IMP_SWIG_KERNEL
    imp_exception = PyErr_NewException((char *)"_IMP_kernel.Exception", NULL, NULL);
    Py_INCREF(imp_exception);
    PyModule_AddObject(m, "Exception", imp_exception);
#else
    PyObject *base = PyImport_ImportModule("_IMP_kernel");
    PyObject *base_dict = PyModule_GetDict(base);
    imp_exception = PyDict_GetItemString(base_dict, "Exception");
#endif

    /* Create or load exception subclasses */
    CREATE_EXCEPTION_CLASS(imp_internal_exception, InternalException);
    CREATE_EXCEPTION_CLASS(imp_model_exception, ModelException);
    CREATE_EXCEPTION_CLASS(imp_usage_exception, UsageException);
    CREATE_EXCEPTION_CLASS(imp_event_exception, EventException);

    /* Create or load subclasses that also derive from Python classes */
    CREATE_EXCEPTION_CLASS_PYTHON(imp_index_exception, IndexException,
                                  IndexError);
    CREATE_EXCEPTION_CLASS_PYTHON(imp_io_exception, IOException, IOError);
    CREATE_EXCEPTION_CLASS_PYTHON(imp_value_exception, ValueException,
                                  ValueError);
    CREATE_EXCEPTION_CLASS_PYTHON(imp_type_exception, TypeException,
                                  TypeError);

#ifndef IMP_SWIG_KERNEL
    Py_DECREF(base);
#endif
  }
}

/* Make sure that exception classes are visible to Python */
#ifdef IMP_SWIG_KERNEL
%pythoncode %{
from _IMP_kernel import Exception, InternalException, ModelException, EventException
from _IMP_kernel import UsageException, IndexException, IOException, ValueException
from _IMP_kernel import TypeException
%}
#endif

%{
static PyObject *imp_exception, *imp_internal_exception, *imp_model_exception,
                *imp_usage_exception, *imp_index_exception, *imp_io_exception,
    *imp_value_exception, *imp_event_exception, *imp_type_exception;
%}

%{
#include <boost/version.hpp>
#if !defined(BOOST_FILESYSTEM_VERSION)
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
#endif

  /* Code to convert C++ exceptions into scripting language errors. Saves
     having lots of catch statements in every single wrapper. */
  static void handle_imp_exception(void)
  {
    try {
      throw;
    /* Map std:: exceptions to IMP equivalents */
    } catch (const std::out_of_range &e) {
      PyErr_SetString(imp_index_exception, e.what());
    } catch (const std::domain_error &e) {
      PyErr_SetString(imp_value_exception, e.what());
    } catch (const std::ios::failure &e) {
      PyErr_SetString(imp_io_exception, e.what());
    } catch (const std::length_error &e) {
      /* Internal error, such as attempt to resize a vector beyond max size */
      PyErr_SetString(imp_internal_exception, e.what());
    /* Map IMP exceptions to Python objects */
    } catch (const IMP::IndexException &e) {
      PyErr_SetString(imp_index_exception, e.what());
    } catch (const IMP::ValueException &e) {
      PyErr_SetString(imp_value_exception, e.what());
    } catch (const IMP::InternalException &e) {
      PyErr_SetString(imp_internal_exception, e.what());
    } catch (const IMP::TypeException &e) {
      PyErr_SetString(imp_type_exception, e.what());
    } catch (const IMP::ModelException &e) {
      PyErr_SetString(imp_model_exception, e.what());
    } catch (const IMP::UsageException &e) {
      PyErr_SetString(imp_usage_exception, e.what());
    } catch (const IMP::IOException &e) {
      PyErr_SetString(imp_io_exception, e.what());
    } catch (const IMP::EventException &e) {
      PyErr_SetString(imp_event_exception, e.what());
    } catch (const IMP::Exception &e) {
      PyErr_SetString(imp_exception, e.what());
    /* Map Boost exceptions to Python exceptions */
    } catch (boost::filesystem::filesystem_error &e) {
      PyErr_SetString(imp_io_exception, e.what());
    /* Catch memory allocation errors, if raised */
    } catch (const std::bad_alloc &e) {
      PyErr_SetString(PyExc_MemoryError, e.what());
    /* Catch any other exceptions raised */
    } catch (const std::exception &e) {
      PyErr_SetString(PyExc_RuntimeError,
                      e.what());
    } catch (const boost::exception &e) {
      PyErr_SetString(PyExc_RuntimeError,
                      boost::diagnostic_information(e).c_str());
    } catch (...) {
      PyErr_SetString(PyExc_RuntimeError,
                      "Unknown error caught by Python wrapper");
    }
  /* SWIG_exception contains "goto fail" so make sure the label is defined */
  fail:
    return;
  }
%}

%exception {
  try {
    $action
  } catch (...) {
    // If Python error indicator is set (e.g. from a failed director method),
    // it will be reraised at the end of the method
    if (!PyErr_Occurred()) {
      handle_imp_exception();
    }
    SWIG_fail;
  }
// If we're doing cleanup as a result of a previous Python exception
// (e.g. StopIteration), don't return a value (otherwise we'll get a
// SystemError "returned a result with an error set").
// SWIG doesn't appear to allow us to do this only for delete_* wrappers,
// and the C preprocessor isn't up to the job, so #ifdelete is mapped to
// #if 0 or #if 1 by tools/build/make_swig_wrapper.py
%#ifdelete $symname
  if (PyErr_Occurred()) {
    SWIG_fail;
  }
%#endif
}

// If Python exceptions are raised in a director method, temporarily reraise
// them as C++ exceptions (will be finally handled as Python exceptions
// again by %exception)
%feature("director:except") {
  if ($error != NULL) {
    throw Swig::DirectorMethodException();
  }
}
