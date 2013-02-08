import IMP.benchmark
import IMP.test
import IMP.base
import os

class Tests(IMP.test.TestCase):
    def test_log_targets(self):
        """Test that profiler produces a file"""
        if not IMP.base.IMP_BASE_HAS_GPERFTOOLS:
            self.skipTest("profiling not available")

        nm= self.get_tmp_file_name("prof.pprof")
        prof= IMP.benchmark.Profiler(nm)
        del prof
        os.unlink(nm)

if __name__ == '__main__':
    IMP.test.main()
