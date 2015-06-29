Exporting C++ code to Python {#swig}
============================

%IMP uses SWIG to wrap code C++ code and export it to Python. Since SWIG is
relatively complicated, we provide a number of helper macros and an example
file (see `modules/example/pyext/swig.i-in`). The key bits are
- the information goes into a file called `swig.i-in` in the module `pyext` directory
- the first part should be one `IMP_SWIG_VALUE(),` `IMP_SWIG_OBJECT()` or
  `IMP_SWIG_DECORATOR()` line per value type, object type or decorator object
  the module exports to Python. Each of these lines looks like

      IMP_SWIG_VALUE(IMP::module_namespace, ClassName, ClassNames);

- then there should be a number of `%%include` lines, one per header file
  in the module which exports a class or function to Python. The header files
  must be in order such that no class is used before a declaration for it
  is encountered (SWIG does not do recursive inclusion)
- finally, any templates that are to be exported to SWIG must have a
  `%%template` call. It should look something like

      namespace IMP {
        namespace module_namespace {
           %template(PythonName) CPPName<Restraint, 3>;
        }
      }
