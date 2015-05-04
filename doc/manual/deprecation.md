Deprecation {#deprecation}
===========

Sometimes it is useful to drop support for code for various reasons, for example
- it represents a failed experiment
- there is better functionality that replaced it
- it wasn't used
- it is broken and not worth fixing

For such code that will be removed, our policy is to mark it as deprecated
for one stable release (e.g. 2.1) and then remove it in the next one (2.2).

\note If you deprecate code in favor of some new mechanism, it is your
      responsibility to update all callers of the old code in %IMP
      (C++ code, test cases, examples, benchmarks) to use the new way of
      doing things, and ensure the test cases still pass.

Code that is deprecated must produce warnings when used. (You can also force
usage of deprecated code to trigger an exception by calling
IMP::set_deprecation_exceptions() or by passing the `--deprecation_exceptions`
command line flag.)

C++
---

C++ code should be marked in the following way (where EXAMPLE is replaced by
your module name and 2.1 is replaced by the release where the code is
deprecated):
- macros should have an `IMPEXAMPLE_DEPRECATED_MACRO(version, replacement)` line added within their definition

          #define MY_DEPRECATED_MACRO(args)                                                     \
                  IMPEXAMPLE_DEPRECATED_MACRO(2.1, "You should use MY_NEW_MACRO(args) instead") \
                  do stuff....

- class methods should have `IMPEXAMPLE_DEPRECATED_METHOD_DECL(version)` added to the end of the declaration and `IMPEXAMPLE_DEPRECATED_METHOD_DEF(version, message)` added in their body

         class IMPEXAMPLEEXPORT MyClass {
           IMPEXAMPLE_DEPRECATED_METHOD_DECL(2.1)
           void my_deprecated_method(args) {
             IMPEXAMPLE_DEPRECATED_METHOD_DEF(2.1, "Use my_new_method(args) instead");
             do stuff....
           }

- functions should have `IMPEXAMPLE_DEPRECATED_FUNCTION_DECL(version)` added to the end of the declaration and `IMPEXAMPLE_DEPRECATED_FUNCTION_DEF(version, message)` added in their body

           IMPEXAMPLE_DEPRECATED_FUNCTION_DECL(2.1)
             void my_deprecated_function(args);

           void my_deprecated_function(args) {
              IMPEXAMPLE_DEPRECATED_FUNCTION_DEF(2.1, "Use my_new_function(args) instead");
              do stuff....
           }

- classes should have `IMPEXAMPLE_DEPRECATED_OBJECT_DECL(version)` or `IMPEXAMPLE_DEPRECATED_VALUE_DECL(version)` added before their constructor declarations and `IMPEXAMPLE_DEPRECATED_OBJECT_DEF(version, message)` or `IMPEXAMPLE_DEPRECATED_VALUE_DEF(version, message)` added in their constructor bodies.

         class IMPEXAMPLEEXPORT MyDeprecatedClass :: public IMP::Object {
         public:
              IMPEXAMPLE_DEPRECATED_OBJECT_DECL(2.1)
              MyDeprecatedClass(args) {
                 IMPEXAMPLE_DEPRECATED_OBJECT_DEF(2.1, "Use MyNewClass instead");
              }
              IMPEXAMPLE_DEPRECATED_OBJECT_DECL(2.1)
              MyDeprecatedClass(other_args) {
                 IMPEXAMPLE_DEPRECATED_OBJECT_DEF(2.1, "Use MyNewClass instead");
              }
         };

- Headers should have `IMPEXAMPLE_DEPRECATED_HEADER(version, message)` in them.

          #ifndef IMP_MY_DEPRECATED_HEADER_H
          #define IMP_MY_DEPRECATED_HEADER_H
          IMPEXAMPLE_DEPREACTED_HEADER(2.1, "Use my_new_header.h")
          ...
          #endif // IMP_MY_DEPRECATED_HEADER_H


- All things should also use the `\deprecated_at` doxygen macro in their docs:

          /** \\deprecated_at{2.1} Replaced by my_new_function(). */

These will provide documentation, and runtime and compile time warning messages to users.

Python
------

For Python code, we provide similar functions and decorators to mark methods,
classes, methods, or functions as deprecated:

    IMP.deprecated_module("2.1", __name__, "Use my_new_module instead")

    @IMP.deprecated_object("2.1", "Use MyNewClass instead")
    class MyClass(object):
        @IMP.deprecated_method("2.1", "Use my_new_method(args) instead")
        def my_deprecated_method(self):
            do stuff...

    @IMP.deprecated_function("2.1", "Use my_new_function(args) instead")
    def my_deprecated_function(args):
        do stuff...

For Python there is no need to use the `\\deprecated_at` macro - the
warning message from the decorator is automatically added to the documentation.

Tests and examples
------------------

Any code that demonstrates the use of %IMP should not rely on deprecated
functionality. This includes tests, examples, and benchmarks.

All examples run as part of the %IMP test suite get passed the
`--deprecation_exceptions` command line flag. Thus, any example that calls
IMP.setup_from_argv() or uses IMP.OptionParser will fail if it tries to
call deprecated code. It is highly recommended that all examples do this.

All unit tests that call IMP.test.main() will also trigger exceptions if they
try to call deprecated code. If for some reason you need to test a deprecated
code pathway, use the IMP.allow_deprecated()
[context manager](http://eigenhombre.com/2013/04/20/introduction-to-context-managers/) as follows:

    with IMP.allow_deprecated():
       my_deprecated_function()
