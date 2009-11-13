/* IMP exception handling */

/* Runtime check functions from exception.h */
namespace IMP
{
enum CheckLevel {NONE=0, USAGE=1, USAGE_AND_INTERNAL=2};
void set_check_level(CheckLevel tf);
CheckLevel get_check_level();
void set_print_exceptions(bool tf);
}

/* Python prints exception messages */                        
%pythoncode %{                                                
set_print_exceptions(False)
%}

/* Create Python exception classes at startup to mirror C++ classes */
%init {
#define CREATE_EXCEPTION_CLASS(VAR, CNAME) \
VAR = PyErr_NewException((char *)"_IMP.CNAME", exception, NULL); \
Py_INCREF(VAR); \
PyModule_AddObject(m, "CNAME", VAR)

  /* Create base exception class */
  exception = PyErr_NewException((char *)"_IMP.Exception", NULL, NULL);
  Py_INCREF(exception);
  PyModule_AddObject(m, "Exception", exception);

  /* Create exception subclasses */
  CREATE_EXCEPTION_CLASS(internal_exception, InternalException);
  CREATE_EXCEPTION_CLASS(model_exception, ModelException);
  CREATE_EXCEPTION_CLASS(usage_exception, UsageException);
  CREATE_EXCEPTION_CLASS(index_exception, IndexException);
  CREATE_EXCEPTION_CLASS(io_exception, IOException);
  CREATE_EXCEPTION_CLASS(value_exception, ValueException);
}

/* Make sure that exception classes are visible to Python, and make certain
   subclasses also derive from Python builtins
   (Note: we can also do this by passing a tuple as the second argument to
   PyErr_NewException, but that only works with Python 2.5 or later.) */
%pythoncode %{
from _IMP import Exception, InternalException, ModelException
from _IMP import UsageException, IndexException, IOException, ValueException

IndexException.__bases__ += (IndexError,)
IOException.__bases__ += (IOError,)
ValueException.__bases__ += (ValueError,)
%}

%{
static PyObject *exception, *internal_exception, *model_exception,
                *usage_exception, *index_exception, *io_exception,
                *value_exception;
%}

%{
  /* Code to convert C++ exceptions into scripting language errors. Saves
     having lots of catch statements in every single wrapper. */
  static void handle_imp_exception(void)
  {
    try {
      throw;
    /* Map std:: exceptions to IMP equivalents */
    } catch (std::out_of_range &e) {
      PyErr_SetString(index_exception, e.what());
    } catch (std::domain_error &e) {
      PyErr_SetString(value_exception, e.what());
    } catch (std::ios::failure &e) {
      PyErr_SetString(io_exception, e.what());
    /* Map IMP exceptions to Python objects */
    } catch (IMP::IndexException &e) {
      PyErr_SetString(index_exception, e.what());
    } catch (IMP::ValueException &e) {
      PyErr_SetString(value_exception, e.what());
    } catch (IMP::InternalException &e) {
      PyErr_SetString(internal_exception, e.what());
    } catch (IMP::ModelException &e) {
      PyErr_SetString(model_exception, e.what());
    } catch (IMP::UsageException &e) {
      PyErr_SetString(usage_exception, e.what());
    } catch (IMP::IOException &e) {
      PyErr_SetString(io_exception, e.what());
    } catch (IMP::Exception &e) {
      PyErr_SetString(exception, e.what());
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
}

// If Python exceptions are raised in a director method, temporarily reraise
// them as C++ exceptions (will be finally handled as Python exceptions
// again by %exception)
%feature("director:except") {
  if ($error != NULL) {
    throw Swig::DirectorMethodException();
  }
}
