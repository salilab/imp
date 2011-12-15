import IMP
import IMP.test
import sys
import random
from StringIO import StringIO

class LogTests(IMP.test.TestCase):
    def test_log_targets(self):
        """Test getting all objects"""
        if IMP.base!="fast":
            allobjs= IMP.base.get_live_objects()
if __name__ == '__main__':
    IMP.test.main()
