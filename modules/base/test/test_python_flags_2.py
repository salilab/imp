import IMP
import IMP.test
import sys
import random
from StringIO import StringIO

class Tests(IMP.test.TestCase):
    # have to be in separate runs than the other flags tests
    def test_false_bool(self):
        """Test python bool flags are false by default"""
        IMP.base.add_bool_flag("myboolflag", "Some bool text")
        extra=IMP.base.setup_from_argv(["python", "zero", "one"],
                                        "test things",
                                        "arg0 arg1", 2)
        print IMP.base.get_bool_flag("myboolflag")
        print IMP.base.get_log_level()
        print extra
        self.assertEqual(IMP.base.get_bool_flag("myboolflag"), False)
if __name__ == '__main__':
    IMP.test.main()
