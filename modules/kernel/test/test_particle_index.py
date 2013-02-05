import IMP
import IMP.test

class Tests(IMP.test.TestCase):

    def test_no_model(self):
        """Check that printing particle indexes in python works"""
        pi = IMP.ParticleIndex(1)
        strpi= str(pi)
        print strpi
        self.assertEqual(strpi, "1")

if __name__ == '__main__':
    IMP.test.main()
