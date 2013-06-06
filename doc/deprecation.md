# Deprecation #

# Deprecating old code # {#deprecation}

Sometimes it is useful to drop support for code for various reasons, for example
- it represents a failed experiment
- there is better functionality that replaced it
- it wasn't used
- it is broken and not worth fixing

Our policy is to support code for one release with warning messages and then remove it in the next. Code
that is deprecated should be marked in the following way (where EXAMPLE is replaced by your module name):
- macros should have an IMPEXAMPLE_DEPRECATED_MACRO(version, replacement) line added within their definition
\code
#define MY_DEPRECATED_MACRO(args)                                                     \
        IMPEXAMPLE_DEPRECATED_MACRO(2.1, "You should use MY_NEW_MACRO(args) instead") \
        do stuff....
\endcode
- functions should have IMPEXAMPLE_DEPRECATED_FUNCTION_DECL(version) added to the end of the definition and IMPEXAMPLE_DEPRECATED_FUNCTION_DEF(version, message) added in their body
\code
       IMPEXAMPLE_DEPRECATED_FUNCTION_DECL(2.1)
         void my_deprecated_function(args);

       void my_deprecated_function(args) {
          IMPEXAMPLE_DEPRECATED_FUNCTION_DEF(2.1, "Use my_new_function(args) instead");
          do stuff....
       }
\endcode
- classes should have IMPEXAMPLE_DEPRECATED_CLASS_DECL(version) added to their constructor declarations and IMPEXAMPLE_DEPRECATED_CLASS_DEF(version, message) added in their constructors.
\code
     class IMPEXAMPLEEXPORT MyDeprecatedClass {
     public:
        IMPEXAMPLE_DEPRECATED_CLASS_DECL(2.1)
          MyDeprecatedClass(args) {
             IMPEXAMPLE_DEPRECATED_CLASS_DEF(2.1, "Use MyNewClass instead");
          }
        IMPEXAMPLE_DEPRECATED_CLASS_DECL(2.1)
          MyDeprecatedClass(other_args) {
             IMPEXAMPLE_DEPRECATED_CLASS_DEF(2.1, "Use MyNewClass instead");
          }
     };
\endcode
- Header should have IMPEXAMPLE_DEPREACTED_HEADER(version, message) in them.
\code
#ifndef IMP_MY_DEPRECATED_HEADER_H
#define IMP_MY_DEPRECATED_HEADER_H
IMPEXAMPLE_DEPREACTED_HEADER(2.1, "Use my_new_header.h")
...
#endif // IMP_MY_DEPRECATED_HEADER_H
\endcode

- All things should have in their docs and message should include the IMP version.
\code
/** \deprecated{Replaced by my_new_function() as of 2.1} */
\endcode

These will provide runtime and compile time warning messages to users.