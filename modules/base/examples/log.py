## \example base/log.py
## Show various ways to use the log and control it.

import IMP.base

class DummyObject(IMP.base.Object):
    def __init__(self):
        IMP.base.Object.__init__(self, "DummyObject%1%")
    def add_log(self):
        # these are done by the IMP_OBJECT_LOG macro in C++
        state=IMP.base.SetLogState(self.get_log_level())
        context= IMP.base.CreateLogContext(self.get_name()+"::add_log")
        self.set_was_used(True)
        IMP.base.add_to_log(IMP.base.VERBOSE,
                            "A verbose message in the object\n")
# we can set the log level for all of IMP
IMP.base.set_log_level(IMP.base.TERSE)

# we can tell it to print the time each even occurs
IMP.base.set_log_timer(True)

# we can create a log context
lc= IMP.base.CreateLogContext("my context")

# we can print a message
IMP.base.add_to_log(IMP.base.TERSE, "This is my log message\n")

o= DummyObject()
o.set_log_level(IMP.base.VERBOSE)
o.add_log()
