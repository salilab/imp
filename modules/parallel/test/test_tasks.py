import IMP
import IMP.test
import IMP.parallel
import util
import sys
import tasks

class Tests(IMP.test.TestCase):
    """Test tasks in parallel jobs"""

    def test_pass_exceptions(self):
        """Test that exceptions can be passed to and from tasks"""
        m = util.Manager()
        m.add_slave(IMP.parallel.LocalSlave())
        c = m.get_context()
        c.add_task(tasks.SimpleTask(IndexError("test")))
        results = list(c.get_results_unordered())
        self.assertIsInstance(results[0], IndexError)
        util.unlink("slave0.output")

    def test_raise_exceptions(self):
        """Test that exceptions raised by a task are propagated"""
        m = util.Manager()
        m.add_slave(IMP.parallel.LocalSlave())
        c = m.get_context()
        c.add_task(tasks.error_task)
        self.assertRaises(IMP.parallel.RemoteError, list,
                          c.get_results_unordered())
        util.unlink("slave0.output")

    def test_floats(self):
        """Check that NaN and Inf floats are pickled correctly"""
        if sys.platform in ('irix6', 'osf1V5'):
            self.skipTest("Cannot reliably handle NaN and Inf on Irix or Alpha")

        m = util.Manager()
        m.add_slave(IMP.parallel.LocalSlave())
        c = m.get_context()

        try:
            inf = float('inf')
            nan = float('nan')
        except ValueError:
            # Python 2.5 on Windows reports 'invalid literal', so use another
            # method to get inf and nan:
            inf = 1e300 * 1e300
            nan = inf - inf
        for f in (inf, nan):
            c.add_task(tasks.SimpleTask(f))
        results = list(c.get_results_unordered())
        util.unlink("slave0.output")


if __name__ == '__main__':
    IMP.test.main()
