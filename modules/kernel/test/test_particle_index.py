import IMP
import IMP.test

class Tests(IMP.test.TestCase):

    def test_no_model(self):
        """Check that printing particle indexes in python works"""
        pi = IMP.kernel.ParticleIndex(1)
        strpi= str(pi)
        print strpi
        self.assertEqual(strpi, "1")

    def test_indexes(self):
        """Check that ParticleIndexesAdaptor works"""
        pis = [IMP.kernel.ParticleIndex(i) for i in range(0,10)]
        opis = IMP.kernel._take_particle_indexes_adaptor(pis)
        self.assertEqual(pis, opis)

if __name__ == '__main__':
    IMP.test.main()
