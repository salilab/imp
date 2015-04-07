Documenting your code {#documenting}
=====================

%IMP is documented using `doxygen`. See
[Documenting your code in doxygen](http://www.doxygen.nl/docblocks.html)
to get started. We use `//!` and `/**` ... `*/` blocks for documentation.
You are encouraged to use Doxygen's
[markdown support](http://www.stack.nl/~dimitri/doxygen/manual/markdown.html) as much as possible.

Python code should provide Python doc strings. We automatically convert these
into a form Doxygen can parse, so you can use Doxygen commands in them.

All headers not in internal directories are parsed through
`doxygen`. Any function that you do not want documented (for example,
because it is not well tested), hide by surrounding with

    #ifndef IMP_DOXYGEN
    void messy_poorly_thought_out_function();
    #endif

We provide a number of extra Doxygen commands to aid in producing nice
%IMP documentation.

- To mark that some part of the API has not yet been well planned and may change
  use `\\unstable{Classname}`. The documentation will include a disclaimer
  and the class or function will be added to a list of unstable classes. It is
  generally better to simply hide such things from `doxygen`.

- To mark a method as not having been well tested yet, use
  `\\untested{Classname}`.

- To mark a method as not having been implemented, use
  `\\unimplemented{Classname}`.

Building the target `IMP-doc` (e.g. by running `make IMP-doc`) will build
documentation for all of the modules, while the `IMP.foobar-doc` target
will build documentation only for the `foobar` module.
