import IMP
import IMP.test
import IMP.algebra
import random


class PeriodicOS(IMP.OptimizerState):

    def __init__(self, m):
        IMP.OptimizerState.__init__(self, m, "MyPOS%1%")
        self.calls = []

    def do_update(self, call):
        self.calls.append(call)


class Tests(IMP.test.TestCase):

    """Tests for RefinerCover"""

    def test_it(self):
        """Test periodic optimizer state base"""
        m = IMP.kernel.Model()
        pos = PeriodicOS(m)
        pos.set_was_used(True)
        period = random.randint(1, 10)
        pos.set_period(period)
        for i in range(0, 100):
            pos.update()
        self.assertEqual(pos.calls, range(0, 100 / period))
        pos.reset()
        pos.calls = []
        for i in range(0, 100):
            pos.update()
        self.assertEqual(pos.calls, range(0, 100 / period))
if __name__ == '__main__':
    IMP.test.main()
