from __future__ import print_function
import IMP
import IMP.test
import sys
import random
from io import StringIO


class Tests(IMP.test.TestCase):
    # have to be in separate runs than the other flags tests

    def test_false_bool(self):
        """Test python bool flags are false by default"""
        IMP.add_bool_flag("myboolflag", "Some bool text")
        extra = IMP.setup_from_argv(["python", "zero", "one"],
                                         "test things",
                                         "arg0 arg1", 2)
        print(IMP.get_bool_flag("myboolflag"))
        print(IMP.get_log_level())
        print(extra)
        self.assertEqual(IMP.get_bool_flag("myboolflag"), False)

if __name__ == '__main__':
    IMP.test.main()
