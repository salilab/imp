%IMP gotchas {#gotchas}
============

Below are suggestions prompted by bugs found in code submitted to %IMP.

- Never use '`using namespace`' outside of a function; instead
  explicitly provide the namespace. (This avoids namespace pollution, and
  removes any ambiguity.)

- Never use the preprocessor to define constants. Use `const`
  variables instead. Preprocessor symbols don't have scope or type
  and so can have unexpected effects.

- Don't expect IMP::Object::get_name() names to be unique; they
  are there for human viewing. If you need a unique identifier
  associated with an object or non-geometric value, just use the
  object or value itself.

- Pass other objects by value or by `const &` (if the object is
      large) and store copies of them.

- Never expose member variables in an object which has
      methods. All such member variables should be private.

- Don't derive a class from another class simply to reuse some
      code that the base class provides - only do so if your derived
      class could make sense when cast to the base class. As above,
      reuse existing code by pulling it into a function.

- Clearly mark any file that is created by a script so that other
      people know to edit the original file.

- Always return a `const` value or `const` reference if you are not
      providing write access. Returning a `const` copy means the
      compiler will report an error if the caller tries to modify the
      return value without creating a copy of it.

- Include files from the local module first, then files from
    other %IMP modules and finally outside includes. This
    makes any dependencies in your code obvious, and by including
    standard headers *after* %IMP headers, any missing includes in the
    headers themselves show up early (rather than being masked by
    other headers you include).

        #include <IMP/mymodule/mymodule_exports.h>
        #include <IMP/mymodule/MyRestraint.h>
        #include <IMP/Restraint.h>
        #include <vector>

- Use `double` variables for all computational intermediates.

- Avoid using nested classes in the API as SWIG can't wrap them
      properly.  If you must use use nested classes, you will have to
      do more work to provide a Python interface to your code.


- Delay initialization of keys until they are actually needed
  (since all initialized keys take up memory within each particle,
  more or less). The best way to do this is to have them be static
  variables in a static function:

      FloatKey get_my_float_key() {
        static FloatKey k("hello");
        return k;
      }

- One is the almost always the right number:
        - Information should be stored in exactly one
          place. Duplicated information easily gets out of sync.
        - A given piece of code should only appear once. Do not copy,
          paste and modify to create new functionality.  Instead,
          figure out a way to reuse the existing code by pulling it
          into an internal function and adding extra parameters. If
          you don't, when you find bugs, you won't remember to fix
          them in all the copies of the code.
        - There should be exactly one way to represent any particular
          state. If there is more than one way, anyone who writes
          library code which uses that type of state has to handle all
          ways.  For %example, there is only one scheme for
          representing proteins, namely the IMP::atom::Hierarchy.
        - Each class/method should do exactly one thing. The presence
          of arguments which dramatically change the behavior of the
          class/method is a sign that it should be split. Splitting
          it can make the code simpler, expose the common code for
          others to use and make it harder to make mistakes by
          getting the mode flag wrong.
        - Methods should take at most one argument of each type (and
          ideally only one argument). If there are several arguments
          of the same types (eg two different `double` parameters) it is
          easy for a user to mix up the order of arguments and the compiler will
          not complain. `int` and `double` count as
          equivalent types for this rule since the compiler will
          transparently convert an `int` into a `double.`
