import IMP
import IMP.test
import sys
import random
from io import StringIO


class Tests(IMP.test.TestCase):

    def test_log_targets(self):
        """Test getting all objects"""
        IMP.set_check_level(IMP.USAGE_AND_INTERNAL)
        if IMP.get_check_level() >= IMP.USAGE_AND_INTERNAL:
            m = IMP._TestObject()
            allobjs = IMP.get_live_objects()
            found = False
            for o in allobjs:
                # the wrapping was hard to get swig to do
                x = o.get_name()
                if m == o:
                    found = True
            self.assertTrue(found,
                            '%s not found in live objects: %s' % (m, allobjs))

if __name__ == '__main__':
    IMP.set_deprecation_exceptions(True)
    IMP.test.main()
