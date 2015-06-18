import IMP.benchmark
import IMP.test
import os


class Tests(IMP.test.TestCase):

    def test_log_targets(self):
        """Test that profiler produces a file"""
        if not IMP.IMP_KERNEL_HAS_GPERFTOOLS:
            self.skipTest("profiling not available")

        nm = self.get_tmp_file_name("prof.pprof")
        with IMP.benchmark.Profiler(nm):
            pass
        os.unlink(nm)

if __name__ == '__main__':
    IMP.test.main()
