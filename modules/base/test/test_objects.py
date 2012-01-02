import IMP
import IMP.test
import sys
import random
from StringIO import StringIO

class LogTests(IMP.test.TestCase):
    def test_log_targets(self):
        """Test getting all objects"""
        if hasattr(IMP.base, "get_live_objects"):
            allobjs= IMP.base.get_live_objects()
            for o in allobjs:
                # the wrapping was hard to get swig to do
                print o.get_name()
if __name__ == '__main__':
    IMP.test.main()
