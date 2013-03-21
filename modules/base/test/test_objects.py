import IMP
import IMP.test
import sys
import random
from StringIO import StringIO

class Tests(IMP.test.TestCase):
    def test_log_targets(self):
        """Test getting all objects"""
        IMP.base.set_check_level(IMP.base.USAGE_AND_INTERNAL)
        if IMP.base.get_check_level() >= IMP.base.USAGE_AND_INTERNAL:
            m = IMP.base._TestObject()
            allobjs= IMP.base.get_live_objects()
            found = False
            for o in allobjs:
                # the wrapping was hard to get swig to do
                x = o.get_name()
                if m == o:
                    found = True
            self.assertTrue(found,
                            '%s not found in live objects: %s' % (m, allobjs))

if __name__ == '__main__':
    IMP.test.main()
