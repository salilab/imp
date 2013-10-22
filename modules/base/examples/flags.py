## \example base/flags.py
# A simple example of how to use IMP flags support in a python
# executable. Run with "--help" to see all available flags.

import IMP.base
import sys

IMP.base.add_string_flag("hello", "Hello world", "How to say hello")

IMP.base.setup_from_argv(sys.argv, "An example for using flags in python.")

if IMP.base.get_string_flag("hello") == "":
    IMP.base.write_help()
    exit(1)

print IMP.base.get_string_flag("hello")
