from __future__ import print_function
import IMP
import IMP.test
import IMP.example
import pickle


class Tests(IMP.test.TestCase):

    def test_object(self):
        """Test example object"""
        e = IMP.example.ExampleObject([1.0, 2.0, 3.0])
        self.assertAlmostEqual(e.get_data(0), 1.0, delta=1e-3)

    def test_serialize(self):
        """Test (un-)serialize of ExampleObject"""
        e = IMP.example.ExampleObject([1.0, 2.0, 3.0])
        self.assertAlmostEqual(e.get_data(0), 1.0, delta=1e-3)
        dump = pickle.dumps(e)
        newe = pickle.loads(dump)
        self.assertAlmostEqual(newe.get_data(0), 1.0, delta=1e-3)


if __name__ == '__main__':
    IMP.test.main()
