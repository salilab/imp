## \example log.py
# Show various ways to use the log and control it.

import IMP

class DummyObject(IMP.Object):

    def __init__(self):
        IMP.Object.__init__(self, "DummyObject%1%")

    def add_log(self):
        # these are done by the IMP_OBJECT_LOG macro in C++
        state = IMP.SetLogState(self.get_log_level())
        context = IMP.CreateLogContext(self.get_name() + "::add_log")
        self.set_was_used(True)
        IMP.add_to_log(IMP.VERBOSE,
                            "A verbose message in the object\n")
# we can set the log level for all of IMP
IMP.set_log_level(IMP.TERSE)

# we can tell it to print the time each even occurs
IMP.set_log_timer(True)

# we can create a log context
lc = IMP.CreateLogContext("my context")

# we can print a message
IMP.add_to_log(IMP.TERSE, "This is my log message\n")

o = DummyObject()
o.set_log_level(IMP.VERBOSE)
o.add_log()
