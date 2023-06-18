import IMP
import IMP.test
import IMP.core
import pickle


class Tests(IMP.test.TestCase):

    """Tests for linear unary function"""

    def test_values(self):
        """Test that linear values are correct"""
        for offset in (0.0, -1.0):
            for slope in (0.0, -5.0, 3.5):
                func = IMP.core.Linear(offset, slope)
                func.set_was_used(True)
                for i in range(15):
                    val = -10.0 + 3.5 * i
                    scoreonly = func.evaluate(val)
                    score, deriv = func.evaluate_with_derivative(val)
                    self.assertEqual(score, scoreonly)
                    self.assertAlmostEqual(score, (val - offset) * slope,
                                           delta=0.001)
                    self.assertAlmostEqual(deriv, slope, delta=0.001)

    def test_accessors(self):
        """Test Linear accessors"""
        func = IMP.core.Linear(0, 0)
        func.set_was_used(True)
        self.assertEqual(func.evaluate(1), 0)
        func.set_slope(10)
        self.assertEqual(func.evaluate(1), 10)
        func.set_offset(5)
        self.assertEqual(func.evaluate(1), -40)

    def test_show(self):
        """Check Linear::show() method"""
        func = IMP.core.Linear(0, 1.0)
        func.set_was_used(True)
        func.show()

    def test_pickle(self):
        """Test (un-)pickle of Linear"""
        func = IMP.core.Linear(2.0, 4.0)
        func.set_name('foo')
        self.assertAlmostEqual(func.evaluate(4.0), 8.0, delta=0.01)
        dump = pickle.dumps(func)
        del func
        f = pickle.loads(dump)
        self.assertEqual(f.get_name(), 'foo')
        self.assertAlmostEqual(f.evaluate(4.0), 8.0, delta=0.01)


if __name__ == '__main__':
    IMP.test.main()
