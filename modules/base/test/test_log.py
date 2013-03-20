import IMP.base
import IMP.test
import sys
from StringIO import StringIO

class Tests(IMP.test.TestCase):
    def test_log_targets(self):
        """Test log targets"""
        if IMP.base.IMP_BASE_HAS_LOG4CXX:
            self.skipTest("Log4CXX log backend doesnt (yet) support targets")
        log_level = IMP.base.get_log_level()
        # Make sure we are not logging at level MEMORY, since that will add
        # extra logs (for ref/unref) and cause assertions to fail
        IMP.base.set_log_level(IMP.base.VERBOSE)
        if not IMP.base.get_log_level():
            self.skipTest("Logging is disabled")
        s= StringIO()
        t=IMP.base.SetLogTarget(s)
        IMP.base.add_to_log(IMP.base.VERBOSE, "Hey there\n")
        IMP.base.add_to_log(IMP.base.VERBOSE, "Big guy")
        del t
        print 'testing'
        self.assertEqual(s.getvalue(), "Hey there\nBig guy")
        del s
        IMP.base.add_to_log(IMP.base.VERBOSE, "what's up")
        s= StringIO()
        t= IMP.base.SetLogTarget(s)
        IMP.base.add_to_log(IMP.base.VERBOSE, "Hey there\n")
        del s
        IMP.base.add_to_log(IMP.base.VERBOSE, "Big guy")
        IMP.base.set_log_level(log_level)

    def test_log_targets_memory(self):
        """Test log targets at MEMORY log level"""
        log_level = IMP.base.get_log_level()
        IMP.base.set_log_level(IMP.base.MEMORY)
        if not IMP.base.get_log_level():
            self.skipTest("Logging is disabled")
        s= StringIO()
        t=IMP.base.SetLogTarget(s)
        # r4694 or earlier will crash here, since the 'Unrefing object' log
        # message emitted at MEMORY log level will attempt to be displayed
        # using the log target that is in the process of being freed
        del t
        IMP.base.set_log_level(log_level)
    def test_log_functions(self):
        """Test function log looks ok"""
        # I don't feel like arranging to capture the output...
        print "calling"
        if not IMP.base.get_log_level():
            self.skipTest("Logging is disabled")
        IMP.base.set_log_level(IMP.base.VERBOSE)
        IMP.base._test_log()
        print "done"

    def test_log_time_functions(self):
        """Test time log looks ok"""
        # I don't feel like arranging to capture the output...
        if IMP.base.IMP_BASE_HAS_LOG4CXX:
            self.skipTest("Log4CXX log backend doesnt (yet) support targets")
        if not IMP.base.get_log_level():
            self.skipTest("Logging is disabled")
        print "calling"
        IMP.base.set_log_timer(True)
        IMP.base.set_log_level(IMP.base.VERBOSE)
        IMP.base._test_log()
        print "done"
if __name__ == '__main__':
    IMP.test.main()
