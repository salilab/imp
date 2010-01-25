import unittest
import IMP
import IMP.test
import sys
from StringIO import StringIO

class DirectoriesTests(IMP.test.TestCase):

    def test_data_directory(self):
        """Test log targets"""
        s= StringIO()
        t=IMP.SetLogTarget(s)
        IMP.log_write("Hey there\n")
        IMP.log_write("Big guy")
        #IMP.set_log_target()
        del t
        print s.getvalue()
        print "del"
        self.assertEqual(s.getvalue(), "Hey there\nBig guy")
        del s
        print "done del"
        IMP.log_write("what's up")
        print "start again"
        s= StringIO()
        t= IMP.SetLogTarget(s)
        print "write"
        IMP.log_write("Hey there\n")
        del s
        print "post del write"
        IMP.log_write("Big guy")
        #IMP.reset_log_target()

if __name__ == '__main__':
    unittest.main()
