from __future__ import print_function
import IMP.test
import sys
from io import BytesIO

class Tests(IMP.test.TestCase):

    def test_log_targets(self):
        """Test log targets"""
        if IMP.IMP_KERNEL_HAS_LOG4CXX:
            self.skipTest("Log4CXX log backend doesnt (yet) support targets")
        log_level = IMP.get_log_level()
        # Make sure we are not logging at level MEMORY, since that will add
        # extra logs (for ref/unref) and cause assertions to fail
        IMP.set_log_level(IMP.VERBOSE)
        if not IMP.get_log_level():
            self.skipTest("Logging is disabled")
        s = BytesIO()
        t = IMP.SetLogTarget(s)
        IMP.add_to_log(IMP.VERBOSE, "Hey there\n")
        IMP.add_to_log(IMP.VERBOSE, "Big guy")
        del t
        print('testing')
        self.assertEqual(s.getvalue(), b"Hey there\nBig guy")
        del s
        IMP.add_to_log(IMP.VERBOSE, "what's up")
        s = BytesIO()
        t = IMP.SetLogTarget(s)
        IMP.add_to_log(IMP.VERBOSE, "Hey there\n")
        del s
        IMP.add_to_log(IMP.VERBOSE, "Big guy")
        IMP.set_log_level(log_level)

    def test_log_targets_memory(self):
        """Test log targets at MEMORY log level"""
        log_level = IMP.get_log_level()
        IMP.set_log_level(IMP.MEMORY)
        if not IMP.get_log_level():
            self.skipTest("Logging is disabled")
        s = BytesIO()
        t = IMP.SetLogTarget(s)
        # r4694 or earlier will crash here, since the 'Unrefing object' log
        # message emitted at MEMORY log level will attempt to be displayed
        # using the log target that is in the process of being freed
        del t
        IMP.set_log_level(log_level)

    def test_log_functions(self):
        """Test function log looks ok"""
        # I don't feel like arranging to capture the output...
        print("calling")
        if not IMP.get_log_level():
            self.skipTest("Logging is disabled")
        IMP.set_log_level(IMP.VERBOSE)
        IMP._test_log()
        print("done")

    def test_log_time_functions(self):
        """Test time log looks ok"""
        # I don't feel like arranging to capture the output...
        if IMP.IMP_KERNEL_HAS_LOG4CXX:
            self.skipTest("Log4CXX log backend doesnt (yet) support targets")
        if not IMP.get_log_level():
            self.skipTest("Logging is disabled")
        print("calling")
        IMP.set_log_timer(True)
        IMP.set_log_level(IMP.VERBOSE)
        IMP._test_log()
        print("done")

    def test_log_state(self):
        """Test SetLogState"""
        s = IMP.SetLogState()
        s.set(IMP.DEFAULT)
        s.set(IMP.VERBOSE)
        sio = BytesIO()
        s.show(sio)

if __name__ == '__main__':
    IMP.test.main()
