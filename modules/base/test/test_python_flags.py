import IMP
import IMP.test
import sys
import random
from StringIO import StringIO

class LogTests(IMP.test.TestCase):
    def test_log_targets(self):
        """Test python flags"""
        if IMP.base.has_gflags:
            IMP.base.add_string_flag("myflag", "mydefault", "Some text")
            IMP.base.setup_from_argv(["python", "--myflag=hi", "--cpu_profile"], 0)
            self.assertEqual(IMP.base.get_string_flag("myflag"), "hi")

if __name__ == '__main__':
    IMP.test.main()
