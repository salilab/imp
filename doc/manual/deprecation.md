Deprecation {#deprecation}
===========

Sometimes it is useful to drop support for code for various reasons, for example
- it represents a failed experiment
- there is better functionality that replaced it
- it wasn't used
- it is broken and not worth fixing

Our policy is to support code for one release with warning messages and then remove it in the next. Code
that is deprecated should be marked in the following way (where EXAMPLE is replaced by your module name):
- macros should have an `IMPEXAMPLE_DEPRECATED_MACRO(version, replacement)` line added within their definition

          #define MY_DEPRECATED_MACRO(args)                                                     \
                  IMPEXAMPLE_DEPRECATED_MACRO(2.1, "You should use MY_NEW_MACRO(args) instead") \
                  do stuff....

- class methods should have `IMPEXAMPLE_DEPRECATED_METHOD_DECL(version)` added to the end of the definition and `IMPEXAMPLE_DEPRECATED_METHOD_DEF(version, message)` added in their body

         class IMPEXAMPLEEXPORT MyClass {
           IMPEXAMPLE_DEPRECATED_METHOD_DECL(2.1)
           void my_deprecated_method(args) {
             IMPEXAMPLE_DEPRECATED_METHOD_DEF(2.1, "Use my_new_method(args) instead");
             do stuff....
           }

- functions should have `IMPEXAMPLE_DEPRECATED_FUNCTION_DECL(version)` added to the end of the definition and `IMPEXAMPLE_DEPRECATED_FUNCTION_DEF(version, message)` added in their body

           IMPEXAMPLE_DEPRECATED_FUNCTION_DECL(2.1)
             void my_deprecated_function(args);

           void my_deprecated_function(args) {
              IMPEXAMPLE_DEPRECATED_FUNCTION_DEF(2.1, "Use my_new_function(args) instead");
              do stuff....
           }

- classes should have `IMPEXAMPLE_DEPRECATED_OBJECT_DECL(version)` or `IMPEXAMPLE_DEPRECATED_VALUE_DECL(version)` added before their constructor declarations and `IMPEXAMPLE_DEPRECATED_OBJECT_DEF(version, message)` or `IMPEXAMPLE_DEPRECATED_VALUE_DEF(version, message)` added in their constructors.

         class IMPEXAMPLEEXPORT MyDeprecatedClass :: public IMP::base::Object {
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

          /** \deprecated_at{2.1} Replaced by my_new_function(). */

These will provide documentation, and runtime and compile time warning messages to users.
