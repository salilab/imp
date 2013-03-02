import IMP
import IMP.test
import sys
import random
from StringIO import StringIO

class Tests(IMP.test.TestCase):
    def test_log_targets(self):
        """Test python flags"""
        IMP.base.add_string_flag("mystringflag", "mydefault", "Some string text")
        IMP.base.add_int_flag("myintflag", 3, "Some int text")
        IMP.base.add_bool_flag("myboolflag", "Some bool text")
        IMP.base.add_float_flag("myfloatflag", 10, "Some float text")
        extra=IMP.base.setup_from_argv(["python", "--mystringflag=hi",
                                        "--log_level=3",
                                        "--myintflag=6", "--myfloatflag=-6.0",
                                        "--myboolflag", "zero", "one"],
                                        "test things",
                                        "arg0 arg1", 2)
        print IMP.base.get_string_flag("mystringflag")
        print IMP.base.get_int_flag("myintflag")
        print IMP.base.get_float_flag("myfloatflag")
        print IMP.base.get_bool_flag("myboolflag")
        print IMP.base.get_log_level()
        print extra
        self.assertEqual(IMP.base.get_string_flag("mystringflag"), "hi")
        self.assertEqual(IMP.base.get_int_flag("myintflag"), 6)
        self.assertEqual(IMP.base.get_float_flag("myfloatflag"), -6.0)
        self.assertEqual(IMP.base.get_bool_flag("myboolflag"), True)
        self.assertEqual(extra[0], "zero")
        self.assertEqual(extra[1], "one")
if __name__ == '__main__':
    IMP.test.main()
