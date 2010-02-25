import unittest
import IMP
import IMP.test
import sys
from StringIO import StringIO

class LogTests(IMP.test.TestCase):

    def test_log_targets(self):
        """Test log targets"""
        log_level = IMP.get_log_level()
        # Make sure we are not logging at level MEMORY, since that will add
        # extra logs (for ref/unref) and cause assertions to fail
        IMP.set_log_level(IMP.VERBOSE)
        s= StringIO()
        t=IMP.SetLogTarget(s)
        IMP.add_to_log("Hey there\n")
        IMP.add_to_log("Big guy")
        del t
        self.assertEqual(s.getvalue(), "Hey there\nBig guy")
        del s
        IMP.add_to_log("what's up")
        s= StringIO()
        t= IMP.SetLogTarget(s)
        IMP.add_to_log("Hey there\n")
        del s
        IMP.add_to_log("Big guy")
        IMP.set_log_level(log_level)

    def test_log_targets_memory(self):
        """Test log targets at MEMORY log level"""
        log_level = IMP.get_log_level()
        IMP.set_log_level(IMP.MEMORY)
        s= StringIO()
        t=IMP.SetLogTarget(s)
        # r4694 or earlier will crash here, since the 'Unrefing object' log
        # message emitted at MEMORY log level will attempt to be displayed
        # using the log target that is in the process of being freed
        del t
        IMP.set_log_level(log_level)

if __name__ == '__main__':
    unittest.main()
