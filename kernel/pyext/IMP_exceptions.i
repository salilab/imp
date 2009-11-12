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

#define CREATE_EXCEPTION_CLASS_PYTHON(VAR, CNAME, PYNAME) \
do { \
  PyObject *base_tuple = PyTuple_Pack(2, exception, PyExc_##PYNAME); \
  VAR = PyErr_NewException((char *)"_IMP.CNAME", base_tuple, NULL); \
  Py_INCREF(VAR); \
  Py_DECREF(base_tuple); \
  PyModule_AddObject(m, "CNAME", VAR); \
} while(0)

  /* Create base exception class */
  exception = PyErr_NewException((char *)"_IMP.Exception", NULL, NULL);
  Py_INCREF(exception);
  PyModule_AddObject(m, "Exception", exception);

  /* Create exception subclasses */
  CREATE_EXCEPTION_CLASS(internal_exception, InternalException);
  CREATE_EXCEPTION_CLASS(model_exception, ModelException);
  CREATE_EXCEPTION_CLASS(usage_exception, UsageException);

  /* Create subclasses that also derive from Python classes */
  CREATE_EXCEPTION_CLASS_PYTHON(index_exception, IndexException, IndexError);
  CREATE_EXCEPTION_CLASS_PYTHON(io_exception, IOException, IOError);
  CREATE_EXCEPTION_CLASS_PYTHON(value_exception, ValueException, ValueError);
}

/* Make sure that exception classes are visible to Python */
%pythoncode %{
from _IMP import Exception, InternalException, ModelException
from _IMP import UsageException, IndexException, IOException, ValueException
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
