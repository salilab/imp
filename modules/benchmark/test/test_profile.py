import IMP.benchmark
import IMP.test
import sys
import os.path
import random
from StringIO import StringIO

class Tests(IMP.test.TestCase):
    def test_log_targets(self):
        """Test that profiler produces a file"""
        nm= self.get_tmp_file_name("prof.pprof")
        prof= IMP.benchmark.Profiler(nm)
        del prof
        self.assert_(os.path.exists(nm))

if __name__ == '__main__':
    IMP.test.main()
