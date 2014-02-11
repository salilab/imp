import IMP
import IMP.test
import IMP.parallel
import sys
import _util
import _tasks


class Tests(IMP.test.TestCase):

    """Test tasks in parallel jobs"""

    def test_pass_exceptions(self):
        """Test that exceptions can be passed to and from tasks"""
        m = _util.Manager(output='passexc%d.out')
        m.add_slave(IMP.parallel.LocalSlave())
        c = m.get_context()
        c.add_task(_tasks.SimpleTask(IndexError("test")))
        results = list(c.get_results_unordered())
        self.assertIsInstance(results[0], IndexError)
        _util.unlink("passexc0.out")

    def test_raise_exceptions(self):
        """Test that exceptions raised by a task are propagated"""
        m = _util.Manager(output='raiseexc%d.out')
        m.add_slave(IMP.parallel.LocalSlave())
        c = m.get_context()
        c.add_task(_tasks.error_task)
        self.assertRaises(IMP.parallel.RemoteError, list,
                          c.get_results_unordered())
        _util.unlink("raiseexc0.out")

    def test_floats(self):
        """Check that NaN and Inf floats are pickled correctly"""
        if sys.platform in ('irix6', 'osf1V5'):
            self.skipTest(
                "Cannot reliably handle NaN and Inf on Irix or Alpha")

        m = _util.Manager(output='floats%d.out')
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
            c.add_task(_tasks.SimpleTask(f))
        results = list(c.get_results_unordered())
        _util.unlink("floats0.out")


if __name__ == '__main__':
    IMP.test.main()
