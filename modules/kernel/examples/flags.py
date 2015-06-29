## \example kernel/flags.py
# A simple example of how to use IMP flags support in a python
# executable. Run with "--help" to see all available flags.

import IMP
import sys

IMP.add_string_flag("hello", "Hello world", "How to say hello")

IMP.setup_from_argv(sys.argv, "An example for using flags in python.")

if IMP.get_string_flag("hello") == "":
    IMP.write_help()
    exit(1)

print(IMP.get_string_flag("hello"))
