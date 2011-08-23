import IMP
import IMP.test
import IMP.parallel

class StartupTests(IMP.test.TestCase):
    """Test startup of parallel jobs"""

    def test_startup_heartbeat(self):
        """Make sure that startup failures cause a timeout"""
        def empty_task():
            pass
        m = IMP.parallel.Manager(python="/path/does/not/exist")
        m.heartbeat_timeout = 1
        m.add_slave(IMP.parallel.LocalSlave())
        c = m.get_context()
        c.add_task(empty_task)
        list(c.get_results_unordered())
#       self.assertRaises(NetworkError, c.get_results_unordered())

if __name__ == '__main__':
    IMP.test.main()
